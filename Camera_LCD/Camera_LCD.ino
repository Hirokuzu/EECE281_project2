#include "SoftwareSerial.h"
#include <VC0706_UART.h>
#include <LiquidCrystal.h>
#include <SPI.h>

#define TIME_DELAY 5000 //5 second delay when taking photos
#define SS_SD 10 // not neede
//use software serial
SoftwareSerial cameraconnection(2,7);//Rx, Tx
VC0706 cam = VC0706(&cameraconnection);
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

const byte LED_R = 3;
const byte LED_G = 6;
const byte LED_B = 9;

boolean lcd_update = true;
boolean alarm_flag = false;
boolean is_armed = false;
boolean corr_pass = true;
unsigned long currentTime;
unsigned long prevTime;
byte num_presses = 0;


void setup(){
  updateLCD();
  
  Serial.begin(115200);
  lcd.begin(16,2);
  lcd.print("Initializing...");
  cam.begin(BaudRate_19200);
  char *reply = cam.getVersion();
  if (reply == 0) {
      return;
  }
  
  testPic();
  prevTime = millis();
}

void loop(){

  if(alarm_flag) {
    currentTime = millis();
    if(currentTime - prevTime > TIME_DELAY){
      takePic();
      prevTime = currentTime;
    }
  }
  
}

void serialEvent() {
  String message;
  while(Serial.available()) {
    char current = (char)Serial.read();
    message += current;
    Serial.print(current);
    if(current == '\n') {
      process_buffer(message);

      message = ""; // reset the message;
    }
  }
}

/*
  Takes a picture and sends it to the other Arduino, returns true if the files was created successfully or not
*/
boolean takePic(){
  if (! cam.takePicture()){
      Serial.println("Failed to snap!");
  }
  // send picture over SPI to SD on Master Arduino
  uint16_t jpglen = cam.getFrameLength();
  Serial.println(jpglen);
  cam.getPicture(jpglen);
  uint8_t *buffer;
  // write to SPI
  while(jpglen != 0){
    uint8_t bytesToRead = min(32, jpglen);
    buffer = cam.readPicture(bytesToRead);
    for(int i = 0; i < bytesToRead; i++) {
      Serial.println(*(char *) buffer+i);
    }
    Serial.flush();
    jpglen -= bytesToRead;
  }
  // close to the file so that it saves and continue looking through camera
  cam.resumeVideo();
// TODO: Implement
  return false;
}

void setRGB(byte R_PWM, byte G_PWM, byte B_PWM) {
  analogWrite(LED_R, R_PWM);
  analogWrite(LED_G, G_PWM);
  analogWrite(LED_B, B_PWM);
}

void updateLCD() {
  if(alarm_flag){
    setRGB(255,0,0);
    lcdAlarm();
    lcdPass();
  }
  else if(is_armed) {
    setRGB(0,0,255);
    lcdArmed();
    lcdPass();
  }
  else { // not armed, not alarmed
    setRGB(0,255,0);
    lcdHome();
    lcdPass();
  }
}

void lcdAlarm() {
  lcd.clear();
  lcd.print("Alarm On.");
}

void lcdArmed() {
  lcd.clear();
  lcd.print("System active.");
}

void lcdHome() {
  lcd.clear();
  lcd.print("System inactive.");
}

void lcdPass() {
  lcd.setCursor(0,1);
  lcd.print("Pass:");
  for(int i = 0; i < num_presses; i++) {
    lcd.print('*');
  }
}

void lcdPrint(String message) {
  lcd.clear();
  if(message.length() > 16) {
    lcd.print(message.substring(0,15));
    lcd.setCursor(0,1);
    lcd.print(message.substring(15));
  }
  else {
    lcd.print(message);
  }
}



int process_buffer(String toProcess) {
  
  if(toProcess.startsWith("0")) {
      alarm_flag = false;
      is_armed = false;
  } else if(toProcess.startsWith("1")) {
    Serial.println("Got a 1");
      alarm_flag = true;
      is_armed = false;
  } else if(toProcess.startsWith("2")) {
        Serial.println("Got a 2");
      alarm_flag = false;
      is_armed = true;
  } else if(toProcess.startsWith("3")) {
        Serial.println("Got a 3");
      alarm_flag = true;
      is_armed = true;
  } else if(toProcess.startsWith("b")) {
        Serial.println("Got a b");
      if(num_presses > 0)
        num_presses--;
  } else if(toProcess.startsWith("c")) {
        Serial.println("Got a c");
      num_presses = 0;
      corr_pass = true;
  } else if(toProcess.startsWith("i")) {
        Serial.println("Got a i");
      num_presses = 0;
      corr_pass = false;
  } else if(toProcess.startsWith("k")) {
        Serial.println("Got a k");
      num_presses++;
  } else if(toProcess.startsWith("w")) {
        Serial.println("Got a w");
      num_presses = 0;
  } else if(toProcess.startsWith("lcd:")) {
      lcdPrint(toProcess.substring(toProcess.indexOf(":") + 1));
      // do nothing
  } else {
    Serial.println("Got nothing");
  }
  lcd_update = true;
}


// takes a picture and prints to serial if it was successful
void testPic() {
  if (! cam.takePicture()){
    Serial.println("Failed to snap!");
  } else {
    Serial.println("Picture was taken correctly!");
  }
}
