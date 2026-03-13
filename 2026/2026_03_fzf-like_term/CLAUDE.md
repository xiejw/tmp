# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

```bash
make run   # go run fzf
make fmt   # go fmt fzf
go build . # produce ./fzf binary
```

## Architecture

Single-file Go TUI fuzzy finder (`main.go`, ~125 lines). No external UI library — all rendering done with raw ANSI sequences.

**Flow:**
1. Terminal set to raw mode via `golang.org/x/term`
2. Main loop each iteration:
   - Query terminal size (`term.GetSize`)
   - Filter `items` slice (case-insensitive substring match)
   - Re-render full screen
   - Read next keystroke(s)

**Input byte values:**
- `3` = Ctrl+C (exit)
- `13` = Enter (select)
- `127` = Backspace
- `[27, 91, 65]` = ↑ arrow
- `[27, 91, 66]` = ↓ arrow

## ANSI Escape Codes

All sequences begin with `ESC` (`\033`, `\x1b`, or `^[`). The most common form is `ESC[` (CSI — Control Sequence Introducer).

### Used by this program

| Sequence | Effect |
|---|---|
| `\033[2J` | Clear entire screen |
| `\033[H` | Move cursor to top-left (row 1, col 1) |
| `\033[{r};{c}H` | Move cursor to row r, col c (1-based) |
| `\033[{n}A` | Move cursor up n lines |
| `\033[36m` | Foreground: cyan |
| `\033[0m` | Reset all attributes |

Rendering is bottom-up: query prompt is always at the last terminal row; items are placed above it using absolute cursor positioning.

### Cursor movement

| Sequence | Effect |
|---|---|
| `\033[{n}A` | Move cursor up n lines |
| `\033[{n}B` | Move cursor down n lines |
| `\033[{n}C` | Move cursor right n columns |
| `\033[{n}D` | Move cursor left n columns |
| `\033[{n}E` | Move cursor to start of line, n lines down |
| `\033[{n}F` | Move cursor to start of line, n lines up |
| `\033[{n}G` | Move cursor to column n (1-based) |
| `\033[{r};{c}H` | Move cursor to row r, col c (1-based) |
| `\033[H` | Move cursor to top-left (row 1, col 1) |
| `\033[6n` | Query cursor position — terminal replies with `\033[{r};{c}R` |
| `\033[s` / `\033[u` | Save / restore cursor position (ANSI, not all terminals) |
| `\0337` / `\0338` | Save / restore cursor position + attributes (DEC) |

### Erase

| Sequence | Effect |
|---|---|
| `\033[0J` / `\033[J` | Erase from cursor to end of screen |
| `\033[1J` | Erase from cursor to beginning of screen |
| `\033[2J` | Erase entire screen (cursor position unchanged) |
| `\033[3J` | Erase entire screen and scrollback buffer |
| `\033[0K` / `\033[K` | Erase from cursor to end of line |
| `\033[1K` | Erase from cursor to beginning of line |
| `\033[2K` | Erase entire current line |

### Scroll

| Sequence | Effect |
|---|---|
| `\033[{n}S` | Scroll up n lines (new lines added at bottom) |
| `\033[{n}T` | Scroll down n lines (new lines added at top) |

### Text attributes (SGR — `\033[{...}m`)

Multiple attributes can be combined: `\033[1;31m` = bold red.

| Code | Effect |
|---|---|
| `0` | Reset all attributes |
| `1` | Bold / bright |
| `2` | Dim / faint |
| `3` | Italic |
| `4` | Underline |
| `5` | Blink (slow) |
| `6` | Blink (rapid) |
| `7` | Reverse video (swap fg/bg) |
| `8` | Hidden / invisible |
| `9` | Strikethrough |
| `21` | Double underline |
| `22` | Normal intensity (reset bold/dim) |
| `23` | Reset italic |
| `24` | Reset underline |
| `25` | Reset blink |
| `27` | Reset reverse |
| `29` | Reset strikethrough |

### Foreground colors

| Code | Color |
|---|---|
| `30` | Black |
| `31` | Red |
| `32` | Green |
| `33` | Yellow |
| `34` | Blue |
| `35` | Magenta |
| `36` | Cyan |
| `37` | White |
| `39` | Default foreground |
| `90`–`97` | Bright variants (black→white) |

### Background colors

| Code | Color |
|---|---|
| `40` | Black |
| `41` | Red |
| `42` | Green |
| `43` | Yellow |
| `44` | Blue |
| `45` | Magenta |
| `46` | Cyan |
| `47` | White |
| `49` | Default background |
| `100`–`107` | Bright variants (black→white) |

### 256-color and true color (24-bit)

| Sequence | Effect |
|---|---|
| `\033[38;5;{n}m` | Foreground: 256-color palette (0–255) |
| `\033[48;5;{n}m` | Background: 256-color palette (0–255) |
| `\033[38;2;{r};{g};{b}m` | Foreground: RGB true color |
| `\033[48;2;{r};{g};{b}m` | Background: RGB true color |

### Cursor visibility and style

| Sequence | Effect |
|---|---|
| `\033[?25l` | Hide cursor |
| `\033[?25h` | Show cursor |
| `\033[?12l` | Disable cursor blinking |
| `\033[?12h` | Enable cursor blinking |
| `\033[0 q` | Reset cursor style to terminal default |
| `\033[1 q` / `\033[2 q` | Blinking / steady block |
| `\033[3 q` / `\033[4 q` | Blinking / steady underline |
| `\033[5 q` / `\033[6 q` | Blinking / steady bar (I-beam) |

### Alternate screen buffer

| Sequence | Effect |
|---|---|
| `\033[?1049h` | Switch to alternate screen buffer (saves main screen) |
| `\033[?1049l` | Switch back to main screen buffer (restores it) |

Full-screen TUI apps (vim, fzf, etc.) enter the alternate buffer on start and leave it on exit, so the user's shell history is preserved.

### Window / terminal title

| Sequence | Effect |
|---|---|
| `\033]0;{text}\007` | Set both icon name and window title |
| `\033]1;{text}\007` | Set icon name only |
| `\033]2;{text}\007` | Set window title only |

(`\007` = BEL character; `\033\\` — ESC followed by `\` — is an equivalent terminator.)

### Hyperlinks (OSC 8)

```
\033]8;;{url}\007{visible text}\033]8;;\007
```

Supported by most modern terminals (iTerm2, Kitty, GNOME Terminal, Windows Terminal).

### Mouse reporting

| Sequence | Effect |
|---|---|
| `\033[?1000h` / `\033[?1000l` | Enable / disable mouse click reporting |
| `\033[?1002h` / `\033[?1002l` | Enable / disable mouse button+motion reporting |
| `\033[?1003h` / `\033[?1003l` | Enable / disable all mouse motion reporting |
| `\033[?1006h` / `\033[?1006l` | Enable / disable SGR extended mouse coordinates |

### Bracketed paste

| Sequence | Effect |
|---|---|
| `\033[?2004h` | Enable bracketed paste mode |
| `\033[?2004l` | Disable bracketed paste mode |

When enabled, pasted text is wrapped in `\033[200~` … `\033[201~` so the app can distinguish typed input from paste.
