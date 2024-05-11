#include <gallery/store.h>

#include <vector>

#include <eve/fs.h>

using eve::base::Error;

namespace gallery {

Error
DirEntry::expand( )
{
    auto DirReader = eve::fs::OpenDir( Path.c_str( ) );

    if ( !DirReader.isOk( ) ) {
        return std::move( DirReader.getError( ) );
    }

    auto Err = DirReader.getValue( )->Run( [&]( auto *Name, bool IsFile ) {
        if ( IsFile ) {
            Files.push_back( Name );
        } else {
            Dirs.push_back( DirEntry{ Path + "/" + Name } );
        }
    } );

    if ( Err.isError( ) ) {
        return Err;
    }

    std::sort( Files.begin( ), Files.end( ) );
    std::sort( Dirs.begin( ), Dirs.end( ),
               []( auto a, auto b ) { return a.Path < b.Path; } );
    Expanded = true;
    return { };
}

std::optional<std::string>
DirEntry::getLast( )
{
    if ( !Files.empty( ) ) {
        Pointer = -1 * Files.size( );
        return Path + "/" + Files.back( );
    }

    if ( Dirs.empty( ) ) {
        return { };
    }

    for ( std::ptrdiff_t i = Dirs.size( ) - 1; i >= 0; i-- ) {
        auto &Dir = Dirs[i];
        if ( !Dir.Expanded ) Dir.expand( ).unwrap( );
        auto r = Dir.getLast( );
        if ( r ) {
            Pointer = i + 1;
            return r;
        }
    }

    return { };
}

std::optional<std::string>
Store::getLast( ) noexcept
{
    if ( Root == nullptr ) {
        Root = std::make_unique<DirEntry>( BaseDir );
        Root->expand( ).unwrap( );
    }
    return Root->getLast( );
}
}  // namespace gallery
