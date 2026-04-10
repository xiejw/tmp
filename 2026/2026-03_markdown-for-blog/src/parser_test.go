package md2html

import (
	"bytes"
	"os"
	"testing"
)

// mdToString converts a markdown string to HTML using Convert.
func mdToString(t *testing.T, md string) string {
	t.Helper()
	f, err := os.CreateTemp("", "md2html-test-*.md")
	if err != nil {
		t.Fatal(err)
	}
	defer os.Remove(f.Name())
	f.WriteString(md)
	f.Close()

	var buf bytes.Buffer
	if err := Convert(f.Name(), &buf, nil, HeadingHooks{}); err != nil {
		t.Fatal(err)
	}
	return buf.String()
}

func check(t *testing.T, got, want string) {
	t.Helper()
	if got != want {
		t.Errorf("got:\n%q\nwant:\n%q", got, want)
	}
}

// === --- Heading tests -------------------------------------------------- ===

func TestHeadingH1(t *testing.T) {
	check(t, mdToString(t, "# Hello"), "<h1>Hello</h1>\n")
}

func TestHeadingH2(t *testing.T) {
	check(t, mdToString(t, "## World"), "<h2>World</h2>\n")
}

func TestHeadingH3(t *testing.T) {
	check(t, mdToString(t, "### Three"), "<h3>Three</h3>\n")
}

// === --- Paragraph tests ------------------------------------------------ ===

func TestParagraphSingle(t *testing.T) {
	check(t, mdToString(t, "Hello world"), "<p>Hello world</p>\n")
}

func TestParagraphMultiple(t *testing.T) {
	md := "First paragraph\n\nSecond paragraph\n"
	want := "<p>First paragraph</p>\n<p>Second paragraph</p>\n"
	check(t, mdToString(t, md), want)
}

func TestParagraphSoftBreak(t *testing.T) {
	md := "Line one\nLine two\n"
	want := "<p>Line one\nLine two</p>\n"
	check(t, mdToString(t, md), want)
}

// === --- Inline link tests ---------------------------------------------- ===

func TestInlineLink(t *testing.T) {
	check(t, mdToString(t, "[Go](https://go.dev)"), "<p><a href=\"https://go.dev\">Go</a></p>\n")
}

// === --- HTML escaping tests -------------------------------------------- ===

func TestHtmlEscapeLt(t *testing.T) {
	check(t, mdToString(t, "a < b"), "<p>a &lt; b</p>\n")
}

func TestHtmlEscapeGt(t *testing.T) {
	check(t, mdToString(t, "a > b"), "<p>a &gt; b</p>\n")
}

func TestHtmlEscapeAmp(t *testing.T) {
	check(t, mdToString(t, "a & b"), "<p>a &amp; b</p>\n")
}

// === --- Fenced code block tests ---------------------------------------- ===

func TestFenceNoLang(t *testing.T) {
	md := "```\ncode here\n```\n"
	want := "<pre><code>code here\n</code></pre>\n"
	check(t, mdToString(t, md), want)
}

func TestFenceWithLang(t *testing.T) {
	md := "```go\nfmt.Println()\n```\n"
	want := "<pre><code class=\"language-go\">fmt.Println()\n</code></pre>\n"
	check(t, mdToString(t, md), want)
}

func TestFenceEscapesHTML(t *testing.T) {
	md := "```\na < b\n```\n"
	want := "<pre><code>a &lt; b\n</code></pre>\n"
	check(t, mdToString(t, md), want)
}

// === --- Blockquote tests ----------------------------------------------- ===

func TestBlockquote(t *testing.T) {
	md := "> Hello\n> World\n"
	want := "<blockquote>\n<p>Hello</p>\n<p>World</p>\n</blockquote>\n"
	check(t, mdToString(t, md), want)
}

// === --- List tests ----------------------------------------------------- ===

func TestUnorderedList(t *testing.T) {
	md := "- one\n- two\n- three\n"
	want := "<ul>\n<li>one</li>\n<li>two</li>\n<li>three</li>\n</ul>\n"
	check(t, mdToString(t, md), want)
}

func TestOrderedList(t *testing.T) {
	md := "1. first\n2. second\n"
	want := "<ol>\n<li>first</li>\n<li>second</li>\n</ol>\n"
	check(t, mdToString(t, md), want)
}

func TestNestedList(t *testing.T) {
	md := "- parent\n  - child\n"
	want := "<ul>\n<li>parent\n<ul>\n<li>child</li>\n</ul>\n</li>\n</ul>\n"
	check(t, mdToString(t, md), want)
}

// === --- Table tests ---------------------------------------------------- ===

func TestTable(t *testing.T) {
	md := "| A | B |\n|---|---|\n| 1 | 2 |\n"
	want := "<table>\n<thead>\n<tr><th>A</th><th>B</th></tr>\n</thead>\n" +
		"<tbody>\n<tr><td>1</td><td>2</td></tr>\n</tbody>\n</table>\n"
	check(t, mdToString(t, md), want)
}

// === --- Template wrapping test ----------------------------------------- ===

func TestTemplate(t *testing.T) {
	tmplFile, err := os.CreateTemp("", "md2html-tmpl-*.tmpl")
	if err != nil {
		t.Fatal(err)
	}
	defer os.Remove(tmplFile.Name())
	tmplFile.WriteString("begin: <<EOF\n<html><body>\nEOF\n\nend: <<EOF\n</body></html>\nEOF\n")
	tmplFile.Close()

	tmpl, err := ParseTemplate(tmplFile.Name())
	if err != nil {
		t.Fatal(err)
	}

	mdFile, err := os.CreateTemp("", "md2html-test-*.md")
	if err != nil {
		t.Fatal(err)
	}
	defer os.Remove(mdFile.Name())
	mdFile.WriteString("# Title\n")
	mdFile.Close()

	var buf bytes.Buffer
	if err := Convert(mdFile.Name(), &buf, tmpl, HeadingHooks{}); err != nil {
		t.Fatal(err)
	}
	want := "<html><body>\n<h1>Title</h1>\n</body></html>\n"
	check(t, buf.String(), want)
}
