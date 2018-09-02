#include "ChartViews.h"

#include <Arduino.h>
#include <Math.h>
static float getMin(float values[], int valuesLength, int count) {
  float ret = values[0];
  for (int i = 0; i < count; i++) {
    ret = ret < values[i] ? ret : values[i];
  }
  return ret;
}

static float getMax(float values[], int valuesLength, int count) {
  float ret = values[0];
  for (int i = 0; i < count; i++) {
    ret = ret > values[i] ? ret : values[i];
  }
  return ret;
}

LineChart::LineChart(int x, int y, int width, int height, float minRange) {
  _x = x;
  _y = y;
  _width = width;
  _height = height;
  _minRange = minRange;
}

void LineChart::draw(U8G2 *u8g2, float *values, uint8_t numValues,
                     uint8_t valuesLen) {
  float max = getMax(values, valuesLen, numValues);
  float min = getMin(values, valuesLen, numValues);

  float shift = -min;
  float range = max - min;

  // the 0.00001 is to trick hlines into the middle
  range = range == 0.0 ? 0.00001 : range;
  float drawRange = range < _minRange ? _minRange : range;
  // normalization, drawing the line in the middle of the screen
  float rm = range / drawRange;
  float ro = _height * (1.0 - (range / drawRange)) / 2.0;

  for (int i = 1; i < numValues; i++) {

    float x1 = _width - _width //
                            * ((float)(numValues - i)) /
                            ((float)valuesLen - 1.0);
    float y1 = _height - _height * ((values[i - 1] + shift) / range) * rm - ro;

    float x2 = _width - _width //
                            * ((float)(numValues - i) - 1.0) /
                            ((float)valuesLen - 1.0);
    float y2 = _height - _height * ((values[i] + shift) / range) * rm - ro;

    u8g2->drawLine(_x + x1, _y + y1, _x + x2, _y + y2);
    // u8g2->drawCircle(_x + x1, _y + y1, 3);
  }

  u8g2->drawFrame(_x, _y, _width, _height);
}
