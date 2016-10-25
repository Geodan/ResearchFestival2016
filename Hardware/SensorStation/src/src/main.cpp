#include <Wire.h>
#include <EEPROM.h>
#include <FS.h>
// ToDo:
// Non blocking connections to wifi and mqtt
// MQTT screen
// Update interval for sensor readings
// Unique mqtt id
// Config reading writing
// Setup process

// OLED
#include <SSD1306.h>
SSD1306  display(0x3c, D3, D5);

//HTU21D
#include <Adafruit_HTU21DF.h>
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

// Wifi
#include <ESP8266WiFi.h>
WiFiClient espClient;
const char* ssid = "Ziggo418DB";
const char* password = "veldekekade3";

// MQTT
#include <PubSubClient.h>
const char* mqtt_server = "gost.geodan.nl";
const uint16_t mqqt_port = 1883;
const unsigned long publishInterval = 1000 * 240; //Milliseconds
const char* mqtt_client_id = "BaseStation1";
const String tempStream = "45";
const String humidityStream = "46";
const String pressureStream = "47";
const String altitudeStream = "48";

PubSubClient client(espClient);

// Barometer MBP280
#include <BMP280.h>
#define P0 1013.25
BMP280 bmp;

// Other
const int buttonPin = 16;
unsigned long lastDataPushTime;
const String configFile = "/config.txt";

//Screen
bool screenAlwaysOn = false;
bool screenOn = true; // Do not change
long screenTimeOut = (1000 * 60) * 1; // Turn screen off after 5 minutes
long screenUpdate = 400; // Update screen every xxxx ms if screen is on
unsigned long lastScreenUpdateTime; // Used for refresh rate on screen
unsigned long lastScreenRequestedTime; // Used for putting screen to sleep
enum Screens { Temperature,Humidity,Pressure,Altitude,Wifi };
Screens currentScreen = Wifi;

//RGB


// Values
float emptyValue = -999;
float temperature = emptyValue;
float humidity = emptyValue;
long pressure = emptyValue;
float noise = emptyValue;
float altitude = emptyValue;
String ipAddress;

void displayReading(String text, String value){
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 - 12, text);
  display.setFont(ArialMT_Plain_16);
  display.drawString(DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2 + 12, value);
  display.display();
}

void writeConfig(){
  File f = SPIFFS.open(configFile, "w");
  f.println("SPIFFS BAM!");
  f.close();
}

void readConfig(){
  File f = SPIFFS.open(configFile, "r");
  String test = f.readStringUntil('\n');
  test.trim();
  displayReading("CONFIG", test);
  f.close();
  delay(5000);
}

void spiffsInit(){
  SPIFFS.begin();
  File f = SPIFFS.open(configFile, "r");
  if(!f){ //First run, format spiffs
      displayReading("BERTHA#1", "STARTING");
      delay(3000);
      SPIFFS.format();
      writeConfig();
  }
  else{
    f.close();
  }
}

void updateHtu21D(){
  temperature = htu.readTemperature();
  humidity = htu.readHumidity();
}

void updateBmp280(){
  double T,P;
  char result = bmp.startMeasurment();

  if(result!=0){
    delay(result);
    result = bmp.getTemperatureAndPressure(T,P);
    //float temp = T;
    pressure = P;
      if(result!=0)
      {
        altitude = bmp.altitude(P,P0);
      }else{
        altitude = -999;
      }
  }else{
    pressure = -999;
    altitude = -999;
  }
}

void updateSensorReadings(){
  updateHtu21D();
  updateBmp280();
}

void showTemperature(){
  displayReading("TEMPERATURE", (String)temperature + " °C");
}

void showHumidity(){
  displayReading("HUMIDITY", (String)humidity + "%");
}

void showPressure(){
  displayReading("PRESSURE", (String)pressure + " hPa");
}

void showAltitude(){
  displayReading("ALTITUDE", (String)altitude + " m");
}

void showNoise(){
  displayReading("NOISE", (String)humidity + " dB");
}

void showIpAddress(){
  displayReading("WIFI", ipAddress);
}

String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void setupWifi() {
  displayReading("CONNECTING", ssid);

  WiFi.disconnect();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
      // ToDo: Add retries, do not loop forever
  }

  ipAddress = ipToString(WiFi.localIP());
  showIpAddress();
}

