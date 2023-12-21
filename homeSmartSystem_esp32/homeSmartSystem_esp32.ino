#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <AsyncTCP.h>
#include "ESP32_MailClient.h"
#include <DHT.h>

// VVC of the relay module with the VINpin
#define RXp2 16
#define TXp2 17

#define PIRPin 4 // motion detection senor
#define relayPin 27 // allows the voltage to pass to the fan
#define LED_House 25
#define DHTPIN 26 // tempreture and humedity sensor
#define LightSensorPin 33

#define LightThreshold 3000
#define FanThreshold  25.0
#define DHTTYPE DHT11


DHT dht(DHTPIN, DHTTYPE);

// Setting wifi cridentials
const char *ssid = "*****";
const char *password = "*****";

// To send Emails using Gmail on port 465 (SSL)
#define emailSenderAccount "*****@gmail.com"
#define emailSenderPassword "**** **** **** ****"
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "[ALERT] ESP Intrusion!"


// Default Recipient Email Address
String receiverEmail = "*****@gmail.com";
String enableEmailChecked = "checked";

const char *PARAM_INPUT_1 = "email_input";
const char *PARAM_INPUT_2 = "enable_email_input";

// The Email Sending data object contains config and data to send
SMTPData smtpData;

AsyncWebServer server(80);

bool isOwnerEntered = false;
bool isFanOn = false;
bool isLightsOn = false;
bool controlLightsManually = false;
bool controlFanManually = false;
bool isMotion = false;
String receivedMessage = "";


String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    return String(t);
  }
}

String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    return String(h);
  }
}

bool readMotion() {
  return digitalRead(PIRPin);
}

void isOwner() {
  for(int i=0; i <= 3; i++) {
      receivedMessage = Serial2.readString();
      receivedMessage.trim();
      if (receivedMessage.equals("open")) {
        isOwnerEntered = true;
        break;
      }
      delay(500);
  } 
}

bool intrusion() {
  for(int i=0; i<=3; i++) {
    receivedMessage = Serial2.readString();
    receivedMessage.trim();
    if (receivedMessage.equals("intrusion")) {
      return true;
    } else {
      return false;
    }
  }
}

void turnLightOn() {
  digitalWrite(LED_House, HIGH);
  isLightsOn = true;
}

void turnLightOff() {
  digitalWrite(LED_House, LOW);
  isLightsOn = false;
}

void turnFanOn() {
  digitalWrite(relayPin, LOW);
  isFanOn = true;
}

void turnFanOff() {
  digitalWrite(relayPin, HIGH);
  isFanOn = false;
}

bool checkOwnerEntry() {
  isOwner();
  if (isOwnerEntered) {
    Serial.println("Owner entered the house!");
    controlLightsManually = false;
    controlFanManually = false;
    return true;
  } else {
    Serial.println("There is no one in the house.");
    isMotion = false;
    return false;
  }
}

bool checkMotion() {
  if (readMotion() == HIGH) {
    Serial.println("Motion detected!");
    return true;
  } else {
    return false;
  }
}

void checkLight() {
  if (controlLightsManually == false) {
    if (analogRead(LightSensorPin) > LightThreshold) {
      turnLightOn();
    } else {
      turnLightOff();
    }
  }
}

void checkTemperatureAndFan() {
  float temperature = dht.readTemperature();
  if (!isnan(temperature)) {
    if (controlFanManually == false) {
      if (temperature > FanThreshold) {
        turnFanOn();
      } else {
        turnFanOff();
      }
    }
  }
}

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}

bool sendEmailNotification(String emailMessage) {
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(receiverEmail);

  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  return true;
}

void checkIntrusion() {
  // Use whatever method you have to send the request, for example, using the WiFiClient library
  // This assumes that you have a server running locally that listens for this URL
  // Replace the IP and port with the actual values of your server

  if (intrusion()) {
    // Adding a warning message to the local website
    Serial.println("Adding intrusion warning to the local website...");
    WiFiClient client;
    if (client.connect("192.168.43.229", 80)) {
      client.print("GET /add-warning?warning=intrusion HTTP/1.1\r\n");
      client.print("Host: 192.168.43.229\r\n");
      client.print("Connection: close\r\n\r\n");
      client.stop();
      Serial.println("Intrusion warning sent");
    } else {
      Serial.println("Failed to connect to server");
    }

    // Send the Email alert
      String emailMessage = String("Someone tried to enter your house with wrong password!!");
      if (sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
      } else {
        Serial.println("Email failed to send");
      }    
  }
}

void handleRoot(AsyncWebServerRequest *request) {
  String jsonResponse = "{";

  // 1. House temperature
  jsonResponse += "\"temperature\":\"" + readDHTTemperature() + "\",";

  // 2. Humidity
  jsonResponse += "\"humidity\":\"" + readDHTHumidity() + "\",";

  // 3. Fan status
  String fanStatus = isFanOn ? "Fan is ON" : "Fan is OFF";
  jsonResponse += "\"fan\":\"" + fanStatus + "\",";

  // 4. Light status
  String lightStatus = isLightsOn ? "Light is ON" : "Light is OFF";
  jsonResponse += "\"lights\":\"" + lightStatus + "\",";

  // 5. Owner existence
  String ownerStatus = isOwnerEntered ? "yes" : "no";
  jsonResponse += "\"ownerStatus\":\"" + ownerStatus + "\"";

  jsonResponse += "}";

  request->send(200, "application/json", jsonResponse);
}

void requestTurnLightOn(AsyncWebServerRequest *request) {
  turnLightOn();
  controlLightsManually = true;
  request->send(200);
}

void requestTurnLightOff(AsyncWebServerRequest *request) {
  turnLightOff();
  controlLightsManually = true;
  request->send(200);
}

void requestTurnFanOn(AsyncWebServerRequest *request) {
  turnFanOn();
  controlFanManually = true;
  request->send(200);
}

void requestTurnFanOff(AsyncWebServerRequest *request) {
  turnFanOff();
  controlFanManually = true;
  request->send(200);
}

void automatic(AsyncWebServerRequest *request) {
  controlLightsManually = false;
  controlFanManually = false;

  request->send(200);
}

void reset(AsyncWebServerRequest *request) {
  isOwnerEntered = false;
  controlLightsManually = true;
  controlFanManually = true;
  isMotion = false;
  turnLightOff();
  turnFanOff();

  request->send(200);
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  
  pinMode(PIRPin, INPUT_PULLUP);
  pinMode(LightSensorPin, INPUT);
  pinMode(LED_House, OUTPUT);
  pinMode(relayPin, OUTPUT);

  dht.begin();
  turnFanOff();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected :");
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());

  // Adding handler for each endpoint
  server.on("/", HTTP_GET, handleRoot);

  server.on("/lights-on", HTTP_GET, requestTurnLightOn);

  server.on("/lights-off", HTTP_GET, requestTurnLightOff);

  server.on("/fan-on", HTTP_GET, requestTurnFanOn);

  server.on("/fan-off", HTTP_GET, requestTurnFanOff);

  server.on("/auto-control", HTTP_GET, automatic);

  server.on("/reset", HTTP_GET, reset);

  server.begin();
}

void loop() {
  
  checkIntrusion();

  if(!isMotion && checkOwnerEntry()){
    while(!isMotion){
      if (checkMotion()){
        isMotion = true;
        break;
      }
    }
  }
  else if(isMotion){
    checkLight();
    checkTemperatureAndFan();
  }
}
