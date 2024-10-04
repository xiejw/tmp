// vim: ft=cpp
#pragma once

#include <cassert>
#include <print>
#include <span>
#include <string_view>
#include <vector>

namespace env {
enum class Direction {
    Up,
    Down,
    Left,
    Right,
};

struct Env {
  private:
    std::span<std::string_view> maze;
    std::size_t                 x{ };
    std::size_t                 y{ };

  public:
    Env( std::span<std::string_view> maze ) : maze( maze ) { check( ); };

    auto check( ) -> void;
    auto render( ) -> void;
    auto step( Direction Dir ) -> bool;
    auto getLegalActions( ) -> std::vector<Direction>;
};

template <class T>
auto
printLegalActions( const std::vector<T> &Actions )
{
    std::print( "Legal Actions: [" );
    for ( auto &Action : Actions ) {
        std::print( "{}, ", Action );
    }
    std::print( "]\n" );
}

template <class T>
auto
sampleLegalActions( const std::vector<T> &Actions ) -> T
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

    auto format( const env::Direction &Dir, format_context &ctx ) const
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
