package main

import (
	"github.com/xiejw/tmp/cgo/liba"
	"github.com/xiejw/tmp/cgo/libb"
)

func main() {
	liba.SayHelloA()
	libb.SayHelloB()
}
