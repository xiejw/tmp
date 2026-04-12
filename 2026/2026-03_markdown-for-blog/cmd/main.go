package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"

	mdv "mdv/src"
)

func usage() {
	fmt.Fprintln(os.Stderr, "usage: mdv [-i input.md] [-o output.html] [-t template.tmpl]")
	fmt.Fprintln(os.Stderr, "")
	fmt.Fprintln(os.Stderr, "  No flags: read stdin, render as ANSI-colored terminal output.")
	fmt.Fprintln(os.Stderr, "  -o:       write HTML to file (implies HTML render).")
	fmt.Fprintln(os.Stderr, "")
	flag.PrintDefaults()
}

func main() {
	flag.Usage = usage
	inputPath := flag.String("i", "", "input markdown file (default: stdin)")
	outputPath := flag.String("o", "", "output HTML file; when set, HTML render is used (default: stdout with CLI render)")
	templatePath := flag.String("t", "", "HTML template file (optional, used with -o)")
	flag.Parse()

	// Parse input.
	var (
		nodes []mdv.Node
		err   error
	)
	if *inputPath != "" {
		nodes, err = mdv.Parse(*inputPath)
	} else {
		nodes, err = parseStdin()
	}
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	// CLI render: no -o flag.
	if *outputPath == "" {
		if err := mdv.RenderCLI(nodes, os.Stdout); err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		return
	}

	// HTML render: -o provided.
	var tmpl *mdv.HtmlTemplate
	if *templatePath != "" {
		tmpl, err = mdv.ParseTemplate(*templatePath)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	}

	out, err := os.Create(*outputPath)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	defer out.Close()

	if err := mdv.RenderHTML(nodes, out, tmpl, mdv.HeadingHooks{}); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}

// parseStdin reads all lines from stdin and returns the parsed AST.
func parseStdin() ([]mdv.Node, error) {
	var lines []string
	sc := bufio.NewScanner(os.Stdin)
	for sc.Scan() {
		lines = append(lines, sc.Text())
	}
	if err := sc.Err(); err != nil {
		return nil, fmt.Errorf("read stdin: %w", err)
	}
	return mdv.ParseLines(lines)
}
