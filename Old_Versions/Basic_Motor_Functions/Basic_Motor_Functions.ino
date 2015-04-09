const int motorDirection = 4;
const int motorSpeed = 5;

const int TURN_SPEED = 100;
const int TURN_TIME = 150;
const int OFF = 0;
const int ON = 1;

void setup() {
  pinMode(motorDirection, OUTPUT);
  pinMode(motorSpeed, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void lockDoor(){
  digitalWrite(motorSpeed, OFF);
  delay(10);
  digitalWrite(motorDirection, HIGH);
  delay(5);
  digitalWrite(motorSpeed, TURN_SPEED);
  delay(TURN_TIME);
  digitalWrite(motorSpeed, OFF);
}

void unlockDoor(){
  digitalWrite(motorSpeed, OFF);
  delay(10);
  digitalWrite(motorDirection, LOW);
  delay(5);
  digitalWrite(motorSpeed, TURN_SPEED);
  delay(TURN_TIME);
  digitalWrite(motorSpeed, OFF);
}
