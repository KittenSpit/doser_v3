#include "WebUI.h"
#include "PumpControl.h"
#include "Config.h"
#include "LogBuffer.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

extern PumpControl pumpControl;
extern WebServer server;
extern LogBuffer logger;

void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "index.html missing");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleStatus() {
  String out = "{";
  out += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  out += "\"pumps\":[";
  for (int i = 0; i < NUM_PUMPS; i++) {
    auto s = pumpControl.getPumpState(i);
    unsigned long rem = pumpControl.getRemainingTime(i);
    out += "{";
    out += "\"idx\":" + String(i) + ",";
    out += "\"running\":" + String(s.running ? "true" : "false") + ",";
    out += "\"remaining_ms\":" + String(rem) + ",";
    out += "\"duty\":" + String(s.duty) + ",";
    out += "\"ml_per_sec\":" + String(s.mlPerSec, 6) + ",";
    out += "\"delivered_ml\":" + String(s.deliveredML, 6);
    out += "}";
    if (i < NUM_PUMPS - 1) out += ",";
  }
  out += "],\"schedules\":[";
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    auto se = pumpControl.getSchedule(i);
    out += "{";
    out += "\"idx\":" + String(i) + ",";
    out += "\"enabled\":" + String(se.enabled ? "true" : "false") + ",";
    out += "\"pump\":" + String(se.pump) + ",";
    out += "\"hour\":" + String(se.hour) + ",";
    out += "\"minute\":" + String(se.minute) + ",";
    out += "\"ml\":" + String(se.ml, 3);
    out += "}";
    if (i < MAX_SCHEDULES - 1) out += ",";
  }
  out += "],\"logs\":" + logger.toJson();
  out += "}";
  server.send(200, "application/json", out);
}

void handleLogs() {
  server.send(200, "application/json", logger.toJson());
}

void handleLogsDownload() {
  String csv = logger.toCsv();
  server.sendHeader("Content-Disposition", "attachment; filename=logs.csv");
  server.send(200, "text/csv", csv);
}

void handlePrime() {
  int pump = server.arg("pump").toInt();
  int sec = server.arg("sec").toInt();
  pumpControl.primePump(pump, sec * 1000);
  logger.add("Prime pump " + String(pump) + " for " + String(sec) + "s");
  server.send(200, "text/plain", "OK");
}

void handlePurge() {
  int pump = server.arg("pump").toInt();
  int sec = server.arg("sec").toInt();
  pumpControl.purgePump(pump, sec * 1000);
  logger.add("Purge pump " + String(pump) + " for " + String(sec) + "s");
  server.send(200, "text/plain", "OK");
}

void handleSchGet() {
  String out = "[";
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    auto se = pumpControl.getSchedule(i);
    out += "{";
    out += "\"idx\":" + String(i) + ",";
    out += "\"enabled\":" + String(se.enabled ? "true" : "false") + ",";
    out += "\"pump\":" + String(se.pump) + ",";
    out += "\"hour\":" + String(se.hour) + ",";
    out += "\"minute\":" + String(se.minute) + ",";
    out += "\"ml\":" + String(se.ml, 3);
    out += "}";
    if (i < MAX_SCHEDULES - 1) out += ",";
  }
  out += "]";
  server.send(200, "application/json", out);
}

void handleSchSet() {
  int idx = server.arg("idx").toInt();
  bool enabled = server.arg("enabled") == "true";
  int pump = server.arg("pump").toInt();
  int hour = server.arg("hour").toInt();
  int minute = server.arg("minute").toInt();
  float ml = server.arg("ml").toFloat();
  pumpControl.setSchedule(idx, enabled, pump, hour, minute, ml);
  logger.add("Set schedule " + String(idx));
  server.send(200, "text/plain", "OK");
}

void setupWebUI() {
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/logs", handleLogs);
  server.on("/logs.csv", handleLogsDownload);
  server.on("/prime", handlePrime);
  server.on("/purge", handlePurge);
  server.on("/sch_get", handleSchGet);
  server.on("/sch_set", handleSchSet);
  server.begin();
}


void webui_begin() {
  AsyncWebServer server(80);
    // Root page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<h1>Doser Web UI</h1><p>Welcome</p>");
    });

    // Example: JSON pump status
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        for (int i = 0; i < NUM_PUMPS; i++) {
            json += "\"pump" + String(i) + "\":";
            json += pumpControl.getRemainingTime(i);
            if (i < NUM_PUMPS - 1) json += ",";
        }
        json += "}";
        request->send(200, "application/json", json);
    });

    server.begin();
    Serial.println("Web UI started at http://" + WiFi.localIP().toString());
}

void webui_handle() {
    // For AsyncWebServer nothing is needed, but we keep it so main.cpp compiles
}