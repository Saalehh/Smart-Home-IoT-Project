#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define RXp2 16
#define TXp2 17
#define PIRPin 4
#define LightSensorPin 27
#define AnalogThreshold 500
#define LED_House 25
#define DHTPIN 26     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11     // DHT 11
#define relayPin 5

bool isCorrectPasswordEntered = false;
bool isOwner = false;

DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  pinMode(PIRPin, INPUT_PULLUP);
  pinMode(LED_House, OUTPUT);
  pinMode(relayPin, OUTPUT);
  // Initialize device.
  dht.begin();
  digitalWrite(relayPin, HIGH);
}

void loop() {
  Serial.println("Message Received:");
  String receivedMessage = Serial2.readString();  // Read the string "1" / "0" according to the given password
  // Trim any leading or trailing whitespace from the received message
  receivedMessage.trim();
  Serial.println(receivedMessage);
  if (!isCorrectPasswordEntered) {
    // Check if the correct password is entered
    if (receivedMessage.equals("1")) {
      Serial.println("Correct Password Entered");
      isCorrectPasswordEntered = true;
      isOwner = true;  // Assume the user is the owner
    }

  } else {
    // Once the correct password is entered, and user is identified as the owner
    if (isOwner) {
      Serial.println("Owner of the house");

      if (digitalRead(PIRPin) == HIGH) {
        Serial.println("Motion Detected!!");

          // Perform actions related to motion detection for the owner
          // For example, open the door using the servo
          if (analogRead(LightSensorPin) > AnalogThreshold) {

            digitalWrite(LED_House, HIGH);
            while(true){
                // Get temperature event and print its value.
                sensors_event_t event;
                dht.temperature().getEvent(&event);
                if (isnan(event.temperature)) {
                  Serial.println(F("Error reading temperature!"));
                }
                  if(event.temperature > 27.0){
                    // we are making the opposite due to some problem in the relay module understanding!!
                    digitalWrite(relayPin, LOW);
                  }else{
                    digitalWrite(relayPin, HIGH);
                  }
            }
          }else{
                while(true){
                  // Get temperature event and print its value.
                  sensors_event_t event;
                  dht.temperature().getEvent(&event);
                  if (isnan(event.temperature)) {
                    Serial.println(F("Error reading temperature!"));
                  }
                  
                  if(event.temperature > 27.0){
                    // we are making the opposite due to some problem in the relay module understanding!!
                    digitalWrite(relayPin, LOW);
                  }else{
                    digitalWrite(relayPin, HIGH);
                  }
            }
          }

  
      }
    } else {
      Serial.println("Incorrect Password. Try Again.");
    }
  }
}
