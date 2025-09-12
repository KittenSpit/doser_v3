#pragma once
#include <Adafruit_SSD1306.h>
#include "PumpControl.h"
class DisplayManager {
public:
  DisplayManager(Adafruit_SSD1306 &d, PumpControl &pc):disp(d),pc(pc){}
  void begin();
  void update();
private:
  Adafruit_SSD1306 &disp;
  PumpControl &pc;
};
