//Wetterstation EGD 1.2             IP: XXX.XXX.XXX.XXX
//Speicherung bei ThingSpeak - Saving on ThingSpeak
//Höhenkorrektur - Altitude correction
//OTA - OTA

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

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

float Hoehenkorrektur = 230/8;

void setup() {
  Serial.begin(115200);
  unsigned status;
  status = bme.begin(0x76);
  if (!status) {
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
  AsyncElegantOTA.begin(&server);    //Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  AsyncElegantOTA.loop();           //OTA
  temp = bme.readTemperature();
  humi = bme.readHumidity();
  pres = bme.readPressure() / 100.0F + Hoehenkorrektur;
  send_data();
  delay(20000);
}

void send_data() {
  if (WiFi.status() != WL_CONNECTED) {
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
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  delay(20000);
}
