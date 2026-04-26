// cmd/chp54_shm.cc
// Usage: chp54_shm write <str1> [str2 ... str10]
//        chp54_shm read
//   write  create shared memory and store up to 10 strings; waits for enter before unlinking
//   read   open existing shared memory, use fstat to verify size, print stored strings
//   Demonstrates POSIX shm_open + mmap for inter-process C-string exchange via a fixed struct.
//
// Example:
//   terminal 1: .build/chp54_shm write hello world foo
//   terminal 2: .build/chp54_shm read
//   terminal 1: press enter to unlink the shared memory and exit

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace forge {

static const char* kShmName    = "/chp54_shm";
static const int   kMaxStrings = 10;
static const int   kMaxStrLen  = 64;

struct SharedStrings {
    int count;
    char strings[kMaxStrings][kMaxStrLen];
};

// === --- Writer --------------------------------------------------------- ===
//

bool ShmWrite(int count, char* const* strs, std::string* err_msg) {
    int fd = shm_open(kShmName, O_CREAT | O_RDWR, 0600);
    if (fd == -1) {
        *err_msg = std::string("shm_open: ") + strerror(errno);
        return true;
    }

    if (ftruncate(fd, (off_t)sizeof(SharedStrings)) == -1) {
        *err_msg = std::string("ftruncate: ") + strerror(errno);
        close(fd);
        shm_unlink(kShmName);
        return true;
    }

    void* addr = mmap(nullptr, sizeof(SharedStrings),
                      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (addr == MAP_FAILED) {
        *err_msg = std::string("mmap: ") + strerror(errno);
        shm_unlink(kShmName);
        return true;
    }

    SharedStrings* shm = static_cast<SharedStrings*>(addr);
    shm->count = count;
    for (int i = 0; i < count; i++) {
        strncpy(shm->strings[i], strs[i], kMaxStrLen - 1);
        shm->strings[i][kMaxStrLen - 1] = '\0';
    }

    printf("wrote %d string(s) to shm '%s' (%zu bytes)\n",
           count, kShmName, sizeof(SharedStrings));
    printf("press enter to unlink and exit...\n");
    getchar();

    munmap(addr, sizeof(SharedStrings));
    shm_unlink(kShmName);
    return false;
}

// === --- Reader --------------------------------------------------------- ===
//

bool ShmRead(std::string* err_msg) {
    int fd = shm_open(kShmName, O_RDONLY, 0);
    if (fd == -1) {
        *err_msg = std::string("shm_open: ") + strerror(errno);
        return true;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        *err_msg = std::string("fstat: ") + strerror(errno);
        close(fd);
        return true;
    }

    if ((size_t)st.st_size != sizeof(SharedStrings)) {
        *err_msg = std::string("unexpected shm size: expected ") +
                   std::to_string(sizeof(SharedStrings)) + ", got " +
                   std::to_string((size_t)st.st_size);
        close(fd);
        return true;
    }

    void* addr = mmap(nullptr, sizeof(SharedStrings), PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (addr == MAP_FAILED) {
        *err_msg = std::string("mmap: ") + strerror(errno);
        return true;
    }

    const SharedStrings* shm = static_cast<const SharedStrings*>(addr);
    printf("read %d string(s) from shm '%s':\n", shm->count, kShmName);
    for (int i = 0; i < shm->count; i++) {
        printf("  [%d] %s\n", i, shm->strings[i]);
    }

    munmap(addr, sizeof(SharedStrings));
    return false;
}

}  // namespace forge

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s write <str1> [str2 ... str10]\n", argv[0]);
        fprintf(stderr, "       %s read\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string err_msg;

    if (strcmp(argv[1], "write") == 0) {
        if (argc < 3) {
            fprintf(stderr, "write: at least one string required\n");
            return EXIT_FAILURE;
        }
        int count = argc - 2;
        if (count > forge::kMaxStrings) count = forge::kMaxStrings;
        if (forge::ShmWrite(count, argv + 2, &err_msg)) {
            fprintf(stderr, "error: %s\n", err_msg.c_str());
            return EXIT_FAILURE;
        }
    } else if (strcmp(argv[1], "read") == 0) {
        if (forge::ShmRead(&err_msg)) {
            fprintf(stderr, "error: %s\n", err_msg.c_str());
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "unknown mode '%s': expected 'write' or 'read'\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
