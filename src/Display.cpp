#include "Display.h"
#include "Config.h"
#include "Logger.h"
#include <WiFi.h>
void DisplayManager::begin(){
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!disp.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    logEvent("DISPLAY","ssd1306 init failed");
  } else {
    disp.clearDisplay();
    disp.setTextSize(1);
    disp.setTextColor(SSD1306_WHITE);
    disp.setCursor(0,0);
    disp.println("Aquarium Doser");
    disp.display();
  }
}
void DisplayManager::update(){
  disp.clearDisplay();
  disp.setCursor(0,0);
  disp.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  for(int i=0;i<NUM_PUMPS;i++){
    auto s = pc.getPumpState(i);
unsigned long rem = 0;
if (s.running) {
    unsigned long elapsed = millis() - s.startTime;
    if (elapsed < s.duration) {
        rem = (s.duration - elapsed) / 1000UL;
    }
}
disp.printf("P%d %s %lds d:%d\n", i, s.running ? "RUN" : "OFF", rem, s.duty);
  }
  disp.display();
}
