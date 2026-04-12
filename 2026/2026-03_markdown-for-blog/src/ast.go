package mdv

// === --- Block nodes ---------------------------------------------------- ===

// Node is the sealed interface for all block-level AST nodes.
// The unexported nodeKind method prevents implementations outside this package.
type Node interface {
	nodeKind() string
}

// HeadingNode represents an h1–h6 heading.
type HeadingNode struct {
	Level   int          // 1–6
	Content []InlineNode // parsed inline content
}

func (HeadingNode) nodeKind() string { return "heading" }

// ParagraphNode holds one or more consecutive non-blank lines.
// Each element of Lines is the parsed inline content of one source line.
// Lines are separated by soft breaks when rendered.
type ParagraphNode struct {
	Lines [][]InlineNode
}

func (ParagraphNode) nodeKind() string { return "paragraph" }

// CodeBlockNode represents a fenced code block.
// Lang is empty when no language tag was given.
// Lines holds raw (unescaped) source lines.
type CodeBlockNode struct {
	Lang  string
	Lines []string
}

func (CodeBlockNode) nodeKind() string { return "codeblock" }

// BlockquoteNode represents a > blockquote.
// Each element of Lines is the parsed inline content of one quoted line.
type BlockquoteNode struct {
	Lines [][]InlineNode
}

func (BlockquoteNode) nodeKind() string { return "blockquote" }

// ListNode represents a <ul> or <ol>.
type ListNode struct {
	Ordered bool
	Items   []ListItem
}

func (ListNode) nodeKind() string { return "list" }

// ListItem holds one bullet's inline content and an optional nested list.
type ListItem struct {
	Content []InlineNode
	Sub     *ListNode // nil if no nested list
}

// TableNode represents a pipe-delimited table.
type TableNode struct {
	Headers [][]InlineNode   // one []InlineNode per header cell
	Rows    [][][]InlineNode // [row][col][]InlineNode
}

func (TableNode) nodeKind() string { return "table" }

// === --- Inline nodes --------------------------------------------------- ===

// InlineNode is the sealed interface for inline-level content fragments.
type InlineNode interface {
	inlineKind() string
}

// TextNode holds a plain text fragment.
// Renderers apply escaping (HTML) or none (CLI) as appropriate.
type TextNode struct {
	Text string
}

func (TextNode) inlineKind() string { return "text" }

// LinkNode holds a [text](url) link.
type LinkNode struct {
	Text []InlineNode // parsed inline content of the link label
	URL  string
}

func (LinkNode) inlineKind() string { return "link" }

// BoldNode holds **text** or __text__.
type BoldNode struct {
	Content []InlineNode
}

func (BoldNode) inlineKind() string { return "bold" }

// ItalicNode holds *text* or _text_.
type ItalicNode struct {
	Content []InlineNode
}

func (ItalicNode) inlineKind() string { return "italic" }
