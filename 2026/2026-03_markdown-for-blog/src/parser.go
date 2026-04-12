package mdv

import (
	"errors"
	"fmt"
	"io"
	"os"
	"strings"
)

// HeadingHookFn is an optional custom renderer for a heading level (HTML only).
// level is 1–6; text is the heading content as plain text; w receives the output.
type HeadingHookFn func(level int, text string, w io.Writer) error

// HeadingHooks holds one optional hook per heading level.
// Index 0 = h1, index 5 = h6. A nil entry uses the default <hN> renderer.
type HeadingHooks [6]HeadingHookFn

// Parse reads a Markdown file and returns its AST.
func Parse(inputPath string) ([]Node, error) {
	data, err := os.ReadFile(inputPath)
	if err != nil {
		return nil, fmt.Errorf("open %q: %w", inputPath, err)
	}
	lines := strings.Split(string(data), "\n")
	if len(lines) > 0 && lines[len(lines)-1] == "" {
		lines = lines[:len(lines)-1]
	}
	return ParseLines(lines)
}

// ParseLines parses a slice of Markdown lines into an AST.
// It is the pure core of Parse; useful for testing without file I/O.
// Returns an error if any reference-style link uses an undefined id.
func ParseLines(lines []string) ([]Node, error) {
	p := &parser{lines: lines, refs: collectLinkRefs(lines)}
	p.run()
	if len(p.errs) > 0 {
		return nil, errors.New(strings.Join(p.errs, "\n"))
	}
	return p.nodes, nil
}

// Convert is a convenience wrapper: parse + render HTML in one call.
// It preserves the original API for existing callers.
func Convert(inputPath string, out io.Writer, tmpl *HtmlTemplate, hooks HeadingHooks) error {
	nodes, err := Parse(inputPath)
	if err != nil {
		return err
	}
	return RenderHTML(nodes, out, tmpl, hooks)
}

// === --- Parser state --------------------------------------------------- ===

type parser struct {
	lines     []string
	pos       int
	nodes     []Node
	paraLines [][]InlineNode    // nil when not accumulating a paragraph
	refs      map[string]string // link reference definitions (lowercase id → url)
	errs      []string          // accumulated parse errors
}

func (p *parser) run() {
	for p.pos < len(p.lines) {
		line := p.lines[p.pos]

		if isBlank(line) {
			p.closeParagraph()
			p.pos++
			continue
		}

		// Link reference definitions are block-level metadata; skip them.
		if _, _, ok := parseLinkDef(line); ok {
			p.closeParagraph()
			p.pos++
			continue
		}

		if level, text, ok := headingParts(line); ok {
			p.closeParagraph()
			p.nodes = append(p.nodes, HeadingNode{Level: level, Content: p.parseInline(text)})
			p.pos++
			continue
		}

		if fc, flen, ok := fenceOpen(line); ok {
			p.closeParagraph()
			p.nodes = append(p.nodes, p.parseFence(fc, flen, line[flen:]))
			continue
		}

		if isBlockquote(line) {
			p.closeParagraph()
			p.nodes = append(p.nodes, p.parseBlockquote())
			continue
		}

		if indent, _, ok := unorderedItem(line); ok {
			p.closeParagraph()
			p.nodes = append(p.nodes, p.parseListAt(indent, false))
			continue
		}
		if indent, _, _, ok := orderedItem(line); ok {
			p.closeParagraph()
			p.nodes = append(p.nodes, p.parseListAt(indent, true))
			continue
		}

		if isTableRow(line) && p.pos+1 < len(p.lines) && isTableSeparator(p.lines[p.pos+1]) {
			p.closeParagraph()
			p.nodes = append(p.nodes, p.parseTable())
			continue
		}

		// Paragraph: accumulate lines.
		p.paraLines = append(p.paraLines, p.parseInline(line))
		p.pos++
	}
	p.closeParagraph()
}

func (p *parser) closeParagraph() {
	if p.paraLines != nil {
		p.nodes = append(p.nodes, ParagraphNode{Lines: p.paraLines})
		p.paraLines = nil
	}
}

// === --- Fenced code block ---------------------------------------------- ===

