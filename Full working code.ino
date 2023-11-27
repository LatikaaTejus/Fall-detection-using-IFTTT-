#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// WiFi credentials
const char* ssid = "Joonika kim";
const char* password = "namjoon@123";

// IFTTT webhook details
const char* iftttEventName = "fall_detected";
const char* iftttKey = "drCGLsAwgYxov0-G5wooga";

// MPU6050 address
const int MPU6050_addr = 0x68;

// MPU6050 variables
int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;

// Threshold values for fall detection
const int fallThreshold = 10000;
const int uprightThreshold = 1500;
bool isFallDetected = false;

// WiFi client instance
WiFiClientSecure client;

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

  // Configure MPU6050
  Wire.begin();
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void loop() {
  // Read MPU6050 accelerometer data
  Wire.beginTransmission(MPU6050_addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_addr, 14, true); // request a total of 14 registers

  // Read accelerometer data
  accelerometer_x = Wire.read() << 8 | Wire.read();
  accelerometer_y = Wire.read() << 8 | Wire.read();
  accelerometer_z = Wire.read() << 8 | Wire.read();

  // Calculate total acceleration magnitude
  int16_t totalAcceleration = abs(accelerometer_x) + abs(accelerometer_y) + abs(accelerometer_z);

  // Check if fall is detected
  if (totalAcceleration < uprightThreshold) {
    isFallDetected = true;
  } else if (totalAcceleration > fallThreshold && isFallDetected) {
    // Fall detected, send alert to IFTTT
    sendIFTTTAlert();
    isFallDetected = false; // reset fall detection
  }

  // Print accelerometer values and fall detection status to Serial monitor
  Serial.print("Accel(x,y,z): ");
  Serial.print(accelerometer_x);
  Serial.print(", ");
  Serial.print(accelerometer_y);
  Serial.print(", ");
  Serial.print(accelerometer_z);
  Serial.print(" | Total Acceleration: ");
  Serial.print(totalAcceleration);
  Serial.print(" | Fall Detected: ");
  Serial.println(isFallDetected ? "Yes" : "No");

  delay(1000); // Adjust delay as per your requirements
}

void sendIFTTTAlert() {
  // Prepare IFTTT webhook URL
  String url = "https://maker.ifttt.com/trigger/fall_detected/json/with/key/drCGLsAwgYxov0-G5wooga";
  url += iftttEventName;
  url += "/with/key/";
  url += iftttKey;

  // Send HTTP POST request to IFTTT
  HTTPClient http;
  http.begin(client, url);
  int httpResponseCode = http.POST("");
  if (httpResponseCode > 0) {
    Serial.print("IFTTT request sent successfully. Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error sending IFTTT request. Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
