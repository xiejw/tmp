package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"strings"
)

const (
	SgbWordsFileName = "data/sgb-words.txt"
)

func main() {
	fileName := SgbWordsFileName
	lines, err := readFileAsLines(fileName)
	if err != nil {
		panic(err)
	}

	fmt.Printf("Got %v words from the file %v\n", len(lines), fileName)
	if len(lines) >= 0 {
		fmt.Printf("First one is %v\n", lines[0])
		fmt.Printf("Last  one is %v\n", lines[len(lines)-1])
	}
}

func readFileAsLines(filename string) ([]string, error) {
	f, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	lines := make([]string, 0)
	r := bufio.NewReader(f)
	for {
		line, err := r.ReadString(10) // 0x0A separator = newline
		if err == io.EOF {
			// do something here
			break
		} else if err != nil {
			return nil, err
		}

		lines = append(lines, strings.Trim(line, "\n"))
	}

	return lines, nil
}
