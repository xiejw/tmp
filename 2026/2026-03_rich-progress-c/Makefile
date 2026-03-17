CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L

BUILD   = .build
TARGET  = ${BUILD}/main
SRCS    = main.c progress.c

run: ${TARGET}
	$^

$(TARGET): $(SRCS) progress.h | ${BUILD}
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -rf $(BUILD)

${BUILD}:
	@mkdir -p ${BUILD}

.PHONY: clean run
