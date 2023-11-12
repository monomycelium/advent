CC = cc
override CFLAGS += -Wall -Wextra -fshort-enums -std=gnu17
override LDFLAGS += -ldl -lm $(shell pkg-config --libs libcurl)

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))
TARGET = $(notdir $(CURDIR))

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

format:
	clang-format -i --style=file $(SRCS) $(wildcard *.h)

clean:
	@rm -rfv $(OBJS) $(TARGET)

.PHONY: format clean
