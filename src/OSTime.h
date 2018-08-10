#ifndef OSTime_h
#define OSTime_h

class OSTime {
public:

  OSTime();

  unsigned long getUptimeSeconds();
  unsigned long getUnixTime();
  void          setUnixTime(unsigned long t);
  unsigned long tick();

private:

  unsigned long _millisLast;
  unsigned long _uptimeSeconds;
  unsigned long _unixtime;
  unsigned long _lastDelta;
  char *uptimeStr;
};

#endif // ifndef OSTime_h
