#include <time.h>

#include "progress.h"

int main(void)
{
    int64_t total = 20 * 25;
    forge_progress_bar_open(total, "Processing items");
    for (int64_t i = 0; i <= total; i += 10) {
        forge_progress_bar_advance(i);
        struct timespec ts = {0, 150 * 1000 * 1000}; /* 150 ms */
        nanosleep(&ts, NULL);
    }
    forge_progress_bar_close();
    return 0;
}
