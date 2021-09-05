#include <DallasTemperature.h>

#include <LiquidCrystal_I2C.h>

#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <AT24CX.h>

#define i2c_address 0x50
DS1307RTC Clock;
AT24C32 eep;
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ONE_WIRE_BUS 2
 
// создаём объект для работы с библиотекой OneWire
OneWire oneWire(ONE_WIRE_BUS);
 
// создадим объект для работы с библиотекой DallasTemperature
DallasTemperature sensor(&oneWire);

// Специальный объект для хранения адреса устройства

bool work = false;

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
  if(!work) lcd.print("Stopped!");
  else{
    lcd.print("Running!");
  }
}
void buttonListen(){
  if(digitalRead(11)){
    work=!work;
    printStatus();
    delay(500);
  }
}
String addZero(uint8_t number){
  if(number>=10) return String(number);
  else{
   return "0"+String(number);
  }
}
void printTemp(){
    float temperature;
  // отправляем запрос на измерение температуры
  sensor.requestTemperatures();
  // считываем данные из регистра датчика
  temperature = sensor.getTempCByIndex(0);
  // выводим температуру в Serial-порт
  lcd.setCursor(0, 1);
  lcd.print(temperature);
  lcd.print((char)223);
}
void setup() {
  //Start serial
   sensor.begin();
  // устанавливаем разрешение датчика от 9 до 12 бит
  sensor.setResolution(12);
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
  printTemp();
}
