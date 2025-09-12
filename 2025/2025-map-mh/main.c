// Design Philosophy
//
// - The whole map table is a double linked list.
// - All items in one bucket (same hash value) are linked continuously.
// - The table has a header points to one bucket header (might note be the
//   first one).
//   - When an item is inserted to an existing bucket, it becomes the new
//     header of the bucket and surrounding pointers are updated.
//   - When an item is inserted into an empty bucket, it becomes the new header
//     of the bucket and the new header of the whole table. Surrounding
//     pointers are updated.
// - Global table iteration is super simple. It follows the links. And it is
//   even safe to delete current item during iteration.
//
// - To support lookup, a third pointer, called 'next', is only set for all
//   items in the same bucket. It is single direction linked and final item in
//   the bucket is pointing to NULL.
#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>

typedef uint64_t u64;
typedef uint64_t map_hash_t;

// map

struct map_tbl;

struct map_slot_s {
    struct map_slot_s *lft;
    struct map_slot_s *rht;
    struct map_slot_s *next;
};

typedef struct {
    size_t              cap;
    struct map_slot_s **bkt;
    struct map_slot_s  *head;
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
    m->bkt     = calloc( 1, sizeof( struct map_slot_s * ) * cap );
    m->head    = NULL;
}

void
map_deinit( map_s *m )
{
    free( m->bkt );
}

void
map_add( map_s *m, map_hash_t hash_value, struct map_slot_s slot )
{
    // Algorithm:
    // Find the slot.
    // If present, link it .
    // If not, put as slot head and link to table head.
}

#define MAP_ADD_INT( pm, ptr, key_field ) \
    map_add( pm, map_hash_int( ( ptr )->key_field ), ( ptr )->slot )

struct int_t {
    int               id;
    char             *data;
    struct map_slot_s slot;
};

struct str_t {
    char             *str;
    char             *data;
    struct map_slot_s slot;
};

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
