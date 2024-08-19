#include "display.h"

Adafruit_SSD1306 display;

bool setup_display(int address)
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
        return false;
    } 
    display.clearDisplay();
    display.setTextColor(1);
    display.setRotation(0); // Can be 0, 90, 180, or 270
    display.setTextWrap(true);
    display.display();
    return true;
}

void display_text(const std::string &text, const int text_size)
{
    display.setTextSize(text_size);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(text.c_str());
    display.display();  
}

void clear_display()
{
    display.clearDisplay();
    display.display();
}


