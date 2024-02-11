#include <stdlib.h>

#ifndef CANVAS_H
#define CANVAS_H
#define MAX_LINES 1024

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

void write_to_file(Buffer *buffer);
void read_from_file(Buffer *buffer);

//void init_buffer(Buffer *buffer);

#endif