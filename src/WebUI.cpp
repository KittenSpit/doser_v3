#include "WebUI.h"
#include "PumpControl.h"
#include "Config.h"
#include "LogBuffer.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Arduino.h>

extern PumpControl pumpControl;
extern LogBuffer logger;

// Declare global async server
AsyncWebServer server(80);

void webui_begin() {
    // Serve index.html with authentication
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(WEB_USER, WEB_PASS)) {
            return request->requestAuthentication();
        }

        if (LittleFS.exists("/index.html")) {
            request->send(LittleFS, "/index.html", "text/html");
        } else {
            request->send(500, "text/plain", "index.html missing");
        }
            server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    });

    // Pump/system status as JSON
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
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
        out += "]";
        out += ",\"logs\":" + logger.toJson();
        out += "}";
        request->send(200, "application/json", out);
    });

    // Logs as JSON
    server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", logger.toJson());
    });

    // Logs as CSV download
    server.on("/logs.csv", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/csv", logger.toCsv());
        response->addHeader("Content-Disposition", "attachment; filename=logs.csv");
        request->send(response);
    });

    // Prime pump
    server.on("/prime", HTTP_GET, [](AsyncWebServerRequest *request) {
        int pump = request->getParam("pump")->value().toInt();
        int sec  = request->getParam("sec")->value().toInt();
        pumpControl.primePump(pump, sec * 1000);
//    logger.add("Prime pump " + String(pump) + " for " + String(sec) + "s");     
        request->send(200, "text/plain", "OK");
    });

    // Purge pump
    server.on("/purge", HTTP_GET, [](AsyncWebServerRequest *request) {
        int pump = request->getParam("pump")->value().toInt();
        int sec  = request->getParam("sec")->value().toInt();
        pumpControl.purgePump(pump, sec * 1000);
 //       logger.add("Purge pump " + String(pump) + " for " + String(sec) + "s");
        request->send(200, "text/plain", "OK");
    });

    // Start pump
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {
        int pump = request->getParam("pump")->value().toInt();
        int sec  = request->getParam("dur")->value().toInt();
        int dir  = request->getParam("dir")->value().toInt();       
        pumpControl.startPump(pump, sec * 1000);
 //       logger.add("Prime pump " + String(pump) + " for " + String(sec) + "s"); 
        request->send(200, "text/plain", "OK");
    });

    // Stop pump
    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
Serial.println("1a/3*** call to /start ***");
        int pump = request->getParam("pump")->value().toInt();       
Serial.print("1b/3*** call to /start *** pump:    ");      
Serial.println(pump);
        pumpControl.stopPump(pump);
Serial.println("2/3*** call to start pump excute...logger called ***");
 //       logger.add("Prime pump " + String(pump) + " for " + String(sec) + "s");
Serial.println("3/3*** legger excute...send OK ***");        
        request->send(200, "text/plain", "OK");
    });


    // Config pump
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
Serial.println("1a/3*** call to /config ***");
        int pump = request->getParam("pump")->value().toInt();   
        float mlps  = request->getParam("mlps")->value().toFloat();   
        pumpControl.configPump(pump,mlps);
Serial.println("2/3*** call to config pump excute...logger called ***");
 //       logger.add("Prime pump " + String(pump) + " for " + String(sec) + "s");
Serial.println("3/3*** legger excute...send OK ***");        
        request->send(200, "text/plain", "OK");
    });




    server.begin();
    Serial.println("Web UI started at http://" + WiFi.localIP().toString());
}

void webui_handle() {
    // Not needed for AsyncWebServer
}
