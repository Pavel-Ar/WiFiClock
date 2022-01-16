#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <SPI.h>
#include <DMD2.h>

#include <fonts/SystemFont5x7.h>

#define pin_A 16
#define pin_B 12
#define pin_sclk 5
#define pin_clk 14
#define pin_r 13
#define pin_noe 15

#define DISPLAYS_WIDE 1
#define DISPLAYS_HIGH 1
#define LEAP_YEAR(Y)( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

const char *ssid     = "ssid";
const char *password = "pass";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.openwrt.pool.ntp.org", 10800, 600000);

//NTPClient timeClient(ntpUDP, "0.openwrt.pool.ntp.org", 10800, 600000);
//NTPClient timeClient(ntpUDP);

SPIDMD dmd(DISPLAYS_WIDE, DISPLAYS_HIGH, pin_noe, pin_A, pin_B, pin_sclk);
DMD_TextBox box(dmd, 1, 0, 31, 16);

unsigned long myTimer1;
String getTime();
String getDate();

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  if (WiFi.getAutoConnect() != true){
      WiFi.setAutoConnect(true);
      WiFi.setAutoReconnect(true);
  }

  Serial.println(F("\nWiFi Connected"));
  timeClient.begin();

  dmd.setBrightness(0);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
}

void loop() {
    while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print ( "." );
    //ESP.restart();
  }
    //delay(3000);
    if (millis() - myTimer1 >= 1000) {
      myTimer1 = millis();
      box.clear();
      box.flush();        //дожидается окончания отправки буфера. А у Вас он видимо переполняется
      box.print(getTime());
      box.print(getDate());
    }
}

String getTime(){
      timeClient.update();
      unsigned long utime = timeClient.getEpochTime();
      unsigned long hours = (utime % 86400L) / 3600;
      String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
      unsigned long minutes = (utime % 3600) / 60;
      String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

      uint8_t brihgtness = ( (hours > 8) && (hours < 20) ) ? 2 : 1;
      dmd.setBrightness(brihgtness);

      Serial.println(timeClient.getFormattedTime());
      return hoursStr+":"+minuteStr;
}

String getDate() {
  unsigned long utime = timeClient.getEpochTime();
  unsigned long rawTime = utime / 86400L;  // in days
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
