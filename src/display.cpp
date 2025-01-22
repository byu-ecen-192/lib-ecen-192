#include "display.h"

bool setup_display() {
    Yboard.display.clearDisplay();
    Yboard.display.setTextColor(1);
    Yboard.display.setRotation(0); // Can be 0, 90, 180, or 270
    Yboard.display.setTextWrap(true);
    Yboard.display.display();
    return true;
}

void display_text(const std::string &text, const int text_size) {
    Yboard.display.setTextSize(text_size);
    Yboard.display.clearDisplay();
    Yboard.display.setCursor(0, 0);
    Yboard.display.print(text.c_str());
    Yboard.display.display();
}

void clear_display() {
    Yboard.display.clearDisplay();
    Yboard.display.display();
}
