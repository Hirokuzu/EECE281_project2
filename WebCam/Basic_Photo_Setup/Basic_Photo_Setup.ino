#include "SoftwareSerial.h"
#include <VC0706_UART.h>
#include <SD.h>
#include <SPI.h>
#define SS_SD  10

//use software serial
SoftwareSerial cameraconnection(2,3);//Rx, Tx
VC0706 cam = VC0706(&cameraconnection);

boolean ALARM_FLAG = false;
unsigned long currentTime;
unsigned long prevTime;
#define TIME_DELAY 5000 //5 second delay when taking photos

void setup(){
  Serial.begin(9600);
  
  if (!SD.begin(SS_SD)) {
      Serial.println("SD Card init failed...");
      return;
  }  
  cam.begin(BaudRate_19200);
  char *reply = cam.getVersion();
  if (reply == 0) {
      Serial.println("Failed to get version");
      return;
  }
  prevTime = millis();
}

void loop(){
  if(ALARM_FLAG){
    currentTime = millis();
    if(currentTime - prevTime > TIME_DELAY){
      takePic();
    }
  }
}


/*
  Takes a picture and stores it to the SD card, returns true if the files was created successfully or not
*/
boolean takePic(){
  if (! cam.takePicture()){ 
      Serial.println("Failed to snap!");
  }
  // Create an image with the name IMAGEXX.JPG
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
      filename[5] = '0' + i/10;
      filename[6] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(filename)) {
          break;
      }
  }
  // Open the file for writing
  File imgFile = SD.open(filename, FILE_WRITE);
  uint16_t jpglen = cam.getFrameLength();
  cam.getPicture(jpglen);
  uint8_t *buffer;
  // write to the file
  while(jpglen != 0){
       uint8_t bytesToRead = min(32, jpglen);
       buffer = cam.readPicture(bytesToRead);     
       imgFile.write(buffer, bytesToRead);
       jpglen -= bytesToRead;   
  } 
  // close to the file so that it saves and continue looking through camera
  imgFile.close();
  cam.resumeVideo();
  
  return SD.exists(filename); 
}


// takes a picture and prints to serial if it was successful
void testPic(){
  if (! cam.takePicture()){ 
    Serial.println("Failed to snap!");
  } else {
    Serial.println("Picture was taken correctly!");
  }  
}
