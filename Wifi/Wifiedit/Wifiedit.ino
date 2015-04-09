#include <stdio.h>
#include <string.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"

const int IRQ = 3;
const int VBAT = 5;
const int CS = 10;
Adafruit_CC3000 cc3000 = Adafruit_CC3000(CS, IRQ, VBAT, SPI_CLOCK_DIVIDER);
#define WLAN_SSID       "Brian White"
#define WLAN_PASS       "t6c3dprg2n2p"
#define WLAN_SECURITY   WLAN_SEC_WPA2 //either WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

Adafruit_CC3000_Server server(80);

char *HTTP_req;             // string that stores the HTTP request
boolean alarm_status = 0;   // 0 = off, 1 = on, only model can turn on alarm, alarm
                            // can be turned off through internet and maybe keypad?
boolean login_verification = 0; //0 = not verified, 1 = verified
boolean system_status = 0;  //0 = system not armed, 1 = system armed

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Hello, CC3000!\n")); 

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
    int i = 0;
    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req[i] = c;// save the HTTP request 1 char at a time
                i++;
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>EECE 281 Project 2</title>");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h1>My Alarm System</h1>");
                    //request(client);
                    client.println("</body>");
                    client.println("</html>"); 
                    Serial.println("HTTP_req");
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

/*
// switch alarm off using Alarm off button
void request(Adafruit_CC3000_ClientRef cl)
{
    char *request = parserequest(HTTP_req);
    
    if ("initial connect") {
        //html login page
    }
    else if ("initial login"){
      if (correct) {
        login_verification = 1;
        //html update page own function
      }
      else
         login_verification = 0;
         //html wrong pass
    }
    
    if (login_verification == 0) {
      if ("update") {
        //html update page 
      }
      else if ("arm system") {
        system_status = 1;
        //html update
      }
      else if ("turn alarm off") {
        alarm_status = 0;
        //html update
      }
    }            
}
*/
//parses HTTP_req, returns only request string
char* parserequest(char *HTTP_req) {
  int j;
  int k;
  char *tag;
  char *req;
  for (j=0;j<3;j++) {
    tag[j]=HTTP_req[j];
  }
  if ((strcmp(tag,"GET")) == 0) {
    Serial.println("Received a GET tag from client");
    for (k=4;k<20;k++) { //less than 20 for now, find suitable value
       if (HTTP_req[k] == ' '){
         break;
       }
       req[k-4] = HTTP_req[k];
    }
    return req;
  }
  else {
    Serial.println("Did not receive a GET tag");    
  }   
}

//html pages
void html_login(Adafruit_CC3000_ClientRef myclient) {
  myclient.println("<form method=\"get\">");
  myclient.println("Password: <input type=\"text\" name=\"pass\" ><br>");
  myclient.println("<input type=\"submit\" value=\"Login\">");
  myclient.println("</form>");
}  

void html_update(Adafruit_CC3000_ClientRef myclient){
  if (system_status == 0) {
     myclient.println("<p>System is not armed.</p>");
  } 
  else {
     myclient.println("<p>System is armed.</p>");
  }

  if (alarm_status == 0) {
     myclient.println("<p>Everything is ok :)</p>");
  }
  else {
     myclient.println("<p>OMG!! ALARM TRIGGERED! D:/p>");
  }
  myclient.println("<form method=\"get\">");
  myclient.println("Action: <input type=\"text\" name=\"action\" ><br>");
  myclient.println("<input type=\"submit\" value=\"Go\">");
  myclient.println("</form>");
  myclient.println("<p>1 = update status</p>");
  myclient.println("<p>2 = arm the system</p>");
  myclient.println("<p>3 = turn off alarm</p>");
}

void html_wrongpass(Adafruit_CC3000_ClientRef myclient) {
  myclient.println("<form method=\"get\">");
  myclient.println("Password: <input type=\"text\" name=\"pass\" ><br>");
  myclient.println("<input type=\"submit\" value=\"Login\">");
  myclient.println("<p> Wrong password. Please try again. </p>");
  myclient.println("</form>");  
}
