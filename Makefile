CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused-function -Wconversion -march=x86-64-v3 -std=c23
LDFLAGS=-g -O2 -flto
INCLUDE_DIRS=src/include/
SRC_DIR=src
SRC_FILES=$(wildcard $(SRC_DIR)/*/*.c)
SRC_FILES+=$(SRC_DIR)/main.c

.PHONY: all clean

all: main

main: $(SRC_FILES)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIRS) -o $@ $^ $(LDFLAGS)

run:
	./main

clean:
	rm -f main
