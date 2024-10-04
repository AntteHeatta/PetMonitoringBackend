#include <Arduino.h>
// #include "sensor_manager/sensor_manager.h"

// SensorManager sensorManager;

// void setup()
// {
//   Serial.begin(9600);
//   sensorManager.initialize();
//   delay(2000);
// }

// void loop()
// {
//   sensorManager.readSensors();

//   float humidity = sensorManager.getHumidity();
//   float temperature = sensorManager.getTemperature();
//   float luminosity = sensorManager.getLuminosity();
//   Serial.print("Humidity: ");
//   Serial.println(humidity);
//   Serial.print("Temperature: ");
//   Serial.println(temperature);
//   Serial.print("Luminosity: ");
//   Serial.println(luminosity);

//   delay(5000);
// }
// #include <WiFiEsp.h>
// #include <WiFiEspClient.h>
// #include <WiFiEspServer.h>
// #include <WiFiEspUdp.h>

// // Emulated serial port for ESP8266
// #include <SoftwareSerial.h>
// SoftwareSerial espSerial(2, 3); // RX, TX (connect to ESP8266 TX, RX via voltage divider)

// // Initialize the WiFi library
// char ssid[] = "TP-Link_B7E9"; // your network SSID (name)
// char pass[] = "66714514";     // your network password
// int status = WL_IDLE_STATUS;

// void setup()
// {
//   // Initialize serial for debugging
//   Serial.begin(9600);
//   // Initialize software serial for ESP8266
//   espSerial.begin(115200); // ESP8266 default baud rate
//   WiFi.init(&espSerial);

//   // Check for the presence of the shield
//   if (WiFi.status() == WL_NO_SHIELD)
//   {
//     Serial.println("WiFi shield not present");
//     while (true)
//       ; // don't continue
//   }

//   // Attempt to connect to WiFi network
//   while (status != WL_CONNECTED)
//   {
//     Serial.print("Attempting to connect to WPA SSID: ");
//     Serial.println(ssid);
//     status = WiFi.begin(ssid, pass);
//     delay(10000);
//   }

//   Serial.println("Connected to wifi");
// }

// void loop()
// {
//   // Your main code
// }
#include <WiFiEsp.h>
#include <SoftwareSerial.h>

// Emulated serial port for ESP8266
SoftwareSerial espSerial(2, 3); // RX, TX (connect to ESP8266 TX, RX via voltage divider)

// WiFi credentials
char ssid[] = "TP-Link_B7E9"; // Your network SSID (name)
char pass[] = "66714514";     // Your network password

void setup()
{
  // Start serial for debugging
  Serial.begin(9600);

  // Start the software serial for ESP8266 communication
  espSerial.begin(115200); // Ensure the baud rate matches your ESP8266

  // Initialize the WiFiEsp library with the SoftwareSerial instance
  WiFi.init(&espSerial);

  // Check for the presence of the ESP8266 module
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi module not present.");
    while (true)
      ; // Don't continue
  }

  // Attempt to connect to the WiFi network
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);

  // Keep trying until connected
  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, pass); // Connect to network
    Serial.println("Connecting to WiFi...");
    delay(10000); // Wait 10 seconds before retrying
  }

  // If connected, print success and IP address
  Serial.println("Connected to WiFi.");
  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Print the signal strength (RSSI)
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void loop()
{
  // Check the connection status periodically
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Still connected to WiFi.");
  }
  else
  {
    Serial.println("Lost connection to WiFi, trying to reconnect...");
    WiFi.begin(ssid, pass);
  }

  delay(5000); // Wait 5 seconds before checking again
}
