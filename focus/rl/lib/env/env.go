package env

type Result[State any] struct {
	Valid  bool
	End    bool
	Reward float32
	State  State
}

type Env[Action any, State any] interface {
	Render()
	State() State
	Step(Action) *Result[State]
	LegalActionMasks() []int64
	Action(int64) Action
}
