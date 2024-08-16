#include <Arduino.h>
#include <WiFi.h>                // WiFi library for ESP32
#include <ESPAsyncWebServer.h>   // Web server library
#include "DHTesp.h"              // DHT sensor library
#include "config.h"  // Include the config file with credentials

DHTesp dht;
AsyncWebServer server(80);

// Define pins for the sensors
const int lightSensorPin = 36;  // GPIO 36 (VP) for analog input (Light Sensor)
const int soundSensorPin = 39;  // GPIO 39 (VN) for analog input (Sound Sensor)

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Sensor Data</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        body { margin: 0; padding: 0; font-family: Arial, sans-serif; }
    </style>
    <meta http-equiv="refresh" content="10">
</head>
<body class="bg-gray-100">
    <div class="flex flex-col items-center justify-center min-h-screen bg-gradient-to-r from-blue-400 to-purple-500 text-white">
        <header class="mb-10">
            <h1 class="text-5xl font-bold drop-shadow-lg shadow-black">
                🌡️ Temperature, Humidity, Sound & Light 🌦️🔊💡
            </h1>
        </header>
        <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-6 w-full max-w-screen-lg px-4">
            <div class="bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105">
                <h2 class="text-2xl font-semibold">Temperature 🌡️</h2>
                <p class="text-3xl mt-4 font-bold text-blue-600">%TEMP% °C</p>
            </div>
            <div class="bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105">
                <h2 class="text-2xl font-semibold">Humidity 💧</h2>
                <p class="text-3xl mt-4 font-bold text-green-600">%HUMIDITY% %</p>
            </div>
            <div class="bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105">
                <h2 class="text-2xl font-semibold">Sound Level 🔊</h2>
                <p class="text-3xl mt-4 font-bold text-red-600">%SOUND%</p>
            </div>
            <div class="bg-white text-black rounded-xl shadow-lg p-6 transition-transform transform hover:scale-105">
                <h2 class="text-2xl font-semibold">Light Level 💡</h2>
                <p class="text-3xl mt-4 font-bold text-yellow-600">%LIGHT%</p>
            </div>
        </div>
    </div>
</body>
</html>
)rawliteral";

String processor(const String& var) {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    int soundLevel = analogRead(soundSensorPin);
    int lightLevel = analogRead(lightSensorPin);

    String soundDisplay = String(soundLevel);

    String html = FPSTR(index_html);
    html.replace("%TEMP%", String(temperature, 1));
    html.replace("%HUMIDITY%", String(humidity, 1));
    html.replace("%SOUND%", soundDisplay);
    html.replace("%LIGHT%", String(lightLevel));

    return html;
}

String prometheusMetrics() {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    int soundLevel = analogRead(soundSensorPin);
    int lightLevel = analogRead(lightSensorPin);

    String metrics = "";
    metrics += "# HELP temperature_celsius Current temperature in Celsius.\n";
    metrics += "# TYPE temperature_celsius gauge\n";
    metrics += "temperature_celsius " + String(temperature, 1) + "\n";
    
    metrics += "# HELP humidity_percent Current humidity in percentage.\n";
    metrics += "# TYPE humidity_percent gauge\n";
    metrics += "humidity_percent " + String(humidity, 1) + "\n";
    
    metrics += "# HELP sound_level Raw sound level from the sensor.\n";
    metrics += "# TYPE sound_level gauge\n";
    metrics += "sound_level " + String(soundLevel) + "\n";
    
    metrics += "# HELP light_level Raw light level from the sensor.\n";
    metrics += "# TYPE light_level gauge\n";
    metrics += "light_level " + String(lightLevel) + "\n";

    return metrics;
}

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
        String html = processor("");
        request->send(200, "text/html", html);
    });

    server.on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request) {
        String metrics = prometheusMetrics();
        request->send(200, "text/plain", metrics);
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
