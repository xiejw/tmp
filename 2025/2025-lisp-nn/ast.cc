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

typedef enum { NODE_LIST, NODE_SYMBOL, NODE_NUMBER } NodeType;

typedef struct Node {
    NodeType                           type;
    std::string                        sym;       // if SYMBOL
    double                             num;       // if NUMBER
    std::vector<std::unique_ptr<Node>> children;  // if LIST
} Node;

//
// --- Utility ---

static std::unique_ptr<Node>
make_node( NodeType t )
{
    auto n = std::make_unique<Node>( );
    n->type = t;
    return n;
}

//
// --- Tokenizer ---
typedef struct {
    const char *src;
    size_t      pos;
} Lexer;

static std::string
next_token( Lexer *lx )
{
    const char *s = lx->src;
    while ( isspace( s[lx->pos] ) ) lx->pos++;  // skip whitespace
    if ( s[lx->pos] == '\0' ) return std::string{ };

    if ( s[lx->pos] == '(' || s[lx->pos] == ')' ) {
        std::string tok{ s + lx->pos, 1 };
        lx->pos++;
        return tok;
    }

    size_t start = lx->pos;
    while ( s[lx->pos] && !isspace( s[lx->pos] ) && s[lx->pos] != '(' &&
            s[lx->pos] != ')' ) {
        lx->pos++;
    }
    return std::string{ s + start, lx->pos - start };
}

//
// --- Parser ---
std::unique_ptr<Node> parse_expr( Lexer *lx );

std::unique_ptr<Node>
parse_list( Lexer *lx )
{
    auto n = make_node( NODE_LIST );

    while ( true ) {
        std::string tok = next_token( lx );
        if ( tok.empty( ) ) break;

        if ( tok == ")" ) {
            return n;
        } else if ( tok == "(" ) {
            auto child = parse_list( lx );
            n->children.push_back( std::move( child ) );
        } else {
            // symbol or number
            std::unique_ptr<Node> child;
            char                 *endptr;
            double                val = strtod( tok.c_str( ), &endptr );
            if ( *endptr == '\0' ) {
                child      = make_node( NODE_NUMBER );
                child->num = val;
            } else {
                child      = make_node( NODE_SYMBOL );
                child->sym = std::move( tok );
            }
            n->children.push_back( std::move( child ) );
        }
    }
    return n;
}

std::unique_ptr<Node>
parse_expr( Lexer *lx )
{
    std::string tok = next_token( lx );
    if ( tok.empty( ) ) return NULL;
    if ( tok == "(" ) {
        return parse_list( lx );
    }
    // TODO panic
    return NULL;
}

//
// --- Printer ---
void
print_ast( Node *n, int depth )
{
    for ( int i = 0; i < depth; i++ ) printf( "  " );
    if ( n->type == NODE_LIST ) {
        printf( "(\n" );
        for ( auto &child : n->children ) {
            print_ast( child.get( ), depth + 1 );
        }
        for ( int i = 0; i < depth; i++ ) printf( "  " );
        printf( ")\n" );
    } else if ( n->type == NODE_SYMBOL ) {
        printf( "SYMBOL: %s\n", n->sym.c_str( ) );
    } else if ( n->type == NODE_NUMBER ) {
        printf( "NUMBER: %g\n", n->num );
    }
}

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

    Lexer lx   = { input, 0 };
    auto  root = parse_expr( &lx );
    print_ast( root.get( ), 0 );
    return 0;
}
