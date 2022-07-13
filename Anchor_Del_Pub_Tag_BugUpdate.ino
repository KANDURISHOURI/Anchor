#include "config.h"

// ---------------------- mqtt topics -----------------
String Beacons;
String DmReadRequest;
String DmReadResponse;
String DmWriteRequest;
String DmWriteResponse;
//----------------------------------------------------
extern String WifiPassword;
extern String WifiSsid;
extern String MqttServer;
extern int AnchorSerialNumber;
extern String AnchorName;
extern uint8_t DeviceType;
extern String MqttTopic;

String WifiMacAddress;
String WifiIpAddress;
String TagIdNumber;

extern uint8_t IndexBit;
uint16_t CurrentTimeinEsp = 0;
uint16_t timeDifference = 0;
uint16_t MqttPort;
int RssiValue;
unsigned long WifiConnectionSlot;
unsigned long PublishBeaconSlot;
unsigned long TagAliveStatusSlot;

struct TagData TagsInfo[] = {NULL}; // Array of Structures
struct TagData TagsToPublish[] = {NULL};
struct TagData *TagsInfoDynamicMemory;
struct TagData t1;

int ArrayCount = 0;    // To know the total number of tags discovered
bool CanStore = false; // waits for the ble beacons to be processed

StaticJsonDocument<500> TagsJson;
JsonArray ArrayJson = TagsJson.to<JsonArray>();

volatile bool ConnectedToWifi = false;
volatile bool ContinueScan = false;
bool IsDmReadRequest = false;
bool IsConnected = false;
extern volatile bool RestartAfterChanges;

char DeviceManagementData[500];
unsigned long AliveStatusTimer = 0;
volatile bool AliveTimer = false;

double Distance;

void resetEeprom()
{
  if (IndexBit != EEPROM_RESET_INCREMENT_CODE)
  {
    eepromReset();
    writeDefaultParameters();
    delay(1000);
    ESP.restart();
  }
}

void initWiFi()
{
  if (WifiPassword[0] == ' ')
  {
    WiFi.begin(WifiSsid.c_str());
  }
  else
  {
    WiFi.begin(WifiSsid.c_str(), WifiPassword.c_str());
  }

  WifiMacAddress = WiFi.macAddress();
}

void getWifiStatus(int numOfAttempts = 20)
{
  int numberOfAttemptsCompleted = 0;
  WifiIpAddress = "X";

  while (numberOfAttemptsCompleted < numOfAttempts)
  {

#if DEBUG
    Serial.print(".");
#endif

    if (WiFi.status() == WL_CONNECTED)
    {
      ConnectedToWifi = true;
      WifiIpAddress = WiFi.localIP().toString();

#if DEBUG
      Serial.println("Wifi conncted");
#endif
      break;
    }
    numberOfAttemptsCompleted++;
    delay(500);
  }
}

void checkAndConnectToWifi()
{

  WifiConnectionSlot = millis() + 30000;

  switch (WiFi.status())
  {
  case WL_CONNECTED:
    WifiIpAddress = WiFi.localIP().toString();
    ConnectedToWifi = true;
    break;
  case WL_IDLE_STATUS:
  case WL_NO_SSID_AVAIL:
    break;
  case WL_SCAN_COMPLETED:
  case WL_CONNECT_FAILED:
  case WL_CONNECTION_LOST:
  case WL_DISCONNECTED:
    ConnectedToWifi = false;
    WiFi.disconnect();
    initWiFi();
    getWifiStatus(WIFI_RECONNECTION_ATTEMPTS);
    break;
  default:
    break;
  }
}

void checkTagAliveStatus()
{
  // Delete print statements in this function after testing//
  Serial.println("Inside Alive Timeout Handler");
  AliveChecking.detach();
  if (ArrayCount)
  {
    Serial.println("Tags Available");
    for (int i = 0; i < ArrayCount; i++)
    {
      int index = i;
      if (millis() - TagsInfo[i].punchTime > 5000)
      {
        TagsInfo[i].punchTime = millis();
        deleteTheTag(index);
        // if (ArrayCount == 0)
        // {
        //   // memset(&TagsToPublish, NULL, sizeof(TagsToPublish));
        //   // memset(&TagsInfo, NULL, sizeof(TagsInfo));
        // }
        Serial.print(TagsInfo[i].tagId);
        Serial.println(" - TagDeleted");
      }
      else
      {
        Serial.print(TagsInfo[i].tagId);
        Serial.println(" - TagAlive");
      }
    }
  }
  else
  {
    Serial.println("There are no tags in the vicinity. Scanning fo ACAC Beacons");
  }
  AliveChecking.attach(5, checkTagAliveStatus);
}

