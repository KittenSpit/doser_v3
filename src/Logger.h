#pragma once
#include <Arduino.h>
#include "Config.h"


// Adjust to taste
#ifndef LOG_CAPACITY
#define LOG_CAPACITY 128
#endif

struct LogEntry {
  uint32_t ts;     // millis()
  String   event;  // short tag/category
  String   msg;    // details
};

class Logger {
public:
  Logger() : head_(0), count_(0) {}

  // Add an entry (now)
  void add(const String& evt, const String& msg = "");

  // printf-style message formatting
  void addf(const String& evt, const char* fmt, ...) __attribute__((format(printf, 3, 4)));

  // Export
  String toJson(bool pretty = false) const;
  String toCSV() const;

  // Maintenance
  void clear();
  int  size() const { return count_; }
  int  capacity() const { return LOG_CAPACITY; }

  // Random access (oldest=0 .. newest=size()-1)
  bool get(int i, LogEntry& out) const;

private:
  LogEntry buffer_[LOG_CAPACITY];
  int head_;   // next write position
  int count_;  // number of valid entries (<= capacity)

  // Helpers
  static void jsonEscapeAppend(String& out, const String& s);
  static String csvEscape(const String& s);

  // Index mapping: i-th oldest -> buffer index
  int idxFromOldest(int i) const;
};

extern Logger logger;
inline void logEvent(const String &e,const String &m){ logger.add(e,m); }