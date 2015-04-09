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
const int door = 2;
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
boolean isLoginVerified = false; //Global flag for login verification
byte entryIndex = 0; //Indicates how many keys user has entered
byte incorrectAttempts = 0;

//Wifi setup
Adafruit_CC3000 cc3000 = Adafruit_CC3000(CS, IRQ, VBAT, SPI_CLOCK_DIVIDER);
#define WLAN_SSID       "EECE281_Group10"
#define WLAN_PASS       "carrots281"
#define WLAN_SECURITY   WLAN_SEC_WPA2 //either WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

Adafruit_CC3000_Server server(80);

String HTTP_req;          // string that stores the HTTP request
String pass = "group10";  // password for webpage login

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
  pinMode(door, INPUT);
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
    Serial.println(F("Initializing failed"));
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
                  client.fastrprintln(F("HTTP/1.1 200 OK"));
                  client.fastrprintln(F("Content-Type: text/html"));
                  client.fastrprintln(F("Connection: close"));
                  client.println();
                  client.fastrprintln(F("<!DOCTYPE html>"));
                  client.fastrprintln(F("<html>"));
                  client.fastrprintln(F("<head>"));
                  client.fastrprintln(F("<title>EECE 281 Project 2</title>"));
                  client.fastrprintln(F("</head>"));
                  client.fastrprintln(F("<body>"));
                  client.fastrprintln(F("<h1>My Alarm System</h1>"));
                  request(client);
                  client.fastrprintln(F("</body>"));
                  client.fastrprintln(F("</html>"));
                  HTTP_req = "";
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
        Serial.println('3');
    } else if(isSystemArmed) {
        float distRead = getDistance();
    
        if (distRead <= MIN_DISTANCE) {
          isSystemBreached = true;
          Serial.print("Window is opened. Distance is: ");
          Serial.println(distRead);
        }
        else {
          Serial.print("Window is closed. Distance is: ");
          Serial.println(distRead);
        }    
        
        if (door == LOW) {
          isSystemBreached = true;
          Serial.print("Door is opened.");
          Serial.println('3');
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
int request(Adafruit_CC3000_ClientRef cl)
{
    
    String myreq = parserequest();
    String init_req = "/";
    String pass_req = "/?pass=";
    String act_req = "/?action=";
    String one_req = "1";
    String two_req = "2";
    String three_req = "3";
    
    if (myreq.compareTo(init_req) == 0) {
       html_login(cl);
       return 0;
    }
    
    else if (myreq.startsWith(pass_req)){
      if (myreq.substring(7).compareTo(pass) == 0) {
        isLoginVerified = true;
        html_update(cl);
        return 0;
      }
      else
         isLoginVerified = false;
         html_wrongpass(cl);
         return 0;
    }
    
    if (isLoginVerified == true) {
      if (myreq.startsWith(act_req)) {
          if (myreq.substring(9).compareTo(one_req) == 0) {
            html_update(cl);
            return 0; 
          }
          else if (myreq.substring(9).compareTo(two_req) == 0) {
            isSystemArmed = true;
            Serial.println(F("System is armed."));
            html_update(cl);
            return 0;
          }
          else if (myreq.substring(9).compareTo(three_req) == 0) {
            isSystemBreached = false;
            Serial.println(F("Alarm turned off."));
            html_update(cl);
            return 0;
          }
      }  
    }
}

//parses HTTP_req, returns only request string
String parserequest() {
  int j;
  String get = "GET";
  String req;
  if (HTTP_req.startsWith(get)) {
    //Serial.println(F("Received a GET tag from client"));
    for (j=4;j<HTTP_req.indexOf('\n');j++) { //less than 20 for now, find suitable value
       if (HTTP_req[j] == ' '){
         break;
       }
       req += HTTP_req[j];
    }
    return req;
  }
  else {
    Serial.println(F("Did not receive a GET tag"));  
  }  
}

//html pages
void html_login(Adafruit_CC3000_ClientRef myclient) {
  myclient.fastrprintln(F("<form method=\"get\">"));
  myclient.fastrprintln(F("Password: <input type=\"text\" name=\"pass\" ><br>"));
  myclient.fastrprintln(F("<input type=\"submit\" value=\"Login\">"));
  myclient.fastrprintln(F("</form>"));
}  

void html_update(Adafruit_CC3000_ClientRef myclient){
  if (isSystemArmed == false) {
     myclient.fastrprintln(F("<p>System is not armed.</p>"));
  } 
  else {
     myclient.fastrprintln(F("<p>System is armed.</p>"));
  }

  if (isSystemBreached == false) {
     myclient.fastrprintln(F("<p>Your house is safe :)</p>"));
  }
  else {
     myclient.fastrprintln(F("<p>OMG!! ALARM TRIGGERED! D:/p>"));
  }
  myclient.fastrprintln(F("<form method=\"get\">"));
  myclient.fastrprintln(F("Action: <input type=\"text\" name=\"action\" ><br>"));
  myclient.fastrprintln(F("<input type=\"submit\" value=\"Go\">"));
  myclient.fastrprintln(F("</form>"));
  myclient.fastrprintln(F("<p>1 = update status</p>"));
  myclient.fastrprintln(F("<p>2 = arm the system</p>"));
  myclient.fastrprintln(F("<p>3 = turn off alarm</p>"));
}

void html_wrongpass(Adafruit_CC3000_ClientRef myclient) {
  myclient.fastrprintln(F("<form method=\"get\">"));
  myclient.fastrprintln(F("Password: <input type=\"text\" name=\"pass\" ><br>"));
  myclient.fastrprintln(F("<input type=\"submit\" value=\"Login\">"));
  myclient.fastrprintln(F("<p> Wrong password. Please try again. </p>"));
  myclient.fastrprintln(F("</form>"));  
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

