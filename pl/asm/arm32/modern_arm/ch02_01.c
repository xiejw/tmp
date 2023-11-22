#include <stdio.h>

// a + b - c
extern int result_(int a, int b, int c);

int main() {
        printf("result is %d", result_(22, 32, 4));
        return 0;
}
