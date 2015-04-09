#include <stdio.h>
#include <string.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"

const int IRQ = 3;
const int VBAT = 5;
const int CS = 10;
Adafruit_CC3000 cc3000 = Adafruit_CC3000(CS, IRQ, VBAT, SPI_CLOCK_DIVIDER);
#define WLAN_SSID       "EECE281_Group7"
#define WLAN_PASS       "carrots281"
#define WLAN_SECURITY   WLAN_SEC_WPA2 //either WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

Adafruit_CC3000_Server server(80);

String HTTP_req;             // string that stores the HTTP request
boolean alarm_status = 0;   // 0 = off, 1 = on, only model can turn on alarm, alarm
                            // can be turned off through internet and maybe keypad?
boolean login_verification = 0; //0 = not verified, 1 = verified
boolean system_status = 0;  //0 = system not armed, 1 = system armed
String pass = "group10";

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Hello, CC3000!\n")); 

  Serial.print(F("Free RAM: ")); 
  Serial.println(getFreeRam(), DEC);
  //Initialize
  Serial.println(F("Initializing CC3000..."));
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
  
} //-------------------------------------------------------------------------------------------------END OF SETUP

void loop()
{
    Adafruit_CC3000_ClientRef client = server.available();
    
    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c; // save the HTTP request 1 char at a time
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
                    request(client); // -------------------------------------------------------------add formated message
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
    } // end if (client)
} //---------------------------------------------------------------------------------------------------------end of loop

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
        login_verification = 1;
        html_update(cl);
        return 0;
      }
      else
         login_verification = 0;
         html_wrongpass(cl);
         return 0;
    }
    
    
    if (login_verification == 1) {
      if (myreq.startsWith(act_req)) {
          if (myreq.substring(9).compareTo(one_req) == 0) {
            html_update(cl);
            return 0; 
          }
          else if (myreq.substring(9).compareTo(two_req) == 0) {
            system_status = 1;
            Serial.println(F("System is armed."));
            html_update(cl);
            return 0;
          }
          else if (myreq.substring(9).compareTo(three_req) == 0) {
            alarm_status = 0;
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
    for ( j = 4 ; j < HTTP_req.indexOf('\n') ; j++) { //less than 20 for now, find suitable value
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
  if (system_status == 0) {
     myclient.fastrprintln(F("<p>System is not armed.</p>"));
  } 
  else {
     myclient.fastrprintln(F("<p>System is armed.</p>"));
  }

  if (alarm_status == 0) {
     myclient.fastrprintln(F("<p>Everything is ok :)</p>"));
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

String processRecentImage(){
  String result[40];
  memset(result, '\0', 40);
  strcpy(result, "<img src=\"");
  //get name of file
  strcat(result, "\" alt=\"Security\">");
  //(result = <img src="IMG00000.JPG" alt="Security">) followed by null
  return result;
  
}