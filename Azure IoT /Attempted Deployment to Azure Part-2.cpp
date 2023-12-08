//Using Arduino IDE and without using AzureIoTHub and AzureIoTUtility so using PubSubClient


#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Wi-Fi Configuration
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Azure IoT Central Configuration
const char* MQTT_BROKER = "your-broker-address"; // Replace with your MQTT broker address
const int MQTT_PORT = 1883; // Default MQTT port for non-SSL
const char* DEVICE_ID = "dhtesp";
const char* SCOPE_ID = "YourScopeID"; // Replace with your Azure IoT Central scope id
const char* PRIMARY_KEY = "YourPrimaryKey"; // Replace with your Azure IoT Central primary key
const char* MQTT_TOPIC = "devices/dhtesp/messages/events/";

// Initialize DHT11 Sensor
#define DHT_PIN 34 // Pin D2 on ESP32
DHT dht(DHT_PIN, DHT11);

// Initialize MQTT Client
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set MQTT Server and Port
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);

  // Connect to MQTT Broker
  while (!mqtt.connected()) {
    Serial.println("Connecting to MQTT Broker...");
    if (mqtt.connect(DEVICE_ID, SCOPE_ID, PRIMARY_KEY)) {
      Serial.println("Connected to MQTT Broker");
    } else {
      Serial.println("MQTT connection failed. Retrying...");
      delay(5000);
    }
  }
}

void loop() {
  // Read temperature and humidity from DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if readings are valid
  if (!isnan(temperature) && !isnan(humidity)) {
    // Create JSON payload
    String payload = "{\"Temperature\": " + String(temperature) + ", \"Humidity\": " + String(humidity) + "}";

    // Publish telemetry data to Azure IoT Central
    if (mqtt.publish(MQTT_TOPIC, payload.c_str())) {
      Serial.println("Telemetry sent: " + payload);
    } else {
      Serial.println("Error sending telemetry");
    }
  } else {
    Serial.println("Failed to read data from DHT11 sensor");
  }

  // Maintain MQTT connection
  if (!mqtt.connected()) {
    reconnect();
  }

  // Delay for 10 seconds before sending the next telemetry
  delay(10000);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(DEVICE_ID, SCOPE_ID, PRIMARY_KEY)) {
      Serial.println("Connected to MQTT Broker");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}
