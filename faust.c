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

void change_mode(Mode new_mode) {
    if (new_mode == INSERT) {
        keypad(stdscr, FALSE);
    }
    if (new_mode == NORMAL) {
        keypad(stdscr, TRUE);
    }
    mode = new_mode;
    int y = getmaxy(stdscr);
    mvprintw(y-1, 0, mode_as_string());
}

void update_lines(size_t start, size_t end, Buffer *buffer) {
    for (size_t i = start; i < end; i++) {
        move(i, 0);
        clrtoeol();
        printw(buffer->rows[i].line);
    }    
}

void faust_delch(int y, int x) {
    mvdelch(y, x);
}

int main(int argc, char *argv[]) {

    //Ncurses initialization
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
        sprintf(info, "%d, %d, %d, %d, %c", (int)buffer.cr, (int)buffer.cc, (int)buffer.length, (int)buffer.rows[buffer.cr].length, get_char_at_cursor(&buffer));
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
                }
                switch (ch) {
                    case (ctrl('s')): 
                    {
                        write_to_file(&buffer);
                        QUIT = 1;
                        break;
                    }
                    case ('i'):
                    {
                        move_cursor_up(&buffer);
                        break;
                    }
                    case ('j'):
                    {
                        move_cursor_backward(&buffer);
                        break;
                    }
                    case ('k'): 
                    {
                        move_cursor_down(&buffer);
                        break;
                    }
                    case ('l'): 
                    {
                        move_cursor_forward(&buffer);
                        break;
                    }
                    case ('f'): 
                    {
                        int find = getch();
                        move_cursor_find_char(&buffer, find);
                        break;
                    }
                    case ('a'):
                    {
                        move_cursor_start_line(&buffer);
                        break;
                    }
                    case ('e'):
                    {
                        move_cursor_end_line(&buffer);
                        break;
                    }
                    case ('w'):
                    {
                        move_cursor_forward_word(&buffer);
                        break;
                    }
                    case ('b'):
                    {
                        move_cursor_backward_word(&buffer);
                        break;
                    }
                    case ('g'):
                    {
                        move_cursor_start(&buffer);
                        break;
                    }
                    case ('G'):
                    {
                        move_cursor_end(&buffer);
                        break;
                    }
                    case ('s'):
                    {
                        shift_left(buffer.cc, &buffer.rows[buffer.cr]);
                        delch();
                        change_mode(INSERT);
                        break;
                    }
                    case ('r'):
                    {
                        int ch = getch();
                        delete_char_at_cursor(&buffer);
                        insert_char_at_cursor(&buffer, ch);
                        break;
                    }
                    case ('F'):
                    {
                        int find = getch();
                        move_cursor_find_char_backward(&buffer, find);
                        break;
                    }
                    case ('d'):
                    {
                        delete_line_at_cursor(&buffer);
                        update_lines(buffer.cr, buffer.length + 1, &buffer);
                        break;
                    }
                    case ('t'):
                    {
                        faust_delch(buffer.cr, buffer.cc - 1);
                    }
                }
                break;
            case (INSERT): {
                if (ch == ESC) {
                    mode = NORMAL;
                    change_mode(NORMAL);
                }
                else {
                    switch (ch) {
                        case (BSPACE): {
                            if (delete_char_at_cursor(&buffer)) {
                                update_lines(buffer.cr, buffer.length + 1, &buffer);
                            }
                            else {
                                faust_delch(buffer.cr, buffer.cc);
                            }
                            break;
                        }
                        case (ENTER): {
                            insert_char_at_cursor(&buffer, ch);
                            update_lines(buffer.cr-1, buffer.length, &buffer);
                            break;
                        }
                        default: {
                            insch(ch);
                            insert_char_at_cursor(&buffer, ch);
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