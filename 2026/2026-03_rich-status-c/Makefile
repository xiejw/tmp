CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
LDFLAGS = -lpthread

BUILD   = .build
TARGET  = ${BUILD}/main
SRCS    = main.c status.c

run: ${TARGET}
	$^

$(TARGET): $(SRCS) status.h | ${BUILD}
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -rf $(BUILD)

${BUILD}:
	@mkdir -p ${BUILD}

.PHONY: clean
