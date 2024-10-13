package env

import (
	"bytes"
	"fmt"
)

type Env struct {
	maze   []string
	curRow int
	curCol int
}

func New(maze []string) *Env {
	return &Env{
		maze:   maze,
		curRow: 0,
		curCol: 0,
	}
}

func (env *Env) Render() {
	var buf bytes.Buffer
	for row, line := range env.maze {
		for col, x := range line {
			if row == env.curRow && col == env.curCol {
				fmt.Fprintf(&buf, "o")
			} else {
				fmt.Fprintf(&buf, "%c", (x))
			}
		}
		fmt.Fprintf(&buf, "\n")
	}
	fmt.Printf("%v", buf.String())
}
