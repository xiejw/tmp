// cmd/chp55_file_lock.cc
// Usage: chp55_file_lock <seconds> [x]
//   <seconds>  how long to hold the lock
//   x          (optional) exclusive lock; default is shared
#include <sys/file.h>   // flock
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

static const char* kTestFile = ".build/test_file";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <seconds> [x]\n", argv[0]);
        return 1;
    }

    int hold_secs = atoi(argv[1]);
    int lock_type = (argc >= 3 && argv[2][0] == 'x') ? LOCK_EX : LOCK_SH;
    const char* lock_name = (lock_type == LOCK_EX) ? "EXCLUSIVE" : "SHARED";

    int fd = open(kTestFile, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "open %s: %s\n", kTestFile, strerror(errno));
        return 1;
    }

    printf("[PID %d] Acquiring %s lock on %s...\n", getpid(), lock_name, kTestFile);

    if (flock(fd, lock_type) == -1) {
        fprintf(stderr, "flock: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("[PID %d] Lock acquired. Holding for %d seconds...\n", getpid(), hold_secs);
    sleep(hold_secs);

    if (flock(fd, LOCK_UN) == -1) {
        fprintf(stderr, "flock unlock: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("[PID %d] Lock released.\n", getpid());
    close(fd);
    return 0;
}
