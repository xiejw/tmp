package md2html

import (
	"fmt"
	"io"
	"os"
	"strings"
)

// HeadingHookFn is an optional custom renderer for a heading level.
// level is 1–6; text is the heading content; w receives the output.
// Return nil on success or a non-nil error to abort conversion.
type HeadingHookFn func(level int, text string, w io.Writer) error

// HeadingHooks holds one optional hook per heading level.
// Index 0 = h1, index 5 = h6. A nil entry uses the default <hN> renderer.
type HeadingHooks [6]HeadingHookFn

// Convert reads Markdown from inputPath, converts it to HTML, and writes the
// result to out. tmpl wraps the output when non-nil. hooks override the default
// heading renderers when set.
func Convert(inputPath string, out io.Writer, tmpl *HtmlTemplate, hooks HeadingHooks) error {
	data, err := os.ReadFile(inputPath)
	if err != nil {
		return fmt.Errorf("open %q: %w", inputPath, err)
	}

	lines := strings.Split(string(data), "\n")
	// Drop the empty string produced by a trailing newline.
	if len(lines) > 0 && lines[len(lines)-1] == "" {
		lines = lines[:len(lines)-1]
	}

	p := &parser{lines: lines, out: out, hooks: hooks}

	if tmpl != nil {
		io.WriteString(out, tmpl.BeginHTML)
	}
	p.run()
	if tmpl != nil {
		io.WriteString(out, tmpl.EndHTML)
	}
	return nil
}

// === --- Parser state --------------------------------------------------- ===

type parser struct {
	lines       []string
	pos         int
	out         io.Writer
	hooks       HeadingHooks
	inParagraph bool
}

func (p *parser) run() {
	for p.pos < len(p.lines) {
		line := p.lines[p.pos]

		if level, text, ok := headingParts(line); ok {
			p.closeParagraph()
			p.renderHeading(level, text)
			p.pos++
			continue
		}

		if isBlank(line) {
			p.closeParagraph()
			p.pos++
			continue
		}

		if fc, flen, ok := fenceOpen(line); ok {
			p.closeParagraph()
			p.parseFence(fc, flen, line[flen:])
			continue
		}

		if isBlockquote(line) {
			p.closeParagraph()
			p.parseBlockquote()
			continue
		}

		if indent, _, ok := unorderedItem(line); ok {
			p.closeParagraph()
			p.parseListAt(indent, false)
			continue
		}
		if indent, _, _, ok := orderedItem(line); ok {
			p.closeParagraph()
			p.parseListAt(indent, true)
			continue
		}

		if isTableRow(line) && p.pos+1 < len(p.lines) && isTableSeparator(p.lines[p.pos+1]) {
			p.closeParagraph()
			p.parseTable()
			continue
		}

		p.parseParagraph()
	}
	p.closeParagraph()
}

// === --- Helpers -------------------------------------------------------- ===

func (p *parser) closeParagraph() {
	if p.inParagraph {
		io.WriteString(p.out, "</p>\n")
		p.inParagraph = false
	}
}

func (p *parser) write(s string) { io.WriteString(p.out, s) }

// === --- Heading -------------------------------------------------------- ===

func headingParts(line string) (level int, text string, ok bool) {
	lv := 0
	for lv < len(line) && line[lv] == '#' {
		lv++
	}
	if lv == 0 || lv > 6 {
		return 0, "", false
	}
	if lv >= len(line) || line[lv] != ' ' {
		return 0, "", false
	}
	return lv, line[lv+1:], true
}

func (p *parser) renderHeading(level int, text string) {
	if fn := p.hooks[level-1]; fn != nil {
		fn(level, text, p.out)
		return
	}
	fmt.Fprintf(p.out, "<h%d>", level)
	renderInline(text, p.out)
	fmt.Fprintf(p.out, "</h%d>\n", level)
}

// === --- Fenced code block ---------------------------------------------- ===

