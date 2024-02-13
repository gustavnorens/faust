#include "canvas.h"
#include "util.h"

void move_cursor_find_char(Buffer *buffer, int find) {
    size_t i = buffer->cr;
    size_t j = buffer->cc + 1;
    int ch = 1; //Distinct from NULL
    while (ch != '\0') {
        ch = buffer->rows[i].line[j];
        if (ch == find) {
            buffer->cr = i;
            buffer->cc = j;
            break;
        }
        else if (ch == '\n') {
            i++;
            j = 0;
        }
        else {
            j++;
        }
    }
}

void move_cursor_find_char_backward(Buffer *buffer, int find) {
    int i = buffer->cr;
    int j = buffer->cc - 1;
    int ch = 1; //Distinct from NULL
    while (1) {
        ch = buffer->rows[i].line[j];
        if (ch == find) {
            buffer->cr = i;
            buffer->cc = j;
            break;
        }
        else if (i == 0 && j <= 0) {
            break;
        }
        else if (j == 0) {
            i--;
            j = buffer->rows[i].length - 1;    
        }
        else {
            j--;
        }
    }
}

void move_cursor_forward(Buffer *buffer) {
    size_t len = buffer->rows[buffer->cr].length;
    if (((buffer->cr == buffer->length - 1 && 
    buffer->cc < len) || 
    buffer->cc < len - 1) && len != 0) {
        buffer->cc++;
    }
}

void move_cursor_backward(Buffer *buffer) {
    if (buffer->cc > 0) {
        buffer->cc--;
    }
}

void move_cursor_start(Buffer *buffer) {
    buffer-> cr = 0;
    buffer-> cc = 0;
}

void move_cursor_down(Buffer *buffer) {
    if (buffer->cr < buffer->length - 1) {
        if (buffer->cr + 1 == buffer->length - 1) {
            buffer->cc = min(buffer->cc, buffer->rows[buffer->cr + 1].length);
            buffer->cr++;
        }
        else {
            buffer->cc = min(buffer->cc, buffer->rows[buffer->cr + 1].length - 1);
            buffer->cr++;
        }
    }
}

void move_cursor_up(Buffer *buffer) {
    if (buffer->cr > 0) {
        buffer->cc = min(buffer->cc, buffer->rows[buffer->cr - 1].length - 1);
        buffer->cr--;
    }
}

void move_cursor_start_line(Buffer *buffer) {
    buffer->cc = 0;
}

void move_cursor_end(Buffer *buffer) {
    buffer->cr = buffer->length - 1;
    buffer->cc = 0;
}

void move_cursor_end_line(Buffer *buffer) {
    if (buffer->cr != buffer->length - 1) {
        buffer->cc = buffer->rows[buffer->cr].length-1;
    }
    else {
        buffer->cc = buffer->rows[buffer->cr].length;
    }
}

void move_cursor_forward_word(Buffer *buffer) { //ugly but works for now
    size_t i = buffer->cr;
    size_t j = buffer->cc;
    int ch = buffer->rows[i].line[j];

    while (word_delim(ch)) {
        if (ch == '\n') {
            i++;
            j = 0;
        }
        else {
            j++;
        }
        ch = buffer->rows[i].line[j];
    }

    while (ch != '\0') {
        ch = buffer->rows[i].line[j];
        if (word_delim(ch)) {
            break;
        }
        else {
            j++;
        }
    }
    if (ch == '\0') {
        move_cursor_end(buffer);
        move_cursor_end_line(buffer);
    }
    else {                    
        buffer->cr = i;
        buffer->cc = j;   
    }
}

void move_cursor_backward_word(Buffer *buffer) { //ugly but works for now
    int i = buffer->cr;
    int j = buffer->cc;

    if (j == 0 && i != 0) {
        i--;
        j = buffer->rows[i].length-1;
    }
    else {
        j--;
    }

    int ch = buffer->rows[i].line[j];
    while (word_delim(ch)) {
        if (i == 0 && j == -1) {
            break;
        }
        else if (j == 0) {
            i--;
            j = buffer->rows[i].length-1;
        }
        else {
            j--;
        }
        ch = buffer->rows[i].line[j];
    }

    while (j != -1) {
        ch = buffer->rows[i].line[j];
        if (word_delim(ch)) {
            break;
        }
        else {
            j--;
        }
    }

    buffer->cr = i;
    buffer->cc = j + 1;
}