/*
[Connect4] is a program to play connect4 with RL algorithrm.

[Connect4]: https://en.wikipedia.org/wiki/Connect_Four
*/
package main

import (
	"log"
	"os"

	"github.com/xiejw/x/rl/connect4/internal/board"
	"github.com/xiejw/x/rl/connect4/internal/policy"
	"github.com/xiejw/x/rl/internal/drawing"
)

func main() {
	// Prepare logging
	fo, err := os.Create("/tmp/connect4_log.txt")
	if err != nil {
		panic(err)
	}

	log.SetOutput(fo)
	defer fo.Close()

	// policy1 := &policy.DummyPolicy{
	// 	SleepSeconds: 1,
	// 	AIFirst:      true,
	// }

	policy2 := &policy.MCPolicy{
		SimulationCount: 500_000,
		Seed:            123,
		AIFirst:         true,
	}
	policy2.Init()

	p := policy2

	drawing.BoardLoop("Connect 4", board.NewBoard(p))
}
