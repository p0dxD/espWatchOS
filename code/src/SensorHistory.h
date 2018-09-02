#ifndef SENSOR_HISTORY_H
#define SENSOR_HISTORY_H
#include <Arduino.h>

#ifndef SENSOR_HISTORY_SIZE
#define SENSOR_HISTORY_SIZE 24
#endif
struct sensor_data {
  uint8_t start;
  uint8_t end;
  float values[SENSOR_HISTORY_SIZE];
};

/**
 * This class reads and writes sensor data to the rtc memory of an esp8266.
 */
class SensorHistory {
private:
  sensor_data _data;

public:

  void init();
  void load(uint8_t offset);
  void save(uint8_t offset);

  void addValue(float value);
  uint8_t getSize();
  uint8_t memSize();
  float getValue(uint8_t index);
  float getLastValue();
  void copyValues(float* values);
  float getAverage();
};

#endif