void timeoutHandler(int index)
{

  Serial.println("Beacon Count 4 reached");

  JsonTagArrayCreation(index);
  publishBeaconData();
  // ArrayCount = 0;

  // memset(&TagsInfo, NULL, sizeof(TagsInfo)); //TODO: Deletion of particular Tag

  TagsJson.clear();
  ArrayJson = TagsJson.to<JsonArray>();
}

void excludeFromArray(int index)
{
  memset(&TagsToPublish, NULL, sizeof(TagsToPublish));
  int count;
  for (int i = 0; i < ArrayCount; i++)
  {
    if (index == i)
    {
      Serial.println("Found the index to be ignored");
      Serial.println(TagsInfo[i].tagId);
    }
    else
    {
      TagsToPublish[count++] = TagsInfo[i];
    }
  }
}

void deleteTheTag(int index)
{
  int arrayCount = ArrayCount;

  if (index >= ArrayCount + 1)
  {
    ;
  }
  for (int i = index; i < ArrayCount - 1; i++)
  {
    TagsInfo[i] = TagsInfo[i + 1];
  }

  arrayCount -= 1;
  if (arrayCount == 0)
  {
    ArrayCount = 0;
    memset(&TagsInfo, NULL, sizeof(TagsInfo));
    AliveTimer = false;
  }
  else
  {
    ArrayCount = arrayCount;
  }
  Serial.printf("ArrayCount After Deletion is %d\n", ArrayCount);
}
/*To Delete in future Versions*/
// void checkTagsAliveStatus()
// {
//   if (ArrayCount)
//   {
//     if (millis() > TagAliveStatusSlot)
//     {
//       TagAliveStatusSlot = millis() + 10000;
//       for (int i = 0; i < ArrayCount; i++)
//       {
//         if (TagsInfo[i].lastMillis != TagsInfo[i].currentMillis)
//         {
//           TagsInfo[i].currentMillis = TagsInfo[i].lastMillis;
//         }
//       }
//     }
//   }
// }

void formatAndAddTagData(TagData currentTagData, uint16_t currentESPTime)
{
  Serial.printf("Array Count is %d\n", ArrayCount);

  int index = -1;

  if (ArrayCount == 0)
  {
    currentTagData.count += 1;
    memcpy(&TagsInfo[0], &currentTagData, sizeof(currentTagData));
    ArrayCount += 1;

    if (!AliveTimer)
    {
      AliveTimer = true;
      AliveChecking.attach(5, checkTagAliveStatus);
    }

    return;
  }

  for (int i = 0; i < ArrayCount; i++)
  {
    if (strcmp((TagsInfo[i].tagId).c_str(), (currentTagData.tagId).c_str()) == 0)
    {
      TagsInfo[i].currentSeconds = currentTagData.currentSeconds;
      TagsInfo[i].lastMillis = TagsInfo[i].currentSeconds;

      index = i;
      TagsInfo[i].beaconCount = currentTagData.beaconCount;

      uint8_t beaconDifference = TagsInfo[i].beaconCount - TagsInfo[i].oldBeaconCount;

      if ((beaconDifference == 1))
      {
        if (TagsInfo[i].rssi < currentTagData.rssi)
        {
          TagsInfo[i].rssi = currentTagData.rssi;
        }
        TagsInfo[i].count += 1;
        TagsInfo[i].oldBeaconCount = TagsInfo[i].beaconCount;

        if (TagsInfo[i].beaconCount == 4)
        {
          timeoutHandler(index);
          deleteTheTag(index);
        }
        return;
      }
    }
  }

  if (index == -1)
  {
    Serial.println("New Tag Addition");
    memcpy(&TagsInfo[ArrayCount], &currentTagData, sizeof(currentTagData));
    ArrayCount += 1;
    return;
  }
}