void setupMqtt() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT connection...");

        if (client.connect(mqtt_client_id)) {
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

void publish(String stream, String data){
  // Do not publish if empty value
  if(data == (String)emptyValue){
    return;
  }

  char sa[50];
  char pa[50];
  String("GOST/Datastreams(" + stream + ")/Observations").toCharArray(sa, 50);
  String("{ \"result\": " + data + "}").toCharArray(pa, 50);
  client.publish(sa, pa);
}

void publishData(){
  if(client.connected()){
    lastDataPushTime = millis();

    publish(tempStream, String(temperature));
    publish(humidityStream, String(humidity));
    publish(pressureStream, String(pressure));
    publish(altitudeStream, String(altitude));
  }
}

void setScreenOn(){
  screenOn = true;
  display.clear();
  display.setContrast(255);
  display.display();
}

void setScreenOff(){
  screenOn = false;
  display.clear();
  display.setContrast(0);
}

void updateScreen(){
  // Turn off screen if on and passed screenTimeOut time
  if(!screenAlwaysOn && screenOn && millis() - lastScreenRequestedTime > screenTimeOut){
    setScreenOff();
    display.display();
  }

  // Do not update if it was updated less than xxx ms
  if(!screenOn || millis() - lastScreenUpdateTime < screenUpdate){
    return;
  }

  switch(currentScreen){
      case Wifi:
        showIpAddress();
        break;
      case Temperature:
        showTemperature();
        break;
      case Humidity:
        showHumidity();
        break;
      case Pressure:
        showPressure();
        break;
      case Altitude:
        showAltitude();
        break;
  }

  lastScreenUpdateTime = millis();
}

// Check if button pressed, set next screen
void checkButton(){
  // Show next view
  if (digitalRead(buttonPin) == HIGH) {
    lastScreenRequestedTime = millis();

    // always instant refresh
    lastScreenUpdateTime = 0;
    // Turn on screen and show last view
    if(!screenOn){
      setScreenOn();
      updateScreen();
      return;
    }

    switch(currentScreen){
        case Wifi:
          currentScreen = Temperature;
          break;
        case Temperature:
          currentScreen = Humidity;
          break;
        case Humidity:
          currentScreen = Pressure;
          break;
        case Pressure:
          currentScreen = Altitude;
          break;
        case Altitude:
          currentScreen = Wifi;
          break;
    }

    updateScreen();
    delay(100);
  }
}

int redPin = 5;
int greenPin = 4;
int bluePin = 12;

void setColor(int red, int green, int blue)
{
//  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
//  #endif
  digitalWrite(redPin, red);
  digitalWrite(greenPin, green);
  digitalWrite(bluePin, blue);
}

void setup() {
  lastDataPushTime = millis();
  Wire.begin(D3, D5);
  Serial.begin(9600);

  pinMode(buttonPin, INPUT);

  display.init();
  display.setContrast(255);

  bmp.begin();
  bmp.setOversampling(4);

  client.setServer(mqtt_server, mqqt_port);

  // TEST
  //spiffsInit();
  //readConfig();
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
/*  Serial.println("red");
  setColor(255, 0, 0);  // red
 delay(10000);
 Serial.println("green");
 setColor(0, 255, 0);  // green
 delay(10000);
 Serial.println("blue");
 setColor(0, 0, 255);  // blue
 delay(10000);
 Serial.println("yellow");
 setColor(255, 255, 0);  // yellow
 delay(4000);
 Serial.println("purple");
 setColor(80, 0, 80);  // purple
 delay(4000);
 Serial.println("aqua");
 setColor(0, 255, 255);  // aqua
 delay(4000);*/

  checkButton();
  // Try connecting to wifi if disconnected, make non blocking
  if (WiFi.status() != WL_CONNECTED){
      ipAddress = "DISCONNECTED";
      setupWifi();
  }

  if (!client.connected()) {
      setupMqtt();
  }
  client.loop();

  updateSensorReadings();
  updateScreen();

  // rename to updateRemoteData and move check
  if(millis() - lastDataPushTime >= publishInterval){
    publishData();
  }
}
