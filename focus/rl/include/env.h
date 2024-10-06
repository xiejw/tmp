// vim: ft=cpp
#pragma once

#include <cassert>
#include <memory>
#include <print>
#include <span>
#include <string_view>
#include <vector>

#include "base.h"

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
    struct State {
        std::size_t X;
        std::size_t Y;
    };

    struct Result {
        f32_t Reward;
        bool  End;
        State State;
    };

  public:
    Env( std::span<std::string_view> maze ) : maze( maze ) { check( ); };

    auto render( ) const -> void;
    auto step( Direction Dir ) -> std::unique_ptr<Result>;

    auto getLegalActionMasks( ) const -> std::vector<int>;

  private:
    auto check( ) -> void;
};

template <class T>
auto
printLegalActions( const std::vector<int> &ActionMasks )
{
    std::print( "Legal Actions: [" );
    for ( std::size_t i = 0; i < ActionMasks.size( ); i++ ) {
        if ( ActionMasks[i] == 1 ) std::print( "{}, ", static_cast<T>( i ) );
    }
    std::print( "]\n" );
}

template <class T>
auto
sampleLegalActions( const std::vector<int> &ActionMasks ) -> T
{
    std::size_t Size = ActionMasks.size( );
    assert( Size > 0 );
    for ( ;; ) {
        std::size_t SampleIndex = std::size_t( rand( ) ) % Size;
        if ( ActionMasks[SampleIndex] == 1 ) {
            return static_cast<T>( SampleIndex );
        }
    }
}

}  // namespace env

// -----------------------------------------------------------------------------
// Formatter for Direction
//
namespace std {
template <>
struct formatter<env::Direction, char> {
    constexpr auto parse( std::format_parse_context &ctx )
    {
        return ctx.begin( );
    }

    auto format( const env::Direction &Dir, format_context &ctx ) const
    {
        static const char *Strs[] = { "Up", "Down", "Left", "Right" };
        return std::format_to( ctx.out( ), "{}",
                               Strs[static_cast<std::size_t>( Dir )] );
    }
};

}  // namespace std
