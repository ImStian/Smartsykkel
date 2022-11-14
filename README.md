# Smartsykkel
Dette er koden tilhørende et prosjekt i emnet IELET2001 - Datakommunikasjon.
Prosjektet samler inn informasjon fra ulike sensorer (BME280, MPU6050, NEO-6M) og sender disse til visning i et dashboard på Ubidots.

###Koden benytter seg av følgende biblioteker:
- PubSubClient
- Adafruit_BME280
- Adafruit_GFX
- Adafruit_SSD1306
- Adafruit_MPU6050
- Wire
- math 
- TinyGPSPlus
- SoftwareSerial (for ESP32)

###Hardware:
- BME280 - Temperatur, trykk og fuktighetsmåler
- MPU6050 - 3-Akse akselerometer og gyroskop
- NEO-6M - GPS
