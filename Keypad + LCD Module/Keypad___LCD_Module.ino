#include <Password.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

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
  keypad.setDebounceTime(100);
}


void loop() {
  keypad.getKey();
}


//Parses key inputs once a key is pressed
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()){
    case PRESSED:
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
        break;
        default:
          lcd.print("*");
          password.append(eKey);
          entryIndex++;
      }
  }
}


void checkPassword(){
  if (password.evaluate()){
    lcd.clear();
    lcd.print("VALID PASSWORD "); //
    password.reset(); //resets password after correct entry
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("Welcome home");
    delay(2000);
    lcd.clear();
  } else {
    lcd.clear();
    lcd.print("INVALID PASSWORD");
    password.reset(); //resets password after INCORRECT entry
    delay(2000);
    lcd.clear();
  }
}


void displayPrependedCode(byte codeIndex) {
  lcd.clear();

  for(byte i = 1; i <= codeIndex; i++) {
    lcd.print("*");
  }
}


void displayAlarmBreach() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALARM TRIGGERED");
  lcd.setCursor(0, 1);
  lcd.print("CALL 911");
}


void displayHome(boolean alarmState) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Security System");
  lcd.setCursor(0, 1);
  
  if(alarmState) {
    lcd.print("Alarm: ON");
  } else {
    lcd.print("Alarm: OFF");
  }
}


void displayAccessGranted(boolean systemState) {
  lcd.clear();
  lcd.print("CODE ACCEPTED");
  lcd.setCursor(0, 1);

  if(systemState) {
    lcd.print("SYSTEM ARMED");
  } else {
    lcd.print("SYSTEM DISARMED");
  }
}


void displayAccessDenied() {
  lcd.clear();
  lcd.print("INVALID CODE");
  lcd.setCursor(0, 1);
  lcd.print("ACCESS DENIED");
}