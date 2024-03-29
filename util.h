#include "canvas.h"
#include "string.h"

#ifndef UTIL_H
#define UTIL_H

int word_delim(int ch);
int max(int x, int y);
int min(int x, int y);
int get_paren(int ch);
int paren_direction(int ch);

void shift_right(size_t start, Row *row);
void shift_left(size_t start, Row *row);
void shift_rows_up(size_t from, Buffer* buffer);
void shift_rows_down(size_t from, Buffer *buffer);

#endif