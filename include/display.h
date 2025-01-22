#ifndef DISPLAY_H
#define DISPLAY_H

#include <string>
#include <yboard.h>

void display_text(const std::string &text, const int text_size);
void clear_display();
bool setup_display();

#endif // DISPLAY_H
