package mdv

import (
	"io"
	"strconv"
	"strings"
	"unicode/utf8"
)

// RenderCLI walks nodes and writes ANSI-colored terminal output to out.
func RenderCLI(nodes []Node, out io.Writer) error {
	r := &cliRenderer{out: out}
	return r.render(nodes, 0)
}

// === --- ANSI constants ------------------------------------------------- ===

const (
	ansiReset     = "\033[0m"
	ansiBold      = "\033[1m"
	ansiItalic    = "\033[3m"
	ansiDim       = "\033[2m"
	ansiUnderline = "\033[4m"
	ansiFgCyan    = "\033[36m"
	ansiFgBlue    = "\033[34m"
	ansiFgMagenta = "\033[35m"
	ansiFgGreen   = "\033[32m"
	ansiFgYellow  = "\033[33m"
	ansiFgWhite   = "\033[37m"
)

var headingColors = [6]string{
	ansiFgCyan,
	ansiFgBlue,
	ansiFgMagenta,
	ansiFgGreen,
	ansiFgYellow,
	ansiFgWhite,
}

// === --- Renderer ------------------------------------------------------- ===

type cliRenderer struct {
	out io.Writer
}

func (r *cliRenderer) render(nodes []Node, depth int) error {
	for _, n := range nodes {
		if err := r.renderNode(n, depth); err != nil {
			return err
		}
	}
	return nil
}

func (r *cliRenderer) renderNode(n Node, depth int) error {
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
		return r.renderList(n, depth)
	case TableNode:
		return r.renderTable(n)
	}
	return nil
}

func (r *cliRenderer) w(s string) { io.WriteString(r.out, s) }

func (r *cliRenderer) renderHeading(n HeadingNode) error {
	color := headingColors[n.Level-1]
	r.w(ansiBold + color + r.renderInline(n.Content) + ansiReset + "\n\n")
	return nil
}

func (r *cliRenderer) renderParagraph(n ParagraphNode) error {
	for _, line := range n.Lines {
		r.w(r.renderInline(line) + "\n")
	}
	r.w("\n")
	return nil
}

func (r *cliRenderer) renderCodeBlock(n CodeBlockNode) error {
	header := "```"
	if n.Lang != "" {
		header += n.Lang
	}
	r.w(ansiBold + header + ansiReset + "\n")
	for _, line := range n.Lines {
		r.w(ansiDim + line + ansiReset + "\n")
	}
	r.w(ansiBold + "```" + ansiReset + "\n\n")
	return nil
}

func (r *cliRenderer) renderBlockquote(n BlockquoteNode) error {
	for _, line := range n.Lines {
		r.w(ansiFgYellow + "│ " + ansiReset + r.renderInline(line) + "\n")
	}
	r.w("\n")
	return nil
}

func (r *cliRenderer) renderList(n *ListNode, depth int) error {
	pad := strings.Repeat("  ", depth)
	for i, item := range n.Items {
		var bullet string
		if n.Ordered {
			bullet = strconv.Itoa(i+1) + "."
		} else {
			bullet = "•"
		}
		r.w(pad + ansiFgGreen + bullet + " " + ansiReset + r.renderInline(item.Content) + "\n")
		if item.Sub != nil {
			r.renderList(item.Sub, depth+1)
		}
	}
	if depth == 0 {
		r.w("\n")
	}
	return nil
}

func (r *cliRenderer) renderTable(n TableNode) error {
	// Compute per-column display widths (in runes, ignoring ANSI escapes).
	cols := len(n.Headers)
	widths := make([]int, cols)
	for i, h := range n.Headers {
		widths[i] = runeLen(r.renderInline(h))
	}
	for _, row := range n.Rows {
		for i, cell := range row {
			if i < cols {
				if w := runeLen(r.renderInline(cell)); w > widths[i] {
					widths[i] = w
				}
			}
		}
	}

	// Box-drawing helpers.
	hbar := func(w int) string { return strings.Repeat("─", w+2) }
	pad := func(s string, w int) string {
		return s + strings.Repeat(" ", w-runeLen(s))
	}

	// Top border.
	r.w("┌")
	for i, w := range widths {
		r.w(hbar(w))
		if i < cols-1 {
			r.w("┬")
		}
	}
	r.w("┐\n")

	// Header row.
	r.w("│")
	for i, h := range n.Headers {
		text := r.renderInline(h)
		r.w(" " + ansiBold + pad(text, widths[i]) + ansiReset + " │")
		_ = i
	}
	r.w("\n")

	// Header/body separator.
	r.w("├")
	for i, w := range widths {
		r.w(hbar(w))
		if i < cols-1 {
			r.w("┼")
		}
	}
	r.w("┤\n")

	// Body rows.
	for _, row := range n.Rows {
		r.w("│")
		for i := 0; i < cols; i++ {
			text := ""
			if i < len(row) {
				text = r.renderInline(row[i])
			}
			r.w(" " + pad(text, widths[i]) + " │")
		}
		r.w("\n")
	}

	// Bottom border.
	r.w("└")
	for i, w := range widths {
		r.w(hbar(w))
		if i < cols-1 {
			r.w("┴")
		}
	}
	r.w("┘\n\n")
	return nil
}

// renderInline returns the ANSI-rendered inline content as a string.
func (r *cliRenderer) renderInline(content []InlineNode) string {
	var sb strings.Builder
	for _, n := range content {
		switch n := n.(type) {
		case TextNode:
			sb.WriteString(n.Text)
		case BoldNode:
			sb.WriteString(ansiBold + r.renderInline(n.Content) + ansiReset)
		case ItalicNode:
			sb.WriteString(ansiItalic + r.renderInline(n.Content) + ansiReset)
		case LinkNode:
			sb.WriteString(ansiUnderline + r.renderInline(n.Text) + ansiReset)
			sb.WriteString(ansiFgYellow + " (" + n.URL + ")" + ansiReset)
		case RawNode:
			sb.WriteString(ansiDim + n.Text + ansiReset)
		}
	}
	return sb.String()
}

// runeLen returns the number of Unicode code points in s, ignoring ANSI
// escape sequences so that column widths are computed on visible characters.
func runeLen(s string) int {
	// Strip ANSI escape sequences before counting.
	clean := stripANSI(s)
	return utf8.RuneCountInString(clean)
}

// stripANSI removes ANSI CSI escape sequences (ESC [ ... m) from s.
func stripANSI(s string) string {
	var sb strings.Builder
	i := 0
	for i < len(s) {
		if s[i] == '\033' && i+1 < len(s) && s[i+1] == '[' {
			i += 2
			for i < len(s) && s[i] != 'm' {
				i++
			}
			i++ // skip 'm'
			continue
		}
		sb.WriteByte(s[i])
		i++
	}
	return sb.String()
}

// indent returns depth*2 spaces.
func indent(depth int) string { return strings.Repeat("  ", depth) }

// Ensure indent is used (renderList inlines the logic but indent is exported).
var _ = indent
