
void setup() {
  pinMode(1, OUTPUT);
  pinMode(4, OUTPUT);

}

void loop() {
  delay(1000);
  digitalWrite(4, LOW);
digitalWrite(1, HIGH);
    delay(1000);
digitalWrite(1, LOW);
digitalWrite(4, HIGH);

}
