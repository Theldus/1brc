CC      ?= cc
CFLAGS  += -O3 -march=native -mtune=native -g

.PHONY: all clean

all: 1b
1b: 1b.o

clean:
	rm -f 1b.o
	rm -f 1b
