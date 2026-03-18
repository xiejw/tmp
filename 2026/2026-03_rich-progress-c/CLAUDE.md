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

**Background thread redraws every 200 ms.** A POSIX thread (`pthread`) started
in `forge_progress_bar_open()` calls `render_bar()` under `s_mutex` at 200 ms
intervals. This keeps the display live even when `advance()` is called
infrequently. `forge_progress_bar_close()` sets `s_running = 0` and joins the
thread before final rendering.

**`advance()` also redraws immediately** (under the mutex), so fast callers get
instant feedback without waiting for the background tick.

**Absolute position, not delta.** `advance(current)` takes the current absolute
count. This avoids accumulated drift and lets callers call it as often as they
like (e.g., every N items) without bookkeeping.

**Terminal width via `ioctl(TIOCGWINSZ)`**, fallback to 80 columns.

**ETA — sliding-window moving average (MA_SIZE=16).** Each `advance()` call
pushes a `{timestamp, count}` sample into a ring buffer. When ≥ 2 samples exist:
- `rate = (newest.count − oldest.count) / (newest.t − oldest.t)`
- `eta  = (total − current) / rate`

Falls back to `elapsed * (1−pct) / pct` until 2 samples are available.
ETA is suppressed at `current == 0`.

**ETA unit scaling** (`fmt_eta` helper):
| Range        | Display |
|-------------|---------|
| < 60 s      | `"12s"` |
| < 3600 s    | `"2m"`  |
| >= 3600 s   | `"1h"`  |

**`forge_progress_bar_close()`** stops the background thread and renders at
`s_current` (the last position passed to `advance()`), then emits a newline.
It does **not** force 100% — if the caller stops early, the bar reflects that.

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
