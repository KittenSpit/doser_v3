#include "Logger.h"
Logger logger;
void Logger::add(const String &evt, const String &msg){
  buffer[head_] = { millis(), evt, msg };
  head_ = (head_+1) % LOG_CAPACITY;
  if(count_ < LOG_CAPACITY) count_++;
}
String Logger::toJson(){
  String out = "[";
  for(int i=0;i<count_;i++){
    int idx = (head_ - count_ + i + LOG_CAPACITY)%LOG_CAPACITY;
    auto &e = buffer[idx];
    out += "{\"ts\":" + String(e.ts) + ",\"event\":\"" + e.event + "\",\"msg\":\"" + e.msg + "\"}";
    if(i<count_-1) out += ",";
  }
  out += "]";
  return out;
}
String Logger::toCSV(){
  String out = "ts,event,msg\n";
  for(int i=0;i<count_;i++){
    int idx = (head_ - count_ + i + LOG_CAPACITY)%LOG_CAPACITY;
    auto &e = buffer[idx];
    out += String(e.ts) + "," + e.event + ","" + e.msg + ""\n";
  }
  return out;
}
