#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

#define ctrl(x) ((x) & 0x1f)

#define BSPACE 127
#define ESC 27
#define ENTER 10

#define MAX_LINES 1024
#define MAX_LINE_SIZE 1024

typedef enum {
    NORMAL,
    INSERT
} Mode;

typedef struct {
    char *line;
    size_t length;
} Row;

typedef struct {
    Row rows[MAX_LINES];
    size_t cr;
    size_t cc;
    size_t length;
    char *filename;
} Buffer;

Mode mode = NORMAL;

int QUIT = 0;

char* mode_as_string(){
    switch(mode) {
        case (NORMAL):
            return "NORMAL";
            break;
        case (INSERT):
            return "INSERT";
            break;
        default:
            return "NORMAL";
            break;
    }
}

int max(int x, int y) { if (x > y) return x; return y;}
int min(int x, int y) {if (x > y) return y; return x;}

void shift_right(size_t start, Row *row) {
    row->length++;
    for (size_t i = row->length; i > start + 1; i--) {
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

void on_backspace(Buffer *buffer){
    Row *cur = &buffer->rows[buffer->cr];
    if (buffer->cc > 0) {
        shift_left(buffer->cc, cur);
        move(buffer->cr, --buffer->cc);        
        delch();
    }
    else if (buffer->cr != 0) {
        Row *above = &buffer->rows[buffer->cr - 1];
        int save_len = above->length;
        for (size_t i = 0; i < cur->length; i++) {
            above->line[i + above->length - 1] = cur->line[i];
        }

        above->length = cur->length + above->length - 1;
        for (size_t i = buffer->cr + 1; i < buffer->length; i++) {
            memcpy(buffer->rows[i-1].line, buffer->rows[i].line, buffer->rows[i].length + 1);
            buffer->rows[i-1].length = buffer->rows[i].length;
        }

        buffer->rows[buffer->length-1].line[0] = '\0';
        buffer->rows[buffer->length-1].length = 0;

        buffer->length--;

        for (size_t i = buffer->cr - 1; i < buffer->length + 1; i++) {
            move(i, 0);
            clrtoeol();
            printw(buffer->rows[i].line);
        }
        
        buffer->cc = save_len-1;
        move(--buffer->cr, buffer->cc);
    }
}

void on_enter(Buffer *buffer) {
    buffer->length++;
    for (size_t i = buffer->length-1; i > buffer->cr + 1; i--) {
        memcpy(buffer->rows[i].line, buffer->rows[i-1].line, buffer->rows[i-1].length + 1);
        buffer->rows[i].length = buffer->rows[i-1].length;
    }
    
    Row *upper = &buffer->rows[buffer->cr];
    Row *lower = &buffer->rows[buffer->cr+1];

    for (size_t i = buffer->cc; i < upper->length+1; i++) {
        lower->line[i-buffer->cc] = upper->line[i];
    }

    upper->line[buffer->cc] = '\n';
    upper->line[buffer->cc+1] = '\0';
    lower->length = upper->length - buffer->cc;
    upper->length = buffer->cc + 1;

    for (size_t i = buffer->cr+1; i < buffer->length; i++) {
        move(i, 0);
        clrtoeol();
        printw(buffer->rows[i].line);        
    }
    
    buffer->cc = 0;
    move(++buffer->cr, buffer->cc);
}

void read_from_file(Buffer *buffer) {
    FILE *file = fopen(buffer->filename, "r");
    int ch = fgetc(file);
    int i = 0;
    int j = 0;
    while (ch != EOF) {
        addch(ch);
        if (ch != '\n') {
            buffer->rows[i].line[j++] = ch;
        }
        else {
            buffer->rows[i].line[j++] = ch;
            buffer->rows[i++].length = j;
            buffer->length++;
            j = 0;
        }
        buffer->rows[i].length = j;
        ch = fgetc(file);
    }
    fclose(file);
}

void write_to_file(Buffer *buffer) {
    FILE *file = fopen(buffer->filename, "w");
    for (size_t i = 0; i < buffer->length; i++) {
        fwrite(buffer->rows[i].line, buffer->rows[i].length, TRUE, file);
    }
    fclose(file);
}

void add_char(Buffer *buffer, char ch) {
    int index = buffer->cc;
    Row *row = &buffer->rows[buffer->cr];
    shift_right(index, row);
    row->line[index] = ch;
    buffer->cc++;
}

int at_pointer(Buffer *buffer) {
    return buffer->rows[buffer->cr].line[buffer->cc];
}

void move_pointer_find_char(Buffer *buffer) {
    int find = getch();
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

void move_pointer_forward(Buffer *buffer) {
    size_t len = buffer->rows[buffer->cr].length;
    if ((buffer->cr == buffer->length - 1 && 
    buffer->cc < len || 
    buffer->cc < len - 1) && len != 0) {
        buffer->cc++;
    }
}

void move_pointer_backward(Buffer *buffer) {
    if (buffer->cc > 0) {
        buffer->cc--;
    }
}

bool word_delim(int ch) {
    return (ch == ' ' || ch == '(' || ch == ')' || ch == '[' || ch == ']' ||
        ch == '{' || ch == '}' || ch == '-' || ch == '_' || ch == '<' ||
        ch == '>' || ch == '=' || ch == '\'' ||  ch == ',' || ch == '.' || ch == '\n');
}

void move_pointer_start(Buffer *buffer) {
    buffer-> cr = 0;
    buffer-> cc = 0;
}

void move_pointer_down(Buffer *buffer) {
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

void move_pointer_up(Buffer *buffer) {
    if (buffer->cr > 0) {
        buffer->cc = min(buffer->cc, buffer->rows[buffer->cr - 1].length - 1);
        buffer->cr--;
    }
}

void move_pointer_start_line(Buffer *buffer) {
    buffer->cc = 0;
}

void move_pointer_end(Buffer *buffer) {
    buffer->cr = buffer->length - 1;
    buffer->cc = 0;
}

void move_pointer_end_line(Buffer *buffer) {
    if (buffer->cr != buffer->length - 1) {
        buffer->cc = buffer->rows[buffer->cr].length-1;
    }
    else {
        buffer->cc = buffer->rows[buffer->cr].length;
    }
}

void move_pointer_forward_word(Buffer *buffer) { //ugly but works for now
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
    buffer->cr = i;
    buffer->cc = j;
}

void move_pointer_backward_word(Buffer *buffer) { //ugly but works for now
    size_t i = buffer->cr;
    size_t j = buffer->cc;

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

void change_mode(Mode new_mode, Buffer* buffer) {
    mode = new_mode;
    int x,y;
    getmaxyx(stdscr, y, x);
    mvprintw(y-1, x, mode_as_string());
}

int main(int argc, char *argv[]) {

    //Ncurses initialization
    refresh();
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
        
    //Buffer initialization
    Buffer buffer = {0};
    buffer.cc = 0;
    buffer.cr = 0;
    buffer.length = 1;
    for (size_t i = 0; i < 1024; i++){
        buffer.rows[i].line = calloc(MAX_LINE_SIZE, sizeof(char));
        *buffer.rows[i].line = '\0';
        buffer.rows[i].length = 0;
    }
    
    //Load user file
    if (argc == 2) {
        buffer.filename = argv[1];
        read_from_file(&buffer);
    }
    else {
        buffer.filename = "out.txt";
    }

    //More init
    int row, col;
    getmaxyx(stdscr, row, col);
    change_mode(NORMAL, &buffer);

    //Main loop
    int ch = 0;
    while (ch != ctrl('q') && QUIT != 1) {
        char *info = malloc(500);
        sprintf(info, "%d, %d, %d, %d, %c", buffer.cr, buffer.cc, buffer.length, buffer.rows[buffer.cr].length, at_pointer(&buffer));
        move(row-1,col-15);
        clrtoeol();
        printw(info);
        free(info);
        refresh();
        move(buffer.cr, buffer.cc);

        ch = getch(); 
        switch (mode) {
            case (NORMAL):
                if (ch == 'h') {
                    change_mode(INSERT, &buffer);
                    keypad(stdscr, FALSE);
                }
                switch (ch) {
                    case (ctrl('s')):
                        write_to_file(&buffer);
                        QUIT = 1;
                        break;
                    case ('i'):
                        move_pointer_up(&buffer);
                        break;
                    case ('j'):
                        move_pointer_backward(&buffer);
                        break;
                    case ('k'):
                        move_pointer_down(&buffer);
                        break;
                    case ('l'):
                        move_pointer_forward(&buffer);
                        break;
                    case ('f'):
                        move_pointer_find_char(&buffer);
                        break;
                    case ('a'):
                        move_pointer_start_line(&buffer);
                        break;
                    case ('e'):
                        move_pointer_end_line(&buffer);
                        break;
                    case ('w'):
                        move_pointer_forward_word(&buffer);
                        break;
                    case ('b'):
                        move_pointer_backward_word(&buffer);
                        break;
                    case ('g'):
                        move_pointer_start(&buffer);
                        break;
                    case ('G'):
                        move_pointer_end(&buffer);
                        break;
                }   
                break;
            case (INSERT): {
                if (ch == ESC) {
                    mode = NORMAL;
                    keypad(stdscr, TRUE);
                    change_mode(NORMAL, &buffer);
                }
                else {
                    Row *cur = &buffer.rows[buffer.cr];
                    switch (ch) {
                        case (ENTER): {
                            insch(ch);
                            on_enter(&buffer);
                            break;
                        }
                        case (BSPACE): {
                            on_backspace(&buffer);
                            break;
                        }
                        default: {
                            insch(ch);
                            add_char(&buffer, ch);
                        }
                    }
                }
            }
                break; 
        }
    }
    refresh();
    endwin();
    return 0;
}