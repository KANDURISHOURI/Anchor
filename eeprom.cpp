#include "eeprom.h"

String WifiSsid;
String WifiPassword;
String MqttServer;
int AnchorSerialNumber;
String AnchorName;
String MqttTopic;
uint8_t DeviceType;
extern uint16_t MqttPort;
extern String Beacons;
extern String DmReadRequest;
extern String DmReadResponse;
extern String DmWriteRequest;
extern String DmWriteResponse;

uint8_t IndexBit;

void readEeprom(uint16_t index, uint8_t dataSize, char *output)
{
  uint8_t j = 0;
  char buff;

  for (int i = index; i < index + dataSize; i++)
  {
    buff = char(EEPROM.read(i));
    if (buff == '\0')
    {
      output[j++] = '\0';
      break;
    }
    output[j++] = buff;
  }
}

String getWifiSsid()
{
  char ssid[WIFI_SSID_SIZE];
  readEeprom(WIFI_SSID_START_IDX, WIFI_SSID_SIZE, ssid);
  String wifiSsid = String(ssid);
  return wifiSsid;
}

String getAnchorSerialNumber()
{
  char id[ANCHOR_SERIAL_NUMBER_SIZE];
  readEeprom(ANCHOR_SERIAL_NUMBER_IDX, ANCHOR_SERIAL_NUMBER_SIZE, id);
  String anchorId = String(id);
  return anchorId;
}

String getAnchorName()
{
  char name[ANCHOR_NAME_SIZE];
  readEeprom(ANCHOR_NAME_IDX, ANCHOR_NAME_SIZE, name);
  String anchorName = String(name);
  return anchorName;
}

String getWifiPswd()
{
  char password[WIFI_PSWD_SIZE];
  readEeprom(WIFI_PSWD_START_IDX, WIFI_PSWD_SIZE, password);
  String wifiPassword = String(password);
  return wifiPassword;
}

String getMqttServer()
{
  char server[MQTT_SERVER_STRING_SIZE];
  readEeprom(MQTT_SERVER_IDX, MQTT_SERVER_STRING_SIZE, server);
  String mqttServer = String(server);
  return mqttServer;
}

String getDeviceType()
{
  char server[DEVICE_TYPE_SIZE];
  readEeprom(DEVICE_TYPE_IDX, DEVICE_TYPE_SIZE, server);
  String deviceType = String(server);
  return deviceType;
}

String getMqttPort()
{
  char server[MQTT_PORT_SIZE];
  readEeprom(MQTT_PORT_IDX, MQTT_PORT_SIZE, server);
  String mqttPort = String(server);
  return mqttPort;
}

String getBeaconTopic()
{
  char server[BEACON_TOPIC_SIZE];
  readEeprom(BEACON_TOPIC_IDX, BEACON_TOPIC_SIZE, server);
  String beaconTopic = String(server);
  return beaconTopic;
}

String getDMReadRequestTopic()
{
  char server[DMREAD_REQUEST_TOPIC_SIZE];
  readEeprom(DMREAD_REQUEST_TOPIC_IDX, DMREAD_REQUEST_TOPIC_SIZE, server);
  String dmReadTopic = String(server);
  return dmReadTopic;
}

String getDMReadResponseTopic()
{
  char server[DMREAD_RESPONSE_TOPIC_SIZE];
  readEeprom(DMREAD_RESPONSE_TOPIC_IDX, DMREAD_RESPONSE_TOPIC_SIZE, server);
  String dmReadTopic = String(server);
  return dmReadTopic;
}

String getDMWriteRequestTopic()
{
  char server[DMWRITE_REQUEST_TOPIC_SIZE];
  readEeprom(DMWRITE_REQUEST_TOPIC_IDX, DMWRITE_REQUEST_TOPIC_SIZE, server);
  String dmWriteTopic = String(server);
  return dmWriteTopic;
}

String getDMWriteResponseTopic()
{
  char server[DMWRITE_RESPONSE_TOPIC_SIZE];
  readEeprom(DMWRITE_RESPONSE_TOPIC_IDX, DMWRITE_RESPONSE_TOPIC_SIZE, server);
  String dmWriteTopic = String(server);
  return dmWriteTopic;
}

String getIndexBit()
{
  char bit[BIT_SIZE];
  readEeprom(BIT_IDX, BIT_SIZE, bit);
  String indexBit = String(bit);
  return indexBit;
}

bool updateParameter(String message, int index, int size_blk)
{
  int msg_size = message.length();
  if (msg_size > size_blk)
  {
    return false;
  }
  for (int i = 0; i < msg_size; i++)
  {
    EEPROM.write(index + i, message[i]);
  }
  EEPROM.write(index + msg_size, '\0');
  EEPROM.commit();
  return true;
}

