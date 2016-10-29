#ifndef WEBSERVER_H
#define WEBSERVER_H

class ESP8266WebServer;
class Settings;

class Webserver {
public:
  Webserver(Settings*);
  void loop();

private:
  String uptime();
  Settings *settings;
  ESP8266WebServer *server;
};

#endif




