#include <print>
#include <span>
#include <string_view>
#include <vector>

struct Env {
    std::span<std::string_view> maze;

    auto render( ) -> void
    {
        for ( auto &row : maze ) {
            std::println( "{}", row );
        }
    }
};

int
main( )
{
    std::vector<std::string_view> maze{
        "sx..",
        "....",
        "..x.",
        "..xg",
    };

    Env Env{ maze };
    Env.render( );
    return 0;
}
