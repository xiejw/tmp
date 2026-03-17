# forge_progress_bar — Design Notes

A single-file C progress bar inspired by Python's `rich` library.

## API

```c
forge_progress_bar_open(int64_t total, const char *title);
forge_progress_bar_advance(int64_t current);   /* absolute position */
forge_progress_bar_close(void);
```

## Layout

```
Processing items [████████████░░░░░░░░░░░░] ETA: 5s  42%
                  ↑ green      ↑ dim        ↑ yellow  ↑ cyan
```

- **Title** — left-aligned, truncated to `PROGRESS_TITLE_MAX` chars (default 20)
- **Bar** — fills remaining terminal width; `█` (filled, green) / `░` (empty, dim)
- **ETA** — yellow; hidden until `current > 0`; unit-scaled (s / m / h)
- **Percentage** — cyan

Bar width = `term_cols - title_len - 1 - 2(brackets) - 1 - right_len`

## Key Design Decisions

**No background thread.** The bar is redrawn synchronously on each
`forge_progress_bar_advance()` call. This keeps the implementation simple and
puts the caller in full control of redraw frequency.

**Absolute position, not delta.** `advance(current)` takes the current absolute
count. This avoids accumulated drift and lets callers call it as often as they
like (e.g., every N items) without bookkeeping.

**Terminal width via `ioctl(TIOCGWINSZ)`**, fallback to 80 columns.

**ETA formula:** `eta = elapsed * (total - current) / current`
ETA is suppressed at `current == 0` (division by zero / meaningless).

**ETA unit scaling** (`fmt_eta` helper):
| Range        | Display |
|-------------|---------|
| < 60 s      | `"12s"` |
| < 3600 s    | `"2m"`  |
| >= 3600 s   | `"1h"`  |

**`forge_progress_bar_close()`** calls `advance(total)` to render the final
100% state, then emits a newline so subsequent output starts cleanly.

## Tuning

| Macro               | Default | How to override              |
|--------------------|---------|------------------------------|
| `PROGRESS_TITLE_MAX` | `20`  | `-DPROGRESS_TITLE_MAX=40`    |

## Build

```bash
make       # build
make run   # build + run demo (~3 s)
make clean
```
