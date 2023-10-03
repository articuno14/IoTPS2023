//C++ code in Ardruino IDE:

#include <DHT.h> //Including the library DHT as discussed earlier
DHT dht(26, DHT11); //sensor type is DHT 11 and the pin no 26 is used
void setup() {
  // put your setup code here, to run once:
  dht.begin();
  delay (2000);
  Serial.begin(115200); //Including the baud rate
}
void loop() {
  // put your main code here, to run repeatedly:
  float temp = dht.readTemperature();
  float humidity =dht.readHumidity();
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" C ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" % ");
  delay(2000); //delay of 2000ms is given
}

