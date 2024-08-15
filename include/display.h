#ifndef DISPLAY_H
#define DISPLAY_H


#include <string>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



void display_text(const std::string &text, const int text_size);
void clear_display();
bool setup_display(int address);


#endif // DISPLAY_H