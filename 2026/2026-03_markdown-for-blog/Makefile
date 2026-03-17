CC     = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -O2
BUILD  = .build
TARGET = $(BUILD)/md2html
TEST   = $(BUILD)/test
SRCS   = main.c md2html.c

all: $(TARGET)

$(TARGET): $(SRCS) md2html.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

$(TEST): test.c md2html.c md2html.h | $(BUILD)
	$(CC) $(CFLAGS) -D_POSIX_C_SOURCE=200809L -o $@ test.c md2html.c

test: $(TEST)
	$(TEST)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
