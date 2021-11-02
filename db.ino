#include <AsyncElegantOTA.h>
#include <Hash.h>
#include <elegantWebpage.h>
#include <FS.h>
#include <SPIFFS.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <StringArray.h>

#include <LiquidCrystal.h>
#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#define EEPROM_SIZE 12

const char *DBname = "/timeDB.csv";

const char *ssid = "ESP-SERVER";
const char *passwd = "2810002606";

int timeZone;
bool work;
uint arrivalMinute=0;
int stages;
struct setting {
  int timeZone;
  time_t arrivalTime;
  int timePerDayMin;
};

AsyncWebServer server(80);

DS1307RTC Clock;

LiquidCrystal lcd(19, 23, 2, 4, 18, 5);


void printInfo() {
  tmElements_t nowTM;
  Clock.read(nowTM);
  lcd.setCursor(11, 0);
  lcd.print(addZero(nowTM.Hour));
  lcd.print(":");
  lcd.print(addZero(nowTM.Minute));
  lcd.setCursor(8, 1);
  lcd.print(addZero(nowTM.Day));
  lcd.print(".");
  lcd.print(addZero(nowTM.Month));
  lcd.print(".");
  lcd.print(1970 + nowTM.Year - 2000);
  lcd.setCursor(0, 0);
  if (work == false) lcd.print("Stopped!");
  else {
    lcd.print("Running!");
    lcd.setCursor(0, 1);
    time_t nowTime = makeTime(nowTM);
    uint minuteOfWorkCount = nowTM.Hour*60+nowTM.Minute-arrivalMinute;
    uint hoursOfWork = trunc(minuteOfWorkCount/60);
    uint minutesOfWork = minutesOfWork - hoursOfWork*60;
    lcd.print(addZero(trunc(minutesOfWork/60)));
    lcd.print(":");
    lcd.print(addZero(minutesOfWork - trunc(minutesOfWork/60)*60));
  }
}
void buttonListen(fs::FS &fs) {
  if (digitalRead(33)){
    work = !work;
    lcd.setCursor(0, 0);
    printInfo();
    File db = fs.open(DBname, FILE_APPEND);
    if (!db) {
      Serial.println("Error writing file!");
    }
    tmElements_t nowTM;
    Clock.read(nowTM);
    if (work){
      db.print(addZero(nowTM.Day));
      db.print(".");
      db.print(addZero(nowTM.Month));
      db.print(".");
      db.print(1970 + nowTM.Year - 2000);
      db.print(",");
      db.print(addZero(nowTM.Hour));
      db.print(":");
      db.print(addZero(nowTM.Minute));
      db.print(",");
    }
    else{
      
      db.print(addZero(nowTM.Hour));
      db.print(":");
      db.println(addZero(nowTM.Minute));
    }
    db.close();
  }
  delay(500);
}
void dbInit(fs::FS &fs){
  File db = fs.open(DBname);
  if (!db) {
    Serial.println("Error open file!");
  }
  db.close();
}

String addZero(uint8_t number){
  if (number >= 10) return String(number);
  else {
    return "0" + String(number);
  }
}

bool isWork(fs::FS &fs) {
  String row;
  char s;
  File db = fs.open(DBname, FILE_READ);
  if (db.seek(db.size() - 1)) {
    s = db.read();
    Serial.print("Last char: `");
  Serial.write(s);
  Serial.println("`");
    if (s == '\n') {
      return false;
    }
    if (s == ',') {

      return true;
    }
  }
}
uint minuteOfWork(fs::FS &fs){
  File db = fs.open(DBname, FILE_READ);
  if (db.seek(db.size() - 6)){
    char timeIn[5];
    for(uint i = 0;i<6;i++){
    timeIn[i]=db.read();
    Serial.print(timeIn[i]);
    }
    db.close();
    Serial.print("Arrival time: ");
    for(uint i=0;i<6;i++){
      Serial.write(timeIn[i]);
    }
    Serial.println();
    return parseHM(timeIn);
  }
}
uint parseHM(char HourMinute[5]){
  uint hour = (HourMinute[0]-'0')*10+(HourMinute[1]-'0');
  uint minute = (HourMinute[3]-'0')*10+(HourMinute[4]-'0');
  return hour*60+minute; 
}
void setup() {
  //Start serial
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  WiFi.softAP(ssid, passwd);
  lcd.begin(16, 2);
  pinMode(33, INPUT);
  digitalWrite(33, LOW);
  dbInit(SPIFFS);
  //update status and settings
  //EEPROM.begin(EEPROM_SIZE);
  //setting startSetting;
  //EEPROM.get(0,startSetting);
  //update work status
  //timeZone = startSetting.timeZone;
  tmElements_t nowTM;
  //time_t lastTime = startSetting.arrivalTime;
  Clock.read(nowTM);
  //tmElements_t lastTM;
  //breakTime(lastTime,lastTM);
  work = isWork(SPIFFS);
  if(work) arrivalMinute = minuteOfWork(SPIFFS);
  // Serial.println(lastTime);
  Serial.print("Work:");
  Serial.println(work);
  Serial.print("Stages:");
  Serial.println(stages);
  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<script>window.location.href = '/set-time-in?time='+Math.trunc(Date.now()/1000)</script>");
  });
  server.on("/Style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/Style.css", "text/css");
  });

  server.on("/timedb", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, DBname, "text/plain");
  });
  server.on("/timedb.csv", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, DBname, String(), true);
  });


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/home.html", "text/html");
  });
  server.on("/set-time-in", HTTP_GET, [](AsyncWebServerRequest *request) {
    String time_s = request->getParam("time")->value();
    time_t time = time_s.toInt() + 3600 * timeZone;
    Clock.set(time);
    printInfo();
    request->send(200, "text/plain", "time updated to " + time_s);
  });
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(SPIFFS.usedBytes() / 1000) + "kB / " + String(SPIFFS.totalBytes() / 1000) + "kB");
  });
  AsyncElegantOTA.begin(&server);
  server.begin();
}

void loop() {
  printInfo();
  buttonListen(SPIFFS);
  AsyncElegantOTA.loop();
}
