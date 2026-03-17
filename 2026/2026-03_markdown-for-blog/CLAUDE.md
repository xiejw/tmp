# md2html — C Markdown-to-HTML Parser

## Overview

A simple C99 command-line tool that converts Markdown to HTML. Single-pass,
line-by-line, no AST, no third-party dependencies.

## Files

```
md2html.h   -- public types and API
md2html.c   -- all parsing/rendering logic
main.c      -- CLI, orchestration
Makefile
```

## Build

```sh
make               # produces .build/md2html
make clean
```

## Usage

```sh
.build/md2html -i input.md [-o output.html] [-t template.tmpl]
# -o defaults to stdout
# -t is optional
```

## Template File Format

```
begin: <<EOF
<!DOCTYPE html><html><body>
EOF

end: <<EOF
</body></html>
EOF
```

Both `begin:` and `end:` blocks are required when `-t` is used.

## Supported Markdown

- Headings: `# H1` through `###### H6`
- Paragraphs
- Fenced code blocks (` ``` ` or `~~~`, with optional language tag)
- Blockquotes (`> ...`)
- Unordered lists (`-`, `*`, `+`) and ordered lists, with nesting via indent
- GFM-style pipe tables (header row + separator row required)
- Inline links: `[text](url)`
- HTML escaping of `<`, `>`, `&`

## Architecture

### Error handling

```c
typedef struct { char *buf; size_t len; size_t cap; } ErrorStack;

void forge_error_stack_init(ErrorStack *stk);
void forge_error_stack_free(ErrorStack *stk);
void forge_emit_error(ErrorStack *stk, const char *fmt, ...);
const char *forge_error_stack_get(const ErrorStack *stk);
```

All functions return `0` on success, `1` on error (written into `ErrorStack`).
Non-fatal parse errors accumulate and are reported at the end.
Fatal errors (malloc failure, file open failure) return immediately.

### Heading hooks

```c
typedef int (*heading_hook_fn)(int level, const char *text, size_t text_len,
                               FILE *out, ErrorStack *stk);
typedef struct { heading_hook_fn hooks[6]; } HeadingHooks;
```

Pass a `HeadingHooks *` to `forge_convert`. `NULL` entries use the default
`<hN>` renderer. Pass `NULL` for the whole struct to use all defaults.

### Public API

```c
int  forge_parse_template(const char *path, HtmlTemplate *tmpl, ErrorStack *stk);
void forge_template_free(HtmlTemplate *tmpl);

int  forge_convert(const char *input_path, FILE *out,
                   const HtmlTemplate *tmpl, HeadingHooks *hooks,
                   ErrorStack *stk);
```

### Parser internals (md2html.c)

- Input file loaded into one `malloc`'d buffer; `\n` → `\0`; separate `char **lines` array.
- Dispatch order per line: heading → fenced code → blockquote → list → table → paragraph.
- Inline renderer (`render_inline`) handles links and HTML escaping in a single linear scan.
- List nesting tracked via indent depth; nested lists recurse into `parse_list_at`.

## Naming conventions

- All public functions: `forge_` prefix, `snake_case`
- All internal/static functions: `snake_case`
- Types: `PascalCase` for structs exposed in the header; `snake_case` for internal structs
