#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "default_settings.h"
#include "Webserver.h"
#include "Settings.h"
#include "Led.h"
#include "ColorConverter.h"

#define MQTT_PORT   1883


// RGB FET
#define RED_PIN    15
#define GREEN_PIN  13
#define BLUE_PIN   12


// W FET
#define W1_PIN     14
#define W2_PIN     4

#define GREEN_LEDPIN    5
#define RED_LEDPIN      1

#define CONNECTION_CHECK_INTERVAL 10 // In seconds

extern "C" { 
  #include "user_interface.h" 
}

WiFiClient wifiClient;
PubSubClient *mqttClient;
Webserver *webserver;
Settings *settings;
unsigned long lastConnectionCheckTime = 0;
unsigned int brightness = 0;

boolean on = false;
boolean mqttInitialized = false;

Led red(RED_PIN);
Led green(GREEN_PIN);
Led blue(BLUE_PIN);
//Led white1(W1_PIN);
//Led white2(W2_PIN);
RgbColor rgb = {0, 0, 0};

bool connectToWifi();
void connectToMqtt();
void mqttCallback(const MQTT::Publish&);
void turnOffLights();
void turnOnLights();
void setBrightness(int);
void setRgb(RgbColor rgbColor);
void checkConnection();
RgbColor strToRgb(String rgbString);
String rgbToStr(RgbColor rgb);

void setup() {
  Serial1.begin(115200);
  Serial1.print("Starting up");

  red.setup();
  green.setup();
  blue.setup();
  //white1.setup();
  //white2.setup();

  pinMode(GREEN_LEDPIN, OUTPUT);  
  pinMode(RED_LEDPIN, OUTPUT);

  digitalWrite(RED_LEDPIN, HIGH);

  settings = new Settings;
  webserver = new Webserver(settings);
  if (!settings->load()) {
    Serial1.println("Initializing settings");
    #ifdef DEFAULT_SETTINGS
      settings->setName(NAME);
      settings->setWifiSSID(WIFI_SSID);
      settings->setWifiPassword(WIFI_PASS);
      settings->setMQTTServer(MQTT_SERVER);
      settings->setMQTTTopic(MQTT_TOPIC);
    #endif
    settings->save();
  }

  mqttClient = new PubSubClient(wifiClient, settings->getMQTTServer(), MQTT_PORT);
  mqttClient->set_callback(mqttCallback);
  if (connectToWifi()) {
    connectToMqtt();
  }
}

void loop() {
  mqttClient->loop();
  webserver->loop();
  if (millis () > lastConnectionCheckTime + 1000 * CONNECTION_CHECK_INTERVAL) {
    checkConnection();
  }

  red.update();
  green.update();
  blue.update();
  //white1.update();
  //white2.update();
}

bool connectToWifi() {
  Serial1.println("Connecting to Wifi");
  int retries = 10;
  WiFi.begin(settings->getWifiSSID().c_str(), settings->getWifiPassword().c_str());
  while ((WiFi.status() != WL_CONNECTED) && retries--) {
    delay(500);
    Serial1.print(" .");
  }

  if (WiFi.status() == WL_CONNECTED) { 
    Serial1.print("IP address: ");
    Serial1.println(WiFi.localIP());
    return true;
  }

  Serial1.println("Failed to connect to Wifi");
  return false;
}

void connectToMqtt() {
  Serial1.println("Connecting to MQTT");
  int retries = 10;

  while (!mqttClient->connect(MQTT::Connect(settings->getName()).set_keepalive(90)) && retries--) {
    Serial1.print(" .");
    delay(1000);
  }

  if(mqttClient->connected()) {
    Serial1.println("Connected to MQTT");
    mqttClient->subscribe(settings->getMQTTTopic() + "/switch");
    mqttClient->subscribe(settings->getMQTTTopic() + "/brightness/set");
    mqttClient->subscribe(settings->getMQTTTopic() + "/rgb/set");

    digitalWrite(RED_LEDPIN, LOW);
    digitalWrite(GREEN_LEDPIN, HIGH);

    mqttInitialized = true;
  }
}

void mqttCallback(const MQTT::Publish& pub) {
  Serial1.print("MQTT topic: ");
  Serial1.println(pub.topic());
  Serial1.print("MQTT message: ");
  Serial1.println(pub.payload_string());

  if (pub.topic() == settings->getMQTTTopic() + "/brightness/set") {
    setBrightness(pub.payload_string().toInt());
    mqttClient->publish(MQTT::Publish(settings->getMQTTTopic() + "/brightness/status", pub.payload_string()).set_retain(1).set_qos(1));
    return;
  }

  if (pub.topic() == settings->getMQTTTopic() + "/rgb/set") {
    setRgb(strToRgb(pub.payload_string()));
    mqttClient->publish(MQTT::Publish(settings->getMQTTTopic() + "/rgb/status", pub.payload_string()).set_retain(1).set_qos(1));
    return;
  }

  if (pub.payload_string() == "on") {
    turnOnLights();
  }
  if (pub.payload_string() == "off") {
    turnOffLights();
  }

  if (mqttInitialized) {
    mqttClient->publish(MQTT::Publish(settings->getMQTTTopic() + "/status", pub.payload_string()).set_retain(1).set_qos(1));
  }
}

void turnOnLights() {
  if (on) {
    return;
  }
  Serial1.println("Turning lights on");
  on = true;
  //white1.set(brightness);
  //white2.set(brightness);
  setRgb(rgb);
}

void turnOffLights() {
  if (!on) {
    return;
  }
  Serial1.println("Turning lights off");
  on = false;
  //white1.set(0);
  //white2.set(0);
  red.set(0);
  green.set(0);
  blue.set(0);
}

void setBrightness(int newBrightness) {
  Serial1.println("Setting brightness to " + String(newBrightness));
  brightness = newBrightness;
  //white1.set(brightness);
  //white2.set(brightness);

  HsvColor hsv = RgbToHsv(rgb);
  hsv.v = newBrightness;
  rgb = HsvToRgb(hsv);
  
  setRgb(rgb);
}

void setRgb(RgbColor rgbColor) {
  rgb = rgbColor;
  red.set(rgb.r);
  green.set(rgb.g);
  blue.set(rgb.b);

  HsvColor hsv = RgbToHsv(rgb);
  brightness = hsv.v;

  if (mqttInitialized) {
    mqttClient->publish(MQTT::Publish(settings->getMQTTTopic() + "/brightness/status", String(brightness)).set_retain(1).set_qos(1));
    mqttClient->publish(MQTT::Publish(settings->getMQTTTopic() + "/rgb/status", rgbToStr(rgb)).set_retain(1).set_qos(1));
  }  
}

RgbColor strToRgb(String rgbString) {
  int r, g, b = 0;
  sscanf(rgbString.c_str(), "%d,%d,%d", &r, &g, &b);

  return {r, g, b};
}

String rgbToStr(RgbColor rgb) {
  return String(rgb.r) + "," + String(rgb.g) + "," + String(rgb.b);
}

void checkConnection() {
  if (WiFi.status() != WL_CONNECTED)  {
    connectToWifi();
  }
  if (WiFi.status() == WL_CONNECTED && !mqttClient->connected()) {
    connectToMqtt();
  }

  lastConnectionCheckTime = millis();
}
