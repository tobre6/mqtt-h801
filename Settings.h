#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

class Settings {
public:
  boolean load();
  void save();

  void setName(String);
  String getName();

  void setWifiSSID(String);
  String getWifiSSID();

  void setWifiPassword(String);
  String getWifiPassword();

  void setMQTTServer(String);
  String getMQTTServer();

  void setMQTTTopic(String);
  String getMQTTTopic();

  void setRelayPin(int);
  int getRelayPin();

private:

  struct CONTAINER {
    char name[32];
    char wifiSSID[32];
    char wifiPassword[32];
    char mqttServer[32];
    char mqttTopic[32];
    char mqttUsername[32]; // For future
    char mqttPassword[32]; // For future
    int relay;
  } container;
};

#endif




