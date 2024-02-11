#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "actions.h"
#include "canvas.h"
#include "util.h"

#define ctrl(x) ((x) & 0x1f)

#define BSPACE 127
#define ESC 27
#define ENTER 10

#define MAX_LINE_SIZE 1024

typedef enum {
    NORMAL,
    INSERT
} Mode;

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

void change_mode(Mode new_mode) {
    mode = new_mode;
    int y = getmaxy(stdscr);
    mvprintw(y-1, 0, mode_as_string());
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
        for (size_t i = 0; i < buffer.length; i++) {
            mvprintw(i,0, buffer.rows[i].line);
        }
        
    }
    else {
        buffer.filename = "out.txt";
    }

    //More init
    int row, col;
    getmaxyx(stdscr, row, col);
    change_mode(NORMAL);

    //Main loop
    int ch = 0;
    while (ch != ctrl('q') && QUIT != 1) {
        char *info = malloc(500);
        sprintf(info, "%d, %d, %d, %d, %c", (int)buffer.cr, (int)buffer.cc, (int)buffer.length, (int)buffer.rows[buffer.cr].length, char_at_cursor(&buffer));
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
                    change_mode(INSERT);
                    keypad(stdscr, FALSE);
                }
                switch (ch) {
                    case (ctrl('s')):
                        write_to_file(&buffer);
                        QUIT = 1;
                        break;
                    case ('i'):
                        move_cursor_up(&buffer);
                        break;
                    case ('j'):
                        move_cursor_backward(&buffer);
                        break;
                    case ('k'):
                        move_cursor_down(&buffer);
                        break;
                    case ('l'):
                        move_cursor_forward(&buffer);
                        break;
                    case ('f'):
                        int find = getch();
                        move_cursor_find_char(&buffer, find);
                        break;
                    case ('a'):
                        move_cursor_start_line(&buffer);
                        break;
                    case ('e'):
                        move_cursor_end_line(&buffer);
                        break;
                    case ('w'):
                        move_cursor_forward_word(&buffer);
                        break;
                    case ('b'):
                        move_cursor_backward_word(&buffer);
                        break;
                    case ('g'):
                        move_cursor_start(&buffer);
                        break;
                    case ('G'):
                        move_cursor_end(&buffer);
                        break;
                }   
                break;
            case (INSERT): {
                if (ch == ESC) {
                    mode = NORMAL;
                    keypad(stdscr, TRUE);
                    change_mode(NORMAL);
                }
                else {
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