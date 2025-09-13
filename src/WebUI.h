#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "FS.h"
#include <LittleFS.h>

// Forward declarations
void setupWebUI();
void handleRoot();
void handleStatus();
void handleLogs();
void handleLogsDownload();
void handlePrime();
void handlePurge();
void handleSchGet();
void handleSchSet();
void webui_begin();
void webui_handle();