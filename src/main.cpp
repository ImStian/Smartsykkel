// Importerer biblioteker
#include "UbidotsEsp32Mqtt.h" // Bibliotek som håndterer kommunikasjon med Ubidots
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Wire.h> 
#include <math.h> 
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>




// Ubidots informasjon:
const char *UBIDOTS_TOKEN = "BBFF-11w8MyGZHRZHDK6M69cHk1etEGFejV";  // Ubidots TOKEN
const char *WIFI_SSID = "esp32-hotspot";      // Wi-Fi SSID
const char *WIFI_PASS = "esp32-hotspot-passord";      // Wi-Fi password
const char *DEVICE_LABEL = "tricycle2";   // device label to subscribe to
const char *VARIABLE_LABEL1_UPLOAD = "altitude";
const char *VARIABLE_LABEL2_UPLOAD = "temperature";
const char *VARIABLE_LABEL3_UPLOAD = "lat";
const char *VARIABLE_LABEL4_UPLOAD = "lng";
const char *VARIABLE_LABEL5_UPLOAD = "velocity";


// Variabler knyttet til opplastning av data
unsigned int uploaddelay = 25000; // Tid mellom opplastning av data (i ms)
unsigned long previousUpload = 0;

// Variabler knyttet til beregning av fart
unsigned velocityTimedelta = 5000;
unsigned long previousVelocityCalcuation = 0;


// Variabler knyttet til kalibrerering av mpu6050 
float accelerationErrorX; // Avvik i måling for X-retning
float accelerationErrorY; // Avvik i måling for Y-retning 
float accelerationErrorZ; // Avvik i måling for Z-retning

float velocityX = 0;
float velocityY = 0;
float velocityZ = 0;

// GPS
    //Pinout for ESP32 GPS
    // RX = 17
    // TX = 16
    // VCC = 3.3 v 
    // GND = GND  
static const uint32_t GPSBaud = 9600;
float gps_latitude;
float gps_longitude;


// Initialiserer objekter
Ubidots ubidots(UBIDOTS_TOKEN); Adafruit_BME280 bme;
Adafruit_MPU6050 mpu; TinyGPSPlus gps;
Adafruit_SSD1306 display(128, 32, &Wire, -1); // OLED-display dimensjoner:(128x32)
SoftwareSerial ss(17, 16); // Serial kommunikasjon med gps (RX=17, TX=16)



// Tilleggsfunksjoner - (Hardware)
void configure_bme(){// Konfigurerer BME280
    Serial.println("Sjekker koblingen til BME280:");
    if(!bme.begin(0x76)){
        Serial.println("ERROR: BME280 er ikke koblet riktig!");
        while(1); // Fanger script i evig løkke
    } Serial.println("BME280 er koblet riktig!");
}


void configure_mpu(){// Konfigurerer MPU6050
    Serial.println("Sjekker koblingen til MPU6050:");
    if(!mpu.begin()){
        Serial.println("ERROR: MPU6050 er ikke koblet riktig!");
        while(1); // Fanger script i evig løkke.
    } Serial.println("MPU6050 er koblet riktig!");

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // Setter aksellerasjonsrange
    Serial.print("Aksellerometer range er satt til: ");
    switch(mpu.getAccelerometerRange()){
        case MPU6050_RANGE_2_G:
            Serial.println("+-2G"); break;
        case MPU6050_RANGE_4_G:
            Serial.println("+-4G"); break;
        case MPU6050_RANGE_8_G:
            Serial.println("+-8G"); break;
        case MPU6050_RANGE_16_G:
            Serial.println("+-16G"); break;
    }
}


void calibrate_mpu(){// Kalibrerer MPU6050 for å minimere målingsavvik
    sensors_event_t a, g, temp;

    Serial.println("Gjennomfører kalibreringsmålinger");
    // Finner målingsavvik ved å ta gjennomsnittet av n-målinger
    int n = 50;
    float allMeasurementsX = 0;
    float allMeasurementsY = 0;
    float allMeasurementsZ = 0;
    for(int i=0; i<n; i++){
            // Gjør målinger
        mpu.getEvent(&a, &g, &temp);
        allMeasurementsX += a.acceleration.x;
        allMeasurementsY += a.acceleration.y;
        allMeasurementsZ += a.acceleration.z;
            // Printer til Serial monitor
        Serial.print("Gjør måling: "); Serial.println(i);
        Serial.print("Akselerasjon X: "); Serial.println(a.acceleration.x);
        Serial.print("Akselerasjon X: "); Serial.println(a.acceleration.y);
        Serial.print("Akselerasjon X: "); Serial.println(a.acceleration.z);
        delay(100);
    }
    // Funksjonen pass-er tre globale variabler.   
    accelerationErrorX = allMeasurementsX / n;
    accelerationErrorY = allMeasurementsY / n;
    accelerationErrorZ = allMeasurementsZ / n;
    Serial.print("accelerationErrorX: "); Serial.println(accelerationErrorX);
    Serial.print("accelerationErrorY: "); Serial.println(accelerationErrorY);
    Serial.print("accelerationErrorZ: "); Serial.println(accelerationErrorZ);
    
}


