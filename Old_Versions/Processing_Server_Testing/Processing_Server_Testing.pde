import processing.net.*; 

Server myServer; //initalize the server variable but dont connect it to anythig

static final int BUFFER_SIZE = 2000000; //make a 2 mb worth of bytes

int val = 0; //used to detect hanging
int port = 32333; //port to use
boolean serverRunning = true;

void setup() {
  // initalizes screen to see if hanging
  size (400,400);
  background(0);
  
  //creates server at port, referenced from location myServer
  myServer = new Server ( this, port);
}

void draw() {
  val  = (val + 1) % 255 ; //computes next hue for screen
  background(val); //changes colour of screen to detect hanging
  
  //myServer.write(val); //will send the value to all clients connected to the server
  
  Client myArduino; //initalize the arduino client that will be contacting the server
  byte buffer[BUFFER_SIZE]; //2mb sized buffer
  
  myArduino = myServer.available(); //returns the next available client, should only ever be arduino
  
  if(myArduino != null){
    //isn't null so it now can see what the client is saying
    for(int i = 0 ; i < BUFFER_SIZE ; i++){ //clears the buffer
      buffer[i] = null;
    }
    
}


