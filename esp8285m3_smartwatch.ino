
#include "defines.h" // create from defines.h.example

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

//#include <OSHelper.h> // https://github.com/pauls-3d-things/iod-os-v2 -> install to Arduino/libraries/
//#include <OSTime.h>

#include "WebTime.h"
#include <U8g2lib.h> // Library manager -> U8g2 by oliver
#include <BME280I2C.h> // Library manager -> BME280 by tyler glenn

#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

U8G2_SSD1607_200X200_F_4W_SW_SPI u8g2(U8G2_R2, /* CLK/clock=*/ 14, /* SDI/data=*/ 13, /* cs=*/ 0, /* dc=*/ 4);  // eInk/ePaper Display

// Sources:
// https://github.com/olikraus/u8g2/wiki/gallery#5-feb-2017-ssd1607-200x200-eink
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// https://github.com/olikraus/u8g2/wiki/fntlistall#20-pixel-height
BME280I2C bme;
BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Sleep,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   0x76 // I2C address. I2C specific.
);
bool usedWifi = false;

void setup(void) {
  Wire.begin(1 /*SDA*/, 3/*SCL*/);
  u8g2.begin();
  while (!bme.begin()) {
    delay(500); // TODO: blink an error code
  }
  settings.mode = BME280::Mode_Forced;
  bme.setSettings(settings);



  if (!Rtc.IsDateTimeValid()) {

    uint8_t tries = 0;
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);

    while (WiFi.status() != WL_CONNECTED) {

      if (tries % 20 == 0) {
        WiFi.begin(WIFI_SSID, WIFI_PASS);
      }

      delay(500);
      tries++;
    }

    WiFiClient client;

    webUnixTime(client) + (2 * 60 * 60); // UTC +2

    // first subtract 2000/01/01 00:00 as the constructor of RtcDateTime requires this
    // then add two hours to get my local timezone, maybe you need to adapt this line
    Rtc.SetDateTime(RtcDateTime((webUnixTime(client) - 946684800) + (2 * 60 * 60)));

    if (!Rtc.GetIsRunning())
    {
      Rtc.SetIsRunning(true);
    }


    // disable wifi
    WiFi.forceSleepBegin();
    delay(1);
    usedWifi = true;
  } else {
    // disable wifi
    WiFi.forceSleepBegin();
    delay(1);
    usedWifi = false;
  }
}

void loop(void) {
  RtcDateTime now = Rtc.GetDateTime();

  float pres = NAN;
  float temp = NAN;
  float hum = NAN;
  float alt = NAN;

  bme.read(pres, temp, hum, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);

  u8g2.setPowerSave(0);  // before drawing, enable charge pump (req. 300ms)
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_fub11_tf);
  u8g2.setDrawColor(1);
  u8g2.drawStr( 4, 20, "@pauls_3d_things");
  u8g2.drawStr( 4, 40, "200x200 e-ink Disp.");
  u8g2.drawStr( 4, 60, "BME280");
  u8g2.drawStr( 4, 80,  (String("Temp: ") + String(temp) + "C").c_str());
  u8g2.drawStr( 4, 100, (String("Hum:  ") + String(hum) + "%").c_str());
  u8g2.drawStr( 4, 120, (String("Pres:  ") + String(pres) + "mb").c_str());
  u8g2.drawStr( 4, 140, (String("Date:  ") + String(now.Year()) + "/" + String(now.Month()) + "/" + String(now.Day())).c_str());
  u8g2.drawStr( 4, 160, (String("Time: ") + String(now.Hour()) + ":" + String(now.Minute())).c_str());
  u8g2.drawStr( 4, 180, (String("Used Wifi: ") + (usedWifi ? "yes" : "no")).c_str());

  u8g2.sendBuffer();
  u8g2.setPowerSave(1);  // set power save mode: disable charge pump
  
  settings.mode = BME280::Mode_Sleep;
  bme.setSettings(settings);
  Rtc.SetIsRunning(false);
  ESP.deepSleep(1000 * 1000 * 60, WAKE_RF_DISABLED); // 10 sec snooze
}
