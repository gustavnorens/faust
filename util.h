#include "canvas.h"

#ifndef UTIL_H
#define UTIL_H

int word_delim(int ch);
int max(int x, int y);
int min(int x, int y);

void shift_right();
void shift_left();
/* void shift_rows_up();
void shift_rows_down(); */

void add_char(Buffer *buffer, char ch);
int char_at_cursor(Buffer *buffer);

#endif