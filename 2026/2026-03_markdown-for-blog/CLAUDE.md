# mdv — Go Markdown-to-HTML Converter

## Overview

A Go command-line tool that converts Markdown to HTML or ANSI-colored terminal
output. Single-pass, line-by-line, no AST, no third-party dependencies.

## Files

```
go.mod
cmd/main.go          -- CLI, flag parsing, orchestration
src/
  ast.go             -- block and inline AST node types
  parser.go          -- markdown → AST (Parse, ParseLines, Convert)
  html_render.go     -- AST → HTML (RenderHTML)
  cli_render.go      -- AST → ANSI terminal output (RenderCLI)
  template.go        -- HtmlTemplate type and ParseTemplate
  parser_test.go
Makefile
```

## Build

```sh
make compile       # produces .build/mdv
make test
make fmt
make clean
```

## Usage

```sh
.build/mdv [-i input.md] [-o output.html] [-t template.tmpl]
```

| Invocation | Input | Output |
|---|---|---|
| `mdv` | stdin | CLI (ANSI) → stdout |
| `mdv -i file.md` | file | CLI (ANSI) → stdout |
| `mdv -i file.md -o out.html` | file | HTML → `out.html` |
| `mdv -o out.html` | stdin | HTML → `out.html` |
| `mdv -i file.md -o out.html -t tmpl.tmpl` | file | HTML with template → `out.html` |

Output mode is determined by `-o`: present → HTML render, absent → CLI (ANSI) render.
`-t` is only meaningful alongside `-o`.

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

Block elements:
- Headings: `# H1` through `###### H6`
- Paragraphs (soft line breaks within a paragraph)
- Fenced code blocks (three backstick or `~~~`, with optional language tag)
- Blockquotes (`> ...`)
- Unordered lists (`-`, `*`, `+`) and ordered lists, with nesting via indent
- Tables (pipe-delimited, header + separator + body rows)
Inline elements:
- Bold: `**text**` or `__text__` → `<strong>`
- Italic: `*text*` or `_text_` → `<em>`
- Inline links: `[text](url)` → `<a href="url">`
- Reference links: `[text][id]` resolved via `[id]: url` definitions
- Inline raw: `` `text` `` → verbatim, no HTML escaping
- HTML escaping of `<`, `>`, `&` (except inside inline raw spans)

Note: bold/italic markers are matched greedily (first closing marker wins).
`_` inside identifiers (e.g. `foo_bar`) should be written in a code span.
Using `[text][id]` with an undefined `id` is a parse error.

## Public API (`src/` package `mdv`)

```go
// AST construction
func Parse(inputPath string) ([]Node, error)
func ParseLines(lines []string) ([]Node, error)

// Rendering
func RenderHTML(nodes []Node, out io.Writer, tmpl *HtmlTemplate, hooks HeadingHooks) error
func RenderCLI(nodes []Node, out io.Writer) error

// Convenience wrapper (parse + HTML render in one call)
func Convert(inputPath string, out io.Writer, tmpl *HtmlTemplate, hooks HeadingHooks) error

// Template
func ParseTemplate(path string) (*HtmlTemplate, error)

// Heading hooks (HTML only)
type HeadingHookFn func(level int, text string, w io.Writer) error
type HeadingHooks [6]HeadingHookFn  // index 0 = h1, index 5 = h6
```

## AST Node Types (`src/ast.go`)

Block nodes implement `Node`:
- `HeadingNode{Level int; Content []InlineNode}`
- `ParagraphNode{Lines [][]InlineNode}`
- `CodeBlockNode{Lang string; Lines []string}`
- `BlockquoteNode{Lines [][]InlineNode}`
- `ListNode{Ordered bool; Items []ListItem}` — `ListItem.Sub *ListNode` for nesting
- `TableNode{Headers [][]InlineNode; Rows [][][]InlineNode}`
- `RawNode{Text string}` — verbatim inline output, no escaping

Inline nodes implement `InlineNode`:
- `TextNode{Text string}`
- `BoldNode{Content []InlineNode}`
- `ItalicNode{Content []InlineNode}`
- `LinkNode{Text []InlineNode; URL string}`

## Architecture

- `parser.go`: reads the file, splits into lines, dispatches each line to a
  block parser in priority order: heading → fence → blockquote → list → table →
  paragraph. `parseInline` converts inline markdown to `[]InlineNode` in one pass.
- `html_render.go`: walks `[]Node`, type-switches to per-node render methods.
  Heading hooks receive the heading text as a plain string.
- `cli_render.go`: same walk but emits ANSI escape sequences. Headings use
  bold + level color; code blocks use dim; blockquotes use a yellow `│` prefix;
  links use underline + yellow URL; bold/italic use SGR 1/3.