func (p *parser) parseFence(fc byte, flen int, afterMarker string) Node {
	lang := strings.TrimSpace(afterMarker)
	p.pos++
	var lines []string
	for p.pos < len(p.lines) {
		line := p.lines[p.pos]
		if fenceClose(line, fc, flen) {
			p.pos++
			break
		}
		lines = append(lines, line)
		p.pos++
	}
	return CodeBlockNode{Lang: lang, Lines: lines}
}

// === --- Blockquote ----------------------------------------------------- ===

func (p *parser) parseBlockquote() Node {
	var lines [][]InlineNode
	for p.pos < len(p.lines) && isBlockquote(p.lines[p.pos]) {
		line := p.lines[p.pos]
		text := ""
		if len(line) > 2 {
			text = line[2:]
		}
		lines = append(lines, p.parseInline(text))
		p.pos++
	}
	return BlockquoteNode{Lines: lines}
}

// === --- Lists ---------------------------------------------------------- ===

func (p *parser) parseListAt(baseIndent int, ordered bool) *ListNode {
	node := &ListNode{Ordered: ordered}

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
			// Deeper indent: attach as Sub to the last item.
			if len(node.Items) > 0 {
				last := &node.Items[len(node.Items)-1]
				last.Sub = p.parseListAt(indent, thisOrdered)
			} else {
				p.parseListAt(indent, thisOrdered)
			}
			continue
		}

		// Same indent: new item.
		item := ListItem{Content: p.parseInline(text)}
		p.pos++

		// Peek: if the next line is deeper, nest immediately.
		if p.pos < len(p.lines) {
			next := p.lines[p.pos]
			ni, _, nok := unorderedItem(next)
			nio, _, _, noko := orderedItem(next)
			if nok && ni > baseIndent {
				item.Sub = p.parseListAt(ni, false)
			} else if noko && nio > baseIndent {
				item.Sub = p.parseListAt(nio, true)
			}
		}

		node.Items = append(node.Items, item)
	}

	return node
}

// === --- Table ---------------------------------------------------------- ===

func (p *parser) parseTable() Node {
	headers := splitTableRow(p.lines[p.pos])
	p.pos += 2 // skip header and separator rows

	node := TableNode{Headers: make([][]InlineNode, len(headers))}
	for i, h := range headers {
		node.Headers[i] = p.parseInline(h)
	}

	for p.pos < len(p.lines) && isTableRow(p.lines[p.pos]) {
		cells := splitTableRow(p.lines[p.pos])
		row := make([][]InlineNode, len(cells))
		for i, c := range cells {
			row[i] = p.parseInline(c)
		}
		node.Rows = append(node.Rows, row)
		p.pos++
	}
	return node
}

// === --- Inline parser -------------------------------------------------- ===

