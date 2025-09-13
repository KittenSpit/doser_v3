#pragma once
#include <Arduino.h>
#include "Config.h"


struct PumpState {
    bool running;             // is pump running
    unsigned long startTime;  // when it started
    unsigned long duration;   // how long to run
    int duty;                 // PWM duty
    float mlPerSec;           // calibration
    float deliveredML;        // delivered volume
};



struct ScheduleEntry {
  bool enabled;
  int pump;
  int hour;
  int minute;
  float ml;
  uint8_t daysMask;  // bitmask for weekdays (0x7F = all days)
};

class PumpControl {
public:
  PumpControl();

  void begin();
  void update();

  void startPump(int idx, unsigned long durationMs);
  void stopPump(int idx);
  void primePump(int idx, unsigned long durationMs);
  void purgePump(int idx, unsigned long durationMs);
  void configPump(int idx, float mlPerSec);

  PumpState getPumpState(int idx) const;
  unsigned long getRemainingTime(int idx) const;

  ScheduleEntry getSchedule(int idx) const;
  void setSchedule(int idx, bool enabled, int pump, int hour, int minute, float ml, uint8_t daysMask = 0x7F);

private:
  PumpState pumps[NUM_PUMPS];
  ScheduleEntry schedules[MAX_SCHEDULES];

  void runSchedule();
};
