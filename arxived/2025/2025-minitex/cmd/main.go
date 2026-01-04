package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

// LoadFileToString reads the contents of a file and returns it as a string.
func LoadFileToString(path string) ([]byte, error) {
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return nil, err
	}
	return data, nil
}

func EmitToken(in []byte) error {
	p := 0
	for p < len(in) {
		switch {
		case in[p] == '\\':
			p = ReadCmd(in, p+1)
		default:
			panic("unimpl")
		}
	}

	return nil
}

func ReadCmd(in []byte, p int) int {
	start := p
	for {
		if p > len(in) {
			panic("out of range")
		}

		c := in[p]

		if (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') {
			p++
			continue
		}

		if start == p {
			panic("cannot find cmd")
		}

		fmt.Printf("token: `%v`\n", string(in[start:p]))
		return p
	}
}

func main() {
	content, err := LoadFileToString("data/1.mt")
	if err != nil {
		fmt.Println("Error:", err)
		os.Exit(1)
	}
	EmitToken(content)
}
