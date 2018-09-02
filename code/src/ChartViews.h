#ifndef ChartViews_h
#define ChartViews_h

#define MAX_VALUES 24

#include <Arduino.h>
#include <U8g2lib.h>

class LineChart {
public:
  LineChart(int x, int y, int width, int height, float minRange);
  void draw(U8G2 *u8g2, float *values, uint8_t valuesSize,
            uint8_t valuesMaxSize);

private:
  int _x, _y, _width, _height, _numEntries;
  float _minRange;
};

#endif // ifndef ChartViews_h
