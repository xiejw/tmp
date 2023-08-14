package liba

// #include "a.h"
// #cgo CFLAGS: -DFLAG_A
import "C"

func SayHelloA() {
	C.sayHelloFromA()
	C.sayPromptFromA()
}
