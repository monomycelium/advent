export CC = cc
export CXX = c++
export ZIG = zig
override CFLAGS += -Wall -Wextra -fshort-enums -std=gnu17
override CXXFLAGS += -Wall -Wextra -fshort-enums -std=gnu++20

CALLER = caller

YEARS := $(filter %/, $(wildcard [0-9][0-9][0-9][0-9]/))
SOURCES_C := $(wildcard $(addsuffix *.c,$(YEARS)))
SOURCES_CXX := $(wildcard $(addsuffix *.cpp,$(YEARS)))
SOURCES_ZIG := $(wildcard $(addsuffix *.zig,$(YEARS)))
OBJECTS += $(SOURCES_C:.c=.so)
OBJECTS += $(SOURCES_CXX:.cpp=.so)
OBJECTS += $(SOURCES_ZIG:.zig=.so)

.PHONY: all
all: $(OBJECTS) caller

%.so: %.c
	$(CC) $(CFLAGS) -shared -fPIC -I$(CALLER) -o $@ $<

%.so: %.cpp
	$(CXX) $(CXXFLAGS) -shared -fPIC -I$(CALLER) -o $@ $<

%.so: %.zig
	$(ZIG) build-lib $(ZIGFLAGS) -dynamic -lc -I$(CALLER) $<
	@mv lib$(shell basename $@) $@
	@rm lib$(shell basename $@).o

.PHONY: caller
caller: $(wildcard $(CALLER)/*.c)
	@$(MAKE) -C caller

.PHONY: format
format:
	clang-format -i --style=file $(wildcard *.h *.hpp) $(SOURCES_C) $(SOURCES_CXX)
	@$(MAKE) -C caller format

.PHONY: clean
clean:
	@$(RM) -rvf $(OBJECTS)
	@$(MAKE) -C caller clean
