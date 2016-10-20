
/*
    MQTT Client for an ESP8266 device that sends binary sensordata to
    a MQTT broker.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_HTU21DF.h"

const char* ssid = "geodan";
const char* password = "";
const char* mqtt_server = "gost.geodan.nl";

const char* door_topic = "GOST/Datastreams(39)/Observations";
const char* pir_topic = "GOST/Datastreams(40)/Observations";
const char* temperature_topic = "GOST/Datastreams(41)/Observations";
const char* hum_topic = "GOST/Datastreams(42)/Observations";

unsigned long time_of_last_loop = millis();

char data[80];

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

const int DOOR_PIN = 5;
const int PIR_PIN = 4;

bool pir_state = false;
bool door_state = false;

void setup_wifi() {

    Serial.print("Connecting to wifi network: ");
    Serial.println(ssid);
    Serial.println(password);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println("Connected: ");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");

        // Attempt to connect
        if (client.connect("ESP8266Client-foosball")) {
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
    Serial.begin(9600);
    setup_wifi();
    Wire.begin(D3, D5);

    pinMode(PIR_PIN, INPUT);
    pinMode(DOOR_PIN, INPUT_PULLUP);

    client.setServer(mqtt_server, 1883);
}

void loop() {

    if (!client.connected()) {
       reconnect();
    }
    client.loop();

    if ((millis() - time_of_last_loop) > 60000) {
        Serial.println("Logging :)");
        Serial.println(String(htu.readTemperature()));
        String payload = "{\"result\" : \" " + String(htu.readTemperature()) + "\"}";
        payload.toCharArray(data, (payload.length() + 1));
        client.publish(temperature_topic, data);

        payload = "{\"result\" : \" " + String(htu.readHumidity()) + "\"}";
        payload.toCharArray(data, (payload.length() + 1));
        client.publish(hum_topic, data);

        time_of_last_loop = millis();
    }

    if (millis() < time_of_last_loop) {
        time_of_last_loop = millis();
    }

    bool new_pir_state = digitalRead(PIR_PIN);
    if (new_pir_state != pir_state && pir_state) {
        pir_state = new_pir_state;
        Serial.println("Movement Done");

        String payload = "{\"result\" : \" " + String(!pir_state)+ "\"}";
        payload.toCharArray(data, (payload.length() + 1));
        client.publish(pir_topic, data);
    }
    else if (new_pir_state != pir_state && !pir_state) {
        pir_state = new_pir_state;
        Serial.println("Movement Began!");

        String payload = "{\"result\" : \" " + String(!pir_state)+ "\"}";
        payload.toCharArray(data, (payload.length() + 1));
        client.publish(pir_topic, data);
    }

    bool new_door_state = digitalRead(DOOR_PIN);
    if (new_door_state != door_state && door_state) {
        door_state = new_door_state;
        Serial.println("Door closed");

        String payload = "{\"result\" : \" " + String(!door_state)+ "\"}";
        payload.toCharArray(data, (payload.length() + 1));
        client.publish(door_topic, data);
    }
    else if (new_door_state != door_state && !door_state) {
        door_state = new_door_state;
        Serial.println("Door opened");

        String payload = "{\"result\" : \" " + String(!door_state)+ "\"}";
        payload.toCharArray(data, (payload.length() + 1));
        client.publish(door_topic, data);
    }
}
