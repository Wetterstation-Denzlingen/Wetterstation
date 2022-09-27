//Wetterstation EGD 2.1             IP: XXX.XXX.XXX.XXX
//Speicherung bei ThingSpeak - Saving on ThingSpeak
//Höhenkorrektur - Altitude correction
//OTA - OTA
// Feinstaub* - Particulate matter
//Kontroll-LEDs - Controlling LED



/*
    *Zu Feinstaub:
    Feinstaub (PM2.5)
    Diese mikroskopischen Partikel mit einer Größe von bis zu 2,5 Mikrometer schweben in der Luft, die wir atmen. Zu dieser Kategorie gehören Rauch, Bakterien und Allergene.
    Feinstaub (PM10)
    Diese größeren mikroskopischen Partikel mit einer Größe von bis zu 10 Mikrometer schweben in der Luft, die wir atmen. Zu dieser Kategorie zählen Staub, Schimmel und Pollen.
    
    *On particulate matter:
    Particulate matter (PM2.5)
    These microscopic particles, up to 2.5 micrometres in size, float in the air we breathe. This category includes smoke, bacteria and allergens.
    Particulate matter (PM10)
    These larger microscopic particles, up to 10 microns in size, are suspended in the air we breathe. This category includes dust, mould and pollen.
*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "SdsDustSensor.h"

int rxPin = D3;
int txPin = D4;
SdsDustSensor sds(rxPin, txPin);

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

Adafruit_BME280 bme; // I2C


#include <ESP8266WiFi.h>
#include "ThingSpeak.h"

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer server(80);

char ssid[] = "XXXX";
char pass[] = "XXXX";
int keyIndex = 0;
WiFiClient  client;

unsigned long myChannelNumber = XXXX;
const char * myWriteAPIKey = "XXXX";

int temp;
int humi;
int pres;
int pm10;
int pm25;


//Check-LEDs
int wifi = D5;
int upload = D6;
int BME = D7;
int feinstaub = D8;

float Hoehenkorrektur = 230 / 8;

void setup() {
  pinMode(wifi, OUTPUT);
  pinMode(upload, OUTPUT);
  pinMode(BME, OUTPUT);
  pinMode(feinstaub, OUTPUT);
  Serial.begin(115200);
  unsigned status;
  status = bme.begin(0x76);
  if (!status) {
    analogWrite(BME, HIGH);
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("HTTP server started");
  sds.begin();
  if (sds.setActiveReportingMode().toString() == "Mode: undefined") {
    analogWrite(feinstaub, HIGH);
  }
  if (! bme.begin()) {
    analogWrite(BME, HIGH);
  }
}

void loop() {
  analogWrite(BME, LOW);
  analogWrite(wifi, LOW);
  analogWrite(upload, LOW);
  analogWrite(feinstaub, LOW);
  AsyncElegantOTA.loop();
  temp = bme.readTemperature();
  humi = bme.readHumidity();
  pres = bme.readPressure() / 100.0F + Hoehenkorrektur;
  PmResult pm = sds.readPm();
  pm25 = pm.pm25;
  pm10 = pm.pm10;
  send_data();
  delay(20000);
}

void control() {
  if (WiFi.status() != WL_CONNECTED) {
    analogWrite(wifi, HIGH);
  }
  if (sds.setActiveReportingMode().toString() == "Mode: undefined") {
    analogWrite(feinstaub, HIGH);
  }
  if (! bme.begin()) {
    analogWrite(BME, HIGH);
  }
}


void send_data() {
  if (WiFi.status() != WL_CONNECTED) {
    analogWrite(wifi, HIGH);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humi);
  ThingSpeak.setField(3, pres);
  ThingSpeak.setField(4, pm10);
  ThingSpeak.setField(5, pm25);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
    if (WiFi.status() != WL_CONNECTED) {
      analogWrite(upload, HIGH);
    }
    delay(20000);
  }
}