// parseInline parses an inline markdown string into a slice of InlineNodes.
// Supported spans (checked in priority order):
//
//	**text** or __text__  →  BoldNode
//	*text*   or _text_    →  ItalicNode
//	[text](url)           →  LinkNode  (inline)
//	[text][id]            →  LinkNode  (reference; id looked up in p.refs)
func (p *parser) parseInline(s string) []InlineNode {
	var result []InlineNode
	var buf strings.Builder

	flush := func() {
		if buf.Len() > 0 {
			result = append(result, TextNode{Text: buf.String()})
			buf.Reset()
		}
	}

	for i := 0; i < len(s); {
		// Raw inline: `text`
		if s[i] == '`' {
			if j := strings.IndexByte(s[i+1:], '`'); j >= 0 {
				flush()
				result = append(result, RawNode{Text: s[i+1 : i+1+j]})
				i += j + 2
				continue
			}
		}
		// Bold: ** (must check before *)
		if i+1 < len(s) && s[i] == '*' && s[i+1] == '*' {
			if j := strings.Index(s[i+2:], "**"); j >= 0 {
				flush()
				result = append(result, BoldNode{Content: p.parseInline(s[i+2 : i+2+j])})
				i += j + 4
				continue
			}
		}
		// Bold: __ (must check before _)
		if i+1 < len(s) && s[i] == '_' && s[i+1] == '_' {
			if j := strings.Index(s[i+2:], "__"); j >= 0 {
				flush()
				result = append(result, BoldNode{Content: p.parseInline(s[i+2 : i+2+j])})
				i += j + 4
				continue
			}
		}
		// Italic: *
		if s[i] == '*' {
			if j := strings.IndexByte(s[i+1:], '*'); j >= 0 {
				flush()
				result = append(result, ItalicNode{Content: p.parseInline(s[i+1 : i+1+j])})
				i += j + 2
				continue
			}
		}
		// Italic: _
		if s[i] == '_' {
			if j := strings.IndexByte(s[i+1:], '_'); j >= 0 {
				flush()
				result = append(result, ItalicNode{Content: p.parseInline(s[i+1 : i+1+j])})
				i += j + 2
				continue
			}
		}
		// Links: [text](url) inline  or  [text][id] reference
		if s[i] == '[' {
			rest := s[i+1:]
			if j := strings.IndexByte(rest, ']'); j >= 0 {
				after := rest[j+1:]
				// Inline link: [text](url)
				if len(after) > 0 && after[0] == '(' {
					urlPart := after[1:]
					if k := strings.IndexByte(urlPart, ')'); k >= 0 {
						flush()
						result = append(result, LinkNode{
							Text: p.parseInline(rest[:j]),
							URL:  urlPart[:k],
						})
						i += 1 + j + 2 + k + 1
						continue
					}
				}
				// Reference link: [text][id]
				if len(after) > 0 && after[0] == '[' {
					idPart := after[1:]
					if k := strings.IndexByte(idPart, ']'); k >= 0 {
						id := strings.ToLower(idPart[:k])
						if url, found := p.refs[id]; found {
							flush()
							result = append(result, LinkNode{
								Text: p.parseInline(rest[:j]),
								URL:  url,
							})
							i += 1 + j + 2 + k + 1
							continue
						}
						p.errs = append(p.errs,
							fmt.Sprintf("undefined link reference %q", idPart[:k]))
						// Fall through: add the raw '[' to buf and retry from i+1.
					}
				}
			}
		}
		buf.WriteByte(s[i])
		i++
	}
	flush()
	return result
}

// === --- Link reference definitions ------------------------------------- ===

// parseLinkDef detects a link reference definition of the form:
//
//	[id]: url
//	[id]: <url>
//
// Leading whitespace is allowed. id is returned un-lowercased.
func parseLinkDef(line string) (id, url string, ok bool) {
	s := strings.TrimLeft(line, " \t")
	if !strings.HasPrefix(s, "[") {
		return "", "", false
	}
	j := strings.IndexByte(s, ']')
	if j < 1 { // need at least one char for id
		return "", "", false
	}
	if j+1 >= len(s) || s[j+1] != ':' {
		return "", "", false
	}
	id = s[1:j]
	rest := strings.TrimSpace(s[j+2:])
	if rest == "" {
		return "", "", false
	}
	if strings.HasPrefix(rest, "<") {
		end := strings.IndexByte(rest, '>')
		if end < 0 {
			return "", "", false
		}
		url = rest[1:end]
	} else {
		url = strings.Fields(rest)[0]
	}
	return id, url, true
}

// collectLinkRefs does a pre-pass over all lines to gather link definitions.
func collectLinkRefs(lines []string) map[string]string {
	refs := make(map[string]string)
	for _, line := range lines {
		if id, url, ok := parseLinkDef(line); ok {
			refs[strings.ToLower(id)] = url
		}
	}
	return refs
}

// === --- Predicates ----------------------------------------------------- ===

func isBlank(line string) bool {
	return strings.TrimSpace(line) == ""
}

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

func isBlockquote(line string) bool {
	return len(line) >= 1 && line[0] == '>' && (len(line) == 1 || line[1] == ' ')
}

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
	i++
	if i >= len(line) || line[i] != ' ' {
		return 0, 0, "", false
	}
	return lead, n, line[i+1:], true
}

func isTableRow(line string) bool {
	t := strings.TrimLeft(line, " ")
	return len(t) > 0 && t[0] == '|'
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
