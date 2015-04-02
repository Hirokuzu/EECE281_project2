#include <Password.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

boolean isSystemArmed = false;  // GLOBAL FLAG TO MAKE KEEP TRACK OF SYSTEM STATE, ON OR OFF

Password password = Password( "4321" );

LiquidCrystal lcd(13, 12, 11, 10, 9, 8); 
//                RS E D4 D5 D6 D7

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {6, A0, 2, 4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 7, 3}; //connect to the column pinouts of the keypad
byte entryIndex = 0; //Indicates how many keys user has entered

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  keypad.addEventListener(keypadEvent); //add an event listener for this keypad
  keypad.setDebounceTime(50);
  welcomeMessage();
  displayHome();
}

void loop() {
  keypad.getKey();
}

//Parses key inputs once a key is pressed
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()){
    case PRESSED:
      if(entryIndex == 0){
        lcd.clear();
        lcd.print("Entering code:");
        lcd.setCursor(0, 1);
      }
      switch (eKey){
        case '#': 
          checkPassword();
          entryIndex = 0; 
        break;
        case '*':
          if(entryIndex != 0) {
            entryIndex--;
            displayPrependedCode(entryIndex); //Removes the last key from the display
            password.prepend();
          }
          else {
            displayHome();
          }
        break;
        default:
          lcd.print("*");
          password.append(eKey);
          entryIndex++;
      }
      break;
    case HOLD:
       if (eKey == '*'){
         password.reset();
         entryIndex = 0;
         displayHome();
       }
  } // END OF OUTER SWITCH STATEMENT
}


void checkPassword(){
  if (password.evaluate()){ 
    acceptPassword(); // accept password and toggle system state
    password.reset(); //resets password
    delay(1000);      // keep accepted password mesage for 1 second
  } else {
    denyPassword();
    password.reset(); //resets password after INCORRECT entry
    delay(1000);      // keep denied password mesage for 1 second
  }
  displayHome();
}

void displayPrependedCode(byte codeIndex) {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);  for(byte i = 1; i <= codeIndex; i++) {
    lcd.print("*");
  }
}

void triggerAlarm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALARM TRIGGERED");
  lcd.setCursor(0, 1);
  lcd.print("CALL 911");
  //TRIGGER ALARM SOUNDS AND FLASH LIGHTS FUNCTION
}

void displayHome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Security System");
  lcd.setCursor(0, 1);
  
  if(isSystemArmed) {
    lcd.print("System: Active");
    // CALL ACTIVATE ALL SENSORS NOW FUNCTION
  } else {
    lcd.print("System: Inactive");
    // CALL DEACTIVATE ALL SENSORS NOW FUNCTION
  }
}

void acceptPassword() {
  lcd.clear();
  lcd.print("PASSWORD VALID");
  lcd.setCursor(0, 1);

  if(!isSystemArmed) {
    lcd.print("SYSTEM ARMED");
    isSystemArmed = !isSystemArmed; // toggle the system state from off to on
  } else {
    lcd.print("SYSTEM DISARMED");
    isSystemArmed = !isSystemArmed;
  }
}

void denyPassword() {
  lcd.clear();
  lcd.print("PASSWORD INVALID");
  lcd.setCursor(0, 1);
  lcd.print("ACCESS DENIED");
}

void welcomeMessage(){
  lcd.clear();
  lcd.print("  EECE 281 L2C");
  lcd.setCursor(0, 1);
  lcd.print("   PROJECT 2");
  delay(1500);
  lcd.clear();
  lcd.print("   Hope you   ");
  lcd.setCursor(0, 1);
  lcd.print("  like it! :) ");
  delay(2000);
}
