package mlvm

type VM interface {
	PrepareProgram() error
}

type Shape struct {
	Dims []int
}

type DataType int

const (
	F32 DataType = iota
	I64
)

// -----------------------------------------------------------------------------
// Shape methods
//

func (s Shape) Rank() int { return len(s.Dims) }

type simpleVM struct {
}
