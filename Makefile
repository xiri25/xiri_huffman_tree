CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-g
INCLUDE_DIRS=src/include/
SRC_FILES=src/main.c \
		  src/file/file.c \
		  src/frequencies/frequencies.c

.PHONY: all clean

all: main

main: $(SRC_FILES)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIRS) -o $@ $^ $(LDFLAGS)

run:
	./main

clean:
	rm -f main
