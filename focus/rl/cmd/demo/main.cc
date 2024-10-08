#include <stdlib.h>
#include <time.h>

#include <print>
#include <string_view>
#include <vector>

#include "env.h"
#include "policy.h"

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
        const auto &ActionMasks = Env.getLegalActionMasks( );
        env::printLegalActions<env::Direction>( ActionMasks );
        auto ActionToTake =
            env::sampleLegalActions<env::Direction>( ActionMasks );
        std::print( "Action To Take: {}\n", ActionToTake );
        auto Result = Env.step( ActionToTake );
        if ( Result->End ) break;
    }
    return 0;
}
