

#include "ChartViews.h"
#include "ESPWatchOS.h"
#include "OSTime.h"
#include "SensorHistory.h"
#include "images/drop.h"
#include "images/logo.h"
#include "images/sun.h"
#include "images/thermo.h"
#include "images/watchface.h"
#include "images/watchface_roman.h"
#include "images/watchface_square.h"
#include <Arduino.h>
#include <Geometry.h>
#include <SPI.h>
#include <Wire.h>

#define NUM_SCREENS 8;
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
float presValues[SENSOR_HISTORY_SIZE];
float tempValues[SENSOR_HISTORY_SIZE];
float humValues[SENSOR_HISTORY_SIZE];

LineChart chartTemp = LineChart(0, 132, 100, 68, 5);
LineChart chartPres = LineChart(100, 132, 100, 68, 30);

long wakeTimeSeconds = 6;

uint8_t yMid(uint8_t fontHeight) { return (200 / 2) + (fontHeight / 2); }

uint8_t xMid(uint8_t fontWidth, uint8_t len) {
  // -1 looks better
  return (200 / 2) - (len * fontWidth / 2) - 1;
}

void drawBigCentered(String str) {
  if (str.length() <= 6) {
    watch.screen.setFont(u8g2_font_inb38_mr);
    watch.screen.drawStr(xMid(30, str.length()), yMid(51), str.c_str());
  } else if (str.length() <= 8) {
    watch.screen.setFont(u8g2_font_inb30_mr);
    watch.screen.drawStr(xMid(24, str.length()), yMid(41), str.c_str());
  } else if (str.length() <= 10) {
    watch.screen.setFont(u8g2_font_inb24_mr);
    watch.screen.drawStr(xMid(20, str.length()), yMid(33), str.c_str());
  } else {
    watch.screen.setFont(u8g2_font_inb16_mf);
    watch.screen.drawStr(xMid(14, str.length()), yMid(26), str.c_str());
  }
}

void drawSmall(String str, uint8_t offsetX, uint8_t offsetY, bool xCentered,
               bool yCentered, const uint8_t *font, uint8_t fontWidth,
               uint8_t fontHeight) {
  uint8_t x = (xCentered ? xMid(fontWidth, str.length()) : 0) + offsetX;
  uint8_t y = (yCentered ? yMid(fontHeight) : 0) + offsetY;
  watch.screen.setFont(font);
  watch.screen.drawStr(x, y, str.c_str());
}

void drawHandle(float width, float length, float div, Rotation r, Point c) {
  Point p0, p1, p2, p3;
  p0.X() = 0;
  p0.Y() = 0;

  p1.X() = -width / 2;
  p1.Y() = -length * div;

  p2.X() = width / 2;
  p2.Y() = -length * div;

  p3.X() = 0;
  p3.Y() = -length;

  p0 = r * p0 + c;
  p1 = r * p1 + c;
  p2 = r * p2 + c;
  p3 = r * p3 + c;

  watch.screen.drawLine(p0.X(), p0.Y(), p1.X(), p1.Y());
  watch.screen.drawLine(p0.X(), p0.Y(), p2.X(), p2.Y());
  watch.screen.drawLine(p3.X(), p3.Y(), p1.X(), p1.Y());
  watch.screen.drawLine(p3.X(), p3.Y(), p2.X(), p2.Y());
}

