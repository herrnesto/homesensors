#include <Arduino.h>
#include <WiFi.h>                // WiFi library for ESP32
#include <ESPAsyncWebServer.h>   // Web server library
#include "DHTesp.h"              // DHT sensor library
#include "config.h"  // Include the config file with credentials

DHTesp dht;
AsyncWebServer server(80);

// Define pins for the sensors
const int lightSensorPin = 36;  // GPIO 36 (VP) for analog input (Sound Sensor)
const int soundSensorPin = 39;  // GPIO 39 (VN) for analog input (Light Sensor)

String processor(const String& var) {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    int soundLevel = analogRead(soundSensorPin);
    int lightLevel = analogRead(lightSensorPin);

    String data = "<div class='flex flex-col items-center justify-center min-h-screen bg-gradient-to-r from-blue-400 to-purple-500 text-white'>";
    data += "<header class='mb-10'>";
    data += "<h1 class='text-5xl font-bold drop-shadow-lg shadow-black'>🌡️ Temperature, Humidity, Sound & Light 🌦️🔊💡</h1>";
    data += "</header>";
    
    data += "<div class='grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-6 w-full max-w-screen-lg px-4'>";
    
    data += "<div class='bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105'>";
    data += "<h2 class='text-2xl font-semibold'>Temperature 🌡️</h2>";
    data += "<p class='text-3xl mt-4 font-bold text-blue-600'>" + String(temperature, 1) + " °C</p>";
    data += "</div>";
    
    data += "<div class='bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105'>";
    data += "<h2 class='text-2xl font-semibold'>Humidity 💧</h2>";
    data += "<p class='text-3xl mt-4 font-bold text-green-600'>" + String(humidity, 1) + " %</p>";
    data += "</div>";
    
    data += "<div class='bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105'>";
    data += "<h2 class='text-2xl font-semibold'>Sound Level 🔊</h2>";
    data += "<p class='text-3xl mt-4 font-bold text-red-600'>" + String(soundLevel) + "</p>";
    data += "</div>";
    
    data += "<div class='bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105'>";
    data += "<h2 class='text-2xl font-semibold'>Light Level 💡</h2>";
    data += "<p class='text-3xl mt-4 font-bold text-yellow-600'>" + String(lightLevel) + "</p>";
    data += "</div>";

    data += "</div>";
    data += "</div>";
    return data;
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Sensor Data</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <meta http-equiv="refresh" content="10">
</head>
<body class="bg-gray-100">
    %DATA%
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(9600);
    dht.setup(25, DHTesp::DHT22);

    // Initialize the sensor pins
    pinMode(soundSensorPin, INPUT);
    pinMode(lightSensorPin, INPUT);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = String(index_html);
        html.replace("%DATA%", processor(""));
        request->send(200, "text/html", html);
    });

    server.begin();
}

void loop() {
    delay(dht.getMinimumSamplingPeriod());

    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    int soundLevel = analogRead(soundSensorPin);
    int lightLevel = analogRead(lightSensorPin);

    // Output to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity, 1);
    Serial.print(" %, Sound Level: ");
    Serial.print(soundLevel);
    Serial.print(", Light Level: ");
    Serial.println(lightLevel);

    delay(2000);
}