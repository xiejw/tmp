#include <eve/fs.h>

#include <errno.h>

using eve::base::Error;
using eve::base::Result;

namespace eve::fs {

Result<std::unique_ptr<DirReader>>
OpenDir( const char *dir )
{
    DIR *OsDir = opendir( dir );
    if ( OsDir == nullptr ) {
        Error e = NewError( "failed to opendir: %s", dir );
        e.emitNote( "os error: %s", strerror( errno ) );

        return Result<std::unique_ptr<DirReader>>::ofError( std::move( e ) );
    }
    return Result<std::unique_ptr<DirReader>>::ofValue(
        std::make_unique<DirReader>( /*name=*/dir, OsDir ) );
}
DirReader::~DirReader( )
{
    if ( Dir != nullptr ) closedir( Dir );
}

Error
DirReader::Run( const std::function<void( const char *Name, bool IsFile )> Fn )
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
            Fn( dp->d_name, true );
        }

        if ( dp->d_type == DT_DIR ) {
            Fn( dp->d_name, false );
        }
    }

    if ( errno != 0 ) {
        Error e = NewError( "failed to readdir in: %s", DirName.c_str( ) );
        e.emitNote( "os error: %s", strerror( errno ) );
        return e;
    }

    return { };
}
}  // namespace eve::fs
