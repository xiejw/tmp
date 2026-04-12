package mdv

import (
	"bytes"
	"os"
	"strings"
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

// === --- Reference link tests ------------------------------------------- ===

func TestRefLink(t *testing.T) {
	md := "[Go][go]\n\n[go]: https://go.dev\n"
	check(t, mdToString(t, md), "<p><a href=\"https://go.dev\">Go</a></p>\n")
}

func TestRefLinkCaseInsensitive(t *testing.T) {
	md := "[Go][GO]\n\n[go]: https://go.dev\n"
	check(t, mdToString(t, md), "<p><a href=\"https://go.dev\">Go</a></p>\n")
}

func TestRefLinkDefNotRendered(t *testing.T) {
	md := "text\n\n[go]: https://go.dev\n"
	check(t, mdToString(t, md), "<p>text</p>\n")
}

func TestRefLinkUndefinedErrors(t *testing.T) {
	_, err := ParseLines(strings.Split("[A][Missing]", "\n"))
	if err == nil {
		t.Fatal("want error for undefined reference, got nil")
	}
}

// === --- Inline raw tests ----------------------------------------------- ===

func TestInlineRaw(t *testing.T) {
	check(t, mdToString(t, "`<b>bold</b>`"), "<p><b>bold</b></p>\n")
}

func TestInlineRawNotEscaped(t *testing.T) {
	check(t, mdToString(t, "`a < b`"), "<p>a < b</p>\n")
}

func TestInlineRawMixedWithText(t *testing.T) {
	check(t, mdToString(t, "see `<em>here</em>` now"), "<p>see <em>here</em> now</p>\n")
}

// === --- Bold / italic tests -------------------------------------------- ===

func TestBoldStar(t *testing.T) {
	check(t, mdToString(t, "**bold**"), "<p><strong>bold</strong></p>\n")
}

func TestBoldUnderscore(t *testing.T) {
	check(t, mdToString(t, "__bold__"), "<p><strong>bold</strong></p>\n")
}

func TestItalicStar(t *testing.T) {
	check(t, mdToString(t, "*italic*"), "<p><em>italic</em></p>\n")
}

func TestItalicUnderscore(t *testing.T) {
	check(t, mdToString(t, "_italic_"), "<p><em>italic</em></p>\n")
}

func TestBoldAndItalicMixed(t *testing.T) {
	check(t, mdToString(t, "**bold** and *italic*"),
		"<p><strong>bold</strong> and <em>italic</em></p>\n")
}

func TestBoldContainingItalic(t *testing.T) {
	check(t, mdToString(t, "**bold and *italic* inside**"),
		"<p><strong>bold and <em>italic</em> inside</strong></p>\n")
}

// === --- AST tests ------------------------------------------------------ ===

// mustParseLines is a test helper that parses markdown lines into an AST.
func mustParseLines(t *testing.T, md string) []Node {
	t.Helper()
	nodes, err := ParseLines(strings.Split(md, "\n"))
	if err != nil {
		t.Fatal(err)
	}
	return nodes
}

func TestASTHeading(t *testing.T) {
	nodes := mustParseLines(t, "# Hello")
	if len(nodes) != 1 {
		t.Fatalf("want 1 node, got %d", len(nodes))
	}
	h, ok := nodes[0].(HeadingNode)
	if !ok {
		t.Fatalf("want HeadingNode, got %T", nodes[0])
	}
	if h.Level != 1 {
		t.Errorf("want level 1, got %d", h.Level)
	}
	if len(h.Content) != 1 {
		t.Fatalf("want 1 inline node, got %d", len(h.Content))
	}
	tn, ok := h.Content[0].(TextNode)
	if !ok {
		t.Fatalf("want TextNode, got %T", h.Content[0])
	}
	if tn.Text != "Hello" {
		t.Errorf("want %q, got %q", "Hello", tn.Text)
	}
}

func TestASTLink(t *testing.T) {
	nodes := mustParseLines(t, "[Go](https://go.dev)")
	if len(nodes) != 1 {
		t.Fatalf("want 1 node, got %d", len(nodes))
	}
	p, ok := nodes[0].(ParagraphNode)
	if !ok {
		t.Fatalf("want ParagraphNode, got %T", nodes[0])
	}
	if len(p.Lines) != 1 || len(p.Lines[0]) != 1 {
		t.Fatalf("want 1 line with 1 inline node")
	}
	ln, ok := p.Lines[0][0].(LinkNode)
	if !ok {
		t.Fatalf("want LinkNode, got %T", p.Lines[0][0])
	}
	if ln.URL != "https://go.dev" {
		t.Errorf("want URL %q, got %q", "https://go.dev", ln.URL)
	}
}

func TestASTNestedList(t *testing.T) {
	nodes := mustParseLines(t, "- parent\n  - child")
	if len(nodes) != 1 {
		t.Fatalf("want 1 node, got %d", len(nodes))
	}
	lst, ok := nodes[0].(*ListNode)
	if !ok {
		t.Fatalf("want *ListNode, got %T", nodes[0])
	}
	if len(lst.Items) != 1 {
		t.Fatalf("want 1 item, got %d", len(lst.Items))
	}
	if lst.Items[0].Sub == nil {
		t.Fatal("want nested Sub list, got nil")
	}
	if len(lst.Items[0].Sub.Items) != 1 {
		t.Errorf("want 1 child item, got %d", len(lst.Items[0].Sub.Items))
	}
}

func TestASTCodeBlock(t *testing.T) {
	nodes := mustParseLines(t, "```go\nfmt.Println()\n```")
	if len(nodes) != 1 {
		t.Fatalf("want 1 node, got %d", len(nodes))
	}
	cb, ok := nodes[0].(CodeBlockNode)
	if !ok {
		t.Fatalf("want CodeBlockNode, got %T", nodes[0])
	}
	if cb.Lang != "go" {
		t.Errorf("want lang %q, got %q", "go", cb.Lang)
	}
	if len(cb.Lines) != 1 || cb.Lines[0] != "fmt.Println()" {
		t.Errorf("unexpected code lines: %v", cb.Lines)
	}
}

// === --- CLI render tests ----------------------------------------------- ===

func cliToString(t *testing.T, md string) string {
	t.Helper()
	nodes := mustParseLines(t, md)
	var buf bytes.Buffer
	if err := RenderCLI(nodes, &buf); err != nil {
		t.Fatal(err)
	}
	return buf.String()
}

func TestCLIHeadingBold(t *testing.T) {
	out := cliToString(t, "# Hello")
	if !strings.Contains(out, ansiBold) {
		t.Error("want bold escape in heading output")
	}
	if !strings.Contains(out, "Hello") {
		t.Error("want heading text in output")
	}
}

func TestCLICodeBlock(t *testing.T) {
	out := cliToString(t, "```\ncode\n```")
	if !strings.Contains(out, ansiDim) {
		t.Error("want dim escape in code block output")
	}
	if !strings.Contains(out, "code") {
		t.Error("want code text in output")
	}
}

func TestCLILink(t *testing.T) {
	out := cliToString(t, "[Go](https://go.dev)")
	if !strings.Contains(out, ansiUnderline) {
		t.Error("want underline escape for link text")
	}
	if !strings.Contains(out, "https://go.dev") {
		t.Error("want URL in link output")
	}
}

func TestCLITable(t *testing.T) {
	out := cliToString(t, "| A | B |\n|---|---|\n| 1 | 2 |\n")
	if !strings.Contains(out, "┌") {
		t.Error("want box-drawing border in table output")
	}
	if !strings.Contains(out, "A") || !strings.Contains(out, "1") {
		t.Error("want cell content in table output")
	}
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
