#include "LogBuffer.h"

LogBuffer::LogBuffer() : head(0), count(0) {}

void LogBuffer::add(const String &entry) {
  logs[head] = String(millis()) + "," + entry; // timestamp (ms) + entry
  head = (head + 1) % LOG_CAPACITY;
  if (count < LOG_CAPACITY) count++;
}

String LogBuffer::toJson() const {
  String out = "[";
  for (int i = 0; i < count; i++) {
    int idx = (head - count + i + LOG_CAPACITY) % LOG_CAPACITY;
    String log = logs[idx];
    int commaIndex = log.indexOf(',');
    String ts = log.substring(0, commaIndex);
    String msg = log.substring(commaIndex + 1);
    out += "{\"ts\":" + ts + ",\"msg\":\"" + msg + "\"}";
    if (i < count - 1) out += ",";
  }
  out += "]";
  return out;
}

String LogBuffer::toCsv() const {
  String out = "timestamp_ms,message\n";
  for (int i = 0; i < count; i++) {
    int idx = (head - count + i + LOG_CAPACITY) % LOG_CAPACITY;
    out += logs[idx] + "\n";
  }
  return out;
}
