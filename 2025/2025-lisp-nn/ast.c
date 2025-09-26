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
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { NODE_LIST, NODE_SYMBOL, NODE_NUMBER } NodeType;

typedef struct Node {
    NodeType      type;
    char         *sym;       // if SYMBOL
    double        num;       // if NUMBER
    struct Node **children;  // if LIST
    int           child_count;
} Node;

//
// --- Utility ---

// Duplicates the s and caller owns it.
static char *
strndup_c( const char *s, size_t n )
{
    char *res = malloc( n + 1 );
    memcpy( res, s, n );
    res[n] = '\0';
    return res;
}

static Node *
make_node( NodeType t )
{
    Node *n = calloc( 1, sizeof( Node ) );
    assert( n != NULL );
    n->type = t;
    return n;
}

//
// --- Tokenizer ---
typedef struct {
    const char *src;
    size_t      pos;
} Lexer;

static char *
next_token( Lexer *lx )
{
    const char *s = lx->src;
    while ( isspace( s[lx->pos] ) ) lx->pos++;  // skip whitespace
    if ( s[lx->pos] == '\0' ) return NULL;

    if ( s[lx->pos] == '(' || s[lx->pos] == ')' ) {
        char *tok = strndup_c( s + lx->pos, 1 );
        lx->pos++;
        return tok;
    }

    size_t start = lx->pos;
    while ( s[lx->pos] && !isspace( s[lx->pos] ) && s[lx->pos] != '(' &&
            s[lx->pos] != ')' ) {
        lx->pos++;
    }
    return strndup_c( s + start, lx->pos - start );
}

//
// --- Parser ---
Node *parse_expr( Lexer *lx );

Node *
parse_list( Lexer *lx )
{
    Node *n        = make_node( NODE_LIST );
    n->children    = NULL;
    n->child_count = 0;

    char *tok;
    while ( ( tok = next_token( lx ) ) ) {
        if ( strcmp( tok, ")" ) == 0 ) {
            free( tok );
            return n;
        } else if ( strcmp( tok, "(" ) == 0 ) {
            free( tok );
            Node *child                   = parse_list( lx );
            n->children                   = realloc( n->children,
                                                     ( n->child_count + 1 ) * sizeof( Node * ) );
            n->children[n->child_count++] = child;
        } else {
            // symbol or number
            Node  *child;
            char  *endptr;
            double val = strtod( tok, &endptr );
            if ( *endptr == '\0' ) {
                child      = make_node( NODE_NUMBER );
                child->num = val;
            } else {
                child      = make_node( NODE_SYMBOL );
                child->sym = tok;  // keep
                tok        = NULL;
            }
            n->children                   = realloc( n->children,
                                                     ( n->child_count + 1 ) * sizeof( Node * ) );
            n->children[n->child_count++] = child;
            free( tok );
        }
    }
    return n;
}

Node *
parse_expr( Lexer *lx )
{
    char *tok = next_token( lx );
    if ( !tok ) return NULL;
    if ( strcmp( tok, "(" ) == 0 ) {
        free( tok );
        return parse_list( lx );
    }
    // TODO what's this?
    free( tok );
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
        for ( int i = 0; i < n->child_count; i++ ) {
            print_ast( n->children[i], depth + 1 );
        }
        for ( int i = 0; i < depth; i++ ) printf( "  " );
        printf( ")\n" );
    } else if ( n->type == NODE_SYMBOL ) {
        printf( "SYMBOL: %s\n", n->sym );
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
    Node *root = parse_expr( &lx );
    print_ast( root, 0 );
    return 0;
}
