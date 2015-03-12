WARNINGFLAGS=-Wall -Wextra -Wunused-parameter -Wmissing-parameter-type	\
-Wlogical-op -Wmissing-prototypes

CFLAGS = -O2 -g -std=gnu99 $(WARNINGFLAGS) -D_GNU_SOURCE -MMD -MF $@.deps
-include $(wildcard *.deps)

LDFLAGS = 

CC = gcc

.PHONY: all clean

all: test64 verify64

linux64.o rv64.o test64: CFLAGS += -m64
linux32.o rv32.o test32: CFLAGS += -m32

test64 test32: LDFLAGS += -lm -lrt

test64: test.c linux64.o rv64.o rnd.o
	$(CC) $(CFLAGS) -o $@ $(filter-out $(wildcard *.h), $^) $(LDFLAGS)

test32: test.c linux32.o rv32.o rnd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

verify32 verify64: LDFLAGS += -pthread

verify64: verify.c linux64.o rv64.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

verify32: CFLAGS += -m32
verify32: verify.c linux32.o rv32.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


clean:
	rm -f test64 test32 verify64 verify32
	rm -f *.o
	rm -f *.deps
