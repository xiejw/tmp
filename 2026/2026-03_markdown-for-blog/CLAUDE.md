# md2html — Go Markdown-to-HTML Converter

## Overview

A simple Go command-line tool that converts Markdown to HTML. Single-pass,
line-by-line, no AST, no third-party dependencies.

## Files

```
go.mod
cmd/main.go     -- CLI, flag parsing, orchestration
src/
  template.go   -- HtmlTemplate type and ParseTemplate
  parser.go     -- all parsing/rendering logic
  parser_test.go
Makefile
```

## Build

```sh
make compile       # produces .build/md2html
make test
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
- Paragraphs (soft line breaks within a paragraph)
- Fenced code blocks (` ``` ` or `~~~`, with optional language tag)
- Blockquotes (`> ...`)
- Unordered lists (`-`, `*`, `+`) and ordered lists, with nesting via indent
- Inline links: `[text](url)`
- HTML escaping of `<`, `>`, `&`

## Public API (`src/` package `md2html`)

```go
type HtmlTemplate struct {
    BeginHTML string
    EndHTML   string
}

type HeadingHookFn func(level int, text string, w io.Writer) error
type HeadingHooks [6]HeadingHookFn  // index 0 = h1, index 5 = h6

func ParseTemplate(path string) (*HtmlTemplate, error)
func Convert(inputPath string, out io.Writer, tmpl *HtmlTemplate, hooks HeadingHooks) error
```

## Architecture

- Input file loaded with `os.ReadFile`; split into lines on `\n`.
- Dispatch order per line: heading → fenced code → blockquote → list → table → paragraph.
- `renderInline` handles `[text](url)` links and HTML escaping in a single pass.
- List nesting tracked via indent depth; nested lists recurse into `parseListAt`.
