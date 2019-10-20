
int ledPin=1;
int sensorPin=2;
int val =0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT);

}

void loop() {
val =analogRead(sensorPin);
if(val > 25){
  Serial.println (val);
}
  // when the sensor detects a signal above the threshold value, LED flashes
  if (val==HIGH) {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }

}
