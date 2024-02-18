#include "util.h"

int max(int x, int y) { if (x > y) return x; return y;}

int min(int x, int y) {if (x > y) return y; return x;}

int word_delim(int ch) {
    return (ch == ' ' || ch == '(' || ch == ')' || ch == '[' || ch == ']' ||
        ch == '{' || ch == '}' || ch == '-' || ch == '_' || ch == '<' ||
        ch == '>' || ch == '=' || ch == '\'' ||  ch == ',' || ch == '.' || ch == '\n');
}

int get_paren(int ch) {
    switch (ch){
    case '(':
        return ')';
        break;
    case ')':
        return '(';
        break;
    case '{':
        return '}';
        break;
    case '}':
        return '{';
        break;
    case '[':
        return ']';
        break;
    case ']':
        return '[';
        break;
    default:
        return 0;
        break;
    }
}
int paren_direction(int ch) {
    if (ch == '(' || ch == '{' || ch == '[') {
        return 1;
    }
    else {
        return 0;
    }

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

void shift_rows_down(size_t from, Buffer *buffer) {
    for (size_t i = buffer->length; i > from; i--) {
        memcpy(buffer->rows[i].line, buffer->rows[i-1].line, buffer->rows[i-1].length + 1);
        buffer->rows[i].length = buffer->rows[i-1].length;
    }
    buffer->rows[from].line[0] = '\n';
    buffer->rows[from].line[1] = '\0';
    buffer->rows[from].length = 1;
    buffer->length++;
}

void shift_rows_up(size_t from, Buffer *buffer) {
    for (size_t i = from + 1; i < buffer->length; i++) {
            memcpy(buffer->rows[i-1].line, buffer->rows[i].line, buffer->rows[i].length + 1);
            buffer->rows[i-1].length = buffer->rows[i].length;
    }
    buffer->rows[buffer->length-1].line[0] = '\0';
    buffer->rows[buffer->length-1].length = 0;
    buffer->length--;
}
