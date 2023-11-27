#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// WiFi credentials
const char* ssid = "Joonika kim";
const char* password = "namjoon@123";

// ThingSpeak credentials
const char* server = "api.thingspeak.com";
const char* apiKey = "PSUC2UUO49ZJT8NC";

// Pulse Sensor variables
const int pulsePin = A0;  // Pulse Sensor output pin
int pulseValue = 0;       // Variable to store pulse readings

// WiFi client instance
WiFiClient client;

void setup() {
  // Initialize Serial monitor
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  // Initialize ThingSpeak
  ThingSpeak.begin(client);
  
  // Configure pulse sensor pin
  pinMode(pulsePin, INPUT);
}

void loop() {
  // Read pulse sensor value
  pulseValue = analogRead(pulsePin);
  
  // Print pulse value to Serial monitor
  Serial.print("Pulse Value: ");
  Serial.println(pulseValue);
  
  // Update ThingSpeak channel with pulse value
  ThingSpeak.setField(1, pulseValue);
  int response = ThingSpeak.writeFields(1, apiKey);
  
  if (response == 200) {
    Serial.println("Data sent to ThingSpeak successfully");
  } else {
    Serial.print("Error sending data to ThingSpeak. HTTP error code: ");
    Serial.println(response);
  }
  
  delay(10000);  // Wait for 10 seconds before sending the next data
}
