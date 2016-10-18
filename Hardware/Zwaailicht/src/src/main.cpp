/*
    MQTT Client for an ESP8266 device that sends binary sensordata to
    a MQTT broker.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "geodan";
const char* password = "";

// MQTT
const char* mqtt_server = "gost.geodan.nl";
const uint16_t mqqt_port = 1883;
const char* mqtt_client_id = "zwaailicht";
const char* inTopic = "Datastreams(35)/Observations";

// Pins
const int relaisPin = 16;

// Other
WiFiClient espClient;
PubSubClient client(espClient);

// Set the relais on or off
void setRelais(bool on){
  if(!on){
    digitalWrite(relaisPin, HIGH);
  }else{
    digitalWrite(relaisPin, LOW);
  }
}

// MQTT callback, message on topic received
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char *json = (char*)payload;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success())
  {
    return;
  }

  bool result = root["result"];
  setRelais(result);
}

void setupWifi() {
    Serial.print("Connecting to wifi network: ");
    Serial.println(ssid);

    WiFi.disconnect();
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println("Connected: ");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void setupMqtt() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");

        if (client.connect(mqtt_client_id)) {
            client.subscribe(inTopic);
            Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            for(int i = 0; i<5000; i++){
                delay(1);
            }
        }
    }
}

void setup() {
    Serial.begin(57600);

    pinMode(relaisPin, OUTPUT);
    // Turn off relais
    setRelais(false);

    client.setServer(mqtt_server, mqqt_port);
    client.setCallback(mqttCallback);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED){
      // turn off light if wifi disconnected
      setRelais(false);
      setupWifi();
    }

    if (!client.connected()) {
      setupMqtt();
    }

    client.loop();
}
