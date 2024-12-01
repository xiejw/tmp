package policy

type Policy[State any, X any] interface {
	Predict(*State, X) int64
}
