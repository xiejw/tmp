package mlvm

import "io"

type VM interface {
	io.Closer

	PrepareProgram(string) (Program, error)
	Unmarshal([]byte) (Program, error)
}

type Program interface {
	io.Closer

	Run([]TensorPtr) ([]TensorPtr, error)
	Marshal() ([]byte, error)
}

type DataType int

const (
	F32 DataType = iota
	I64
)

type Shape struct {
	dtype DataType
	dims  []int
}

type TensorPtr interface {
	Name() string
	Shape() Shape // Shape
	Ptr() any     // Raw pointer
}

// -----------------------------------------------------------------------------
// Shape methods
//

func NewShape(dtype DataType, dims ...int) Shape {
	return Shape{dtype: dtype, dims: dims}
}

func (s Shape) DataType() DataType { return s.dtype }
func (s Shape) Rank() int          { return len(s.dims) }
func (s Shape) Dims() []int        { return s.dims }
func (s Shape) NumElems() int {
	c := 1
	for _, d := range s.dims {
		c *= d
	}
	return c
}
func (s Shape) ByteSizes() int {
	switch s.dtype {
	case F32:
		return 4 * s.NumElems()
	case I64:
		return 8 * s.NumElems()
	default:
		panic("unsupported")
	}
}

type simpleVM struct {
}
