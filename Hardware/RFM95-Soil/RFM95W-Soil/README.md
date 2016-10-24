# Plant moisture sensor platform

This is the firmware for a plant moisture sensor platform running on a 3.3V (!) Arduino microprocessor. I used an Arduino Pro Mini 3.3V. It uses three [soil moisture sensors](http://www.hackerstore.nl/Artikel/309) for its data and a [RFM95W](http://webshop.ideetron.nl/RFM95W) for communication over LoRaWAN.
We use [The Things Network](www.thethingsnetwork.org) as our LoRaWAN network.

# Hardware

Connect the three moisture sensors as usual: VCC to VCC, GND to GND and AO to analog pin 0, 1 and 2.
The RFM95W is somewhat more difficult. Connect it as follows:


RFM95W      3.3V Arduino
--------------------------

NSS         10
RST         9
D0          2
D1          5
D2          6
MOSI        11 (or MOSI equivalent on your arduino)
MISO        12 (or MOSI equivalent on your arduino)
SCK         13 (or MOSI equivalent on your arduino)

Some of these can also be change in the pin mapping in the software.

# Software

To use the platform, you need to change the DEVEUI, APPEUI and APPKEY to your TTN keys.

When the firmware is uploaded, the platform will send its sensor data to The Things Network. This data is send in a simple binary format, which you will need to decode. You can use Node-RED for that or you can use the TTN decode functionality in the Dashboard. If you use Node-RED, you can use the Node-RED flow in node-red-setup.txt. Just make sure you have the right username, password and topic in the MQTT-IN node.
