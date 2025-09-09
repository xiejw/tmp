#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>

typedef uint64_t u64;
typedef int      map_handle_t;
typedef uint64_t map_hash_t;

struct int_t {
    int          id;
    char        *data;
    map_handle_t hh;
};

struct str_t {
    char        *str;
    char        *data;
    map_handle_t hh;
};

struct map_handle_s {};

typedef struct {
    size_t                cap;
    struct map_handle_s **tbl;
} map_s;

map_hash_t
map_hash_u64( u64 u )
{
    return (map_hash_t)( u );
}

map_hash_t
map_hash_int( int u )
{
    return (map_hash_t)( u );
}

void
map_init( map_s *m )
{
    size_t cap = 64;
    m->cap     = cap;
    m->tbl     = calloc( 1, sizeof( struct map_handle_s * ) * cap );
}

void
map_deinit( map_s *m )
{
    free( m->tbl );
}

void
map_add( map_s *m, map_hash_t hash_value, map_handle_t hh )
{
}

#define MAP_ADD_INT( pm, ptr, key_field ) \
    map_add( pm, map_hash_int( ( ptr )->key_field ), ( ptr )->hh )

int
main( )
{
    map_s m;

    map_init( &m );
    struct int_t t;
    t.id   = 123;
    t.data = "123";
    MAP_ADD_INT( &m, &t, id );

    // map_add(
    // map_find(
    // map_del(
    // map_iter(
    // map_replace(

    map_deinit( &m );

    return 0;
}
