// cmd/chp55_daemon_pid.cc
// Usage: chp55_daemon_pid [pid_file]
//   pid_file  path to pid file (default: .build/daemon.pid)
//   Creates and locks a pid file, writes current PID, sleeps 5s, then truncates and exits.
//
// Example:
//   terminal 1: .build/chp55_daemon_pid
//   terminal 2: .build/chp55_daemon_pid   # fails: pid file locked by another instance
//   terminal 1: exits after 5s, releasing the lock

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace forge {

/* Return 0 on success with fd written to *fd_out, -1 on error. */
int LockPidFile(const char* path, int* fd_out, std::string* err_msg) {
    int fd = open(path, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        *err_msg = std::string("open failed: ") + strerror(errno);
        return -1;
    }

    struct flock fl;
    fl.l_type   = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0;

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        if (errno == EAGAIN || errno == EACCES)
            *err_msg = "pid file locked by another instance";
        else
            *err_msg = std::string("fcntl F_SETLK failed: ") + strerror(errno);
        close(fd);
        return -1;
    }

    if (ftruncate(fd, 0) == -1) {
        *err_msg = std::string("ftruncate failed: ") + strerror(errno);
        close(fd);
        return -1;
    }

    char buf[32];
    int n = snprintf(buf, sizeof(buf), "%d\n", (int)getpid());
    if (write(fd, buf, (size_t)n) != n) {
        *err_msg = std::string("write failed: ") + strerror(errno);
        close(fd);
        return -1;
    }

    *fd_out = fd;
    return 0;
}

}  // namespace forge

int main(int argc, char* argv[]) {
    const char* pid_file = (argc > 1) ? argv[1] : ".build/daemon.pid";

    std::string err_msg;
    int fd = -1;
    if (forge::LockPidFile(pid_file, &fd, &err_msg) == -1) {
        fprintf(stderr, "error: %s\n", err_msg.c_str());
        return EXIT_FAILURE;
    }

    printf("PID %d locked %s, sleeping 5s...\n", (int)getpid(), pid_file);
    sleep(5);

    ftruncate(fd, 0);
    close(fd);

    printf("done, pid file truncated and lock released.\n");
    return EXIT_SUCCESS;
}
