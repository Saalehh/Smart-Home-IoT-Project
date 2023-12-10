#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 26     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11


// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

#define relayPin 5

DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  // Initialize device.
  dht.begin();
}

void loop() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    if(event.temperature > 27.00){
      Serial.print(F("Temperature is high: "));
      Serial.print(event.temperature);
      Serial.println(F("°C"));
      digitalWrite(relayPin, LOW);
    }else{
      digitalWrite(relayPin, HIGH);
    }
  }
}
