// Using Arduino IDE // <AzureIoTHub.h> and <AzureIoTUtility.h> are not available libraries

#include <WiFi.h>
#include <AzureIoTHub.h>
#include <AzureIoTUtility.h>
#include "DHT.h"

// Define your Wi-Fi credentials
const char* ssid = "Node";
const char* password = "yhya1010";

// Azure IoT Central settings
const char* iotCentralScopeId = "0ne00A72ED4";
const char* deviceId = "dhtesp";
const char* deviceKey = "NzN2ZRRyYcHiDe5l9aBphtJqVe4JIE6m6kmhYd421IA=";

// DHT sensor settings
#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Define the message buffer size
#define MESSAGE_MAX_LEN 256

// Azure IoT Hub client
IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize DHT sensor
  dht.begin();

  // Initialize Azure IoT Central client
  iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(deviceKey, MQTT_Protocol);
  if (iotHubClientHandle == NULL) {
    Serial.println("Error creating IoT Hub client.");
    while (1);
  }
  IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "ESP32_DHT11");

  // Set up a callback for receiving cloud-to-device messages (optional)
  IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
}

void loop() {
  // Read sensor data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Create a JSON message with telemetry data
  char telemetryMessage[MESSAGE_MAX_LEN];
  snprintf(telemetryMessage, MESSAGE_MAX_LEN, "{\"deviceId\":\"%s\", \"Temperature\":%.2f, \"Humidity\":%.2f}", deviceId, temperature, humidity);

  // Send telemetry message to Azure IoT Central
  IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)telemetryMessage, strlen(telemetryMessage));
  if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendConfirmationCallback, NULL) != IOTHUB_CLIENT_OK) {
    Serial.println("Failed to send telemetry message to Azure IoT Central.");
  } else {
    Serial.println("Telemetry message sent to Azure IoT Central.");
  }

  // Wait for a while before sending the next message
  delay(10000);  // Send telemetry every 10 seconds

  // Do Azure IoT Hub background work
  IoTHubClient_LL_DoWork(iotHubClientHandle);
}

// Callback function for receiving cloud-to-device messages (optional)
void receiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback) {
  const char* messageText;
  if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&messageText) == IOTHUB_MESSAGE_OK) {
    Serial.println("Received message from Azure IoT Central:");
    Serial.println(messageText);
  }
  IoTHubMessage_Destroy(message);
}

// Callback function for send confirmation
void sendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback) {
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
    Serial.println("Telemetry message sent successfully to Azure IoT Central.");
  } else {
    Serial.println("Failed to send telemetry message to Azure IoT Central.");
  }
}
