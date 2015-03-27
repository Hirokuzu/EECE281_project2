#include <LiquidCrystal.h>
#include <Keypad.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);
//Pin order:      RS E D4 D5 D6 D7

// const byte rows = 4; //Keypad has 4 rows
// const byte cols = 3; //and 3 columns
// const int LOOP_DELAY = 100;

// char keys[rows][cols] = { //@TODO: Try with ints instead
//   {'1','2','3'},
//   {'4','5','6'},
//   {'7','8','9'},
//   {'#','0','*'}
// };
// byte rowPins[rows] = {6, 4, 3, 2}; //Keypad row pins = {2, 7, 6, 4}
// byte colPins[cols] = {4, 7, 6}; //Keypad column pins = {3, 1, 5}
// Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

// char numReceived;


void setup(){
  //Serial.begin(9600);
  // keypad.addEventListener(keypadEvent); //Run keypadEvent() when a key is pressed
  lcd.begin(16, 2);
}


void loop() {
  // char numReceived = keypad.getKey(); //@TODO: Move to keypadEvent() if possible
  
  // Serial.print("Key pressed was ");
  // Serial.println(numReceived);
  
  // delay(LOOP_DELAY);
  displayAlarmBreach();
  delay(3000);
  displayHome(0);
  delay(3000);
  displayHome(1);
  delay(3000);
  displayAccessStatus(0,0);
  delay(3000);
  displayAccessStatus(0,1);
  delay(3000);
  displayAccessStatus(1,1);
  delay(3000);
  displayAccessStatus(1,0);
  delay(3000);
}


// void keypadEvent(KeypadEvent key){
//   //@TODO: Put receive keys here?
// }


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


void displayAccessStatus(boolean codeCheck, boolean systemState) {
  lcd.clear();
  lcd.setCursor(0, 0);

  if(codeCheck) {
    lcd.print("ACCESS GRANTED");
  } else {
    lcd.print("ACCESS DENIED");
  }

  lcd.setCursor(0, 1);

  if(systemState) {
    lcd.print("SYSTEM ARMED");
  } else {
    lcd.print("SYSTEM DISARMED");
  }
}