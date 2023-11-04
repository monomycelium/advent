CC = cc
ZIG = zig
override CFLAGS += -Wall -Wextra -fshort-enums -std=gnu11

YEARS := $(filter %/, $(wildcard [0-9][0-9][0-9][0-9]/))
SOURCES_C := $(wildcard $(addsuffix *.c,$(YEARS)))
SOURCES_Z := $(wildcard $(addsuffix *.zig,$(YEARS)))
OBJECTS += $(SOURCES_C:.c=.so)
OBJECTS += $(SOURCES_Z:.zig=.so)

.PHONY: all
all: $(OBJECTS) caller

%.so: %.c
	$(CC) $(CFLAGS) -shared -I. -o $@ $<

%.so: %.zig
	$(ZIG) build-lib $(ZIGFLAGS) -dynamic -lc -I. $<
	@mv lib$(shell basename $@) $@
	@rm lib$(shell basename $@).o

caller: caller.c
	$(CC) $(CFLAGS) $(LDFLAGS) -ldl -o $@ $<

.PHONY: format
format:
	clang-format -i --style=file caller.c $(wildcard *.h) $(SOURCES_C)

.PHONY: clean
clean:
	rm -rv caller $(OBJECTS)
