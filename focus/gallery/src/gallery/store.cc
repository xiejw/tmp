#include <gallery/store.h>

#include <vector>

#include <eve/fs.h>

#include <eve/base/error.h>

namespace gallery {
std::optional<std::string>
Store::Last( ) noexcept
{
    auto DirReader = eve::fs::OpenDir( BaseDir.c_str( ) );

    if ( !DirReader.isOk( ) ) {
        DirReader.getError( ).dumpAndPanic( );
    }

    std::vector<std::string> Dirs{ };
    std::vector<std::string> Files{ };

    auto Err = DirReader.getValue( )->Run( [&]( auto *Name, bool IsFile ) {
        if ( IsFile ) {
            Files.push_back( Name );
        } else {
            Dirs.push_back( Name );
        }
    } );

    if ( Err.isError( ) ) {
        Err.dumpAndPanic( );
    }

    if ( Files.empty( ) ) {
        unimplemented( "empty files does not support yet" );
    }

    return BaseDir + "/" + Files.back( );
}
}  // namespace gallery
