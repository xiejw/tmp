package main

import (
	"fmt"

	"github.com/dlclark/regexp2"
)

func main() {
	PrintMatchOrNot(`a[bc]`, `ab`)
	PrintMatchOrNot(`a[bc]`, `ad`)
	PrintMatchOrNot(`a+(?=bc)`, `aaaadabcaadbc`)
	PrintMatchOrNot(`a+(?!bc)`, `abcaadbc`)
	PrintMatchOrNot(`\s+(?!\S)`, `World   `)
	PrintMatchOrNot(`\s+(?!\S)`, `   World`)
	PrintMatchOrNot(`\s+(?!\S)`, `Hello   World  `)
}

func PrintMatchOrNot(pat, str string) {
	re := regexp2.MustCompile(pat, 0)
	m, _ := re.FindStringMatch(str)
	if m == nil {
		fmt.Printf("%5v With pat `%v` and str `%v`\n", "false", pat, str)
		return
	}

	idx := m.Groups()[0].Capture.Index
	l := m.Groups()[0].Capture.Length
	fmt.Printf("%5v With pat `%v` and str `%v`. Group: `%v` idx: %d len: %v\n", "true", pat, str, m.String(), idx, l)
}
