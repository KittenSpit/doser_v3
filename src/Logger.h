#pragma once
#include <Arduino.h>
#include "Config.h"

struct LogEntry {
  unsigned long ts;
  String event;
  String msg;
};

class Logger {
public:
  void add(const String &evt, const String &msg);
  String toJson();
  String toCSV();
  int count(){ return count_; }
  LogEntry get(int i){ return buffer[(head_ - count_ + i + LOG_CAPACITY)%LOG_CAPACITY]; }
private:
  LogEntry buffer[LOG_CAPACITY];
  int head_=0;
  int count_=0;
};

extern Logger logger;
inline void logEvent(const String &e,const String &m){ logger.add(e,m); }
