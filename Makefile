WARNINGFLAGS=-Wall -Wextra -Wunused-parameter -Wmissing-parameter-type	\
-Wlogical-op -Wmissing-prototypes

CFLAGS := -O2 -g -std=gnu99 $(WARNINGFLAGS) -D_GNU_SOURCE
-include $(wildcard .*.deps)
dot-target = $(dir $@).$(notdir $@)
depfile = $(dot-target).deps

LDFLAGS = 

CC = gcc
LONG_BIT := $(shell getconf LONG_BIT)

TMPDIR ?= /tmp

try-run = $(shell set -e;               \
        TMP="$(TMPDIR)/.$$$$.tmp";      \
        TMPO="$(TMPDIR)/.$$$$.o";       \
        if ($(1)) >/dev/null 2>&1;      \
        then echo "$(2)";               \
        else echo "$(3)";               \
        fi;                             \
        rm -f "$$TMP" "$$TMPO")

cc-option = $(call try-run,\
        $(CC) $(CFLAGS) $(1) -c -x c /dev/null -o "$$TMPO",$(1),$(2))

CFLAGS += $(call cc-option,-m$(LONG_BIT),)

.PHONY: all clean

all: test verify

test: LDFLAGS += -lm -lrt

verify: LDFLAGS += -pthread

test: linux$(LONG_BIT).o
test: rv$(LONG_BIT).o
test: rnd.o
verify: linux$(LONG_BIT).o
verify: rv$(LONG_BIT).o

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MF $(depfile) -c -o $@ $<

%: %.c
	$(CC) $(CFLAGS) -MMD -MF $(depfile) -o $@ $(filter-out $(wildcard *.h), $^) $(LDFLAGS)

clean:
	rm -f test verify
	rm -f *.o
	rm -f .*.deps
