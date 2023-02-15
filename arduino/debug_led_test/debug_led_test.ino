// digital pins connected to debugging LEDs
#define greenLEDpin 2
#define redLEDpin 3

void setup() {
  // enable debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

}

void loop() {
  delay(500);
  digitalWrite(greenLEDpin, HIGH);
  delay(500);
  digitalWrite(greenLEDpin, LOW);
  delay(500);
  digitalWrite(redLEDpin, HIGH);
  delay(500);
  digitalWrite(redLEDpin, LOW);

}
