#include "SensorHistory.h"

void SensorHistory::init() {
  _data.end = 0;
  _data.start = 0;

  for (uint8_t i = 0; i < SENSOR_HISTORY_SIZE; i++) {
    _data.values[i] = NAN;
  }
}

void SensorHistory::load(uint8_t offset) {
  // read data from rtcMemory
  ESP.rtcUserMemoryRead(offset, (uint32_t *)&_data, sizeof(_data));
}

void SensorHistory::save(uint8_t offset) {
  // read data from rtcMemory
  ESP.rtcUserMemoryWrite(offset, (uint32_t *)&_data, sizeof(_data));
}

void SensorHistory::addValue(float value) {
  _data.values[_data.end] = value;

  // move index to next value
  _data.end = (_data.end + 1) % SENSOR_HISTORY_SIZE;

  if (_data.end == _data.start) {
    // in this case the array is full, and we loose the oldest value
    _data.start = (_data.start + 1) % SENSOR_HISTORY_SIZE;
  }
};

uint8_t SensorHistory::getSize() {
  uint8_t ret = 0;

  if (_data.end > _data.start) {
    // example: [1^, 2, 3*, 4]  with *end, ^start
    // 2 - 0 = 2, as we have values [1,2]
    ret = _data.end - _data.start;
  } else {
    // example: [1, 2, 3*, 4^]  with *end, ^start
    // 4 - (3 - 2 - 1) = 4, as we have values [4,1,2,3]
    ret = SENSOR_HISTORY_SIZE - (_data.start - _data.end - 1);
  }

  return ret;
};

uint8_t SensorHistory::memSize() { return sizeof(_data); }

float SensorHistory::getValue(uint8_t index) {
  return _data.values[(_data.start + index) % SENSOR_HISTORY_SIZE];
};

float SensorHistory::getLastValue() {
  return _data.values[(_data.end - 1) % SENSOR_HISTORY_SIZE];
};

void SensorHistory::copyValues(float *values) {
  for (uint8_t i = 0; i < getSize(); i++) {
    values[i] = _data.values[(_data.start + i) % SENSOR_HISTORY_SIZE];
  }
}

float SensorHistory::getAverage() {
  float sum = 0;
  uint8_t size = getSize();

  for (uint8_t i = 0; i < size; i++) {
    sum += _data.values[(_data.start + i) % SENSOR_HISTORY_SIZE];
  }

  return sum / size;
}
