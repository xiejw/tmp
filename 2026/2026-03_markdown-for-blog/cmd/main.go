package main

import (
	"flag"
	"fmt"
	"os"

	md2html "md2html/src"
)

func main() {
	inputPath := flag.String("i", "", "input markdown file (required)")
	outputPath := flag.String("o", "", "output HTML file (default: stdout)")
	templatePath := flag.String("t", "", "template file (optional)")
	flag.Parse()

	if *inputPath == "" {
		fmt.Fprintln(os.Stderr, "usage: md2html -i input.md [-o output.html] [-t template.tmpl]")
		os.Exit(1)
	}

	var tmpl *md2html.HtmlTemplate
	if *templatePath != "" {
		var err error
		tmpl, err = md2html.ParseTemplate(*templatePath)
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
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

	if err := md2html.Convert(*inputPath, out, tmpl, md2html.HeadingHooks{}); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}
