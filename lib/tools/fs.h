// vim: ft=cpp
#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

namespace eve::fs {

struct FsDir {
  private:
    std::string DirName;
    DIR        *Dir = nullptr;

  public:
    FsDir( std::string DirName, DIR *Dir ) : DirName( DirName ), Dir( Dir )
    {
        DEBUG( "opening dir: %s\n", this->DirName.c_str( ) );
    };

    ~FsDir( )
    {
        if ( Dir != nullptr ) {
            closedir( Dir );
            DEBUG( "closing dir: %s\n", this->DirName.c_str( ) );
        }
    }

    // The name lifetime is limited to the invocation.
    void Run( const std::function<void( const char *RegFileName )> Fn )
    {
        struct dirent *dp;
        for ( ;; ) {
            errno = 0;
            dp    = readdir( Dir );
            if ( dp == NULL ) break;

            if ( strcmp( dp->d_name, "." ) == 0 ||
                 strcmp( dp->d_name, ".." ) == 0 ) {
                continue;
            }

            if ( dp->d_name[0] == '.' ) {
                continue;
            }

            if ( dp->d_type == DT_REG ) {
                Fn( dp->d_name );
            }
        }

        if ( errno != 0 ) {
            ALERT( "error for readir" );
        }
    }
};

// Return nullptr if error.
FsDir *
OpenDir( const char *dir )
{
    DIR *OsDir = opendir( dir );
    if ( dir == nullptr ) {
        return nullptr;
    }
    return new FsDir( /*name=*/dir, OsDir );
}

bool CopyFile(const char *to, const char *from)
{
    int fd_to, fd_from;
    char buf[4096];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return false;

    fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return true;
    }

  out_error:
    saved_errno = errno;

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return false;
}
}  // namespace eve::fs
