#include "canvas.h"
#include <stdio.h>

void read_from_file(Buffer *buffer) {
    FILE *file = fopen(buffer->filename, "r");
    int ch = fgetc(file);
    int i = 0;
    int j = 0;
    while (ch != EOF) {
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
        fwrite(buffer->rows[i].line, buffer->rows[i].length, 1, file);
    }
    fclose(file);
}