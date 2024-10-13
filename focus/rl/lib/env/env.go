package env

import (
	"bytes"
	"fmt"
)

type Dir int

const (
	Up Dir = iota
	Down
	Left
	Right
	DirMax
)

type State struct {
	Row int
	Col int
}

type Result struct {
	Valid  bool
	End    bool
	Reward float32
	State  *State
}

type Env struct {
	maze  []string
	state State
}

func New(maze []string) *Env {
	return &Env{
		maze:  maze,
		state: State{0, 0},
	}
}

func (env *Env) Render() {
	var buf bytes.Buffer
	state := &env.state

	for row, line := range env.maze {
		for col, x := range line {
			if row == state.Row && col == state.Col {
				fmt.Fprintf(&buf, "o")
			} else {
				fmt.Fprintf(&buf, "%c", (x))
			}
		}
		fmt.Fprintf(&buf, "\n")
	}
	fmt.Printf("%v", buf.String())
}

func (env *Env) Step(dir Dir) *Result {
	curState := &env.state
	switch dir {
	case Up:
		curState.Row--
	case Down:
		curState.Row++
	case Left:
		curState.Col--
	case Right:
		curState.Col++
	}

	valid := env.isStateValid()
	if !valid {
		return &Result{Valid: valid}
	}

	end := env.isGameEnd()
	if end {
		return &Result{Valid: valid, End: end}
	}

	stateCopy := *curState
	return &Result{
		Valid:  valid,
		End:    end,
		Reward: -1,
		State:  &stateCopy,
	}
}

func (env *Env) LegalActionMasks() []int {
	var masks [DirMax]int
	row := env.state.Row
	col := env.state.Col
	maxRow := len(env.maze)
	maxCol := len(env.maze[row])

	if row >= 1 && env.maze[row-1][col] != 'x' {
		masks[Up] = 1
	}
	if row < maxRow-1 && env.maze[row+1][col] != 'x' {
		masks[Down] = 1
	}
	if col >= 1 && env.maze[row][col-1] != 'x' {
		masks[Left] = 1
	}
	if col < maxCol-1 && env.maze[row][col+1] != 'x' {
		masks[Right] = 1
	}
	return masks[:]
}

func (env *Env) isStateValid() bool {
	row := env.state.Row
	col := env.state.Col

	return row >= 0 && row < len(env.maze) &&
		col >= 0 && col < len(env.maze[row]) &&
		env.maze[row][col] != 'x'
}

func (env *Env) isGameEnd() bool {
	return env.maze[env.state.Row][env.state.Col] == 'g'
}
