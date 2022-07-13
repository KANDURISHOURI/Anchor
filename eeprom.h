#include <EEPROM.h>

#define EEPROM_MAX_SIZE 1024

#define ANCHOR_SERIAL_NUMBER_IDX 0
#define ANCHOR_NAME_IDX 15
#define DEVICE_TYPE_IDX 22
#define WIFI_SSID_START_IDX 26
#define WIFI_PSWD_START_IDX 78
#define MQTT_SERVER_IDX 130
#define MQTT_PORT_IDX 206
#define BEACON_TOPIC_IDX 215
#define DMREAD_REQUEST_TOPIC_IDX 292
#define DMREAD_RESPONSE_TOPIC_IDX 370
#define DMWRITE_REQUEST_TOPIC_IDX 450
#define DMWRITE_RESPONSE_TOPIC_IDX 530
#define BIT_IDX 610

#define ANCHOR_SERIAL_NUMBER_SIZE 12
#define ANCHOR_NAME_SIZE 5
#define DEVICE_TYPE_SIZE 3
#define WIFI_SSID_SIZE 50
#define WIFI_PSWD_SIZE 50
#define MQTT_SERVER_STRING_SIZE 75
#define MQTT_PORT_SIZE 5
#define BEACON_TOPIC_SIZE 75
#define DMREAD_REQUEST_TOPIC_SIZE 75
#define DMREAD_RESPONSE_TOPIC_SIZE 75
#define DMWRITE_REQUEST_TOPIC_SIZE 75
#define DMWRITE_RESPONSE_TOPIC_SIZE 75
#define BIT_SIZE 3

bool updateWifiSsid(String message);
bool updateWifiPswd(String message);
bool updateMqttServer(String message);
bool updateIndexBit(String message) ;
bool updateAnchorSerialNumber(String message) ;
bool updateAnchorName(String message);
bool updateDeviceType(String message);
bool updateMqttPort(String message);
bool updateMqttTopic(String message);
bool updateBeaconTopic(String message);
bool updateBeaconTopic(String message);
bool updateDMReadRequestTopic(String message);
bool updateDMReadResponseTopic(String message);
bool updateDMWriteRequestTopic(String message) ;
bool updateDMWriteResponseTopic(String message);

String getMqttServer();
String getWifiPswd();
String getWifiSsid();
String getIndexBit();
String getAnchorName();
String getAnchorSerialNumber();
String getDeviceType();
String getMqttPort();
String getBeaconTopic();
String getDMReadResponseTopic();
String getDMReadRequestTopic();
String getDMWriteResponseTopic();
String getDMWriteRequestTopic() ;

void eepromInit() ;
bool eepromReset();
void writeDefaultParameters();
