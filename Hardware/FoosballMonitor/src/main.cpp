
/*
    MQTT Client for an ESP8266 device that sends binary sensordata to
    a MQTT broker.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "geodan";
const char* password = "";
const char* mqtt_server = "gost.geodan.nl";

const char* door_topic = "GOST/Datastreams(39)/Observations";
char data[80];

WiFiClient espClient;
PubSubClient client(espClient);

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

    pinMode(PIR_PIN, INPUT);
    pinMode(DOOR_PIN, INPUT_PULLUP);

    client.setServer(mqtt_server, 1883);
}

void loop() {

    if (!client.connected()) {
       reconnect();
    }
    client.loop();

    bool new_pir_state = digitalRead(PIR_PIN);
    if (new_pir_state != pir_state && pir_state) {
        pir_state = new_pir_state;
        Serial.println("Movement Done");
    }
    else if (new_pir_state != pir_state && !pir_state) {
        pir_state = new_pir_state;
        Serial.println("Movement Began!");
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
