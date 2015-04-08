#include "SoftwareSerial.h"
#include <VC0706_UART.h>
#include <SPI.h>

//use software serial
SoftwareSerial cameraconnection(2,3);//Rx, Tx
VC0706 cam = VC0706(&cameraconnection);


void setup() 
{
    Serial.begin(9600);
    Serial.println("VC0706 Camera Snapshot Test ...");
    

    if(true == cameraInit()){
        snapShot();
    }else{
        Serial.println("camera init error...");
    }
}

void loop() 
{
    //nothing to do
}

bool cameraInit()
{
    cam.begin(BaudRate_19200);
    char *reply = cam.getVersion();
    if (reply == 0) {
        Serial.println("Failed to get version");
        return false;
    } else {
        Serial.println("version:");
        Serial.println("-----------------");
        Serial.println(reply);
        Serial.println("-----------------");
        return true;
    }
}

void snapShot()
{
    Serial.println("Snap in 3 secs...");
    delay(3000);
    if (! cam.takePicture()){ 
        Serial.println("Failed to snap!");
    }else { 
        Serial.println("Picture taken!");
    }

    // Open the file for writing
    
    uint16_t jpglen = cam.getFrameLength();
    
    cam.getPicture(jpglen);
    uint8_t *buffer;
    
    while(jpglen != 0){
      
         uint8_t bytesToRead = min(32, jpglen);
         buffer = cam.readPicture(bytesToRead); 
         
         //imgFile.write(buffer, bytesToRead); //replace this with transmiting bits at a time
         jpglen -= bytesToRead;   
    } 

    cam.resumeVideo();    
}
