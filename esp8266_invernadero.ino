#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h> //https://github.com/tzapu/WiFiManage
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>

HTTPClient https;
WiFiClientSecure client;
WiFiManager wifiManager;

bool shouldSaveConfig = false;
String serie = "default";

//Arduino JSON


//Configuracion del dht11
#define DHTPIN D6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Constantes del sistema
const String host = "kassen.herokuapp.com";
const String url = "/api/insert";
int periodo = 900000;
int httpsPort = 443;

//Variables
float temp;
float hum;
int lum;
unsigned long timeNow = 0;

void setup()
{
  Serial.begin(115200);
  Serial.printf("Empezando programa...");
  pinMode(D0, WAKEUP_PULLUP);
  pinMode(D2, OUTPUT);
  SPIFFS.begin();
  startWifi();
  if (shouldSaveConfig){
    writeConfig(); }
  readConfig();
  delay(1000);
  dht.begin();
  client.setInsecure();
}

void loop()
{
  Serial.println("--------------");
  timeNow = millis();
  https.begin(client, host, httpsPort, url, true);
  readSensor();
  sendPost();
  Serial.println("Sleep por 15 minutos");
  ESP.deepSleep(900000000);
  while (millis() < timeNow + 5000)
  {
    yield();
  }
}

void readSensor()
{
  digitalWrite(D4, HIGH);
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  int sensorValue = analogRead(A0); // read the input on analog pin 0
  lum = map(sensorValue, 0, 1024, 0, 100);
  digitalWrite(D4, LOW);
}

void sendPost()
{
  StaticJsonDocument<200> data;
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
}

void saveConfigCallback()
{
  shouldSaveConfig = true;
}

void readConfig()
{
  File configFile = SPIFFS.open("/config.json", "r");
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  serie = (const char*)doc["serie"];
  Serial.print("Serie es igual a ");
  Serial.println(serie);
  configFile.close();
}

void writeConfig()
{   Serial.println("saving config");
    StaticJsonDocument<200> doc;
    doc["serie"] = serie;
    File configFile = SPIFFS.open("/config.json", "w");
    serializeJson(doc, configFile); 
    configFile.close();
}

void startWifi()
{
  WiFiManagerParameter serie_id("Serie", "serie", "", 10);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&serie_id);
  wifiManager.setConfigPortalTimeout(60);
  if(!wifiManager.autoConnect("Invernadero", "kassen123")){
    delay(10000);
    Serial.println("Sleep por 15 minutos");
    ESP.deepSleep(900000000);  
  }
  Serial.println("connected..(✿ ♡‿♡) yeey UwU >.<");
  serie = serie_id.getValue();
}
