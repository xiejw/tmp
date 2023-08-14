#include <stdio.h>

#include "a.h"

void sayPromptFromA() {
#ifdef PROMPT
        printf("sayHelloFromA:%s\n", PROMPT);
#else
        printf("sayHelloFromA:%s\n", "(undefined)");
#endif
}

void sayHelloFromA() {
#ifdef FLAG_A

#ifdef FLAG_B
        printf("sayHelloFromA with FLAG_A and FLAG_B\n");
#else
        printf("sayHelloFromA with FLAG_A\n");
#endif // FLAG_B

#else
        printf("sayHelloFromA\n");
#endif // FLAG_A
}
