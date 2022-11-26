# Smartsykkel
Dette er koden tilhørende et prosjekt i emnet IELET2001 - Datakommunikasjon.
Prosjektet samler inn informasjon fra ulike sensorer (BME280, MPU6050, NEO-6M) og sender disse til visning i et dashboard på Ubidots.

#### Nødvendige biblioteker:
- PubSubClient
- Adafruit_BME280
- Adafruit_GFX
- Adafruit_SSD1306
- Adafruit_MPU6050
- Wire
- math 
- TinyGPSPlus
- SoftwareSerial (for ESP32)

#### Hardware:
- BME280 - Temperatur, trykk og fuktighetsmåler
- MPU6050 - 3-Akse akselerometer og gyroskop
- NEO-6M - GPS
- OLED Skjerm 128x64

## Konfigurering:
For å få denne koden til å fungere til eget bruk er det nødvendig med litt konfigurering av koden.
Det meste av dette finner du under "Ubidots Informasjon":
```cpp
const char *UBIDOTS_TOKEN = "";  // Ubidots TOKEN
const char *WIFI_SSID = "";      // Wi-Fi SSID
const char *WIFI_PASS = "";      // Wi-Fi password
const char *DEVICE_LABEL = "";   // device label to subscribe to
```
- <em>UBIDOTS_TOKEN</em> setter du inn den brukerspesifike tokenen du finner på Ubidots.
- <em>WIFI_SSID</em> skriver du inn navnet (SSID) på nettverket ESP32 skal koble til.
- <em>WIFI_PASS</em> skriver du inn passordet til nettverket du definerte under <em>WIFI_SSID</em>.
- <em>DEVICE_LABEL</em> er navnet på enheten du oppretter inne på Ubidots.

Når du kjører koden for første gang vil variablene som tilhører enheten automatisk bli lagt inn på Ubidots. Om du ønsker å vise fram målingene dine, oppretter du widgets på et dashboard inne på Ubidots og velger disse variablene.

## NB:
Kodefilene er av filtypen ".cpp". For å laste opp til ESP32 krever det derfor at man kopierer innholdet i disse kodefilene over til Arduino IDE. ".cpp" blir brukt framfor ".ino" fordi av erfaring kan det til tider oppstå problemer med å opprette skisser med ".ino"-filer, som gjør det vrient å lese av koden.
