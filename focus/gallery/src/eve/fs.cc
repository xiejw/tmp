#include <eve/fs.h>

namespace eve::fs {
std::unique_ptr<FsDir>
OpenDir( const char *dir )
{
    DIR *OsDir = opendir( dir );
    if ( dir == nullptr ) {
        return nullptr;
    }
    return std::make_unique<FsDir>( /*name=*/dir, OsDir );
}
}  // namespace eve::fs
