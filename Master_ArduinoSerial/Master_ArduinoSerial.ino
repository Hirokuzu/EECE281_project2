//Libraries for Wifi & Keypad. Statuses should listen to Door/Keypad/US/HTTP(CC3000); send status to slave
#include <stdio.h>
#include <string.h>
#include <Adafruit_CC3000.h>
#include "utility/debug.h"
#include <Password.h>
#include <Keypad.h>
#include <SPI.h>

/* PIN ASSIGNMENTS */
const int ECHO_PIN = A0; //TO BE CHANGED 
const int TRIG_PIN = A1;
const int IRQ = 3; //Wifi constants
const int CS_SD = 4;
const int VBAT = 5;
const int CS = 10;

const byte ROWS = 4; //Four rows, keypad constants
const byte COLS = 3; //Three columns

const int MIN_DISTANCE = 27;

/* VARIABLE DECLARATIONS */
boolean isSystemArmed = false; //Global flag for alarm state
boolean isSystemBreached = false; //Global flag for alarm breach
byte entryIndex = 0; //Indicates how many keys user has entered
byte incorrectAttempts = 0;

//Wifi setup
Adafruit_CC3000 cc3000 = Adafruit_CC3000(CS, IRQ, VBAT, SPI_CLOCK_DIVIDER);
#define WLAN_SSID       "mywifi"
#define WLAN_PASS       "yaywifi21"
#define WLAN_SECURITY   WLAN_SEC_WPA2 //either WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

Adafruit_CC3000_Server server(80);

String HTTP_req;          // string that stores the HTTP request
boolean alarm_status = 1;   // 0 = off, 1 = on, only model can turn on alarm, alarm
                            // can be turned off through internet and maybe keypad?


Password password = Password( "4321" );

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {A2, A3, A4, A5}; //Connected to the row pinouts of the keypad
byte colPins[COLS] = {6, 7, 8}; //Connected to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup()
{
  pinMode(CS, OUTPUT);
  pinMode(ECHO_PIN,INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  
  Serial.begin(115200);
  
  digitalWrite(CS, HIGH); //ensure SS stays high for now

  keypad.addEventListener(keypadEvent); //Adds an event listener for this keypad
  keypad.setDebounceTime(50);
  
  Serial.println(F("Group 10's Home Security System!\n")); 

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  //Initialize
  Serial.println("Initializing CC3000...");
  if (!cc3000.begin())
  {
    Serial.println(F("Iniatializing failed"));
    while(1);
  }
  
  //Delete old connection data on CC3000
  Serial.println(F("Deleting old connection profiles"));
  if (!cc3000.deleteProfiles()) {
    Serial.println(F("Failed"));
    while(1);
  }
  
  //Connect to WiFi
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print(F("Attempting to connect to ")); 
  Serial.println(ssid);
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed"));
    while(1);
  }
   
  Serial.print(F("Connected to "));
  Serial.println(ssid);
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  
  
  // Display the IP address DNS, Gateway, etc.
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
  server.begin();
  Serial.println(F("Listening for connections..."));
 
}

