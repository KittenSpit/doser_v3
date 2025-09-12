#pragma once
#include <Arduino.h>

#define LOG_CAPACITY 50   // max number of log entries kept in memory

class LogBuffer {
public:
  LogBuffer();

  void add(const String &entry);
  String toJson() const;
  String toCsv() const;

private:
  String logs[LOG_CAPACITY];
  int head;
  int count;
};
