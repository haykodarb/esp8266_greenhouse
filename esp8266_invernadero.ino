#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
const uint8_t fingerprint[20] = {0xd2, 0xb8, 0xf1, 0x80, 0xac, 0x29, 0x63, 0xf8, 0x22, 0xaa, 0x9e, 0x99, 0x4d, 0x73, 0x7b, 0x59, 0x3b, 0x92, 0xb4, 0xe8};
HTTPClient https;
WiFiClientSecure client;
ESP8266WiFiMulti WiFiMulti;

//Arduino JSON
#include <ArduinoJson.h>
StaticJsonDocument<200> data;

//Configuracion del dht11 
#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTPIN D3 
#define DHTTYPE    DHT11 
DHT dht(DHTPIN, DHTTYPE);

// Datos web
const int serie = 1;
const String host = "kassen.now.sh";
const String url = "/insert";

const char* ssid = "aram";
const char* password = "";


//Variables
float temp;
float hum;
int lum;
int periodo = 300000;
int httpsPort = 443;
unsigned long timeNow = 0;



void setup(){
Serial.begin(115200);
Serial.printf("Empezando programa...");
delay(1000);
WiFi.mode(WIFI_STA);
WiFi.setSleepMode(WIFI_NONE_SLEEP);
WiFiMulti.addAP(ssid, password);
while (WiFiMulti.run() != WL_CONNECTED) {
  yield();
  }
Serial.println();
dht.begin(); 
client.setInsecure();
}


void loop() {
Serial.println("--------------");
timeNow = millis(); 
https.begin(client, host, 443, url, true);
readSensor();
sendPost();
while( millis() < timeNow + periodo){
  yield();  }
}

void readSensor() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  int sensorValue = analogRead(A0);   // read the input on analog pin 0
  lum = map(sensorValue, 0, 1024, 0, 100);
}

void sendPost(){ 
  String jsonOutput;
  data["temp"] = temp;
  data["serie"] = serie;
  data["hum"] = hum;
  data["lum"] = lum;
  serializeJson(data, jsonOutput);
  https.addHeader("Content-Type", "application/json");
  Serial.print("Sent post with string: ");
  Serial.println(jsonOutput);
  int resCode = https.POST(jsonOutput);
  Serial.print("Recieved response code: ");
  Serial.println(resCode);
  Serial.print("Recieved response: ");
  Serial.println(https.getString());
  if (resCode == -1) {
  ESP.reset();
  }
 }
