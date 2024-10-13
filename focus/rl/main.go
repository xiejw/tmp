package main

import (
	"rl/lib/env"
)

var (
	MAZE []string = []string{"sx..", "....", "..x.", "..xg"}
)

func main() {
	e := env.New(MAZE)
	e.Render()
}
