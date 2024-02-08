#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>

#define ctrl(x) ((x) & 0x1f)

#define BSPACE 127
#define ESC 27
#define ENTER 10

typedef enum {
    NORMAL,
    INSERT
} Mode;

typedef struct {
    char *line;
    size_t length;
} Row;

typedef struct {
    Row rows[1024];
    size_t pointer_row;
    size_t pointer_col;
    size_t length;
    char *filename;
} Buffer;

Mode mode = NORMAL;

int QUIT = 0;

void sit_move(int y, int x, Buffer *buffer) {
    move(y, x);
    buffer->pointer_row = y;
    buffer->pointer_col = x;
}

char* stringify_mode(){
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
    return "NORMAL";
}

void print_mode(int y, int x) {
    int old_y, old_x;
    getyx(stdscr, old_y, old_x);
    mvprintw(y, x, stringify_mode());
    move(old_y, old_x);
}

int max(int x, int y) { if (x > y) return x; return y;}
int min(int x, int y) {if (x > y) return y; return x;}

void shift_right(size_t start, Row *row, char ch) {
    char tmp[row->length + 1];
    strcpy(tmp, row->line);
    row->line[start] = ch;
    for (size_t i = start; i < row->length + 1; i++) {
        row->line[i+1] = tmp[i];
    }    
}

void shift_left(size_t start, Row *row) {
    for (size_t i = start-1; i < row->length; i++) {
        row->line[i] = row->line[i+1];
    }
    row->line[row->length] = '\0';
}

void on_backspace(Buffer *buffer){
    Row *cur = &buffer->rows[buffer->pointer_row];
    if (buffer->pointer_col > 0) {
        delch();
        shift_left(buffer->pointer_col, cur);
        cur->length--;
        sit_move(buffer->pointer_row, buffer->pointer_col-1, buffer);        
    }
    else if (buffer->pointer_row != 0) {
        Row *above = &buffer->rows[buffer->pointer_row - 1];
        int save_len = above->length;
        for (size_t i = 0; i < cur->length; i++) {
            above->line[i + above->length - 1] = cur->line[i];
        }

        above->length = cur->length + above->length - 1;
        for (size_t i = buffer->pointer_row + 1; i < buffer->length; i++) {
            memcpy(buffer->rows[i-1].line, buffer->rows[i].line, buffer->rows[i].length + 1);
            buffer->rows[i-1].length = buffer->rows[i].length;
        }

        buffer->rows[buffer->length-1].line[0] = '\0';
        buffer->rows[buffer->length-1].length = 0;

        buffer->length--;

        for (size_t i = buffer->pointer_row - 1; i < buffer->length + 1; i++) {
            move(i, 0);
            clrtoeol();
            printw(buffer->rows[i].line);
        }
        
        sit_move(buffer->pointer_row-1, save_len-1, buffer);
    }
}

void on_enter(Buffer *buffer) {
    int cr,cc;
    getyx(stdscr, cr, cc);

    buffer->length++;
    for (size_t i = buffer->length-1; i > cr + 1; i--) {
        memcpy(buffer->rows[i].line, buffer->rows[i-1].line, buffer->rows[i-1].length + 1);
        buffer->rows[i].length = buffer->rows[i-1].length;
    }
    
    Row *upper = &buffer->rows[cr];
    Row *lower = &buffer->rows[cr+1];

    for (size_t i = cc; i < upper->length+1; i++) {
        lower->line[i-cc] = upper->line[i];
    }

    upper->line[cc] = '\n';
    upper->line[cc+1] = '\0';
    lower->length = upper->length - cc;
    upper->length = cc + 1;

    for (size_t i = cr+1; i < buffer->length; i++) {
        move(i, 0);
        clrtoeol();
        printw(buffer->rows[i].line);        
    }
    
    sit_move(cr+1, 0, buffer);
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


int at_pointer(Buffer *buffer) {
    return buffer->rows[buffer->pointer_row].line[buffer->pointer_col];
}

void f_motion(Buffer *buffer) {
    char *line = buffer->rows[buffer->pointer_row].line;
    int ch = getch();
    while (line[buffer->pointer_col++] != '\0') {
        if (at_pointer(buffer) == ch) {
            sit_move(buffer->pointer_row,  buffer->pointer_col, buffer);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    refresh();
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    Buffer buffer = {0};
    buffer.pointer_col = 0;
    buffer.pointer_row = 0;
    buffer.length = 1;
    for (size_t i = 0; i < 1024; i++){
        buffer.rows[i].line = calloc(1024, sizeof(char));
        *buffer.rows[i].line = '\0';
        buffer.rows[i].length = 0;
    }
    
    if (argc == 2) {
        buffer.filename = argv[1];
        read_from_file(&buffer);
    }
    else {
        buffer.filename = "out.txt";
    }

    int row, col;
    getmaxyx(stdscr, row, col);
    mvprintw(row-1, 0, stringify_mode());
    move(0,0);

    int ch = 0;
    int y,x;
    while (ch != ctrl('q') && QUIT != 1) {
        getyx(stdscr,y,x);
        
        char *str = malloc(500);
        sprintf(str, "%d, %d, %d, %d, %c", buffer.pointer_row, buffer.pointer_col, buffer.length, buffer.rows[buffer.pointer_row].length, at_pointer(&buffer));
        move(row-1,col-15);
        clrtoeol();
        printw(str);
        refresh();
        sit_move(y,x, &buffer);

        ch = getch(); 
        switch (mode) {
            case (NORMAL):
                if (ch == 'h') {
                    mode = INSERT;
                    print_mode(row-1, 0);
                    keypad(stdscr, FALSE);
                }
                switch (ch) {
                    case (ctrl('s')):
                        write_to_file(&buffer);
                        QUIT = 1;
                        break;
                    case ('i'):
                        if (y > 0) {
                            sit_move(y-1, min(x, buffer.rows[buffer.pointer_row - 1].length - 1), &buffer);
                        }
                        break;
                    case ('j'):
                        if (x > 0) {
                            sit_move(y, x-1, &buffer);
                        }
                        break;
                    case ('k'):
                        if (y < buffer.length - 1) {
                            if (buffer.pointer_row + 1 == buffer.length - 1) {
                                sit_move(y+1, min(x, buffer.rows[buffer.pointer_row + 1].length), &buffer);
                            }
                            else {
                                sit_move(y+1, min(x, buffer.rows[buffer.pointer_row + 1].length - 1), &buffer);
                            }
                        }
                        break;
                    case ('l'):
                        size_t len = buffer.rows[buffer.pointer_row].length;
                        if ((y == buffer.length - 1 && 
                        x < len || 
                        x < len - 1) && len != 0) {
                            sit_move(y, x+1, &buffer);
                        }
                        break;
                    case ('f'):
                        f_motion(&buffer);
                        break;
                }
                break;
            case (INSERT): {
                if (ch == ESC) {
                    mode = NORMAL;
                    keypad(stdscr, TRUE);
                    print_mode(row-1, 0);
                }
                else {
                    Row *cur = &buffer.rows[buffer.pointer_row];
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
                            if (x < cur->length) {
                                shift_right(x, cur, ch);
                                insch(ch);
                                sit_move(y, ++x, &buffer);
                                cur->length++;
                            }
                            else {
                                cur->line[buffer.pointer_col++] = ch;
                                cur->length++;
                                addch(ch);
                            }
                            break;
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