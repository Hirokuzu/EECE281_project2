#include <Password.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

int pos = 0;
Password password = Password( "4321" );

LiquidCrystal lcd(13, 12, 11, 10, 9, 8); 
// RS E D4 D5 D6 D7

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

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  lcd.begin(16, 2);
  Serial.begin(9600);
  keypad.addEventListener(keypadEvent); //add an event listener for this keypad
  keypad.setDebounceTime(100);
}

void loop(){
  keypad.getKey();
    
//  char key = keypad.getKey();
//
//  if (key != NO_KEY){
//    Serial.println(key);
//    lcd.clear();
//    process_the_key(key);
//  }
}

//void process_the_key(char key_pressed){
//  switch(key_pressed){
//   case '1' :
//      displayHome(0);
//      break;
//   case '2' :
//      displayHome(1);
//      break;
//   case '3' :
//      displayAccessGranted(0);
//      break;
//   case '4' :
//      displayAccessGranted(1);
//      break;
//   case '5' :
//      displayAccessDenied();
//      break;
//   case '6' :
//      displayAlarmBreach();
//     break;
//   default :
//      lcd.write(" INVALID CODE! ");
//   } 
//}
//
//void displayAlarmBreach() {
//  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print("ALARM TRIGGERED");
//  lcd.setCursor(0, 1);
//  lcd.print("CALL 911");
//}
//
//
//void displayHome(boolean alarmState) {
//  lcd.clear();
//  lcd.setCursor(0, 0);
//  lcd.print("Security System");
//  lcd.setCursor(0, 1);
//  if(alarmState) {
//    lcd.print("Alarm: ON");
//  } else {
//    lcd.print("Alarm: OFF");
//  }
//}
//
//
//void displayAccessGranted(boolean systemState) {
//  lcd.clear();
//  lcd.print("CODE ACCEPTED");
//  lcd.setCursor(0, 1);
//
//  if(systemState) {
//    lcd.print("SYSTEM ARMED");
//  } else {
//    lcd.print("SYSTEM DISARMED");
//  }
//}
//
//
//void displayAccessDenied() {
//  lcd.clear();
//  lcd.print("INVALID CODE");
//  lcd.setCursor(0, 1);
//  lcd.print("ACCESS DENIED");
//}


//take care of some special events
void keypadEvent(KeypadEvent eKey){
  switch (keypad.getState()){
    case PRESSED:
      lcd.print(eKey);
      switch (eKey){
        case '#': 
          guessPassword(); 
        break;
        default:
          password.append(eKey);
      }
  }
}

void guessPassword(){
  if (password.evaluate()){
    lcd.clear();
    lcd.print("VALID PASSWORD "); //
    password.reset(); //resets password after correct entry
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("Welcome home");
    delay(2000);
    lcd.clear();
  }
  else{
    lcd.clear();
    lcd.print("INVALID PASSWORD");
    password.reset(); //resets password after INCORRECT entry
    delay(2000);
    lcd.clear();
  }
}