func fenceOpen(line string) (fc byte, flen int, ok bool) {
	if len(line) == 0 {
		return 0, 0, false
	}
	c := line[0]
	if c != '`' && c != '~' {
		return 0, 0, false
	}
	n := 0
	for n < len(line) && line[n] == c {
		n++
	}
	if n < 3 {
		return 0, 0, false
	}
	return c, n, true
}

func fenceClose(line string, fc byte, flen int) bool {
	n := 0
	for n < len(line) && line[n] == fc {
		n++
	}
	if n < flen {
		return false
	}
	return strings.TrimSpace(line[n:]) == ""
}

func (p *parser) parseFence(fc byte, flen int, afterMarker string) {
	lang := strings.TrimSpace(afterMarker)
	if lang != "" {
		fmt.Fprintf(p.out, "<pre><code class=\"language-%s\">", lang)
	} else {
		p.write("<pre><code>")
	}
	p.pos++

	for p.pos < len(p.lines) {
		line := p.lines[p.pos]
		if fenceClose(line, fc, flen) {
			p.pos++
			break
		}
		writeEscaped(p.out, line)
		p.write("\n")
		p.pos++
	}
	p.write("</code></pre>\n")
}

// === --- Blockquote ----------------------------------------------------- ===

func isBlockquote(line string) bool {
	return len(line) >= 1 && line[0] == '>' && (len(line) == 1 || line[1] == ' ')
}

func (p *parser) parseBlockquote() {
	p.write("<blockquote>\n")
	for p.pos < len(p.lines) && isBlockquote(p.lines[p.pos]) {
		line := p.lines[p.pos]
		text := ""
		if len(line) > 2 {
			text = line[2:]
		}
		p.write("<p>")
		renderInline(text, p.out)
		p.write("</p>\n")
		p.pos++
	}
	p.write("</blockquote>\n")
}

// === --- Lists ---------------------------------------------------------- ===

func unorderedItem(line string) (indent int, text string, ok bool) {
	i := 0
	for i < len(line) && line[i] == ' ' {
		i++
	}
	if i >= len(line) {
		return 0, "", false
	}
	c := line[i]
	if c != '-' && c != '*' && c != '+' {
		return 0, "", false
	}
	if i+1 >= len(line) || line[i+1] != ' ' {
		return 0, "", false
	}
	return i, line[i+2:], true
}

func orderedItem(line string) (indent, num int, text string, ok bool) {
	i := 0
	for i < len(line) && line[i] == ' ' {
		i++
	}
	lead := i
	if i >= len(line) || line[i] < '0' || line[i] > '9' {
		return 0, 0, "", false
	}
	n := 0
	for i < len(line) && line[i] >= '0' && line[i] <= '9' {
		n = n*10 + int(line[i]-'0')
		i++
	}
	if i >= len(line) || (line[i] != '.' && line[i] != ')') {
		return 0, 0, "", false
	}
	i++ // skip '.' or ')'
	if i >= len(line) || line[i] != ' ' {
		return 0, 0, "", false
	}
	return lead, n, line[i+1:], true
}

func (p *parser) parseListAt(baseIndent int, ordered bool) {
	tag := "ul"
	if ordered {
		tag = "ol"
	}
	fmt.Fprintf(p.out, "<%s>\n", tag)

	for p.pos < len(p.lines) {
		line := p.lines[p.pos]

		if isBlank(line) {
			p.pos++
			break
		}

		var (
			indent      int
			text        string
			thisOrdered bool
			ok          bool
		)
		if indent, text, ok = unorderedItem(line); ok {
			thisOrdered = false
		} else if indent, _, text, ok = orderedItem(line); ok {
			thisOrdered = true
		} else {
			break
		}

		if indent < baseIndent {
			break
		}
		if indent > baseIndent {
			// Deeper indent: nested list inside the previous <li>.
			p.parseListAt(indent, thisOrdered)
			p.write("</li>\n")
			continue
		}

		// Same indent level: emit this item.
		p.write("<li>")
		renderInline(text, p.out)
		p.pos++

		// Peek ahead: if the next line is deeper, nest immediately.
		if p.pos < len(p.lines) {
			next := p.lines[p.pos]
			ni, _, nok := unorderedItem(next)
			nio, _, _, noko := orderedItem(next)
			if nok && ni > baseIndent {
				p.write("\n")
				p.parseListAt(ni, false)
				p.write("</li>\n")
			} else if noko && nio > baseIndent {
				p.write("\n")
				p.parseListAt(nio, true)
				p.write("</li>\n")
			} else {
				p.write("</li>\n")
			}
		} else {
			p.write("</li>\n")
		}
	}

	fmt.Fprintf(p.out, "</%s>\n", tag)
}

