#include <stdio.h>

#include "a.h"

void sayPromptFromB() {
#ifdef PROMPT
        printf("sayHelloFromB:%s\n", PROMPT);
#else
        printf("sayHelloFromB:%s\n", "(undefined)");
#endif
}

void sayHelloFromB() {
#ifdef FLAG_A

#ifdef FLAG_B
        printf("sayHelloFromB with FLAG_A and FLAG_B\n");
#else
        printf("sayHelloFromB with FLAG_A\n");
#endif // FLAG_B

#else
        printf("sayHelloFromB\n");
#endif // FLAG_A
}
