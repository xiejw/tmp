#include <gallery/store.h>

#include <vector>

#include <eve/fs.h>

using eve::base::Error;

namespace gallery {

Error
DirEntry::expand( )
{
    if ( Expanded ) return { };

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
    expand( ).unwrap( );
    if ( !Files.empty( ) ) {
        Pointer = -1 * Files.size( );
        return Path + "/" + Files.back( );
    }

    if ( Dirs.empty( ) ) {
        return { };
    }

    for ( std::ptrdiff_t i = Dirs.size( ) - 1; i >= 0; i-- ) {
        auto &Dir = Dirs[i];
        auto  r   = Dir.getLast( );
        if ( r ) {
            Pointer = i + 1;
            return r;
        }
    }

    return { };
}

std::optional<std::string>
DirEntry::getPrev( )
{
    expand( ).unwrap( );
    if ( Pointer == 0 ) return getLast( );

    if ( Pointer < -1 ) {
        Pointer++;
        return Path + "/" + Files[-1 * Pointer - 1];
    }

    if ( Dirs.empty( ) ) {
        return { };
    }

    if ( Pointer == -1 ) {
        // Scan all dirs
        for ( int i = Dirs.size( ) - 1; i >= 0; i-- ) {
            auto &Dir    = Dirs[i];
            auto  Result = Dir.getLast( );
            if ( Result ) {
                Pointer = i + 1;
                return Result;
            }
        }
        return { };
    }

    int  DirIndex = Pointer - 1;
    auto Result   = Dirs[DirIndex].getPrev( );

    if ( Result ) {
        return Result;
    }

    // Scan remaining dirs
    for ( int i = DirIndex - 1; i >= 0; i-- ) {
        auto &Dir    = Dirs[i];
        auto  Result = Dir.getLast( );
        if ( Result ) {
            Pointer = i + 1;
            return Result;
        }
    }

    return { };
}

Error
Store::init( ) noexcept
{
    if ( Root == nullptr ) {
        Root = std::make_unique<DirEntry>( BaseDir );
        return { };
    }
    return { };
}

std::optional<std::string>
Store::getLast( ) noexcept
{
    init( ).unwrap( );
    return Root->getLast( );
}

std::optional<std::string>
Store::getPrev( ) noexcept
{
    init( ).unwrap( );
    return Root->getPrev( );
}
}  // namespace gallery
