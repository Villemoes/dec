CFLAGS := -O2 -g -std=gnu99 -Wall -Wextra -D_GNU_SOURCE
-include $(wildcard .*.deps)
dot-target = $(dir $@).$(notdir $@)
depfile = $(dot-target).deps

LDFLAGS = 

CC = gcc
LONG_BIT := $(shell getconf LONG_BIT)
NTHR := $(strip $(shell getconf _NPROCESSORS_ONLN 2> /dev/null))

ifneq ($(NTHR),)
verify: CFLAGS += -DNTHR=$(NTHR)
endif

TMPDIR ?= /tmp

try-run = $(shell set -e;               \
        TMP="$(TMPDIR)/.$$$$.tmp";      \
        TMPO="$(TMPDIR)/.$$$$.o";       \
        if ($(1)) >/dev/null 2>&1;      \
        then echo "$(2)";               \
        else echo "$(3)";               \
        fi;                             \
        rm -f "$$TMP" "$$TMPO")

# Unknown warning options are not fatal by default with clang, so use -Werror.
cc-option = $(call try-run,\
        $(CC) $(CFLAGS) -Werror $(1) -c -x c /dev/null -o "$$TMPO",$(1),$(2))

CFLAGS += $(call cc-option,-m$(LONG_BIT),)
CFLAGS += $(call cc-option,-Wunused-parameter,)
CFLAGS += $(call cc-option,-Wmissing-parameter-type,)
CFLAGS += $(call cc-option,-Wlogical-op,)
CFLAGS += $(call cc-option,-Wmissing-prototypes,)

CFLAGS += -DLONG_BIT=$(LONG_BIT)

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