bool updateWifiSsid(String message)
{
  return updateParameter(message, WIFI_SSID_START_IDX, WIFI_SSID_SIZE);
}

bool updateWifiPswd(String message)
{
  return updateParameter(message, WIFI_PSWD_START_IDX, WIFI_PSWD_SIZE);
}

bool updateMqttServer(String message)
{
  return updateParameter(message, MQTT_SERVER_IDX, MQTT_SERVER_STRING_SIZE);
}

bool updateAnchorSerialNumber(String message)
{
  return updateParameter(message, ANCHOR_SERIAL_NUMBER_IDX, ANCHOR_SERIAL_NUMBER_SIZE);
}

bool updateAnchorName(String message)
{
  return updateParameter(message, ANCHOR_NAME_IDX, ANCHOR_NAME_SIZE);
}

bool updateDeviceType(String message)
{
  return updateParameter(message, DEVICE_TYPE_IDX, DEVICE_TYPE_SIZE);
}

bool updateMqttPort(String message)
{
  return updateParameter(message, MQTT_PORT_IDX, MQTT_PORT_SIZE);
}

bool updateBeaconTopic(String message)
{
  return updateParameter(message, BEACON_TOPIC_IDX, BEACON_TOPIC_SIZE);
}

bool updateDMReadRequestTopic(String message)
{
  return updateParameter(message, DMREAD_REQUEST_TOPIC_IDX, DMREAD_REQUEST_TOPIC_SIZE);
}

bool updateDMReadResponseTopic(String message)
{
  return updateParameter(message, DMREAD_RESPONSE_TOPIC_IDX, DMREAD_RESPONSE_TOPIC_SIZE);
}
bool updateDMWriteRequestTopic(String message)
{
  return updateParameter(message, DMWRITE_REQUEST_TOPIC_IDX, DMWRITE_REQUEST_TOPIC_SIZE);
}

bool updateDMWriteResponseTopic(String message)
{
  return updateParameter(message, DMWRITE_RESPONSE_TOPIC_IDX, DMWRITE_RESPONSE_TOPIC_SIZE);
}

void writeDefaultParameters()
{

  updateParameter("Surya", WIFI_SSID_START_IDX, WIFI_SSID_SIZE);
  updateParameter("Password99*", WIFI_PSWD_START_IDX, WIFI_PSWD_SIZE);
  updateParameter("173.248.136.206", MQTT_SERVER_IDX, MQTT_SERVER_STRING_SIZE);
  updateParameter("3", BIT_IDX, BIT_SIZE);
  updateParameter("2", ANCHOR_SERIAL_NUMBER_IDX, ANCHOR_SERIAL_NUMBER_SIZE);
  updateParameter("A2", ANCHOR_NAME_IDX, ANCHOR_NAME_SIZE);
  updateParameter("1", DEVICE_TYPE_IDX, DEVICE_TYPE_SIZE);
  updateParameter("1883", MQTT_PORT_IDX, MQTT_PORT_SIZE);
  updateParameter("Beacons", BEACON_TOPIC_IDX, BEACON_TOPIC_SIZE);
  updateParameter("DmReadRequest", DMREAD_REQUEST_TOPIC_IDX, DMREAD_REQUEST_TOPIC_SIZE);
  updateParameter("DmReadResponse", DMREAD_RESPONSE_TOPIC_IDX, DMREAD_RESPONSE_TOPIC_SIZE);
  updateParameter("DmWriteRequest", DMWRITE_REQUEST_TOPIC_IDX, DMWRITE_REQUEST_TOPIC_SIZE);
  updateParameter("DmWriteResponse", DMWRITE_RESPONSE_TOPIC_IDX, DMWRITE_RESPONSE_TOPIC_SIZE);
}

void eepromInit()
{
  WifiSsid = getWifiSsid();
  WifiPassword = getWifiPswd();
  MqttServer = getMqttServer();
  IndexBit = atoi(getIndexBit().c_str());
  AnchorSerialNumber = atoi(getAnchorSerialNumber().c_str());
  AnchorName = getAnchorName();
  DeviceType = atoi(getDeviceType().c_str());
  MqttPort = atoi(getMqttPort().c_str());
  Beacons = getBeaconTopic();
  DmReadRequest = getDMReadRequestTopic();
  DmReadResponse = getDMReadResponseTopic();
  DmWriteRequest = getDMWriteRequestTopic();
  DmWriteResponse = getDMWriteResponseTopic();
}

bool eepromReset()
{
  for (int i = 0; i < EEPROM_MAX_SIZE; i++)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}
