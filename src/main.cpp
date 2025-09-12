#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <LittleFS.h>

#include "Config.h"
#include "PumpControl.h"
#include "Logger.h"
#include "Display.h"

PumpControl pumpControl;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DisplayManager displayMgr(display, pumpControl);

extern void webui_begin();
extern void webui_handle();

unsigned long lastSchedCheck = 0;

void setup(){
  Serial.begin(115200);
  delay(50);
  if(!LittleFS.begin()){ Serial.println("LittleFS mount failed"); }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  int tries=0;
  while(WiFi.status()!=WL_CONNECTED && tries<30){ delay(500); Serial.print("."); tries++; }
  Serial.println();
  if(WiFi.status()==WL_CONNECTED) logEvent("WIFI","connected "+WiFi.localIP().toString());
  else logEvent("WIFI","connect_failed");
  configTime(0,0,"pool.ntp.org");
  pumpControl.begin();
  displayMgr.begin();
  webui_begin();
  ArduinoOTA.setHostname("doser-esp32");
  ArduinoOTA.onStart([](){ logEvent("OTA","start"); });
  ArduinoOTA.onEnd([](){ logEvent("OTA","end"); });
  ArduinoOTA.begin();
  logEvent("SYSTEM","setup complete");
}

void loop(){
  ArduinoOTA.handle();
  pumpControl.update();
  webui_handle();
  displayMgr.update();
  if(millis() - lastSchedCheck > 30000){
    lastSchedCheck = millis();
    time_t nowt = time(nullptr);
    struct tm tm;
    localtime_r(&nowt, &tm);
    int hh = tm.tm_hour; int mm = tm.tm_min;
    for(int i=0;i<MAX_SCHEDULES;i++){
      auto s = pumpControl.getSchedule(i);
      auto se = pumpControl.getSchedule(i); 
      if(!s.enabled) continue;
      if(s.hour==hh && s.minute==mm){
        int p = s.pump;
        auto s = pumpControl.getPumpState(p);
        float mlps = s.mlPerSec;
        unsigned long sec = 5;
        if(mlps > 0.0f) sec = (unsigned long) ceil(se.ml / mlps);
        pumpControl.startPump(p, sec * 1000UL);
        logEvent("SCHEDULE_RUN", "slot " + String(i) + " pump " + String(p) + " ml=" + String(se.ml));
      }
    }
  }
}
