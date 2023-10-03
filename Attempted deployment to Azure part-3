# Using Thonny IDE

import time
import network
from umqtt.simple import MQTTClient
import dht
import machine

# Set up Wi-Fi
WIFI_SSID = "Node"
WIFI_PASSWORD = "yhya1011"
wifi = network.WLAN(network.STA_IF)
wifi.active(True)
wifi.connect(WIFI_SSID, WIFI_PASSWORD)
while not wifi.isconnected():
    pass

# Azure IoT Central Configuration
SCOPE_ID = "0ne00A72ED4"
DEVICE_ID = "dhtesp"
DEVICE_KEY = "NzN2ZRRyYcHiDe5l9aBphtJqVe4JIE6m6kmhYd421IA="

# Initialize DHT11 Sensor
dht_pin = machine.Pin(34)  # Pin D2 on ESP32
dht_sensor = dht.DHT11(dht_pin)

# MQTT Configuration
MQTT_BROKER = "127.0.0.1"
MQTT_PORT = 8883
MQTT_CLIENT_ID = DEVICE_ID
MQTT_TOPIC = f"devices/{DEVICE_ID}/messages/events/"
MQTT_USERNAME = f"{SCOPE_ID}/{DEVICE_ID}/?api-version=2018-06-30"
MQTT_PASSWORD = DEVICE_KEY

# Initialize MQTT Client
mqtt = MQTTClient(client_id=MQTT_CLIENT_ID, server=MQTT_BROKER, port=MQTT_PORT,
                   user=MQTT_USERNAME, password=MQTT_PASSWORD, ssl=True)

# Connect to MQTT Broker
mqtt.connect()

# Main Loop
while True:
    try:
        # Read temperature and humidity from DHT11
        dht_sensor.measure()
        temperature = dht_sensor.temperature()
        humidity = dht_sensor.humidity()

        # Create JSON payload
        payload = '{{"Temperature": {}, "Humidity": {}}}'.format(temperature, humidity)

        # Publish telemetry data to Azure IoT Central
        mqtt.publish(MQTT_TOPIC, payload)
        print("Telemetry sent:", payload)

        # Delay for 10 seconds before sending the next telemetry
        time.sleep(10)

    except Exception as e:
        print("Error:", str(e))
