#include "util.h"

int max(int x, int y) { if (x > y) return x; return y;}

int min(int x, int y) {if (x > y) return y; return x;}

int word_delim(int ch) {
    return (ch == ' ' || ch == '(' || ch == ')' || ch == '[' || ch == ']' ||
        ch == '{' || ch == '}' || ch == '-' || ch == '_' || ch == '<' ||
        ch == '>' || ch == '=' || ch == '\'' ||  ch == ',' || ch == '.' || ch == '\n');
}

void shift_right(size_t start, Row *row) {
    row->length++;
    for (size_t i = row->length; i > start; i--) {
        row->line[i] = row->line[i-1];
    }    
}

void shift_left(size_t start, Row *row) {
    for (size_t i = start-1; i < row->length; i++) {
        row->line[i] = row->line[i+1];
    }
    row->length--;
    row->line[row->length] = '\0';
}

void add_char(Buffer *buffer, char ch) {
    int index = buffer->cc;
    Row *row = &buffer->rows[buffer->cr];
    shift_right(index, row);
    row->line[index] = ch;
    buffer->cc++;
}

int char_at_cursor(Buffer *buffer) {
    return buffer->rows[buffer->cr].line[buffer->cc];
}        
