#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>

#include "Webserver.h"
#include "Settings.h"
#include "default_settings.h"
#include "version.h"


const char compile_date[] = __DATE__ " " __TIME__;

Webserver::Webserver(Settings* settings) {
  this->settings = settings;
  server = new ESP8266WebServer(80);
  Serial.println(compile_date);

  server->on("/", [&](){

    String html = "<html><head><title>" + this->settings->getName() + "</title>";
    html += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" crossorigin=\"anonymous\">";
    html += "<link href=\"https://maxcdn.bootstrapcdn.com/font-awesome/4.6.3/css/font-awesome.min.css\" rel=\"stylesheet\" />";
    html += "</head><body>";
    html += "<div class=\"container\"><div class=\"starter-template\">";
    html += "<h1>" + this->settings->getName() + "</h1>";
    html += "<p class=\"lead\">Version: <a target=\"_blank\" href=\"https://github.com/2ndalpha/mqtt-switch/commit/" + String(VERSION) + "\">" + String(VERSION) + "</a><br/>";
    html += "Built: " + String(compile_date) + "<br/>";
    html += "Uptime: " + uptime() + "</p>";
    html += "<p class=\"lead\">MQTT Server: " + this->settings->getMQTTServer() + "<br/>";
    html += "MQTT Topic: " + this->settings->getMQTTTopic() + "<br/>";
    html += "<p><a href=\"ota\" class=\"btn btn-primary\"><i class=\"fa fa-refresh\" aria-hidden=\"true\"></i> Upgrade</a></p>";
    html += "</div></div></body></html>";
    
    this->server->send(200, "text/html", html);
  });

  server->on("/ota", [&](){
    Serial.println("Commence OTA");
    this->server->sendHeader("Location", "/", true);
    this->server->send ( 302, "text/plain", "");
    ESPhttpUpdate.update("ota.clockwise.ee", 80, "/MQTT-H801.bin");
  });

  server->begin();
}

void Webserver::loop() {
  server->handleClient();
}

String Webserver::uptime() {
  long up = millis() / 1000;
  int minutes = (up / 60) % 60;
  int hours = (up / (60 * 60)) % 24;
  int days = (up / (60 * 60 * 24));

  if (minutes == 0 && hours == 0 && days == 0) {
    return "< 1 minute";
  }

  String result = String(hours) + ":";

  if (minutes < 10) {
    result += "0";
  }
  result += String(minutes);

  if (days > 1) {
    result = "1 day, " + result;
  }
  else if (days > 1) {
    result = String(days) + " days, " + result;
  }
  
  return result;
}

