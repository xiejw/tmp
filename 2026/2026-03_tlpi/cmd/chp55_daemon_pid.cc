// cmd/chp55_daemon_pid.cc
// Usage: chp55_daemon_pid [pid_file]
//   pid_file  path to pid file (default: .build/daemon.pid)
//   Creates and locks a pid file, writes current PID, sleeps 5s, then truncates and exits.

#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace forge {

int CreateAndLockPidFile(const char* path, std::string* err_msg) {
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
    if (write(fd, buf, n) != n) {
        *err_msg = std::string("write failed: ") + strerror(errno);
        close(fd);
        return -1;
    }

    return fd;
}

}  // namespace forge

int main(int argc, char* argv[]) {
    const char* pid_file = (argc > 1) ? argv[1] : ".build/daemon.pid";

    std::string err_msg;
    int fd = forge::CreateAndLockPidFile(pid_file, &err_msg);
    if (fd == -1) {
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
