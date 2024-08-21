package mlvm

import (
	"testing"
)

// -----------------------------------------------------------------------------
// Shape methods
//

func TestShapeRank(t *testing.T) {

	s := Shape{
		Dims: []int{1, 2, 4},
	}

	if s.Rank() != 3 {
		t.Errorf("rank mismatch.")
	}

}
