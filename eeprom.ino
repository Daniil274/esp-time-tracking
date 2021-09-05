#include <LiquidCrystal_I2C.h>

#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <AT24CX.h>

#define i2c_address 0x50
DS1307RTC Clock;
AT24C32 eep;
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool work = false;

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
void getUARTtime(){
  if (Serial.available() > 0) {
     String tim = Serial.readString();
     time_t time = tim.toInt();
     Clock.set(time);
    }
}
void printTime(){
  tmElements_t nowTM;
  Clock.read(nowTM);
  lcd.setCursor(11,0);
  lcd.print(addZero(nowTM.Hour));
  lcd.print(":");
  lcd.print(addZero(nowTM.Minute));

}
void printDate(){
  tmElements_t nowTM;
  Clock.read(nowTM);
  lcd.setCursor(8, 1);
  lcd.print(addZero(nowTM.Day));
  lcd.print(".");
  lcd.print(addZero(nowTM.Month));
  lcd.print(".");
  lcd.print(1970+nowTM.Year-2000);
}
void printStatus(){
  lcd.setCursor(0,0);
  if(work==false) lcd.print("Stopped!");
  else{
    lcd.print("Running!");
  }
}
void buttonListen(){
  if(digitalRead(11)){
    work=!work;
    delay(500);
  }
}
String addZero(uint8_t number){
  if(number>=10) return String(number);
  else{
   return "0"+String(number);
  }
}
void setup() {
  //Start serial
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  getUARTtime();
  printTime();
  printDate();
  printStatus();
  buttonListen();
}
