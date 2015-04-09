#include "SoftwareSerial.h"
#include <VC0706_UART.h>
#include <LiquidCrystal.h>
#include <SPI.h>

#define TIME_DELAY 5000 //5 second delay when taking photos
#define SS_SD 10 // not neede
//use software serial
SoftwareSerial cameraconnection(2,7);//Rx, Tx
VC0706 cam = VC0706(&cameraconnection);
LiquidCrystal lcd(14, 15, 16, 17, 18, 19); //use 14-19 for analog pins acting as digital

const byte LED_R = 3;
const byte LED_G = 6;
const byte LED_B = 9;

boolean lcd_update = true;
boolean lcd_print = false; // overwrite LCD;
boolean alarm_flag = false;
boolean is_armed = false;
boolean corr_pass = true;
unsigned long currentTime;
unsigned long prevTime;
byte num_presses = 0;

String message;
String lcd_msg;

void setup(){
  updateLCD();
  
  Serial.begin(115200);
  lcd.begin(16,2);
  lcd.print("Initializing...");
  cam.begin(BaudRate_19200);
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.println(F("Version not found."));
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
  if(lcd_update) {
    updateLCD();
  }
}

void serialEvent() {
  while(Serial.available()) {
    char current = (char)Serial.read();
    message += current;
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
      Serial.println(F("Failed to snap!"));
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
  if(lcd_print) {
    lcdPrint(lcd_msg);
  }
  else if(alarm_flag){
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
  lcd_update = false;
}

void lcdAlarm() {
  lcd.clear();
  lcd.print(F("Alarm On."));
}

void lcdArmed() {
  lcd.clear();
  lcd.print(F("System active."));
}

void lcdHome() {
  lcd.clear();
  lcd.print(F("System inactive."));
}

void lcdPass() {
  lcd.setCursor(0,1);
  lcd.print("Pass:");
  for(int i = 0; i < num_presses; i++) {
    lcd.print('*');
  }
}

void lcdPrint(String message) {
  if(lcd_print){
    lcd.clear();
    if(message.length() > 16) {
      Serial.println("Message needs more than one row");
      lcd.print(message.substring(0,16));
      lcd.setCursor(0,1);
      if(message.length() < 33) {
        lcd.print(message.substring(16));
      }
      else {
        lcd.print(message.substring(15,32));
      }
    }
    else {
      Serial.println("Message fits in a row!");
      lcd.print(message);
    }
  }
  lcd_print = false;
}



int process_buffer(String toProcess) {
  if(toProcess.startsWith("0")) {
      alarm_flag = false;
      is_armed = false;
      lcd_update = true;
  } else if(toProcess.startsWith("1")) {
      alarm_flag = true;
      is_armed = false;
      lcd_update = true;
  } else if(toProcess.startsWith("2")) {
      alarm_flag = false;
      is_armed = true;
      lcd_update = true;
  } else if(toProcess.startsWith("3")) {
      alarm_flag = true;
      is_armed = true;
      lcd_update = true;
  } else if(toProcess.startsWith("b")) {
      if(num_presses > 0)
        num_presses--;
        lcd_update = true;
  } else if(toProcess.startsWith("c")) {
      num_presses = 0;
      corr_pass = true;
      lcd_update = true;
  } else if(toProcess.startsWith("i")) {
      num_presses = 0;
      corr_pass = false;
  } else if(toProcess.startsWith("k")) {
      num_presses++;
  } else if(toProcess.startsWith("w")) {
      num_presses = 0;
  } else if(toProcess.startsWith("lcd:")) {
    Serial.println("LCD");
      lcd_msg = toProcess.substring(toProcess.indexOf(":") + 1);
      lcd_msg.trim();
      lcd_print = true;
      // do nothing
  } else {
    Serial.println(F("Got nothing"));
  }
  lcd_update = true;
}


// takes a picture and prints to serial if it was successful
void testPic() {
  if (! cam.takePicture()){
    Serial.println(F("Failed to snap!"));
  } else {
    Serial.println(F("Picture was taken correctly!"));
  }
}
