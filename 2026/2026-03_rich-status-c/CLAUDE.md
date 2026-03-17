# forge_status — C Rich Status Module

A minimal terminal spinner module inspired by Python's `rich.status`.

## API

All symbols are prefixed with `forge_status_`.

```c
#include "status.h"

void forge_status_open(const char *fmt, ...);   // start spinner + message
void forge_status_printf(const char *fmt, ...); // log a line above spinner
void forge_status_close(void);                  // stop spinner, clean up
```

## Usage Example

```c
forge_status_open("Processing items...");
for (int i = 0; i < 10; i++) {
    forge_status_printf("hello %d", i);
    sleep(2);
}
forge_status_close();
```

## Build

```
make        # produces ./main
make clean
```

Requires: gcc, pthreads (`-lpthread`), a terminal supporting ANSI escape codes.

## Files

| File       | Purpose                        |
|------------|--------------------------------|
| `status.h` | Public API header              |
| `status.c` | Spinner thread + implementation|
| `main.c`   | Demo / example                 |
| `Makefile` | Build rules                    |

## Implementation Notes

- A background `pthread` animates a braille spinner at 100 ms intervals.
- `forge_status_printf` erases the spinner line, prints the log line, then lets the thread redraw the spinner.
- A `pthread_mutex_t` protects the shared message buffer and stdout writes.
- ANSI codes used: `\r\033[2K` (erase line), `\033[32m`/`\033[0m` (green color).
