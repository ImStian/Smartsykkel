// Libraries:
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

char topicSubscribe[100]; // Usikker om nødvendig.
#define VARIABLE_LABEL_SUBSCRIBE "spectacular_control" // Usikker om nødvendig.

// Ubidots og nettverksinformasjon:
#define SSID "esp32-hotspot"
#define PASSWORD "esp32-hotspot-passord"
#define TOKEN "BBFF-11w8MyGZHRZHDK6M69cHk1etEGFejV"
#define MQTT_CLIENT_NAME "esp32"
#define DEVICE_LABEL "spectacular16942069" // Ubidots navn
char mqttBroker[] = "things.ubidots.com";

// Diverse deklarasjoner for opplastning:
#define VARIABLE_LABEL1 "temperature"
char payload1[100];
char topic1[100];
char str_temperature[10]; // Størrelse på opplastningsvindu

#define VARIABLE_LABEL2 "altitude"
char payload2[100];
char topic2[100];
char str_altitude[10]; // Størrelse på opplastningsvindu

// Konstanter:
#define SEALEVELPRESSURE_HPA (1013.25)

// Objekter
WiFiClient ubidots; PubSubClient client(ubidots);
Adafruit_MPU6050 mpu; Adafruit_BME280 bme;


void connect(){// Kobler til WiFi og Ubidots
    WiFi.begin(SSID, PASSWORD);
    Serial.println(); Serial.print("Venter for WiFi-tilkobling...");
    
    // Printer "." fram til vellykket tilkobling:
    while(WiFi.status() != WL_CONNECTED){
        Serial.print("."); delay(50);
    }
    Serial.println(); Serial.println("WiFi tilkoblet!");
    Serial.print("IP adresse: "); Serial.println(WiFi.localIP());
    client.setServer(mqttBroker, 1883);
    sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LABEL_SUBSCRIBE); // Usikker om nødvendig.
    client.subscribe(topicSubscribe); // Usikker om nødvendig.
}

void reconnect(){ // Funksjon for å koble til på nytt
    while(!client.connected()){
        Serial.println("Forsøker på koble til Ubidots vha. MQTT");

        if(client.connect(MQTT_CLIENT_NAME, TOKEN, "")){
            Serial.println("Tilkoblet!");
            client.subscribe(topicSubscribe); // Er denne nødvendig?
        }
        else{
            Serial.print("Mislykket, rc="); Serial.print(client.state());
            Serial.println(" prøver igjen om 2 sekunder.");
            delay(2000); // Venter 2sekunder og prøver på nytt.
        }
    }
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

void configure_bme(){// Konfigurerer BME280
    Serial.println("Sjekker koblingen til BME280:");
    if(!bme.begin(0x76)){
        Serial.println("ERROR: BME280 er ikke koblet riktig!");
        while(1); // Fanger script i evig løkke
    } Serial.println("BME280 er koblet riktig!");
}



void setup(){
    Serial.begin(115200); connect();
    configure_mpu(); configure_bme();
}

void loop(){
    if(!client.connected()){
        client.subscribe(topicSubscribe); // Usikker om nødvendig
        reconnect();
    }

    // Sensormålinger:
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp); // Leser alle mpu sensorer
    float Ax = a.acceleration.x; // Akselerasjon x-retning
    float Ay = a.acceleration.y; // Akselerasjon y-retning
    float temperature = temp.temperature;

    float pressure = bme.readPressure()/100.0F; //Trykk i hPa
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);



    // Lager json-objektene som skal opplastes til Ubidots:
         // Variabel1: (Temperature)
    sprintf(topic1, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    sprintf(payload1, "%s", ""); // Tømmer payload
    sprintf(payload1, "{\"%s\":", VARIABLE_LABEL1); // Legger til Variabel1
    dtostrf(temperature, 1, 2, str_temperature);
    sprintf(payload1, "%s {\"value\": %s}}", payload1, str_temperature); // Legger til verdi til payload
        // Variabel2: (Altitude)
    sprintf(topic2, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    sprintf(payload2, "%s", ""); // Tømmer payload
    sprintf(payload2, "{\"%s\":", VARIABLE_LABEL2); // Legger til Variabel1
    dtostrf(altitude, 1, 2, str_altitude);
    sprintf(payload2, "%s {\"value\": %s}}", payload2, str_altitude); // Legger til verdi til payload

    //Publiserer:
    Serial.println("Publiserer alt til Ubidots.");
    client.publish(topic1, payload1); Serial.println("Nummer 1: Publisert");
    client.publish(topic2, payload2); Serial.println("Nummer 2: Publisert");
    
    client.loop();
    delay(1000);


}
