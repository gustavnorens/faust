#include "canvas.h"


#ifndef ACTIONS_H
#define ACTIONS_H

int char_at_cursor(Buffer *buffer);

void add_char_at_cursor(Buffer *buffer, char ch);

void move_cursor_find_char(Buffer *buffer, int find);
void move_cursor_forward(Buffer *buffer);
void move_cursor_backward(Buffer *buffer);
void move_cursor_start(Buffer *buffer);
void move_cursor_down(Buffer *buffer);
void move_cursor_up(Buffer *buffer);
void move_cursor_forward_word(Buffer *buffer);
void move_cursor_backward_word(Buffer *buffer);
void move_cursor_start_line(Buffer *buffer);
void move_cursor_end(Buffer *buffer);
void move_cursor_end_line(Buffer* buffer);

#endif