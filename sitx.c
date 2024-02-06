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
    size_t row_length;
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

void shift_chars(size_t start, size_t length, char *str, char ch, bool bf) {
    if (bf) {
        char tmp[length];
        strcpy(tmp, str);
        str[start] = ch;
        for (size_t i = start; i < length; i++) {
            str[i+1] = tmp[i];
        }    
    }
    else {
        char tmp[length];
        strcpy(tmp, str);
        for (size_t i = start-1; i < length - 1; i++) {
            str[i] = tmp[i+1];
        }
    }
}

void shift_rows(int current_row) {

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
    buffer.row_length = 1;
    for (size_t i = 0; i < 1024; i++){
        buffer.rows[i].line = calloc(1024, sizeof(char));
        buffer.rows[i].length = 0;
    }
    
    printf("Got here");
    if (argc == 2) {
        FILE *file = fopen(argv[1], "r");
        int ch = fgetc(file);
        int i = 0;
        int j = 0;
        while (ch != EOF) {
            addch(ch);
            if (ch != '\n') {
                buffer.rows[i].line[j++] = ch;
            }
            else {
                buffer.rows[i].line[j++] = ch;
                buffer.rows[i++].length = j;
                buffer.row_length++;
                j = 0;
            }
            buffer.rows[i].length = j;
            ch = fgetc(file);
        }
        fclose(file);
    }

    int row, col;
    getmaxyx(stdscr, row, col);
    mvprintw(row-1, 0, stringify_mode());
    move(0,0);

    int ch = 0;
    int y,x;
    while (ch != ctrl('q') && QUIT != 1) {
        refresh();
        getyx(stdscr,y,x);
        
        char *str = malloc(500);
        sprintf(str, "%d, %d", buffer.pointer_row, buffer.pointer_col);
        mvprintw(row-1, col-7, str);
        move(y,x);

        ch = getch(); 
        switch (mode) {
            case (NORMAL):
                if (ch == 'h') {
                    mode = INSERT;
                    print_mode(row-1, 0);
                    keypad(stdscr, FALSE);
                }
                else if (ch == 'i') {
                    if (y > 0) {
                        sit_move(y-1, min(x, buffer.rows[buffer.pointer_row - 1].length - 1), &buffer);
                    }
                }
                else if (ch == 'j') {
                    if (x > 0) {
                        sit_move(y, x-1, &buffer);
                    }
                }
                else if (ch == 'k') {
                    if (y < buffer.row_length - 1) {
                        if (buffer.pointer_row + 1 == buffer.row_length - 1) {
                            sit_move(y+1, min(x, buffer.rows[buffer.pointer_row + 1].length), &buffer);
                        }
                        else {
                            sit_move(y+1, min(x, buffer.rows[buffer.pointer_row + 1].length), &buffer);
                        }                        
                    }
                }
                else if (ch == 'l') {
                    bool last_row = y == buffer.row_length -1;
                    if (last_row && x < buffer.rows[buffer.pointer_row].length 
                    || x < buffer.rows[buffer.pointer_row].length - 1) {
                            sit_move(y, x+1, &buffer);
                    }
                }
                else if (ch == ctrl('s')) {
                    FILE *file = fopen("out.txt", "w");
                    for (size_t i = 0; i < 1024; i++){
                        if (buffer.rows[i].length > 0) {
                            fwrite(buffer.rows[i].line, buffer.rows[i].length, TRUE, file);
                        }
                    }
                    fclose(file);
                    QUIT = 1;
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
                            if (x == cur->length){
                                addch(ch);
                                cur->line[buffer.pointer_col] = ch;
                                cur->length++;
                                buffer.pointer_row++;
                                buffer.pointer_col = 0;
                                buffer.row_length++;
                            }
                            else {
                                insch(ch);
                            }
                            break;
                        }
                        case (BSPACE): {
                            if (buffer.pointer_col >= 0) {
                                shift_chars(x, cur->length, cur->line, ch, FALSE);
                                cur->length--;
                                sit_move(y,--x, &buffer);
                                delch();
                            } 
                            else {

                            }
                            break;
                        }
                        default: {
                            if (x < cur->length) {
                                shift_chars(x, cur->length, cur->line, ch, TRUE);
                                insch(ch);
                                sit_move(y, ++x, &buffer);
                                cur->length++;
                            }
                            else {
                                cur->line[buffer.pointer_col] = ch;
                                cur->length++;
                                buffer.pointer_col++; 
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