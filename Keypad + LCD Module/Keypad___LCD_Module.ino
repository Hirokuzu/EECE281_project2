#include <Password.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

boolean isSystemArmed = false; //Global flag for alarm state
boolean isSystemBreached = false; //Global flag for alarm breach
byte entryIndex = 0; //Indicates how many keys user has entered
byte incorrectAttempts = 0;

Password password = Password( "4321" );

LiquidCrystal lcd(13, 12, A0, A1, A2, A3); 
//                RS E D4 D5 D6 D7

const byte ROWS = 4; //Four rows
const byte COLS = 3; //Three columns
const byte RGB_LED_PINR = 9;
const byte RGB_LED_PING = 10;
const byte RGB_LED_PINB = 11;
const boolean ON = 0;
const boolean OFF = 1;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {5, 6, 7, 8}; //Connected to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4}; //Connected to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


void setup() {
  pinMode(RGB_LED_PINR, OUTPUT);
  pinMode(RGB_LED_PING, OUTPUT);
  pinMode(RGB_LED_PINB, OUTPUT);
  lcd.begin(16, 2);
  Serial.begin(9600);
  keypad.addEventListener(keypadEvent); //Adds an event listener for this keypad
  keypad.setDebounceTime(50);
  displayWelcomeMessage();
  displayHomePage();
}


void loop() {
  keypad.getKey();

  if(isSystemBreached) {
    setRgbLed(ON, OFF, OFF);
  } else if(isSystemArmed) {
    setRgbLed(OFF, ON, OFF);
  } else {
    setRgbLed(OFF, OFF, ON);
  }
}


//Parses key inputs once a key is pressed
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()) {
    case PRESSED:
      if(entryIndex == 0) {
        displayPasscodePrompt();
      }
      switch (eKey){
        case '#': //"Enter" key
          entryIndex = 0;
          checkPassword();
          displayHomePage();
        break;
        case '*': //"Backspace" key
          if(entryIndex != 0) {
            entryIndex--;
            displayPrependedCode(); //Removes the last key from the display
            password.prepend();
          } else {
            displayHomePage();
          }
        break;
        default:
          if(entryIndex <= 16) {
            entryIndex++;
            lcd.print("*");
            password.append(eKey);
          }
      }
      break;
    case HOLD:
       if (eKey == '*') { //"Clear" key
         entryIndex = 0;
         password.reset();
         displayHomePage();
       }
  }
}


//Checks password and toggles alarm state if correct
void checkPassword(){
  if (password.evaluate()){ 
    displayPasscodeAccepted();
    password.reset();
    incorrectAttempts = 0;

    if(!isSystemBreached) {
      isSystemArmed = !isSystemArmed; //Toggle alarm state
    }

    isSystemBreached = 0;
  } else {
    displayPasscodeRejected();
    password.reset();
    incorrectAttempts++;

    if(incorrectAttempts >= 3) {
      isSystemBreached = 1;
      incorrectAttempts = 0;
    }

    Serial.println(incorrectAttempts);
  }
}


void setRgbLed(boolean rState, boolean gState, boolean bState) {
  digitalWrite(RGB_LED_PINR, rState);
  digitalWrite(RGB_LED_PING, gState);
  digitalWrite(RGB_LED_PINB, bState);
}


void displayPrependedCode() {
  lcd.setCursor(0, 1);
  lcd.print("                "); //Clears second row only
  lcd.setCursor(0, 1);

  for(byte i = 1; i <= entryIndex; i++) {
    lcd.print("*");
  }
}


void displayHomePage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if(isSystemBreached) {
    lcd.print("ALARM TRIGGERED");
    lcd.setCursor(0, 1);
    lcd.print("CALL 911");
  } else {
    lcd.print("Security System");
    lcd.setCursor(0, 1);
    
    if(isSystemArmed) {
      lcd.print("System: Active");
    } else {
      lcd.print("System: Inactive");
    }
  }
}


void displayPasscodePrompt() {
  lcd.clear();
  lcd.print("Enter passcode:");
  lcd.setCursor(0, 1);
}


void displayPasscodeAccepted() {
  lcd.clear();
  lcd.print("PASSCODE VALID");
  lcd.setCursor(0, 1);

  if(isSystemBreached) { //Alarm will be disarmed
    lcd.print("ALARM DISARMED");
  } else {
    if(isSystemArmed) { //System will be disarmed
      lcd.print("SYSTEM DISARMED");
    } else { //System will be armed
      lcd.print("SYSTEM ARMED");
    }    
  }

  delay(1000);
}


void displayPasscodeRejected() {
  lcd.clear();
  lcd.print("PASSCODE INVALID");
  lcd.setCursor(0, 1);
  lcd.print("ACCESS DENIED");
  delay(1000);
}


void displayWelcomeMessage(){
  lcd.clear();
  lcd.print("  EECE 281 L2C");
  lcd.setCursor(0, 1);
  lcd.print("    PROJECT 2");
  delay(1500);
  lcd.clear();
  lcd.print("    Hope you");
  lcd.setCursor(0, 1);
  lcd.print("   like it! :)");
  delay(2000);
}