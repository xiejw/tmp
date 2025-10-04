package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

// LoadFileToString reads the contents of a file and returns it as a string.
func LoadFileToString(path string) (string, error) {
	data, err := ioutil.ReadFile(path)
	if err != nil {
		return "", err
	}
	return string(data), nil
}

func main() {
	content, err := LoadFileToString("data/1.mt")
	if err != nil {
		fmt.Println("Error:", err)
		os.Exit(1)
	}
	fmt.Println(content)
}
