
#include "Arduino.h"
#include "display.h"
#include "yboard.h"

void setup() {
    Serial.begin(9600);
    Yboard.setup();
    setup_display();
}

void loop() { display_text("Hello, World!", 2); }
