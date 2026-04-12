package mdv

import (
	"fmt"
	"io"
	"strings"
)

// RenderHTML walks nodes and writes HTML to out.
// tmpl wraps the output when non-nil.
// hooks override the default heading renderer for specific levels.
func RenderHTML(nodes []Node, out io.Writer, tmpl *HtmlTemplate, hooks HeadingHooks) error {
	if tmpl != nil {
		io.WriteString(out, tmpl.BeginHTML)
	}
	r := &htmlRenderer{out: out, hooks: hooks}
	if err := r.render(nodes); err != nil {
		return err
	}
	if tmpl != nil {
		io.WriteString(out, tmpl.EndHTML)
	}
	return nil
}

// === --- Renderer ------------------------------------------------------- ===

type htmlRenderer struct {
	out   io.Writer
	hooks HeadingHooks
}

func (r *htmlRenderer) render(nodes []Node) error {
	for _, n := range nodes {
		if err := r.renderNode(n); err != nil {
			return err
		}
	}
	return nil
}

func (r *htmlRenderer) renderNode(n Node) error {
	switch n := n.(type) {
	case HeadingNode:
		return r.renderHeading(n)
	case ParagraphNode:
		return r.renderParagraph(n)
	case CodeBlockNode:
		return r.renderCodeBlock(n)
	case BlockquoteNode:
		return r.renderBlockquote(n)
	case *ListNode:
		return r.renderList(n)
	case TableNode:
		return r.renderTable(n)
	}
	return nil
}

func (r *htmlRenderer) renderHeading(n HeadingNode) error {
	if fn := r.hooks[n.Level-1]; fn != nil {
		var sb strings.Builder
		renderInlineToString(n.Content, &sb)
		return fn(n.Level, sb.String(), r.out)
	}
	fmt.Fprintf(r.out, "<h%d>", n.Level)
	r.renderInline(n.Content)
	fmt.Fprintf(r.out, "</h%d>\n", n.Level)
	return nil
}

func (r *htmlRenderer) renderParagraph(n ParagraphNode) error {
	io.WriteString(r.out, "<p>")
	for i, line := range n.Lines {
		if i > 0 {
			io.WriteString(r.out, "\n")
		}
		r.renderInline(line)
	}
	io.WriteString(r.out, "</p>\n")
	return nil
}

func (r *htmlRenderer) renderCodeBlock(n CodeBlockNode) error {
	if n.Lang != "" {
		fmt.Fprintf(r.out, "<pre><code class=\"language-%s\">", n.Lang)
	} else {
		io.WriteString(r.out, "<pre><code>")
	}
	for _, line := range n.Lines {
		writeEscaped(r.out, line)
		io.WriteString(r.out, "\n")
	}
	io.WriteString(r.out, "</code></pre>\n")
	return nil
}

func (r *htmlRenderer) renderBlockquote(n BlockquoteNode) error {
	io.WriteString(r.out, "<blockquote>\n")
	for _, line := range n.Lines {
		io.WriteString(r.out, "<p>")
		r.renderInline(line)
		io.WriteString(r.out, "</p>\n")
	}
	io.WriteString(r.out, "</blockquote>\n")
	return nil
}

func (r *htmlRenderer) renderList(n *ListNode) error {
	tag := "ul"
	if n.Ordered {
		tag = "ol"
	}
	fmt.Fprintf(r.out, "<%s>\n", tag)
	for _, item := range n.Items {
		io.WriteString(r.out, "<li>")
		r.renderInline(item.Content)
		if item.Sub != nil {
			io.WriteString(r.out, "\n")
			r.renderList(item.Sub)
		}
		io.WriteString(r.out, "</li>\n")
	}
	fmt.Fprintf(r.out, "</%s>\n", tag)
	return nil
}

func (r *htmlRenderer) renderTable(n TableNode) error {
	io.WriteString(r.out, "<table>\n<thead>\n<tr>")
	for _, cell := range n.Headers {
		io.WriteString(r.out, "<th>")
		r.renderInline(cell)
		io.WriteString(r.out, "</th>")
	}
	io.WriteString(r.out, "</tr>\n</thead>\n<tbody>\n")
	for _, row := range n.Rows {
		io.WriteString(r.out, "<tr>")
		for _, cell := range row {
			io.WriteString(r.out, "<td>")
			r.renderInline(cell)
			io.WriteString(r.out, "</td>")
		}
		io.WriteString(r.out, "</tr>\n")
	}
	io.WriteString(r.out, "</tbody>\n</table>\n")
	return nil
}

// renderInline writes inline content directly to r.out with HTML escaping.
func (r *htmlRenderer) renderInline(content []InlineNode) {
	for _, n := range content {
		switch n := n.(type) {
		case TextNode:
			writeEscaped(r.out, n.Text)
		case BoldNode:
			io.WriteString(r.out, "<strong>")
			r.renderInline(n.Content)
			io.WriteString(r.out, "</strong>")
		case ItalicNode:
			io.WriteString(r.out, "<em>")
			r.renderInline(n.Content)
			io.WriteString(r.out, "</em>")
		case LinkNode:
			io.WriteString(r.out, `<a href="`)
			writeEscaped(r.out, n.URL)
			io.WriteString(r.out, `">`)
			r.renderInline(n.Text)
			io.WriteString(r.out, `</a>`)
		case RawNode:
			io.WriteString(r.out, n.Text)
		}
	}
}

// renderInlineToString reconstructs inline content as plain text (no escaping).
// Used to produce the text argument for HeadingHookFn.
func renderInlineToString(content []InlineNode, sb *strings.Builder) {
	for _, n := range content {
		switch n := n.(type) {
		case TextNode:
			sb.WriteString(n.Text)
		case BoldNode:
			renderInlineToString(n.Content, sb)
		case ItalicNode:
			renderInlineToString(n.Content, sb)
		case LinkNode:
			renderInlineToString(n.Text, sb)
		}
	}
}

// === --- HTML escaping -------------------------------------------------- ===

func writeEscaped(w io.Writer, s string) {
	for i := 0; i < len(s); i++ {
		writeEscapedByte(w, s[i])
	}
}

func writeEscapedByte(w io.Writer, c byte) {
	switch c {
	case '<':
		io.WriteString(w, "&lt;")
	case '>':
		io.WriteString(w, "&gt;")
	case '&':
		io.WriteString(w, "&amp;")
	default:
		w.Write([]byte{c})
	}
}
