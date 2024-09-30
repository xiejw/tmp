#include <expected>
#include <format>
#include <print>
#include <span>
#include <string_view>
#include <vector>

#include <stdlib.h>
#include <time.h>
#include <cassert>

#include <eve/base/error.h>

namespace env {
enum class Direction {
    Up,
    Down,
    Left,
    Right,
};

struct Env {
    std::span<std::string_view> maze;
    std::size_t                 x{ };
    std::size_t                 y{ };

    auto check( ) -> void
    {
        if ( maze.empty( ) ) {
            panic( "Is maze got set?" );
        }
        if ( x >= maze.size( ) || y >= maze[x].size( ) ) {
            panic( "The current possition (%d, %d) is out of range", x, y );
        }
        if ( maze[x][y] == 'x' ) {
            panic( "The current possition (%d, %d) is not valid", x, y );
        }
    }

    auto render( ) -> void
    {
        for ( std::size_t row = 0; row < maze.size( ); row++ ) {
            for ( std::size_t col = 0; col < maze[std::size_t( row )].size( );
                  col++ ) {
                if ( row == x && col == y ) {
                    std::print( "o" );
                } else {
                    std::print( "{}",
                                maze[std::size_t( row )][std::size_t( col )] );
                }
            }
            std::print( "\n" );
        }
    }

    auto step( Direction Dir ) -> bool
    {
        switch ( Dir ) {
        case env::Direction::Up:
            x--;
            break;
        case env::Direction::Down:
            x++;
            break;
        case env::Direction::Left:
            y--;
            break;
        case env::Direction::Right:
            y++;
            break;
        };

        this->check( );

        return maze[x][y] == 'g';
    }

    auto getLegalActions( ) -> std::vector<Direction>
    {
        std::vector<Direction> Actions{ };
        if ( x >= 1 && maze[x - 1][y] != 'x' ) {
            Actions.push_back( Direction::Up );
        }
        if ( x < maze.size( ) - 1 && maze[x + 1][y] != 'x' ) {
            Actions.push_back( Direction::Down );
        }
        if ( y >= 1 && maze[x][y - 1] != 'x' ) {
            Actions.push_back( Direction::Left );
        }
        if ( y < maze[x].size( ) - 1 && maze[x][y + 1] != 'x' ) {
            Actions.push_back( Direction::Right );
        }
        return Actions;
    }
};

template <class T>
auto
printLegalActions( std::vector<T> &Actions )
{
    std::print( "Legal Actions: [" );
    for ( auto &Action : Actions ) {
        std::print( "{}, ", Action );
    }
    std::print( "]\n" );
}

template <class T>
auto
sampleLegalActions( std::vector<T> &Actions ) -> T
{
    std::size_t Size = Actions.size( );
    assert( Size > 0 );
    std::size_t SampleIndex = std::size_t( rand( ) ) % Size;
    return Actions[SampleIndex];
}

}  // namespace env

namespace std {
template <>
struct formatter<env::Direction, char> {
    constexpr auto parse( std::format_parse_context &ctx )
    {
        return ctx.begin( );
    }

    auto format( env::Direction &Dir, format_context &ctx ) const
    {
        switch ( Dir ) {
        case env::Direction::Up:
            return std::format_to( ctx.out( ), "Up" );
        case env::Direction::Down:
            return std::format_to( ctx.out( ), "Down" );
        case env::Direction::Left:
            return std::format_to( ctx.out( ), "Left" );
        case env::Direction::Right:
            return std::format_to( ctx.out( ), "Right" );
        };
    }
};

}  // namespace std

int
main( )
{
    srand( unsigned( time( NULL ) ) );
    std::vector<std::string_view> maze{
        "sx..",
        "....",
        "..x.",
        "..xg",
    };

    env::Env Env{ maze };
    Env.check( );

    for ( ;; ) {
        Env.render( );
        auto Actions = Env.getLegalActions( );
        env::printLegalActions( Actions );
        auto ActionToTake = env::sampleLegalActions( Actions );
        std::print( "Action To Take: {}\n", ActionToTake );
        if ( Env.step( ActionToTake ) ) break;
    }
    return 0;
}
