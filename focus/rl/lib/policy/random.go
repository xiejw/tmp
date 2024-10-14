package policy

import (
	"math/rand/v2"
)

type Random[State any] struct {
}

func NewRandom[State any]() *Random[State] {
	return new(Random[State])
}

func (p *Random[State]) Predict(_s State, mask []int64) int64 {
	n := int64(len(mask))
	for {
		i := rand.Int64N(n)
		if mask[i] != 0 {
			return i
		}
	}
}
