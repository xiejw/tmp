package mlvm

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

// -----------------------------------------------------------------------------
// Shape methods
//

func TestShapeRank(t *testing.T) {
	s := NewShape(F32, 1, 2, 4)
	assert.Equal(t, F32, s.DataType(), "dtype mismatch.")
	assert.Equal(t, 3, s.Rank(), "rank mismatch.")
	assert.Equal(t, []int{1, 2, 4}, s.Dims(), "dims mismatch.")
	assert.Equal(t, 8, s.NumElems(), "elem count mismatch.")
	assert.Equal(t, 32, s.ByteSizes(), "elem count mismatch.")
}