void configure_oled(){ //Funksjon som sjekker om OLED-skjermen er koblet til riktig
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);}

  display.clearDisplay(); // Clearer skjerm for å evt. fjerne det tidligere innholdet
  display.setTextSize(1); // Velger størrelse på tekst
  display.setTextColor(WHITE); // Velger farge på tekst 
  display.setCursor(0, 10); //
}


void configure_gps(){// Konfigurerer GPS
    ss.begin(GPSBaud);
    Serial.println(TinyGPSPlus::libraryVersion());
    Serial.println("Setter opp GPS");
}


void OLED(float temp, float vel, float alt){// Viser målinger på skjermen
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(temp);
    display.print(" Celcius");
    
    display.setCursor(0, 10);
    display.print("Velocity:  ");
    display.print(vel);
    display.print(" m/s");


    display.setCursor(0, 20);
    display.print("Altitude:  ");
    display.print(alt);
    display.print(" meter");
    display.display();
}



// Tilleggsfunksjoner - (Software)
float calculateVelocity(float acceleration){
    return (acceleration * velocityTimedelta);
}


float vectorLength(float x, float y){
    float length = pow((pow(x, 2) + pow(y, 2)), 0.5); 
      return length;
} 


void VisLokasjon(){  
  //Henter lokasjon data
  Serial.print(F("Lokasjon: ")); 
  if (gps.location.isValid()) 
  {
    gps_latitude = gps.location.lat(); //Henter breddegrad
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    gps_longitude = gps.location.lng(); //Henter lengdegrad
  }
  else
  {
    Serial.print(F("UGYLDIG"));
  }
  Serial.println();
}


// Hovedfunksjoner
void setup(){
    Serial.begin(115200); // Starter Serial-monitor

        // Konfigurerer hardware
    configure_bme(); configure_oled();
    configure_mpu(); configure_gps();
    calibrate_mpu();

        //Starter kommunikasjon med Ubidots:
    ubidots.setDebug(false); // sett til true for å skru på debugging
    ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
    ubidots.setup(); ubidots.reconnect();
}

void loop(){
    if(!ubidots.connected()){     // Passer på at ESP32 heletiden er tilkoblet ubidots.
      ubidots.reconnect();
    }

    // Beregner komponentene til fartsvektoren og leser GPS-data
    if((millis() - previousVelocityCalcuation) > velocityTimedelta){         
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        Serial.print("Acceleration (x,y,z): "); 
            Serial.print(a.acceleration.x - accelerationErrorX); 
            Serial.print(a.acceleration.y - accelerationErrorY); 
            Serial.println(a.acceleration.z - accelerationErrorZ);

        velocityX += calculateVelocity((a.acceleration.x - accelerationErrorX));
        velocityY += calculateVelocity((a.acceleration.y - accelerationErrorY));
        velocityZ += calculateVelocity((a.acceleration.z - accelerationErrorZ));

        previousVelocityCalcuation = millis();

        
            // Denne koden viser GPS data hver gang den får gyldige avlesinger
        while (ss.available() > 0)
            if (gps.encode(ss.read()))
            VisLokasjon();
        if (millis() > 5000 && gps.charsProcessed() < 10){
            Serial.println(F("Ingen GPS funnet, antenne problem eller feil oppkobling."));
            while(true);
        }
    }



    // ESP32 er i "sovemodus" fram til den skal laste opp noe
    if((millis() - previousUpload) > uploaddelay){
            // Gjennomfører målinger på BME280
        float altitude = bme.readAltitude(1013); // Parameteren som er brukt: "Sea level pressure"
        float temperature = bme.readTemperature();


            // Beregner lengden av fartsvektoren:
        float velocity = vectorLength(velocityX, velocityY);
            // Oppdaterer skjerm
        OLED(temperature, velocity, altitude);
            // Laster opp data
        previousUpload = millis();
        ubidots.add(VARIABLE_LABEL1_UPLOAD, altitude); // Legger til variablen til det som skal bli lastet opp
        ubidots.add(VARIABLE_LABEL2_UPLOAD, temperature);
        ubidots.add(VARIABLE_LABEL3_UPLOAD, gps_latitude);
        ubidots.add(VARIABLE_LABEL4_UPLOAD, gps_longitude);
        ubidots.add(VARIABLE_LABEL5_UPLOAD, velocity)
        ubidots.publish(DEVICE_LABEL); // Laster opp all dataen som har blitt lagt til.
    }
   

    ubidots.loop();
    
}
