#include "canvas.h"


#ifndef ACTIONS_H
#define ACTIONS_H



int get_char_at_cursor(Buffer *buffer);

int replace_one_char(Buffer *buffer, int ch);
void delete_line_at_cursor(Buffer *buffer);
int delete_char_before_cursor(Buffer *buffer);
int delete_char_at_cursor(Buffer *buffer);

void move_cursor_find_char(Buffer *buffer, int find);
void move_cursor_find_char_backward(Buffer *buffer, int find);
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

int insert_char_at_cursor(Buffer *buffer, int ch);
int insert_char_at_cursor_and_move(Buffer *buffer, int ch);


#endif