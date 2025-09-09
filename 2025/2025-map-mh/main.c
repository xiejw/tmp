#include <stdio.h>

typedef int map_handle_t;

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

typedef struct {
} map_s;

void
map_init( map_s *m )
{
}

void
map_deinit( map_s *m )
{
}

int
main( )
{
    map_s m;

    map_init( &m );

    // map_add(
    // map_find(
    // map_del(
    // map_iter(
    // map_replace(

    map_deinit( &m );

    return 0;
}