void drawWatchFace(const uint8_t *bits) {

  Point c;
  c.X() = 100;
  c.Y() = 100;

  Rotation rh, rm;
  rh.RotateZ(2 * PI *
             (((now.Hour() > 12 ? now.Hour() - 12 : now.Hour()) / 12.0) +
              (now.Minute() / 60.0) / 12.0));
  rm.RotateZ(2 * PI * (now.Minute() / 60.0));

  watch.screen.drawXBM(0, 0, 200, 200, bits);
  watch.screen.setFont(u8g2_font_fub14_tn);
  watch.screen.drawStr(
      134, 107,
      String(String(now.Day() < 9 ? " " : "") + String(now.Day())).c_str());
  drawSmall(String(temp, 1) + "C", 0, 125, true, false, u8g2_font_profont15_mf,
            7, 15);

  for (int i = 0; i < 9; i++) {
    drawHandle(i, 50, 0.6, rh, c);
  }

  for (int i = 0; i < 5; i++) {
    drawHandle(i, 75, 0.4, rm, c);
  }
}
// render functions
void drawScreen(int screenPtr) {
  String str;
  String dow;
  watch.beginDraw();
  U8G2 s = watch.screen; // save some space on each line

  s.setColorIndex(1);
  s.drawBox(0, 0, 200, 200);
  s.setColorIndex(0);

  watch.screen.setDrawColor(0);

  switch (screenPtr) {
  case 1:

    /* write background pattern, then: */
    str = String((now.Hour() < 10 ? String(0) : String("")) +
                 String(now.Hour()) + //
                 ":" + (now.Minute() < 10 ? "0" : "") + String(now.Minute()));

    switch (now.DayOfWeek()) {
    case 1:
      dow = "Monday";
      break;
    case 2:
      dow = "Tuesday";
      break;
    case 3:
      dow = "Wednesday";
      break;
    case 4:
      dow = "Thursday";
      break;
    case 5:
      dow = "Friday";
      break;
    case 6:
      dow = "Saturday";
      break;
    case 0:
      dow = "Sunday";
      break;
    default:
      dow = "No-Day";
    }
    drawSmall(dow + " " + String(now.Year()) + "-" + String(now.Month()) + "-" +
                  String(now.Day()),
              0, 20, true, false, u8g2_font_t0_22_tf, 11, 21);

    drawBigCentered(str);
    // drawSmall(str, 0, 0, false, true, u8g2_font_logisoso54_tf, 69, 85);

    watch.updateSensorHistory(false);
    drawSmall(String(watch.presHistory.getLastValue(), 1) + " (" +
                  String(watch.presHistory.getAverage(), 1) + ")",
              101, 142, false, false, u8g2_font_t0_11_tf, 6, 11);
    drawSmall(String(watch.tempHistory.getLastValue(), 1) + " (" +
                  String(watch.tempHistory.getAverage(), 1) + ")",
              1, 142, false, false, u8g2_font_t0_11_tf, 6, 11);

    watch.presHistory.copyValues(presValues);
    watch.tempHistory.copyValues(tempValues);
    watch.humHistory.copyValues(humValues);

    chartPres.draw(&watch.screen, presValues, watch.presHistory.getSize(),
                   SENSOR_HISTORY_SIZE);
    chartTemp.draw(&watch.screen, tempValues, watch.tempHistory.getSize(),
                   SENSOR_HISTORY_SIZE);

    // humChart.draw(&watch.screen, humValues, watch.humHistory.getSize(),
    //               SENSOR_HISTORY_SIZE);

    break;
  case 2:
    s.drawXBM(76, 24, 48, 48, thermo_bits);
    drawBigCentered(String(temp, 1));
    drawSmall(String("Temp. (C)"), 0, 190, true, false, u8g2_font_inb16_mf, 14,
              26);
    break;
  case 3:
    s.drawXBM(76, 24, 48, 48, drop_bits);
    drawBigCentered(String(hum, 1));
    drawSmall(String("Humid. (%)"), 0, 190, true, false, u8g2_font_inb16_mf, 14,
              26);

    break;
  case 4:
    s.drawXBM(76, 24, 48, 48, sun_bits);
    drawBigCentered(String(pres, 1));
    drawSmall(String("Pres.(mb)"), 0, 190, true, false, u8g2_font_inb16_mf, 14,
              26);
    break;
  case 5:
    drawWatchFace(face_bits);
    break;
  case 6:
    drawWatchFace(facesquare_bits);
    break;
  case 7:
    drawWatchFace(faceroman_bits);
    break;
  case 0:
  default:
    s.drawXBM(0, 0, 200, 200, logo_bits);
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
    delay(10);

    if (watch.isButtonDown()) {
      wakeTimeSeconds += DEFAULT_WAKETIME_ADD;
      screenPtr = (screenPtr + 1) % NUM_SCREENS;
      watch.setMemory(0, screenPtr);

      drawScreen(screenPtr);
    }
  }

  uint8_t h = watch.getDateTime().Hour();
  if (h > 9 && h < 21) {
    watch.sleep(1000 * 1000 * 60 /* 1 minute */);
  } else {
    watch.sleep(1000 * 1000 * 60 * 30 /* 30 minutes */);
  }
}