
#define relayPin 14 


void setup() {

  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(relayPin, HIGH);
  delay(2000);
  digitalWrite(relayPin, LOW);
  delay(2000);

}
