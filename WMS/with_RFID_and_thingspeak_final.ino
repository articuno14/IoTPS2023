#include <HX711_ADC.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>

#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

const int SS_PIN = 5;     // MFRC522 Slave Select pin
const int RST_PIN = 27;    // MFRC522 Reset pin

const char* ssid = "Node";       // Change to your Wi-Fi SSID
const char* password = "yhya1010"; // Change to your Wi-Fi password
const char* server = "api.thingspeak.com";
const String apiKey = "847REF1XVRRN0U18"; // Change to your ThingSpeak API Key

const int HX711_dout = 25; // MCU > HX711 dout pin
const int HX711_sck = 26; // MCU > HX711 sck pin
const int DHT_PIN = 16;   // DHT data pin

HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;

DHT dht(DHT_PIN, DHT11);
MFRC522 mfrc522(SS_PIN, RST_PIN);

byte chocolateTag[4] = {0xD3, 0x1B, 0x79, 0x42}; // Tag ID for chocolate
byte waterTag[4] = {0x73, 0x03, 0xFC, 0x43};     // Tag ID for water bottles  
byte chipsTag[4] = {0x8B, 0x65, 0x39, 0x13};     // Tag ID for chips

void setup() {
  Serial.begin(57600); // Setting serial monitor baud rate
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
  unsigned long stabilizingTime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingTime, _tare);
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU > HX711 wiring and pin designations");
    while (1);
  } else {
    LoadCell.setCalFactor(1.0); // User-set calibration value
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  calibrate(); // Start calibration procedure

  dht.begin();

  connectWiFi();
  
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");
}

void loop() {
  static boolean newDataReady = false;
  const int serialPrintInterval = 1000;
  static unsigned long previousMillis = 0;

  if (LoadCell.update()) {
    newDataReady = true;
  }

  if (newDataReady && (millis() - previousMillis >= serialPrintInterval)) {
    float weight = LoadCell.getData();
    Serial.print("Load cell output val: ");
    Serial.println(weight);

    float pieceWeight = 0.0; // Weight per piece

    // Determine piece weight based on the item scanned
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      byte scannedTagId[4];
      memcpy(scannedTagId, mfrc522.uid.uidByte, 4);

      if (compareTags(scannedTagId, chocolateTag)) {
        pieceWeight = 8.5; // Weight of one chocolate piece
        // Perform actions for chocolate item
        Serial.println("Details for Chocolate:");
        // Add actions or data processing specific to chocolate here
      } else if (compareTags(scannedTagId, waterTag)) {
        pieceWeight = 20.0; // Weight of one water bottle
        // Perform actions for water bottle item
        Serial.println("Details for Water Bottle:");
        // Add actions or data processing specific to water bottle here
      } else if (compareTags(scannedTagId, chipsTag)) {
        pieceWeight = 10.0; // Weight of one chip piece
        // Perform actions for chips item
        Serial.println("Details for Chips:");
        // Add actions or data processing specific to chips here
      } else {
        Serial.println("Unknown tag scanned");
      }

      float piecesLeft = weight / pieceWeight; // Calculate pieces based on weight

      float temperature = dht.readTemperature();
      float humidity = dht.readHumidity();

      Serial.print("Pieces Left: ");
      Serial.println(piecesLeft);
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C, Humidity: ");
      Serial.print(humidity);
      Serial.println("%");

      Serial.println("Details:");
      Serial.print("Total Weight: ");
      Serial.print(weight);
      Serial.println(" grams");
      Serial.print("Piece Weight: ");
      Serial.print(pieceWeight);
      Serial.println(" grams");

      sendDataToThingSpeak(weight, piecesLeft, temperature, humidity); // Send data to ThingSpeak

      mfrc522.PICC_HaltA();
    }

    newDataReady = false;
    previousMillis = millis(); // Update previousMillis for next interval
  }

  delay(1000); // Adjust delay according to your requirements
}

void connectWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void sendDataToThingSpeak(float weight, int pieces, float temperature, float humidity) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + String(server) + "/update?api_key=" + String(apiKey);
    url += "&field1=" + String(weight, 2); // 2 decimal places for weight
    url += "&field2=" + String(pieces);
    url += "&field3=" + String(temperature, 2); // 2 decimal places for temperature
    url += "&field4=" + String(humidity, 2); // 2 decimal places for humidity

    Serial.print("Sending data to ThingSpeak: ");
    Serial.println(url);

    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}

bool compareTags(byte* tag1, byte* tag2) {
  for (int i = 0; i < 4; i++) {
    if (tag1[i] != tag2[i]) {
      return false;
    }
  }
  return true;
}

void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");

  boolean _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      if (Serial.available() > 0) {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }
    }
    if (LoadCell.getTareStatus() == true) {
      Serial.println("Tare complete");
      _resume = true;
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    LoadCell.update();
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
  float newCalibrationValue = LoadCell.getNewCalibration(known_mass); //get the new calibration value

  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);
  Serial.println(", use this as calibration value (calFactor) in your project sketch.");
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");

  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;

      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}

void changeSavedCalFactor() {
  float oldCalibrationValue = LoadCell.getCalFactor();
  boolean _resume = false;
  Serial.println("***");
  Serial.print("Current value is: ");
  Serial.println(oldCalibrationValue);
  Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
  float newCalibrationValue;
  while (_resume == false) {
    if (Serial.available() > 0) {
      newCalibrationValue = Serial.parseFloat();
      if (newCalibrationValue != 0) {
        Serial.print("New calibration value is: ");
        Serial.println(newCalibrationValue);
        LoadCell.setCalFactor(newCalibrationValue);
        _resume = true;
      }
    }
  }
  _resume = false;
  Serial.print("Save this value to EEPROM adress ");
  Serial.print(calVal_eepromAdress);
  Serial.println("? y/n");
  while (_resume == false) {
    if (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 'y') {
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.begin(512);
#endif
        EEPROM.put(calVal_eepromAdress, newCalibrationValue);
#if defined(ESP8266)|| defined(ESP32)
        EEPROM.commit();
#endif
        EEPROM.get(calVal_eepromAdress, newCalibrationValue);
        Serial.print("Value ");
        Serial.print(newCalibrationValue);
        Serial.print(" saved to EEPROM address: ");
        Serial.println(calVal_eepromAdress);
        _resume = true;
      }
      else if (inByte == 'n') {
        Serial.println("Value not saved to EEPROM");
        _resume = true;
      }
    }
  }
  Serial.println("End change calibration value");
  Serial.println("***");
}
