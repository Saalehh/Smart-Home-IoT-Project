#include <Keypad.h>
#include <Servo.h>

const int servoPin = 11;  // Pin to which the servo (motor) is connected
Servo myServo;  // Use the Servo class from the ESP32 Servo library

const int ROW_NUM    = 4; // four rows
const int COLUMN_NUM = 4; // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};    // connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};   // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

String enteredPassword = "";
String correctPassword = "1234";  // Change this to your desired password

int wrongAttempts = 0;
const int maxWrongAttempts = 3;  // Set the maximum allowed consecutive wrong attempts

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);
  resetServo();
}

void loop() {
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') {
      if (enteredPassword == correctPassword) {
        wrongAttempts = 0;
        Serial.print("open");
        unlockDoor();
      } else {
        wrongAttempts++;
        if (wrongAttempts >= maxWrongAttempts) {
          Serial.print("intrusion");
        }
        resetPassword();      
      }
    } else {
      enteredPassword += key;
    }
  }
}

void unlockDoor() {
  myServo.write(180);  // Rotate the servo to unlock position
  delay(5000);        // Keep the door unlocked for 5 seconds
  resetPassword();
  resetServo();
  wrongAttempts = 0;  // Reset wrong attempts after a successful unlock
}

void resetPassword() {
  enteredPassword = "";
}

void resetServo() {
  myServo.write(0);  // Rotate the servo to the locked position
}

