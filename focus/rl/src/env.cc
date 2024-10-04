#include "env.h"

#include <eve/base/error.h>

namespace env {

auto
Env::check( ) -> void
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

auto
Env::render( ) -> void
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

auto
Env::step( Direction Dir ) -> bool
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
auto
Env::getLegalActions( ) -> std::vector<Direction>
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
}  // namespace env
