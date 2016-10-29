#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "default_settings.h"
#include "Webserver.h"
#include "Settings.h"
#include "Led.h"

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
long brightness = 0;
boolean on = false;
boolean mqttInitialized = false;

Led red(RED_PIN);
Led green(GREEN_PIN);
Led blue(BLUE_PIN);
Led white(W1_PIN);
Led white2(W2_PIN);

bool connectToWifi();
void connectToMqtt();
void mqttCallback(const MQTT::Publish&);
void turnOffLights();
void turnOnLights();
void setBrightness(int);
void checkConnection();

void setup() {
  Serial1.begin(115200);
  Serial1.print("Starting up");

  red.setup();
  green.setup();
  blue.setup();
  white.setup();
  white2.setup();

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
  white.update();
  white2.update();
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
  white.set(brightness);
  white2.set(brightness);
}

void turnOffLights() {
  if (!on) {
    return;
  }
  Serial1.println("Turning lights off");
  on = false;
  white.set(0);
  white2.set(0);
}

void setBrightness(int newBrightness) {
  Serial1.println("Setting brightness to " + String(newBrightness));
  brightness = newBrightness;
  white.set(brightness);
  white2.set(brightness);

  if (mqttInitialized) {
    mqttClient->publish(MQTT::Publish(settings->getMQTTTopic() + "/brightness", String(brightness)).set_retain(1).set_qos(1));
  }
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

