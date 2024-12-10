CC = gcc
CFLAGS = -Wall -Wextra -O2 -ldl

SRCS = main.c terminal.c shell.c ui.c plugin.c config.c logging.c utils.c line_editor.c
OBJS = $(addprefix ./trash/, main.o terminal.o shell.o ui.o plugin.o config.o logging.o utils.o line_editor.o)

all: prepare_dirs boitak

prepare_dirs:
	rm -rf ./trash ./build
	mkdir ./trash ./build

boitak: $(OBJS)
	$(CC) $(CFLAGS) -o ./build/boitak $(OBJS)

./trash/main.o: main.c terminal.h ui.h plugin.h config.h logging.h utils.h line_editor.h
	$(CC) $(CFLAGS) -c main.c -o $@

./trash/terminal.o: terminal.c terminal.h shell.h logging.h
	$(CC) $(CFLAGS) -c terminal.c -o $@

./trash/shell.o: shell.c shell.h logging.h
	$(CC) $(CFLAGS) -c shell.c -o $@

./trash/ui.o: ui.c ui.h plugin.h utils.h line_editor.h
	$(CC) $(CFLAGS) -c ui.c -o $@

./trash/plugin.o: plugin.c plugin.h logging.h
	$(CC) $(CFLAGS) -c plugin.c -o $@

./trash/config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c -o $@

./trash/logging.o: logging.c logging.h
	$(CC) $(CFLAGS) -c logging.c -o $@

./trash/utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c -o $@

./trash/line_editor.o: line_editor.c line_editor.h
	$(CC) $(CFLAGS) -c line_editor.c -o $@

clean:
	rm -rf ./trash ./build
