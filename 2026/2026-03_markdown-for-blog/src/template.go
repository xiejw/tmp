package mdv

import (
	"fmt"
	"os"
	"strings"
)

// HtmlTemplate holds the HTML fragments that wrap the converted markdown output.
type HtmlTemplate struct {
	BeginHTML string
	EndHTML   string
}

// ParseTemplate reads a template file and returns an HtmlTemplate.
//
// Template format:
//
//	begin: <<EOF
//	...html...
//	EOF
//
//	end: <<EOF
//	...html...
//	EOF
func ParseTemplate(path string) (*HtmlTemplate, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("open template %q: %w", path, err)
	}
	content := string(data)

	begin, err := extractHeredoc(content, "begin")
	if err != nil {
		return nil, fmt.Errorf("template %q: %w", path, err)
	}
	end, err := extractHeredoc(content, "end")
	if err != nil {
		return nil, fmt.Errorf("template %q: %w", path, err)
	}
	return &HtmlTemplate{BeginHTML: begin, EndHTML: end}, nil
}

// extractHeredoc finds the block "key: <<EOF\n...\nEOF" and returns its body.
func extractHeredoc(content, key string) (string, error) {
	prefix := key + ": <<EOF\n"
	idx := strings.Index(content, prefix)
	if idx < 0 {
		return "", fmt.Errorf("missing %q: <<EOF block", key)
	}
	body := content[idx+len(prefix):]

	// Empty block: EOF immediately after the opening line.
	if strings.HasPrefix(body, "EOF") {
		return "", nil
	}

	// Find the closing "\nEOF" marker.
	i := strings.Index(body, "\nEOF")
	if i < 0 {
		return "", fmt.Errorf("missing closing EOF for %q block", key)
	}
	return body[:i+1], nil // include the '\n' on the last content line
}