/*Converts RSSI to Distance*/
void convertRssiToDistance(int rssi)
{
  double val = (-69 - (rssi)) * 0.05;
  Serial.println(val);
  Distance = powf(10, val);
  Serial.printf("Distance : %f\n", Distance);
  Serial.println();
}

/* BLE Scan Callback -  Receives the BLE Devices found and processes the data */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (!advertisedDevice.haveManufacturerData())
    {
      sendAnchorKeepAliveStatus();
      return;
    }

    std::string manufRawData = advertisedDevice.getManufacturerData();
    char *mdHexRep = BLEUtils::buildHexData(nullptr, (uint8_t *)manufRawData.data(), (uint8_t)manufRawData.length());
    std::string manufData(mdHexRep);
    free(mdHexRep);

    if (strcmp(manufData.substr(0, 4).c_str(), "acac") == 0)
    {

      /*New Logic Tag Info Structure Initializations*/
      struct TagData currentTagData = {
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          0,
          ".",
          0};

      AliveStatusTimer = millis();

      currentTagData.rssi = advertisedDevice.getRSSI();
      currentTagData.tagId = manufData.substr(4, 8).c_str();
      currentTagData.txPower = atoi(manufData.substr(40, 2).c_str());

      char *timeBuffer;
      long timeNumber = strtol(manufData.substr(42, 2).c_str(), &timeBuffer, 16);
      currentTagData.currentSeconds = timeNumber;

      char *syncBuffer;
      long syncTime = strtol((manufData.substr(46, 2) + manufData.substr(44, 2)).c_str(), &syncBuffer, 16);
      currentTagData.timeSynchroMS = syncTime;

      currentTagData.beaconCount = atoi(manufData.substr(48, 2).c_str());
      Serial.printf("Beacon Count is %d \n", currentTagData.beaconCount);

      CurrentTimeinEsp = millis();

      formatAndAddTagData(currentTagData, CurrentTimeinEsp);
    }
    /*************************************************************************************************************************/
    /// For FeasyBeacon
    //      if (strcmp(manufData.substr(0, 4).c_str(), "4c00") == 0) {
    //        if ((manufRawData.length() == 25) && (strcmp(manufData.substr(40, 4).c_str(), "4330") == 0)) {
    //          RssiValue =  advertisedDevice.getRSSI();
    //          TagIdNumber = manufData.substr(44, 4).c_str();
    //          formatAndAddTagData(RssiValue, TagIdNumber, TagsInfo);
    //        }
    //      }

    //      }
    //      else {
    //        JsonTagArrayCreation();
    //        publishBeaconData();
    //        PublishBeaconSlot = millis();
    //        ArrayCount = 0;
    //        TagInfo_t.rssi = 0;
    //        TagInfo_t.count = 0;
    //        TagInfo_t.tagId = ".";
    //        memset(& TagsInfo, NULL, sizeof(TagsInfo));
    //        memset(&t1, NULL, sizeof(t1));
    //
    //        TagsJson.clear();
    //        ArrayJson = TagsJson.to<JsonArray>();
    //      }
    /******************************************************************************************************************************/
  }
};

void initBle()
{
  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  BLEDevice::init("");

  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(SCAN_TIME, scanEndCallback, false);
}

void JsonTagArrayCreation(int index)
{
  JsonObject nested = ArrayJson.createNestedObject();
  nested["id"] = TagsInfo[index].tagId;
  nested["rssi"] = TagsInfo[index].rssi;

#if DEBUG
  Serial.printf("Final RSSI == %d for TagId %s ", TagsInfo[index].rssi, TagsInfo[index].tagId);
  Serial.println(" ");
#endif
}

void publishBeaconData()
{
  if (ArrayJson.size() == NULL)
  {
    return;
  }
  DynamicJsonDocument responseData(1024);
  responseData["tags"] = ArrayJson;
  responseData["name"] = ANCHOR_NAME;
  responseData["serialNumber"] = ANCHOR_SERIAL_NUMBER;

  char jsonBuffer[500];
  serializeJson(responseData, jsonBuffer);
  if (MqttClient.publish(Beacons.c_str(), jsonBuffer))
  {

#if DEBUG
    Serial.println("Beacon Published");
#endif
    memset(&jsonBuffer, NULL, sizeof(jsonBuffer));
  }
}

