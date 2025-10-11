/* A simple DSL for neural network.
 *
 * Design Goals:
 * - It is deadly simple so very easy to parse by program and human.
 * - It is declarative so no hidden magic.
 * - It is self contained so easy to audit.
 *
 * Design Inspiration:
 * - It looks like protobuf and the compiler will compiles down to C code with
 *   user chosen struct type name.
 *
 *    // Define the network.
 *    (network
 *      (config name=nn2)
 *      (params (input_size 784) (hidden_units 128))
 *      (layers
 *        (linear name=fc1 in=input_size out=hidden_units act=relu)
 *        (linear name=fc2 in=hidden_units out=10 act=softmax)
 *      )
 *    )
 *
 *    %% // A separator
 *
 *    // Generate generator
 *
 *    %out_name generator nn2
 *    %param input_size 32
 *    %generate
 *
 *    // Generate policy
 *
 *    %out_name policy nn2
 *    %param input_size 64
 *    %generate
 *
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "parser.h"

//
// --- Example ---
int
main( )
{
    const char *input =
        "(network\n"
        "  (params (input_size 784) (hidden_units 128))\n"
        "  (layers\n"
        "    (linear name=fc1 in=input_size out=hidden_units act=relu)\n"
        "    (linear name=fc2 in=hidden_units out=10 act=softmax)))";

    printf( "=== --- RAW input --- ===\n%s\n", input );
    printf( "=== --- Result    --- ===\n" );

    // TODO bug input is assume to ends at \0
    auto root = luma::parse_expr( input );
    luma::print( root.get( ) );
    return 0;
}
