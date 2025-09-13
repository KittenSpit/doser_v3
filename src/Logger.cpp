#include "Logger.h"

Logger logger;

// ========== Private helpers ==========

void Logger::jsonEscapeAppend(String& out, const String& s) {
  // Minimal JSON string escaper
  for (uint32_t i = 0; i < s.length(); i++) {
    char c = s[i];
    switch (c) {
      case '\"': out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b";  break;
      case '\f': out += "\\f";  break;
      case '\n': out += "\\n";  break;
      case '\r': out += "\\r";  break;
      case '\t': out += "\\t";  break;
      default:
        // Control chars (0x00..0x1F) should be escaped as \u00XX
        if ((uint8_t)c < 0x20) {
          char buf[7];
          snprintf(buf, sizeof(buf), "\\u%04X", (unsigned)c);
          out += buf;
        } else {
          out += c;
        }
    }
  }
}

String Logger::csvEscape(const String& s) {
  String out = s;
  out.replace("\"", "\"\""); // double quotes inside
  if (out.indexOf(',') != -1 || out.indexOf('"') != -1 || out.indexOf('\n') != -1 || out.indexOf('\r') != -1) {
    out = "\"" + out + "\"";
  }
  return out;
}

int Logger::idxFromOldest(int i) const {
  // Map 0..count_-1 (oldest..newest) to buffer index
  return (head_ - count_ + i + LOG_CAPACITY) % LOG_CAPACITY;
}

// ========== Public API ==========

void Logger::add(const String& evt, const String& msg) {
  LogEntry& e = buffer_[head_];
  e.ts    = millis();
  e.event = evt;
  e.msg   = msg;

  head_ = (head_ + 1) % LOG_CAPACITY;
  if (count_ < LOG_CAPACITY) count_++;
}

void Logger::addf(const String& evt, const char* fmt, ...) {
  char tmp[256]; // keep modest to avoid heap churn; adjust if needed
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  add(evt, String(tmp));
}

String Logger::toJson(bool pretty) const {
  const char* nl  = pretty ? "\n" : "";
  const char* sp1 = pretty ? "  " : "";
  const char* sp2 = pretty ? "    " : "";

  String out;
  out.reserve(count_ * 48); // rough guess to reduce reallocs
  out += "[";
  if (pretty && count_ > 0) out += nl;

  for (int i = 0; i < count_; i++) {
    if (pretty) out += sp1;
    out += "{";
    if (pretty) out += nl, out += sp2;

    // "ts"
    out += "\"ts\":";
    out += String(buffer_[idxFromOldest(i)].ts);

    // ,"event":"..."
    out += ",";
    if (pretty) out += nl, out += sp2;
    out += "\"event\":\"";
    jsonEscapeAppend(out, buffer_[idxFromOldest(i)].event);
    out += "\"";

    // ,"msg":"..."
    out += ",";
    if (pretty) out += nl, out += sp2;
    out += "\"msg\":\"";
    jsonEscapeAppend(out, buffer_[idxFromOldest(i)].msg);
    out += "\"";

    if (pretty) out += nl, out += sp1;
    out += "}";
    if (i < count_ - 1) out += ",";
    if (pretty) out += nl;
  }

  out += "]";
  return out;
}

String Logger::toCSV() const {
  String out;
  out.reserve(count_ * 32);
  out += "ts,event,msg\n";
  for (int i = 0; i < count_; i++) {
    const LogEntry& e = buffer_[idxFromOldest(i)];
    out += String(e.ts);
    out += ",";
    out += csvEscape(e.event);
    out += ",";
    out += csvEscape(e.msg);
    out += "\n";
  }
  return out;
}

void Logger::clear() {
  head_  = 0;
  count_ = 0;
}

bool Logger::get(int i, LogEntry& outEntry) const {
  if (i < 0 || i >= count_) return false;
  outEntry = buffer_[idxFromOldest(i)];
  return true;
}
