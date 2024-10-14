package main

import (
	"log"

	"rl/lib/env"
	"rl/lib/policy"
)

var (
	MAZE []string = []string{"sx..", "....", "..x.", "..xg"}
)

func main() {
	var e env.Env[env.MazeDir, *env.MazeState] = env.New(MAZE)
	s := e.State()

	for {
		e.Render()
		masks := e.LegalActionMasks()
		p := policy.NewRandom[*env.MazeState]()
		actionIdx := p.Predict(s, masks)

		log.Printf("action %v", actionIdx)

		result := e.Step(e.Action(actionIdx))
		if result.End {
			break
		}

		s = result.State
	}
}
