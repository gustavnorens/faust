CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lm  -lncurses

SRC = faust.c util.c actions.c canvas.c

OBJ = $(SRC:.c=.o)

HDR = util.h actions.h canvas.h

TARGET = faust

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean: $(TARGET)
	rm -f $(OBJ)