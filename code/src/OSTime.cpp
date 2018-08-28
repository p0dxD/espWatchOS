#include <Arduino.h>
#include <OSTime.h>
#include <limits.h>

OSTime::OSTime() {
  _millisLast    = 0;
  _uptimeSeconds = 0;
  tick();
}

/**
 * Add all passed seconds (diffSeconds) to the counters. In most cases there are
 * milliseconds left (deltaMillis) we need to add next time.
 */
unsigned long OSTime::tick() {
  unsigned long millisNow = millis();
  unsigned long diffMillis = 0, diffSeconds = 0, deltaMillis = 0;

  if (_millisLast < millisNow) {
    diffMillis  = millisNow - _millisLast;
    diffSeconds = diffMillis / 1000;
    deltaMillis = diffMillis % 1000;

    _uptimeSeconds += diffSeconds;
    _unixtime      += diffSeconds;
  } else {
    // overflow case
    diffMillis  = /*overflow*/ millisNow + /*rest*/ (ULONG_MAX - _millisLast);
    diffSeconds = diffMillis / 1000;
    deltaMillis = diffMillis % 1000;

    _uptimeSeconds += diffSeconds;
    _unixtime      += diffSeconds;
  }

  _millisLast = millisNow - deltaMillis;

  unsigned long ret =  diffMillis - _lastDelta; /* problem: millisLast is too
                                                   old */
  _lastDelta = deltaMillis;
  return ret;
}

unsigned long OSTime::getUptimeSeconds() {
  return _uptimeSeconds;
}

unsigned long OSTime::getUnixTime() {
  return _unixtime;
}

void OSTime::setUnixTime(unsigned long t) {
  _unixtime = t;
}
