#include <LiquidCrystal.h>

LiquidCrystal lcd(9, 8, 5, 4, 3, 2); 
// RS E D4 D5 D6 D7
void setup(){
  //Serial.begin(9600);
  lcd.begin(16, 2);
}

void loop(){
  delay(4000);
  alarmTriggeredMessage();
  delay(4000);
  invalidCodeMessage();
  delay(4000);
  systemArmedMessage();
  delay(4000);
  systemDisarmedMessage();
  delay(4000);
  codeEntryMessage();
  delay(4000);
}


void alarmTriggeredMessage(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("  ALARM TRIGGERED  ");
}

void invalidCodeMessage(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" INVALID CODE! ");
  lcd.setCursor(0,1);
  lcd.print("   TRY AGAIN!   ");
}

void systemArmedMessage(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SYSTEM ARMED!"); 
}

void systemDisarmedMessage(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SYSTEM DISARMED!");
}

void codeEntryMessage(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Security System");
  lcd.setCursor(0,1);
  lcd.print("Enter PIN:");
}
