package env

type Result[State any] struct {
	Valid  bool
	End    bool
	Reward float32
	State  *State
}

type Env[Action ~int64, State any] interface {
	Render()
	GetState() *State
	GetLegalActionIdMasks() []int64
	Step(Action) *Result[State]
}
