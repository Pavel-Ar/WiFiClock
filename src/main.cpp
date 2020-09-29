#include <SPI.h>
#include <DMD2.h>

#include <fonts/Arial_Black_16.h>
#include <fonts/Arial14.h>
#include <fonts/Droid_Sans_12.h>
#include <fonts/Droid_Sans_16.h>
#include <fonts/Droid_Sans_24.h>
#include <fonts/SystemFont5x7.h>

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define pin_A 16
#define pin_B 12
#define pin_sclk 5
#define pin_clk 14
#define pin_r 13
#define pin_noe 15

#define DISPLAYS_WIDE 1
#define DISPLAYS_HIGH 1
#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )
String getDate(unsigned long secs);

const int WIDTH = 1;
const uint8_t *FONT = SystemFont5x7;
const char *ssid     = "wifi";
const char *password = "psw";
uint32_t myTimer0, myTimer1;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3*60*60 ,6*60*60);

SPIDMD dmd(DISPLAYS_WIDE, DISPLAYS_HIGH, pin_noe, pin_A, pin_B, pin_sclk);
DMD_TextBox box(dmd, 1, 0, 32, 16);

void setup() {
  Serial.begin(115200);

    WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( '.' );
  }
  if (WiFi.getAutoConnect() != true){     //configuration will be saved into SDK flash area
    WiFi.setAutoConnect(true);            //on power-on automatically connects to last used hwAP
    WiFi.setAutoReconnect(true);          //automatically reconnects to hwAP in case it's disconnected
  }
  Serial.println(F("\nWiFi Connected"));

  timeClient.begin();

  dmd.setBrightness(10);
  dmd.selectFont(FONT);
  dmd.begin();
}

void loop() {
  timeClient.update();
  if (millis() - myTimer0 >= 10*1000) {
    myTimer0 = millis();
      unsigned long utime = timeClient.getEpochTime();
      unsigned long hours = (utime % 86400L) / 3600;
      String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
      unsigned long minutes = (utime % 3600) / 60;
      String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);
      unsigned long seconds = utime % 60;
      String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

      box.clear();
      box.print(hoursStr+":"+minuteStr);
      box.print(getDate(utime));
  }
  if (millis() - myTimer1 >= 60*1000) {
        myTimer1 = millis();
      Serial.println(timeClient.getFormattedTime()+" "+getDate(timeClient.getEpochTime()) );
  }
}

String getDate(unsigned long secs) {
  unsigned long rawTime = secs / 86400L;  // in days
  unsigned long days = 0, year = 1970;
  uint8_t month;
  static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

  while((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
    year++;
  rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
  days=0;
  for (month=0; month<12; month++) {
    uint8_t monthLength;
    if (month==1) { // february
      monthLength = LEAP_YEAR(year) ? 29 : 28;
    } else {
      monthLength = monthDays[month];
    }
    if (rawTime < monthLength) break;
    rawTime -= monthLength;
  }
  String monthStr = ++month < 10 ? "0" + String(month) : String(month); // jan is month 1
  String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month
  return dayStr+"/"+monthStr;
}