void publishDmData()
{

  DynamicJsonDocument jsonBuffer(500);
  jsonBuffer["serialNumber"] = AnchorSerialNumber;
  jsonBuffer["name"] = AnchorName;
  jsonBuffer["deviceType"] = DeviceType;
  jsonBuffer["wifiSsid"] = WifiSsid;
  jsonBuffer["wifiPassword"] = WifiPassword;
  jsonBuffer["mqttHost"] = MqttServer;
  jsonBuffer["mqttPort"] = MqttPort;

  JsonObject root = jsonBuffer.as<JsonObject>();
  JsonObject mqtt_topic = root.createNestedObject("mqttTopics");
  mqtt_topic["beacons"] = Beacons;
  mqtt_topic["dmRead"][0] = DmReadRequest;
  mqtt_topic["dmRead"][1] = DmReadResponse;
  mqtt_topic["dmWrite"][0] = DmWriteRequest;
  mqtt_topic["dmWrite"][1] = DmWriteResponse;

  char jsonBuffer1[500];
  serializeJson(jsonBuffer, jsonBuffer1);

  if (MqttClient.publish(DmReadResponse.c_str(), jsonBuffer1))
  {

#if DEBUG
    Serial.println("Dm Data Published");
#endif
  }
}

void publishDmAck()
{
  StaticJsonDocument<100> dmAckData;
  dmAckData["serialNumber"] = AnchorSerialNumber;
  dmAckData["name"] = AnchorName;
  dmAckData["deviceType"] = DeviceType;
  dmAckData["isAck"] = true;

  char jsonBuffer1[100];
  serializeJson(dmAckData, jsonBuffer1);

  if (MqttClient.publish(DmWriteResponse.c_str(), jsonBuffer1))
  {

#if DEBUG
    Serial.println("Dm Ack Published");
#endif
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
#if DEBUG
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
#endif

  for (int i = 0; i < length; i++)
  {

#if DEBUG
    Serial.print((char)payload[i]);
#endif

    DeviceManagementData[i] = (char)payload[i];
  }
  Serial.println();

  if (strcmp(String(topic).c_str(), DmReadRequest.c_str()) == 0)
  {
    IsDmReadRequest = true;
  }
  StaticJsonDocument<500> doc;
  if (strcmp(String(topic).c_str(), DmWriteRequest.c_str()) == 0)
  {
    DeserializationError error = deserializeJson(doc, DeviceManagementData);

    if (error)
    {
#if DEBUG
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
#endif
      return;
    }
    const char *dmData;
    if (doc["serialNumber"] == AnchorSerialNumber)
    {
      AnchorSerialNumber = doc["serialNumber"];
      dmData = doc["name"];
      AnchorName = String(dmData);
      DeviceType = doc["deviceType"];
      dmData = doc["wifiSsid"];
      WifiSsid = String(dmData);
      dmData = doc["wifiPassword"];
      WifiPassword = String(dmData);
      dmData = doc["mqttHost"];
      MqttServer = String(dmData);
      MqttPort = doc["mqttPort"];

      dmData = doc["mqttTopics"]["beacons"];
      Beacons = String(dmData);
      dmData = doc["mqttTopics"]["dmRead"][0];
      DmReadRequest = String(dmData);
      dmData = doc["mqttTopics"]["dmRead"][1];
      DmReadResponse = String(dmData);
      dmData = doc["mqttTopics"]["dmWrite"][0];
      DmWriteRequest = String(dmData);
      dmData = doc["mqttTopics"]["dmWrite"][1];
      DmWriteResponse = String(dmData);

#if DEBUG
      Serial.println(AnchorSerialNumber);
      Serial.println(AnchorName);

      Serial.println(DeviceType);
      Serial.println(WifiSsid);

      Serial.println(WifiPassword);
      Serial.println(MqttServer);

      Serial.println(MqttPort);
      Serial.println(Beacons);

      Serial.println(DmReadRequest);
      Serial.println(DmReadResponse);

      Serial.println(DmWriteRequest);
      Serial.println(DmWriteResponse);
#endif

#if UPDATE_ENABLED
      updateAnchorSerialNumber(String(AnchorSerialNumber));
      updateAnchorName(AnchorName);
      updateWifiSsid(WifiSsid);
      updateWifiPswd(WifiPassword);
      updateMqttServer(MqttServer);
      updateMqttPort(String(MqttPort));
      updateBeaconTopic(Beacons);
      updateDeviceType(String(DeviceType));
      updateDMReadRequestTopic(DmReadRequest);
      updateDMReadResponseTopic(DmReadResponse);
      updateDMWriteRequestTopic(DmWriteRequest);
      updateDMWriteResponseTopic(DmWriteResponse);
#endif

      publishDmAck();
      delay(1000);
      ESP.restart();
    }
  }
}

void setupMQTT()
{
  MqttClient.setServer(MqttServer.c_str(), MqttPort);
  MqttClient.setCallback(callback);
}

void scanEndCallback(BLEScanResults sr)
{
  ContinueScan = true;
}

void reconnect()
{

#if DEBUG
  Serial.println("Connecting to MQTT Broker...");
#endif

  while (!MqttClient.connected())
  {
#if DEBUG
    Serial.println("Reconnecting to MQTT Broker..");
#endif

    if (MqttClient.connect(String(ANCHOR_SERIAL_NUMBER).c_str()))
    {
#if DEBUG
      Serial.println("Connected.");
#endif
      MqttClient.subscribe(DmReadRequest.c_str());
      MqttClient.subscribe(DmWriteRequest.c_str());
      //      PublishBeaconSlot = millis();
      AliveStatusTimer = millis();
      // Publisher.attach(2, timeoutHandler);
    }
  }
}

void sendAnchorKeepAliveStatus()
{
  if (millis() - AliveStatusTimer > 3000)
  {
    AliveStatusTimer = millis();
    StaticJsonDocument<100> aliveStatus;
    aliveStatus["serialNumber"] = AnchorSerialNumber;
    aliveStatus["name"] = AnchorName;

    char jsonBuffer1[100];
    serializeJson(aliveStatus, jsonBuffer1);
    if (MqttClient.publish(MQTT_ANCHOR_ALIVE_STATUS, jsonBuffer1))
    {
      Serial.println("Alive Status Published");
    }
    else
    {
      Serial.println("Anchor Busy");
    }
  }
  delay(20);
}

void setup()
{
  Serial.begin(115200);

  EEPROM.begin(EEPROM_MAX_SIZE);
  eepromInit();
  resetEeprom();

  /* Dynamic Memory Allocation for Tags Array */
  TagsInfoDynamicMemory = (TagData *)malloc(5 * sizeof(TagData));

  if (TagsInfoDynamicMemory == NULL)
  {
    Serial.println("Please make sure to Allocate Memory");
  }

  if (strcmp(ANCHOR_NAME, AnchorName.c_str()) != 0)
  {
    AnchorName = String(ANCHOR_NAME);
    updateAnchorName(AnchorName);
    getAnchorName();
  }

  if (ANCHOR_SERIAL_NUMBER != AnchorSerialNumber)
  {
    AnchorSerialNumber = ANCHOR_SERIAL_NUMBER;
    updateAnchorName(String(AnchorSerialNumber));
    getAnchorSerialNumber();
  }

  initWiFi();
  getWifiStatus(10);

  initBle();
  setupMQTT();
  CanStore = true;
}

void loop()
{
  /*To Delete in future Versions*/
  // checkTagsAliveStatus();
  if (millis() > WifiConnectionSlot)
  {
    checkAndConnectToWifi();
  }

  if (!MqttClient.connected())
  {
    reconnect();
  }

  if (ContinueScan)
  {
    ContinueScan = false;
    bool isScanStarted = pBLEScan->start(SCAN_TIME, scanEndCallback, false);
    if (!isScanStarted)
    {
#if DEBUG
      Serial.println("Error encountered while Starting the BLE Scan");
#endif
    }
  }

  if (IsDmReadRequest)
  {
    IsDmReadRequest = false;
    publishDmData();
  }

  MqttClient.loop();
  pBLEScan->clearResults();
  delay(1000);
}
