SRC = run_commands.c
BIN = $(SRC:%.c=%)
OBJ = $(SRC:%.c=%.o)
CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -g 
LIBS = 
HEADERS = defs.h
all: $(BIN)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN): %: %.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o : %.c $(HEADERS)
	gcc -c $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	-rm -f $(BIN) $(OBJ)
