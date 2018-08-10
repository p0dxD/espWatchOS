#include "ESPWatchOS.h"
#include "WebTime.h"
#include "defines.h"   // create from defines.h.example
#include <BME280I2C.h> // Library manager -> BME280 by tyler glenn
#include <ESP8266WiFi.h>
#include <RtcDS1307.h> // Library manager -> Rtc_by_Makuna
#include <U8g2lib.h>   // Library manager -> U8g2 by oliver
#include <Wire.h>

#define PIN_SDA 1
#define PIN_SCL 3

#define PIN_CLK 14
#define PIN_SDI 13
#define PIN_CS 0
#define PIN_DC 4

#define PIN_BTN 2

// Sources:
// https://github.com/olikraus/u8g2/wiki/gallery#5-feb-2017-ssd1607-200x200-eink
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// https://github.com/olikraus/u8g2/wiki/fntlistall#20-pixel-height

RtcDS1307<TwoWire> Rtc(Wire);

BME280I2C bme;
BME280I2C::Settings settings(BME280::OSR_X1, BME280::OSR_X1, BME280::OSR_X1,
                             BME280::Mode_Sleep, BME280::StandbyTime_1000ms,
                             BME280::Filter_Off, BME280::SpiEnable_False,
                             0x76 // I2C address. I2C specific.
);

U8G2_SSD1607_200X200_F_4W_SW_SPI u8g2(U8G2_R2, PIN_CLK, PIN_SDI, PIN_CS,
                                      PIN_DC);

void ESPWatchOS::begin() {
  pinMode(PIN_BTN, INPUT);

  Wire.begin(PIN_SDA, PIN_SCL);
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

    // first subtract 2000/01/01 00:00 as the constructor of RtcDateTime
    // requires this then add two hours to get my local timezone, maybe you need
    // to adapt this line
    Rtc.SetDateTime(
        RtcDateTime((webUnixTime(client) - 946684800) + (2 * 60 * 60)));
    if (!Rtc.GetIsRunning()) {
      Rtc.SetIsRunning(true);
    }

    _usedWifi = true;
  } else {
    _usedWifi = false;
  }

  // disable wifi
  WiFi.forceSleepBegin();
  delay(1);
  u8g2.initDisplay();
  if (_usedWifi) {
    u8g2.clearDisplay();
  }

  screen = u8g2;
}

RtcDateTime ESPWatchOS::getDateTime() { return Rtc.GetDateTime(); }
void ESPWatchOS::setMemory(uint8_t memoryAddress, uint8_t value) {
  Rtc.SetMemory(memoryAddress, value);
}
uint8_t ESPWatchOS::getMemory(uint8_t memoryAddress) {
  return Rtc.GetMemory(memoryAddress);
}

void ESPWatchOS::readSensors(float &pres, float &temp, float &hum) {
  // if you dont like metrics: thange the params below
  bme.read(pres, temp, hum, BME280::TempUnit_Celsius, BME280::PresUnit_hPa);
}

bool ESPWatchOS::isButtonDown() {
  int btnState = digitalRead(PIN_BTN);
  if (btnState == LOW) {
    delay(DEBOUNCE_MILLIS);
    return true;
  }

  return false;
}

bool ESPWatchOS::usedWifi() { return _usedWifi; }
void ESPWatchOS::beginDraw() {
  u8g2.setPowerSave(0); // before drawing, enable charge pump (req. 300ms)
  u8g2.clearBuffer();
}

void ESPWatchOS::endDraw() {
  u8g2.sendBuffer();
  u8g2.setPowerSave(1); // set power save mode: disable charge pump
}

void ESPWatchOS::testDraw() {
  beginDraw();
  u8g2.setFont(u8g2_font_fub11_tf);
  u8g2.drawStr(0, 100, "Test");
  endDraw();
}

void ESPWatchOS::sleep(uint64_t sleepMicros) {
  settings.mode = BME280::Mode_Sleep;
  bme.setSettings(settings);
  ESP.deepSleep(sleepMicros, WAKE_RF_DISABLED); // 60 sec snooze
}