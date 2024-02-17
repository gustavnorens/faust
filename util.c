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

int delete_char_at_cursor(Buffer *buffer) {
    Row *cur = &buffer->rows[buffer->cr];
    if (buffer->cc > 0) {
        shift_left(buffer->cc, cur);
        buffer->cc--;
        return 0;
    }
    else if (buffer->cr != 0) {
        Row *above = &buffer->rows[buffer->cr - 1];
        int save_len = above->length;
        for (size_t i = 0; i < cur->length; i++) {
            above->line[i + above->length - 1] = cur->line[i];
        }
        above->length = cur->length + above->length - 1;
        shift_rows_up(buffer->cr, buffer);
        
        buffer->cr--;
        buffer->cc = save_len-1;
        return 1;
    }
}

void insert_newline_at_cursor(Buffer *buffer) {
    shift_rows_down(buffer->cr + 1, buffer);

    Row *upper = &buffer->rows[buffer->cr];
    Row *lower = &buffer->rows[buffer->cr+1];

    for (size_t i = buffer->cc; i < upper->length+1; i++) {
        lower->line[i-buffer->cc] = upper->line[i];
    }

    upper->line[buffer->cc] = '\n';
    upper->line[buffer->cc+1] = '\0';
    lower->length = upper->length - buffer->cc;
    upper->length = buffer->cc + 1;

    buffer->cr++;
    buffer->cc = 0;
}

void insert_char_at_cursor(Buffer *buffer, char ch) {
    if (ch == '\n') {
        insert_newline_at_cursor(buffer);
    }
    else {
        int index = buffer->cc;
        Row *row = &buffer->rows[buffer->cr];
        shift_right(index, row);
        row->line[index] = ch;
        buffer->cc++;
    }    
}

int get_char_at_cursor(Buffer *buffer) {
    return buffer->rows[buffer->cr].line[buffer->cc];
}        
