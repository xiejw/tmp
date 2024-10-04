#include <stdlib.h>
#include <time.h>

#include <print>
#include <string_view>
#include <vector>

#include "env.h"

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

    for ( ;; ) {
        Env.render( );
        const auto &Actions = Env.getLegalActions( );
        env::printLegalActions( Actions );
        auto ActionToTake = env::sampleLegalActions( Actions );
        std::print( "Action To Take: {}\n", ActionToTake );
        if ( Env.step( ActionToTake ) ) break;
    }
    return 0;
}
