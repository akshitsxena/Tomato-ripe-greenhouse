#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>

// Wi-Fi credentials
const char* ssid = " Galaxy S22+";
const char* password = "";

#define TB_SERVER "eu.thingsboard.cloud"
#define TOKEN ""

constexpr uint16_t MAX_MESSAGE_SIZE = 128U;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

void connectToThingsBoard() {
  if (!tb.connected()) {
    Serial.println("Connecting to ThingsBoard server");
    
    if (!tb.connect(TB_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
    } else {
      Serial.println("Connected to ThingsBoard");
    }
  }
}

void sendDataToThingsBoard(float t, float h, int soil,int light) {
  String jsonData = "{\"temperature\":" + String(t) + ", \"humidity\":" + String(h) + ",\"soil moisture\":" + String(soil) +  ",\"light\":" + String(light) + "}";
  tb.sendTelemetryJson(jsonData.c_str());
  Serial.println("Data sent");
}


#define DHTPIN 32  
#define DHTTYPE DHT11
#define soil_moisture_pin 26

DHT dht(DHTPIN, DHTTYPE);


// Sensor pins
const int soilMoisturePin = 26; // GPIO34
const int ldrPin = 34;          // GPIO26

void setup() 
{
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  pinMode(26,INPUT);
  pinMode(34,INPUT);
  pinMode(32,INPUT);
  pinMode(15,OUTPUT);
  digitalWrite(15,LOW);
  dht.begin();

  // Connect to Wi-Fi
unsigned long startAttemptTime = millis();
const unsigned long timeout = 10000; // 10 seconds

WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
  delay(500);
  Serial.print(".");
}

if (WiFi.status() != WL_CONNECTED) {
  Serial.println("Failed to connect to Wi-Fi");
  // Handle the failure (e.g., retry, go to deep sleep, etc.)
} else {
  Serial.println("Connected to Wi-Fi");
}

connectToThingsBoard();
}

void loop() {
  // Read temperature and humidity from DHT11
  delay(2000);
  int light= (analogRead(34));
  if(light < 1000)
  {
    digitalWrite(15,HIGH);
  }
  else
  {
    digitalWrite(15,LOW);
  }
  delay(500);
  int soil= analogRead(soil_moisture_pin);

   float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.println(F("°F "));
  Serial.print(F(" Light: "));
  Serial.println(light);
  Serial.print(F(" Soil Moisture "));
  Serial.println(soil);

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
     if (!tb.connected()) {
    connectToThingsBoard();
  }

  sendDataToThingsBoard(t,h,soil,light);
  } else {
    Serial.println("Wi-Fi not connected");
  }
  tb.loop();
  // Wait before sending the next set of readings
  delay(300); // ThingSpeak allows updates every 15 seconds
}