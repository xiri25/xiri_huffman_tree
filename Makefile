CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused-function -Wconversion
LDFLAGS=-g -O2
INCLUDE_DIRS=src/include/
SRC_FILES=src/main.c \
		  src/file/file.c \
		  src/frequencies/frequencies.c \
		  src/huffman_tree/huffman_tree.c \
		  src/memory/arena.c

.PHONY: all clean

all: main

main: $(SRC_FILES)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIRS) -o $@ $^ $(LDFLAGS)

run:
	./main

clean:
	rm -f main
