#include <gallery/store.h>

#include <vector>

#include <eve/fs.h>

namespace gallery {
std::optional<std::string>
Store::Last( ) noexcept
{
    auto FsDirReader = eve::fs::OpenDir( BaseDir.c_str( ) );

    if ( FsDirReader == nullptr ) {
        PANIC( "failed to open dir: %s", BaseDir.c_str( ) );
    }

    std::vector<std::string> Dirs{ };
    std::vector<std::string> Files{ };

    FsDirReader->Run( [&]( auto *Name, bool IsFile ) {
        if ( IsFile ) {
            Files.push_back( Name );
        } else {
            Dirs.push_back( Name );
        }
    } );

    if ( Files.empty( ) ) {
        PANIC( "empty files does not support yet" );
    }

    return BaseDir + "/" + Files.back( );
}
}  // namespace gallery
