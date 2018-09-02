#ifndef ESPWATCHOS_H
#define ESPWATCHOS_H

#define DEBOUNCE_MILLIS 125

#include "SensorHistory.h"
#include <RtcDS1307.h> // Library manager -> Rtc_by_Makuna
#include <U8g2lib.h>   // Library manager -> U8g2 by oliver

class ESPWatchOS {
private:
  bool _usedWifi = false;

public:
  SensorHistory presHistory = SensorHistory();
  SensorHistory tempHistory = SensorHistory();
  SensorHistory humHistory = SensorHistory();
  U8G2 screen;

  void begin();
  void sleep(uint64_t sleepMicros);

  RtcDateTime getDateTime();
  void setMemory(uint8_t memoryAddress, uint8_t value);
  uint8_t getMemory(uint8_t memoryAddress);

  void readSensors(float &pres, float &temp, float &hum);
  void updateSensorHistory(bool forceSave);

  bool isButtonDown();
  bool usedWifi();

  void beginDraw();
  void endDraw();

  void testDraw();
};

#endif