

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "ESPWatchOS.h"
#include "OSTime.h"

#define NUM_SCREENS 5;
#define DEFAULT_WAKETIME_ADD 5;

ESPWatchOS watch;
OSTime tick;

// all the technical stuff is hidden in the watch object
void setup(void) { watch.begin(); }

// available values for the screen
RtcDateTime now;
float pres = NAN;
float temp = NAN;
float hum = NAN;
long wakeTimeSeconds = 4;

// render functions
void drawScreen(int screenPtr) {
  watch.beginDraw();
  U8G2 s = watch.screen; // save some space on each line

  switch (screenPtr) {
  case 1:
    s.setFont(u8g2_font_logisoso46_tn);
    s.drawStr(200 / 4, 200 / 2 - 23,
              ((now.Hour() < 10 ? String(0) : String("")) +
               String(now.Hour()) + //
               ":" + (now.Minute() < 10 ? "0" : "") + String(now.Minute()))
                  .c_str());
    break;
  case 2:
    s.setFont(u8g2_font_logisoso46_tn);
    s.drawStr(200 / 4, 200 / 2 - 23, (String(temp) + " °C").c_str());
    break;
  case 3:
    s.setFont(u8g2_font_logisoso46_tn);
    s.drawStr(200 / 4, 200 / 2 - 23, (String(hum) + " %").c_str());
    break;
  case 4:
    s.setFont(u8g2_font_logisoso46_tn);
    s.drawStr(200 / 4, 200 / 2 - 23, (String(pres) + " mb").c_str());
    break;
  case 0:
    s.setFont(u8g2_font_fub11_tf);
    s.setDrawColor(1);
    s.drawStr(4, 20, "@pauls_3d_things");
    s.drawStr(4, 40, "200x200 e-ink Disp.");
    s.drawStr(4, 80, (String("Temp: ") + String(temp) + "C").c_str());
    s.drawStr(4, 100, (String("Hum:  ") + String(hum) + "%").c_str());
    s.drawStr(4, 120, (String("Pres:  ") + String(pres) + "mb").c_str());
    s.drawStr(4, 140,
              (String("Date:  ") + String(now.Year()) + "/" +
               String(now.Month()) + "/" + String(now.Day()))
                  .c_str());
    s.drawStr(
        4, 160,
        (String("Time: ") + String(now.Hour()) + ":" + String(now.Minute()))
            .c_str());
    s.drawStr(
        4, 180,
        (String("Used Wifi: ") + (watch.usedWifi() ? "yes" : "no")).c_str());
  }
  watch.endDraw();
}

// logic
void loop(void) {
  uint8_t screenPtr = watch.getMemory(0) % NUM_SCREENS;

  now = watch.getDateTime();
  watch.readSensors(pres, temp, hum);

  drawScreen(screenPtr);

  long bootTime = tick.getUptimeSeconds();
  // stay awake for user input (to change screens)
  while (((long)tick.getUptimeSeconds()) - bootTime < wakeTimeSeconds) {
    tick.tick();
    delay(50);

    if (watch.isButtonDown()) {
      wakeTimeSeconds += DEFAULT_WAKETIME_ADD;
      screenPtr = (screenPtr + 1) % NUM_SCREENS;
      watch.setMemory(0, screenPtr);
      
      drawScreen(screenPtr);
    }
  }

  watch.sleep(1000 * 1000 * 60 /* 1 minute */);
}