// === --- Table ---------------------------------------------------------- ===

func isTableRow(line string) bool {
	return len(strings.TrimLeft(line, " ")) > 0 && strings.TrimLeft(line, " ")[0] == '|'
}

func isTableSeparator(line string) bool {
	p := strings.TrimLeft(line, " ")
	if strings.HasPrefix(p, "|") {
		p = p[1:]
	}
	for _, c := range p {
		if c != '|' && c != '-' && c != ':' && c != ' ' && c != '\t' {
			return false
		}
	}
	return strings.ContainsRune(line, '-')
}

func splitTableRow(line string) []string {
	line = strings.TrimLeft(line, " \t")
	if strings.HasPrefix(line, "|") {
		line = line[1:]
	}
	var cells []string
	for line != "" {
		idx := strings.IndexByte(line, '|')
		if idx < 0 {
			if cell := strings.TrimSpace(line); cell != "" {
				cells = append(cells, cell)
			}
			break
		}
		cells = append(cells, strings.TrimSpace(line[:idx]))
		line = line[idx+1:]
	}
	return cells
}

func (p *parser) parseTable() {
	p.write("<table>\n<thead>\n<tr>")
	for _, cell := range splitTableRow(p.lines[p.pos]) {
		p.write("<th>")
		renderInline(cell, p.out)
		p.write("</th>")
	}
	p.write("</tr>\n</thead>\n")
	p.pos += 2 // skip header row and separator row

	p.write("<tbody>\n")
	for p.pos < len(p.lines) && isTableRow(p.lines[p.pos]) {
		p.write("<tr>")
		for _, cell := range splitTableRow(p.lines[p.pos]) {
			p.write("<td>")
			renderInline(cell, p.out)
			p.write("</td>")
		}
		p.write("</tr>\n")
		p.pos++
	}
	p.write("</tbody>\n</table>\n")
}

// === --- Paragraph ------------------------------------------------------ ===

func (p *parser) parseParagraph() {
	line := p.lines[p.pos]
	if !p.inParagraph {
		p.write("<p>")
		p.inParagraph = true
	} else {
		p.write("\n")
	}
	renderInline(line, p.out)
	p.pos++
}

// === --- Inline renderer ------------------------------------------------ ===

// renderInline writes s to w, converting [text](url) links to <a> tags and
// escaping HTML special characters.
func renderInline(s string, w io.Writer) {
	for i := 0; i < len(s); {
		if s[i] == '[' {
			rest := s[i+1:]
			if j := strings.IndexByte(rest, ']'); j >= 0 {
				if j+1 < len(rest) && rest[j+1] == '(' {
					urlPart := rest[j+2:]
					if k := strings.IndexByte(urlPart, ')'); k >= 0 {
						text := rest[:j]
						url := urlPart[:k]
						io.WriteString(w, `<a href="`)
						writeEscaped(w, url)
						io.WriteString(w, `">`)
						writeEscaped(w, text)
						io.WriteString(w, `</a>`)
						i += 1 + j + 2 + k + 1
						continue
					}
				}
			}
		}
		writeEscapedByte(w, s[i])
		i++
	}
}

// writeEscaped writes s to w with HTML escaping of <, >, and &.
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

// === --- Predicates ----------------------------------------------------- ===

func isBlank(line string) bool {
	return strings.TrimSpace(line) == ""
}
