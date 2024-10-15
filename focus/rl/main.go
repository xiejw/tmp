package main

import (
	"log"

	"rl/lib/env"
	"rl/lib/policy"
)

var (
	MAZE []string = []string{"sx..", "....", "..x.", "..xg"}
)

type Action = env.MazeDir
type State = env.MazeState

func main() {
	var e env.Env[Action, State] = env.New(MAZE)
	s := e.GetState()

	for {
		e.Render()
		masks := e.GetLegalActionIdMasks()
		p := policy.NewRandom[State]()
		actionIdx := p.Predict(s, masks)

		log.Printf("action %v", actionIdx)

		result := e.Step(Action(actionIdx))
		if result.End {
			break
		}

		s = result.State
	}
}
