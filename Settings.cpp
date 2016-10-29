#include "Settings.h"
#include "FS.h"

#define SETTINGS_FILE "/settings"

boolean Settings::load() {

  Serial.println("Loading settings");

  memset(&container, 0x00, sizeof(CONTAINER));

  if (!SPIFFS.begin()) {
    Serial.println("Failed to load settings!");
    return false;
  }

  File file = SPIFFS.open(SETTINGS_FILE, "r");
  if (!file) {
    Serial.println("No settings file, formatting");
    SPIFFS.format();
    return false;
  }

  uint8_t *bytes = (uint8_t*)&container;
  for (int i = 0; i < sizeof(CONTAINER); i++) {
    bytes[i] = file.read();
  }
  file.close();
  
  return true;
}

void Settings::save() {
  File file = SPIFFS.open(SETTINGS_FILE, "w");
  if (!file) {
    Serial.println("Failed to save settings");
  }
  uint8_t *bytes = (uint8_t*)&container;
  for (int i = 0; i < sizeof(CONTAINER); i++) {
    file.write(bytes[i]);
  }
  file.close();
}

void Settings::setName(String name) {
  strlcpy(container.name, name.c_str(), sizeof(container.name));
}

String Settings::getName() {
  return String(container.name);
}

void Settings::setWifiSSID(String ssid) {
  strlcpy(container.wifiSSID, ssid.c_str(), sizeof(container.wifiSSID));
}

String Settings::getWifiSSID() {
  return String(container.wifiSSID);
}

void Settings::setWifiPassword(String password) {
  strlcpy(container.wifiPassword, password.c_str(), sizeof(container.wifiPassword));
}

String Settings::getWifiPassword() {
  return String(container.wifiPassword);
}

void Settings::setMQTTServer(String server) {
  strlcpy(container.mqttServer, server.c_str(), sizeof(container.mqttServer));
}

String Settings::getMQTTServer() {
  return String(container.mqttServer);
}

void Settings::setMQTTTopic(String topic) {
  strlcpy(container.mqttTopic, topic.c_str(), sizeof(container.mqttTopic));
}

String Settings::getMQTTTopic() {
  return String(container.mqttTopic);
}

void Settings::setRelayPin(int relayPin) {
  container.relay = relayPin;
}

int Settings::getRelayPin() {
  return container.relay;
}


