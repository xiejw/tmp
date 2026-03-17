# C Projects — Coding Style & Conventions

Applies to all C projects under this directory.

---

## Naming

- All **public** symbols (functions, types, macros): `forge_` prefix, `snake_case`.
  - Functions: `forge_emit_error(...)`, `forge_convert(...)`
  - Types exposed in headers: `PascalCase` is acceptable for structs (`ErrorStack`, `HtmlTemplate`).
- **Internal / static** symbols: `snake_case`, no prefix.
- Constants / macros: `UPPER_SNAKE_CASE`.

## Code Style

- Standard: **C99** (`-std=c99`).
- Compiler flags: `-Wall -Wextra -Wpedantic -O2`.
- Each logical concern gets its own function. No monolithic functions.
- No third-party libraries — stdlib only unless the project explicitly states otherwise.
- One blank line between top-level definitions. Section banners for logical groups:
  ```c
  /* =========================================================================
   * Section name
   * ========================================================================= */
  ```

## Error Handling

All fallible functions use this pattern:

```c
/* Return 0 on success, 1 on error. Errors are written into stk. */
int forge_foo(..., ErrorStack *stk);
```

### ErrorStack

Dynamic char buffer that accumulates error messages. Lives on the caller's stack.

```c
typedef struct {
    char  *buf;
    size_t len;
    size_t cap;
} ErrorStack;

void        forge_error_stack_init(ErrorStack *stk);  /* zero-init */
void        forge_error_stack_free(ErrorStack *stk);  /* free heap buf */
void        forge_emit_error(ErrorStack *stk, const char *fmt, ...);
const char *forge_error_stack_get(const ErrorStack *stk); /* NULL if empty */
```

Usage:

```c
ErrorStack stk = {0};
forge_error_stack_init(&stk);

if (forge_foo(&stk)) {
    fprintf(stderr, "%s\n", forge_error_stack_get(&stk));
    forge_error_stack_free(&stk);
    return 1;
}

forge_error_stack_free(&stk);
```

### Severity levels

| Situation | Action |
|---|---|
| Fatal (malloc fail, file open fail) | `forge_emit_error` + `return 1` immediately |
| Non-fatal parse/format error | `forge_emit_error` + continue; caller checks at the end |

`forge_emit_error` implementation: `vsnprintf` into a doubling buffer (initial cap 256). Each call appends a newline separator. If `realloc` fails, the message is silently truncated — acceptable for an error-reporting path.

## Build

Each project has a `Makefile` with at minimum:

```makefile
all:    # builds the main binary into .build/
test:   # compiles and runs test binary
clean:  # rm -rf .build
```

Test binaries link `md2html.c` (or equivalent lib file) directly — **not** `main.c`.
POSIX functions in tests require `-D_POSIX_C_SOURCE=200809L`.

## File Layout

```
project/
  foo.h       -- public types and API
  foo.c       -- implementation (static internals + public functions)
  main.c      -- CLI / entry point only
  test.c      -- test cases
  Makefile
  CLAUDE.md   -- project-specific notes
```
