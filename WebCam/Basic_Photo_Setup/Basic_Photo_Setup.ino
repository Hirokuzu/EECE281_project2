#include "SoftwareSerial.h"
#include <VC0706_UART.h>
#include <SD.h>
#include <SPI.h>
#define SS_SD  10

//use software serial
SoftwareSerial cameraconnection(2,3);//Rx, Tx
VC0706 cam = VC0706(&cameraconnection);

void setup() 
{
    Serial.begin(11520); //gotta go fast (was set to 9600 origionally)
    Serial.println("VC0706 Camera Snapshot Test ...");
    
    if (!SD.begin(SS_SD)) { //starts the sd card and checks that it's there
        Serial.println("SD Card init failed...");
        return;
    }  
    if(true == cameraInit()){ //checks that the camera can start
        snapShot(); //takes a photo from the camera and prints it to the sd card
    }else{
        Serial.println("camera init error..."); 
    }
}

void loop() 
{
  
}

bool cameraInit()
{
    cam.begin(BaudRate_19200); //sets the baud rate for the camera which uses hardward at the fastest speed it can handle
    char *reply = cam.getVersion(); //gets the version of the camera/buffer to determine that it's working
    if (reply == 0) { //will be 0 if it doesnt work
        return false;
    } else {
        return true;
    }
}

void snapShot()
{
    if (! cam.takePicture()){ 
        Serial.println("Failed to snap!");
    }else { 
        Serial.println("Picture taken!");
    }
    // Create an image with the name IMAGExx.JPG
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
    uint16_t jpglen = cam.getFrameLength(); //largest jpg can be 1 228 800 bytes (1.17Mb), gets size of jpg
    cam.getPicture(jpglen); //takes the picture from the camera
    uint8_t *buffer; //pointer to a buffer of 64 bytes
    while(jpglen != 0){
         uint8_t bytesToRead = min(32, jpglen); //determines how many bytes to read, 32 bytes at a time
         buffer = cam.readPicture(bytesToRead); // puts the number of bytes into the buffer
         imgFile.write(buffer, bytesToRead); //writes from the buffer to the imgFile on the sd card
         jpglen -= bytesToRead; //decrements the jpglen with is the amount of space left over
    } 
    imgFile.close(); //once it's done it closes the file as it's been written and is now on the sd card

    cam.resumeVideo(); //starts watching again and waiting to take a picture
}
