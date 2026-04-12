package main

import (
	"flag"
	"fmt"
	"os"

	mdv "mdv/src"
)

func main() {
	inputPath := flag.String("i", "", "input markdown file (required)")
	outputPath := flag.String("o", "", "output HTML file (default: stdout)")
	templatePath := flag.String("t", "", "template file (optional, ignored with -cli)")
	cliMode := flag.Bool("cli", false, "render as ANSI-colored terminal output instead of HTML")
	flag.Parse()

	if *inputPath == "" {
		fmt.Fprintln(os.Stderr, "usage: mdv -i input.md [-o output.html] [-t template.tmpl] [-cli]")
		os.Exit(1)
	}

	nodes, err := mdv.Parse(*inputPath)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	out := os.Stdout
	if *outputPath != "" {
		f, err := os.Create(*outputPath)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		defer f.Close()
		out = f
	}

	if *cliMode {
		if err := mdv.RenderCLI(nodes, out); err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
		return
	}

	var tmpl *mdv.HtmlTemplate
	if *templatePath != "" {
		tmpl, err = mdv.ParseTemplate(*templatePath)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
	}

	if err := mdv.RenderHTML(nodes, out, tmpl, mdv.HeadingHooks{}); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}
