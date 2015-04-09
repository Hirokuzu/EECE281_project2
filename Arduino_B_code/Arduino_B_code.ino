#include <Wire.h>

/* PIN ASSIGNMENTS */
const int ECHO_PIN = 7;
const int TRIG_PIN = 6;
const int BUZZER_PIN = A0;
//const int LED1_PIN = ;
//const int LED2_PIN = ;

const int MIN_DISTANCE = 27;

int piezzoBuzzer;
boolean priority = false; //Initialize priority flag as low
boolean isSystemArmed = false; //Global flag for alarm state
boolean isSystemBreached = false; //Global flag for alarm breach


void setup() {
  pinMode(ECHO_PIN,INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Wire.begin(2);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(9600);
}


void loop() {
//  if(isSystemBreached) {
//    sendIntruderPhoto();
//    activateAlarm();
//  } else if(isSystemArmed) {
//    float distRead = getDistance();
//
//    if (distRead <= MIN_DISTANCE) {
//      priority = true;
//      isSystemBreached = true;
//      Serial.print("Door is opened. Distance is: ");
//      Serial.println(distRead);
//    }
//    else {
//      Serial.print("Door is closed. Distance is: ");
//      Serial.println(distRead);
//    }    
//  }

Serial.println("yo its a message");
delay(1000);
}


void receiveEvent(int howMany) {
  
  boolean priority_r = Wire.read();
  Serial.print("Status received, priority is: ");
  Serial.println(priority_r);
  
  if(priority_r) {
    
    Serial.println("High priority detected. Status changed.");
    
    isSystemArmed = Wire.read();
    Serial.print("isSystemArmed received. isSystemArmed bit is: ");
    Serial.println(isSystemArmed);
    
    isSystemBreached = Wire.read();
    Serial.print("isSystemBreached received. isSystemBreached bit is: ");
    Serial.println(isSystemBreached);
  }
}


void requestEvent() {
  Wire.write(priority);
  Wire.write(isSystemArmed);
  Wire.write(isSystemBreached);
  priority = false;
  Serial.print("Status sent.");
}


float getDistance() {
  float echoTime; //time of pulse in microseconds
  float distance; //distance in cm
  float speedSound = 33.15; 
  
  digitalWrite(TRIG_PIN, LOW); //10us pulse to trigger
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
   
  echoTime = pulseIn(ECHO_PIN, HIGH); //waits for ECHO_PIN to go HIGH, starts timer, stops timer when ECHO_PIN goes LOW
  
  distance = echoTime/speedSound/2;
 
  return distance;
}


void sendIntruderPhoto() {
  //take picture
  //post to web
}


//FOR NOW THIS ACTUALLY JUST TURNS AN LED ON
void activateAlarm(){
  piezzoBuzzer = digitalRead(BUZZER_PIN);

  if(piezzoBuzzer == LOW) { //LED lights in sync with Piezzo Buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    // digitalWrite(LED1_PIN, HIGH);
    // digitalWrite(LED2_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    // digitalWrite(LED1_PIN, LOW);
    // digitalWrite(LED2_PIN, LOW);
  }

  Serial.println("ALARM IS ACTIVE");
}