void loop()
{
    Adafruit_CC3000_ClientRef client = server.available();

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
          if (client.available()) {   // client data available to read
              char c = client.read(); // read 1 byte (character) from client
              HTTP_req += c;  // save the HTTP request 1 char at a time
              // last line of client request is blank and ends with \n
              // respond to client only after last line received
              if (c == '\n' && currentLineIsBlank) {
                  
                  // send a standard http response header
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-Type: text/html");
                  client.println("Connection: close");
                  client.println();
                  // send web page
                  client.println("<!DOCTYPE html>");
                  client.println("<html>");
                  client.println("<head>");
                  client.println("<title>EECE 281 Project 2</title>");
                  client.println("</head>");
                  client.println("<body>");
                  client.println("<h1>ALARM TRIGGERED</h1>");
                  client.println("<p>The alarm in your house has been triggered.</p>");
                  client.println("<p> If this is a possible break-in, please call 911. </p>");
                  client.println("<p> Press the button below to turn it off. </p>");
                  client.println("<form action=\"\">");
                  button(client);
                  client.println("</form>");
                  client.println("</body>");
                  client.println("</html>");
                  
                  HTTP_req = "";    // finished with request, empty string
                  break;
              }
              // every line of text received from the client ends with \r\n
              if (c == '\n') {
                  // last character on line of received text
                  // starting new line with next character read
                  currentLineIsBlank = true;
              } 
              else if (c != '\r') {
                  // a text character was received from client
                  currentLineIsBlank = false;
              }
          } // end if (client.available())
        } // end while (client.connected())
        
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } //end if (client)
    
    keypad.getKey();
    
    if(isSystemBreached) {
        Serial.println(''); //WHAT TO SEND TO SERIAL !??!?!!?!
        activateAlarm();
    } else if(isSystemArmed) {
        float distRead = getDistance();
    
        if (distRead <= MIN_DISTANCE) {
          isSystemBreached = true;
          Serial.print("Door is opened. Distance is: ");
          Serial.println(distRead);
        }
        else {
          Serial.print("Door is closed. Distance is: ");
          Serial.println(distRead);
        }    
    }
      
//****TESTS for MASTER-SLAVE communication****//

    isSystemArmed = false;
    isSystemBreached = false;
    Serial.println('0');
  
    isSystemArmed = false;
    isSystemBreached = true;
    Serial.println('1');
    
    isSystemArmed = true;
    isSystemBreached = false;
    Serial.println('2');
  
    isSystemArmed = true;
    isSystemBreached = true;
    Serial.println('3');
}

// switch alarm off using Alarm off button
void button(Adafruit_CC3000_ClientRef cl)
{
    if (HTTP_req[6] == '*') {  // see if button was pressed
        // "*alarmcontrol=off" was detected, means button was pressed, turn alarm off
        
        alarm_status = 0; //alarm off
    }
    else if (HTTP_req[6] == 'H'){ //button wasn't pressed, specific parameter so that favicon.ico request is ignored
        alarm_status = 1;
    }
    
    if (alarm_status == 1) {    //alarm on
        Serial.print("Alarm is on.");
        cl.println(" <input type=\"checkbox\" name=\"*alarmcontrol\" value=\"off\" onclick=\"submit()\">Turn off alarm");
    }
    else {              // alarm off
        Serial.print("Alarm is off.");
        Serial.println('0'); //System status displayed as unarmed, unbreached
        cl.println("<p> [ALARM TURNED OFF] </p>");
    }
}

// Tries to read the IP address and other connection details
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}


//Parses key inputs once a key is pressed
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()) {
    case PRESSED:
      switch (eKey){
        case '#': //"Enter" key
          entryIndex = 0;
          Serial.println('w');
          checkPassword();
        break;
        case '*': //"Backspace" key
          if(entryIndex != 0) {
            entryIndex--;
            Serial.println('b');
            password.prepend();
          } else {
            Serial.println('w'); //Ensure LCD is cleared and display home page
          }
        break;
        default:
          if(entryIndex <= 16) {
            entryIndex++;
            Serial.println('k');
            password.append(eKey);
          }
      }
      break;
    case HOLD:
       if (eKey == '*') { //"Clear" key
         entryIndex = 0;
         Serial.println('w');
         password.reset();
       }
  }
}

//Checks password and toggles alarm state if correct
void checkPassword(){
  if (password.evaluate()){ 
    Serial.println('c');
    password.reset();

    if(!isSystemBreached) {
      isSystemArmed = !isSystemArmed; //Toggle alarm state
    }

    incorrectAttempts = 0;
    isSystemBreached = false;
    
  } else {
    Serial.println('i');
    password.reset();
    incorrectAttempts++;

    if(incorrectAttempts >= 3) {
      isSystemBreached = true;
      incorrectAttempts = 0;
    }
  }
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

