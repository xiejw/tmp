CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
LDFLAGS = -lpthread

TARGET  = main
SRCS    = main.c status.c

$(TARGET): $(SRCS) status.h
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean
