
/*
    MQTT Client for an ESP8266 device that sends binary sensordata to
    a MQTT broker.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "geodan";
const char* password = "";
const char* mqtt_server = "gost.geodan.nl";

const char* outTopic = "GOST/Datastreams(32)/Observations";

WiFiClient espClient;
PubSubClient client(espClient);
char data[80];

void setup_wifi() {

    Serial.print("Connecting to wifi network: ");
    Serial.println(ssid);

    pinMode(A0, INPUT);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    digitalWrite(13, LOW);
    delay(500);
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
    digitalWrite(13, HIGH);

    Serial.println("Connected: ");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");

        // Attempt to connect
        if (client.connect("ESP8266Client-Plants```````````````````````````````````````````````````````````````````````````````````````````````````````````````````````")) {
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
    digitalWrite(13, LOW);
    delay(500);
    digitalWrite(13, HIGH);
    delay(500);
    Serial.begin(115200);
    setup_wifi();
    Serial.println(mqtt_server);
    client.setServer(mqtt_server, 1883);
}

void loop() {
    delay(1000);
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    uint16 out = analogRead(A0);
    Serial.println(out);
    String payload = "{ \"result\": " + String(out) + "}";
    Serial.println(payload);
    payload.toCharArray(data, (payload.length() + 1));
    Serial.println(data);

    client.publish(outTopic, data);
}
