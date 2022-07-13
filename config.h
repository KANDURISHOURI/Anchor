#define DEBUG true
#define UPDATE_ENABLED true

#define ANCHOR_NAME "A2"
#define ANCHOR_SERIAL_NUMBER 5

#define MQTT_PORT 1883
#define MQTT_ANCHOR_ALIVE_STATUS "AnchorAliveStatus"

#define SCAN_TIME 10
#define WIFI_RECONNECTION_ATTEMPTS 10
#define EEPROM_RESET_INCREMENT_CODE 9

#include "eeprom.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>

/*WIFI Object */
WiFiClient EspClient;

/*MQTT Object */
PubSubClient MqttClient(EspClient);

/*Ticker Object */
Ticker Publisher;

/*BLE Objects */
BLEScan *pBLEScan;
BLEAdvertising *pAdvertising;


/*Old Logic Tag Info Structure*/
struct TagInfo
{
  int rssi;
  int count;
  uint8_t oldSeconds;
  uint8_t currentSeconds;
  uint16_t timeSynchroMS;
  uint8_t txPower;
  String tagId;
} TagInfo_t = {
    0,
    0,
    0,
    0,
    0,
    0,
    "."};

/* New Logic Tag Info Structure   -   Currently In Use */
struct TagData
{
  int rssi;
  int count;
  unsigned long lastMillis;
  unsigned long currentMillis;
  uint8_t oldSeconds;
  uint8_t currentSeconds;
  uint16_t timeSynchroMS;
  uint8_t txPower;
  uint16_t timeDifference;
  uint8_t beaconCount;
  uint8_t oldBeaconCount;
  String tagId;
};
