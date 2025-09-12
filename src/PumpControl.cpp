#include "PumpControl.h"
#include "LogBuffer.h"
#include <TimeLib.h>   // for weekday(), hour(), minute()

extern LogBuffer logger;

PumpControl::PumpControl() {
  for (int i = 0; i < NUM_PUMPS; i++) {
    pumps[i] = {false, 0, 0, 255, 1.0f, 0.0f};
  }
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    schedules[i] = {false, 0, 0, 0, 0.0f, 0x7F}; // default all days enabled
  }
}

void PumpControl::begin() {
  // configure GPIO pins (PWM outputs)
  for (int i = 0; i < NUM_PUMPS; i++) {
    pinMode(i + 5, OUTPUT); // example: pumps on GPIO5+
    digitalWrite(i + 5, LOW);
  }
}

void PumpControl::update() {
  unsigned long now = millis();

  // check running pumps
  for (int i = 0; i < NUM_PUMPS; i++) {
    auto &p = pumps[i];
    if (p.running && now - p.startTime >= p.duration) {
      stopPump(i);
      logger.add("Pump " + String(i) + " stopped (completed cycle)");
    }
  }

  // check schedules
  runSchedule();
}

void PumpControl::startPump(int idx, unsigned long durationMs) {
  if (idx < 0 || idx >= NUM_PUMPS) return;
  auto &p = pumps[idx];
  p.running = true;
  p.startTime = millis();
  p.duration = durationMs;
  p.deliveredML += (p.mlPerSec * durationMs / 1000.0f);
  analogWrite(idx + 5, p.duty);
  logger.add("Pump " + String(idx) + " started (" + String(durationMs) + " ms)");
}

void PumpControl::stopPump(int idx) {
  if (idx < 0 || idx >= NUM_PUMPS) return;
  auto &p = pumps[idx];
  p.running = false;
  analogWrite(idx + 5, 0);
}

void PumpControl::primePump(int idx, unsigned long durationMs) {
  startPump(idx, durationMs);
}

void PumpControl::purgePump(int idx, unsigned long durationMs) {
  if (idx < 0 || idx >= NUM_PUMPS) return;
  auto &p = pumps[idx];
  p.running = true;
  p.startTime = millis();
  p.duration = durationMs;
  analogWrite(idx + 5, p.duty);
  logger.add("Pump " + String(idx) + " purge (" + String(durationMs) + " ms)");
  // NOTE: if reverse polarity purge is needed, add H-bridge support here
}

PumpState PumpControl::getPumpState(int idx) const {
  if (idx < 0 || idx >= NUM_PUMPS) return PumpState();
  return pumps[idx];
}

unsigned long PumpControl::getRemainingTime(int idx) const {
  if (idx < 0 || idx >= NUM_PUMPS) return 0;
  const auto &p = pumps[idx];
  if (!p.running) return 0;
  unsigned long elapsed = millis() - p.startTime;
  return (elapsed >= p.duration) ? 0 : (p.duration - elapsed);
}

ScheduleEntry PumpControl::getSchedule(int idx) const {
  if (idx < 0 || idx >= MAX_SCHEDULES) return ScheduleEntry();
  return schedules[idx];
}

void PumpControl::setSchedule(int idx, bool enabled, int pump, int hour, int minute, float ml, uint8_t daysMask) {
  if (idx < 0 || idx >= MAX_SCHEDULES) return;
  schedules[idx] = {enabled, pump, hour, minute, ml, daysMask};
}

void PumpControl::runSchedule() {
  static int lastMinute = -1;
  int curMinute = minute();
  int curHour = hour();
  int curWeekday = weekday(); // Sunday=1 .. Saturday=7
  uint8_t mask = 1 << ((curWeekday - 1) & 7);

  if (curMinute == lastMinute) return; // prevent double triggering within same minute
  lastMinute = curMinute;

  for (int i = 0; i < MAX_SCHEDULES; i++) {
    auto &se = schedules[i];
    if (!se.enabled) continue;
    if (se.hour == curHour && se.minute == curMinute) {
      if ((se.daysMask & mask) != 0) {
        unsigned long durationMs = (unsigned long)(se.ml / pumps[se.pump].mlPerSec * 1000.0f);
        startPump(se.pump, durationMs);
        logger.add("Schedule " + String(i) + " ran pump " + String(se.pump) + " (" + String(se.ml, 3) + " ml)");
      }
    }
  }
}
