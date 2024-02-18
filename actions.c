#include "canvas.h"
#include "util.h"

int get_char_at_cursor(Buffer *buffer) {
    return buffer->rows[buffer->cr].line[buffer->cc];
}

void move_cursor_forward(Buffer *buffer) {
    size_t len = buffer->rows[buffer->cr].length;
    if (buffer->cr + 1 != buffer->length) {
        if (buffer->cc < len - 1) {
            buffer->cc++;
        }
        else {
            buffer->cr++;
            buffer->cc = 0;
        }
    }
    else {
        if (buffer->cc < len) {
            buffer->cc++;
        }
    }
}
void move_cursor_backward(Buffer *buffer) {
    if (buffer->cr > 0) {
        if (buffer->cc > 0) {
            buffer->cc--;
        }
        else {
            buffer->cr--;
            buffer->cc = buffer->rows[buffer->cr].length - 1;
        }
    }
    else {
        if (buffer->cc > 0) {
            buffer->cc--;
        }
    }
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
void move_cursor_end_line(Buffer *buffer) {
    if (buffer->cr != buffer->length - 1) {
        buffer->cc = buffer->rows[buffer->cr].length-1;
    }
    else {
        buffer->cc = buffer->rows[buffer->cr].length;
    }
}
void move_cursor_start_buffer(Buffer *buffer) {
    buffer-> cr = 0;
    buffer-> cc = 0;
}
void move_cursor_end_buffer(Buffer *buffer) {
    buffer->cr = buffer->length - 1;
    buffer->cc = 0;
}
void move_cursor_find_char(Buffer *buffer, int find) {
    int cr = buffer->cr;
    int cc = buffer->cc;
    move_cursor_forward(buffer);
    int ch = get_char_at_cursor(buffer);

    while (ch != find && ch != '\0') {
        move_cursor_forward(buffer);
        ch = get_char_at_cursor(buffer);
    }
    if (ch != find) {
        buffer->cr = cr;
        buffer->cc = cc;
    }
}
void move_cursor_find_char_backward(Buffer *buffer, int find) {
    int cr = buffer->cr;
    int cc = buffer->cc;

    move_cursor_backward(buffer);
    int ch = get_char_at_cursor(buffer);

    while (ch != find && (buffer->cr != 0 || buffer->cc != 0))  {
        move_cursor_backward(buffer);
        ch = get_char_at_cursor(buffer);
    }

    if (ch != find) {
        buffer->cr = cr;
        buffer->cc = cc;
    }
}
void move_cursor_forward_word(Buffer *buffer) {
    move_cursor_forward(buffer);
    int ch = get_char_at_cursor(buffer);

    while (word_delim(ch)) {
        move_cursor_forward(buffer);
        ch = get_char_at_cursor(buffer);
        if (ch == '\0') {
            break;
        }
    }


    while (!word_delim(ch)) {
        move_cursor_forward(buffer);
        ch = get_char_at_cursor(buffer);
        if (ch == '\0') {
            break;
        }
    }
    /* size_t i = buffer->cr;
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
        move_cursor_end_buffer(buffer);
        move_cursor_end_line(buffer);
    }
    else {                    
        buffer->cr = i;
        buffer->cc = j;   
    } */
}
void move_cursor_backward_word(Buffer *buffer) {
    move_cursor_backward(buffer);
    int ch = get_char_at_cursor(buffer);

    while (word_delim(ch)) {
        move_cursor_backward(buffer);
        ch = get_char_at_cursor(buffer);
        if (buffer->cr == 0 && buffer->cc == 0) {
            break;
        }
    }

    while (!word_delim(ch) ) {
        move_cursor_backward(buffer);
        ch = get_char_at_cursor(buffer);
        if (buffer->cr == 0 && buffer->cc == 0) {
            return;
        }
    }
    move_cursor_forward(buffer);
}
void move_cursor_next_paren(Buffer *buffer) {
    int paren = get_char_at_cursor(buffer);
    int matching = get_paren(paren);
    int dir = paren_direction(paren);
    int ch = paren;

    if (matching) {
        int cr = buffer->cr;
        int cc = buffer->cc;
        int count = 1;

        if (dir) {
            while (!(ch == matching && count == 0) && ch != '\0'){
                move_cursor_forward(buffer);
                ch = get_char_at_cursor(buffer);
                if (ch == paren) {
                    count++;
                }
                else if (ch == matching) {
                    count--;
                }
            }
        }
        else {
            while (!(ch == matching && count == 0) && !(buffer->cr == 0 && buffer->cc == 0)) {
                move_cursor_backward(buffer);
                ch = get_char_at_cursor(buffer);
                if (ch == paren) {
                    count++;
                }
                else if (ch == matching) {
                    count--;
                }
            }
        }
        
        if (ch != matching) {
            buffer->cr = cr;
            buffer->cc = cc;
        }
    }
}

void delete_line_at_cursor(Buffer *buffer) {
    if (buffer->length >  1) {
        shift_rows_up(buffer->cr, buffer);
        buffer->cc = 0;
        if (buffer->cr == buffer->length) {
            buffer->cr++;
        }
    }
    else {
        buffer->cc = 0;
        buffer->rows[buffer->cr].line[0] = '\0';
        buffer->rows[buffer->cr].length = 0;
    }
    
}
int delete_char_before_cursor(Buffer *buffer) {
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
        above->length += cur->length - 1;
        shift_rows_up(buffer->cr, buffer);

        buffer->cc = save_len -1;
        buffer->cr--;
        return 1;
    }
    else {
        return -1;
    }
}
int delete_char_at_cursor(Buffer *buffer) {
    Row *cur = &buffer->rows[buffer->cr];
    if (buffer->cc == cur->length) {
        return -1;
    }
    else if (get_char_at_cursor(buffer) != '\n') {
        shift_left(buffer->cc + 1, cur);
        return 0;
    }
    else {
        Row *below = &buffer->rows[buffer->cr + 1];
        for (size_t i = 0; i < below->length; i++) {
            cur->line[cur->length - 1 + i] = below->line[i];
        }
        cur->length += below->length - 1;
        shift_rows_up(buffer->cr + 1, buffer);
        return 1;
    }
}

int insert_char_at_cursor(Buffer *buffer, int ch) {
    if (ch == '\n') {
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
        return 1;
    }
    else {
        int index = buffer->cc;
        Row *row = &buffer->rows[buffer->cr];
        shift_right(index, row);
        row->line[index] = ch;
        return 0;
    }    
}
int insert_char_at_cursor_and_move(Buffer *buffer, int ch) {
    int res = insert_char_at_cursor(buffer, ch);
    if (res) {
        buffer->cr++;
        buffer->cc = 0;
    }
    else {
        buffer->cc++;
    }
    return res;
}

int replace_one_char(Buffer *buffer, int ch) {
    int x = delete_char_at_cursor(buffer);
    int y = insert_char_at_cursor(buffer, ch);
    return y ^ x;
}
