package libb

// #include "a.h"
import "C"

func SayHelloB() {
	C.sayHelloFromB()
	C.sayPromptFromB()
}
