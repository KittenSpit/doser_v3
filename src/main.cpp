#include <Arduino.h>
//#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>
#include "FS.h"
#include <LittleFS.h>

#include "Config.h"
#include "PumpControl.h"
#include "Logger.h"
#include "Display.h"
#include "WebUI.h"

PumpControl pumpControl;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DisplayManager displayMgr(display, pumpControl);


//extern void webui_begin();
//extern void webui_handle();

unsigned long lastSchedCheck = 0;

void setup(){
  Serial.begin(115200);
  delay(50);
  Serial.println("\n\nDoser v3 starting...");
  delay(50);
  if(!LittleFS.begin(true)){ Serial.println("LittleFS mount failed"); }
  else { Serial.println("LittleFS mounted"); 
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    Serial.println("LittleFS open");
    while (file) {
        Serial.printf("FILE: %s (%d bytes)\n", file.name(), file.size());
        file = root.openNextFile();
    }
        Serial.println("LittleFS close");
  }

  WiFi.mode(WIFI_STA);
  
  Serial.println("Connecting to WiFi SSID: " + String(WIFI_SSID));
  Serial.println("WiFi password: " + String(WIFI_PASS));
  Serial.println("Connecting WiFi");
  delay(500);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries=0;
  delay(500);
  while(WiFi.status()!=WL_CONNECTED && tries<30){ 
    delay(50); 
    tries++; 
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  if(WiFi.status()==WL_CONNECTED) logEvent("WIFI","connected "+WiFi.localIP().toString());
  else logEvent("WIFI","connect_failed");


  
  configTime(0,0,"pool.ntp.org");
  Serial.println("PumpControl Begin");  
  pumpControl.begin();
  Serial.println("PumpControl Success.... displaMgr Begin");  
  displayMgr.begin();
  Serial.println("displayMgr Success.... webui Begin");
  webui_begin();
Serial.println("webui Success");

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
