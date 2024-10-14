package env

import (
	"bytes"
	"fmt"
)

type MazeDir int

const (
	Up MazeDir = iota
	Down
	Left
	Right
	DirMax
)

type MazeState struct {
	Row int
	Col int
}

type MazeEnv struct {
	maze  []string
	state MazeState
}

func New(maze []string) *MazeEnv {
	return &MazeEnv{
		maze:  maze,
		state: MazeState{0, 0},
	}
}

func (env *MazeEnv) Render() {
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

func (env *MazeEnv) Step(dir MazeDir) *Result[*MazeState] {
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
		return &Result[*MazeState]{Valid: valid}
	}

	end := env.isGameEnd()
	if end {
		return &Result[*MazeState]{Valid: valid, End: end}
	}

	stateCopy := *curState
	return &Result[*MazeState]{
		Valid:  valid,
		End:    end,
		Reward: -1,
		State:  &stateCopy,
	}
}

func (env *MazeEnv) State() *MazeState {
	return &env.state
}

func (env *MazeEnv) Action(x int64) MazeDir {
	return MazeDir(x)
}

func (env *MazeEnv) LegalActionMasks() []int64 {
	var masks [DirMax]int64
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

func (env *MazeEnv) isStateValid() bool {
	row := env.state.Row
	col := env.state.Col

	return row >= 0 && row < len(env.maze) &&
		col >= 0 && col < len(env.maze[row]) &&
		env.maze[row][col] != 'x'
}

func (env *MazeEnv) isGameEnd() bool {
	return env.maze[env.state.Row][env.state.Col] == 'g'
}
