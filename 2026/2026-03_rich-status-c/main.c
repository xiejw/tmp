#include <unistd.h>

#include "status.h"

int main(void)
{
    forge_status_open("Processing items...");
    for (int i = 0; i < 10; i++) {
        forge_status_printf("hello %d", i);
        sleep(2);
    }
    forge_status_close();
    return 0;
}
