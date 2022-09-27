//Wetterstation EGD 1.1             IP: XXX.XXX.XXX.XXX
//Speicherung bei ThingSpeak - Saving on Thingspeak

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

char ssid[] = "XXXXX";        //Deine SSID - your network SSID (name)
char pass[] = "XXXXX";        //Dein Netzwerk-Passwort - your network password
int keyIndex = 0;             //Dein Netzwerk Schlüssel-Index (nur für WEP benötigt) - your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = "XXXX";              //ThingSpeak-Kanal-Nummer - ThingSpeak Channel number
const char * myWriteAPIKey = "XXXX";                 //ThingSpeak Schreiben-API - Thingspeak write-API

int temp;
int humi;
int pres;

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
  ThingSpeak.begin(client);  //Initialisiere Thingspeak - Initialize ThingSpeak
}

void loop() {
  temp=bme.readTemperature();
  humi=bme.readHumidity();
  pres = bme.readPressure()/100;
  send_data();
  delay(20000);
}

void send_data() {
  //Verbinden/Wiederverbindenzum Internet - Connect/reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);  //Verbinde zum WPA/WPA2-Netzwerk - Connect to WPA/WPA2 network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  //Fülle die Felder mit Werten - set the fields with the values
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humi);
  ThingSpeak.setField(3, pres);

  //Schreibe auf Thingspeak - write to the ThingSpeak channel
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
  else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  delay(20000);
}
