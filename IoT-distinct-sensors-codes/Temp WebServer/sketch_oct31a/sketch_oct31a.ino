#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define RXp2 16
#define TXp2 17
#define PIRPin 4
#define LightSensorPin 27
#define AnalogThreshold 500
#define LED_House 25
#define DHTPIN 26
#define DHTTYPE DHT11
#define relayPin 5

const char* ssid = "SALEH";
const char* password = "12345678";

DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);

bool isOwnerEntered = false;



String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println(h);
    return String(h);
  }
}

bool isOwner() {
  String receivedMessage = Serial2.readString();
  receivedMessage.trim();
  if (receivedMessage.equals("1")) {
    return true;
  } else {
    return false;
  }
}

bool readMotion() {
  return digitalRead(PIRPin) == HIGH;
}

float readLight() {
  return analogRead(LightSensorPin);
}

void turnLightOn() {
  digitalWrite(LED_House, HIGH);
}

void turnLightOff() {
  digitalWrite(LED_House, LOW);
}

void turnFanOn() {
  digitalWrite(relayPin, LOW);
}

void turnFanOff() {
  digitalWrite(relayPin, HIGH);
}


void checkOwnerEntry() {
  isOwnerEntered = isOwner();
  if (isOwnerEntered) {
    Serial.println("Owner entered the house!");
  }
}

// Function to send dynamic updates to the web page
void sendUpdate(const String& endpoint, const String& value) {
  String response = value;
  server.on(endpoint.c_str(), HTTP_GET, [response](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", response.c_str());
  });
}


void checkMotionAndLight() {
  if (isOwnerEntered && readMotion()) {
    Serial.println("Motion detected!");
    
    float lightIntensity = readLight();
    Serial.println("Light Intensity: " + String(lightIntensity));
    
    if (lightIntensity < AnalogThreshold) {
      Serial.println("Turning on the lights!");
      turnLightOn();
      sendUpdate("/motion", "Motion Detected! Lights are ON");
    }
  }
}

void checkTemperature() {
  float temperature = dht.readTemperature();
  if (!isnan(temperature)) {    
    if (temperature > 27.0) {
      Serial.println("Turning on the fan!");
      turnFanOn();
      sendUpdate("/temperatureStatus", "Fan is ON");
    } else {
      turnFanOff();
      sendUpdate("/temperatureStatus", "Fan is OFF");
    }
  }
}



const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Smart House System</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <p>
    <i class="fas fa-lightbulb" style="color:#FFD700;"></i>
    <span class="dht-labels">Motion</span>
    <span id="motion">%MOTION%</span>
  </p>
  <p>
    <i class="fas fa-wind" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperature Status</span>
    <span id="temperatureStatus">%TEMPERATURE_STATUS%</span>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("motion").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/motion", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperatureStatus").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperatureStatus", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

String processor(const String& var) {
  Serial.println(var);
  if (var == "TEMPERATURE") {
    return readDHTTemperature();
  } else if (var == "HUMIDITY") {
    return readDHTHumidity();
  } else if (var == "MOTION") {
    return readMotion() ? "Motion Detected" : "No Motion";
  } else if (var == "TEMPERATURE_STATUS") {
    return (dht.readTemperature() > 27.0) ? "Fan is ON" : "Fan is OFF";
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  pinMode(PIRPin, INPUT_PULLUP);
  pinMode(LED_House, OUTPUT);
  pinMode(relayPin, OUTPUT);
  dht.begin();
  digitalWrite(relayPin, HIGH);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/motion", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readMotion() ? "Motion Detected" : "No Motion");
  });
  server.on("/temperatureStatus", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", (dht.readTemperature() > 27.0) ? "Fan is ON" : "Fan is OFF");
  });
    server.on("/lightStatus", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readLight() > AnalogThreshold ? "Light is ON" : "Light is OFF");
  });
  server.begin();
}

void loop() {
  checkOwnerEntry();
  checkMotionAndLight();
  checkTemperature();
  delay(1000);
}
