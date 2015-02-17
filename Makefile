WARNINGFLAGS=-Wall -Wextra -Wunused-parameter -Wmissing-parameter-type	\
-Wlogical-op -Wmissing-prototypes

CFLAGS = -O2 -g -std=gnu99 $(WARNINGFLAGS) -D_GNU_SOURCE

LDFLAGS = 

CC = gcc

.PHONY: all clean

all: test64 test32 verify

linux64.o rv64.o test64: CFLAGS += -m64
linux32.o rv32.o test32: CFLAGS += -m32

test64 test32: LDFLAGS += -lgsl -lblas -lm

test64: test.c linux64.o rv64.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test32: test.c linux32.o rv32.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -static

verify: LDFLAGS += -pthread
verify: linux64.o rv64.o

clean:
	rm -f test64 test32 verify
	rm -f *.o
