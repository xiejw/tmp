#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define COLOR_CYAN  "\033[36m"
#define COLOR_RESET "\033[0m"

static const char *items[]     = { "apple",      "banana", "cherry", "date",
                                   "elderberry", "fig",    "grape",  "honeydew",
                                   "kiwi",       "lemon" };
static const int   ITEMS_COUNT = 10;

static int
contains_icase( const char *haystack, const char *needle )
{
    if ( *needle == '\0' ) return 1;
    size_t nlen = strlen( needle );
    size_t hlen = strlen( haystack );
    if ( nlen > hlen ) return 0;
    for ( size_t i = 0; i <= hlen - nlen; i++ ) {
        size_t j;
        for ( j = 0; j < nlen; j++ ) {
            if ( tolower( (unsigned char)haystack[i + j] ) !=
                 tolower( (unsigned char)needle[j] ) )
                break;
        }
        if ( j == nlen ) return 1;
    }
    return 0;
}

static struct termios old_state;

static void
make_raw( void )
{
    struct termios raw;
    tcgetattr( STDIN_FILENO, &old_state );
    raw = old_state;
    raw.c_lflag &= ~( ECHO | ICANON | ISIG | IEXTEN );
    raw.c_iflag &= ~( IXON | ICRNL | BRKINT | INPCK | ISTRIP );
    raw.c_cflag |= CS8;
    raw.c_oflag &= ~OPOST;
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &raw );
}

static void
restore_term( void )
{
    tcsetattr( STDIN_FILENO, TCSAFLUSH, &old_state );
}

static void
get_term_size( int *cols, int *rows )
{
    struct winsize ws;
    ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws );
    *cols = ws.ws_col;
    *rows = ws.ws_row;
}

int
main( void )
{
    char query[256]         = { 0 };
    int  query_len          = 0;
    int  selected_index     = 0;
    char selected_item[256] = { 0 };

    const char *filtered[ITEMS_COUNT];
    int         filtered_count = 0;

    make_raw( );

    for ( ;; ) {
        // 1. Get terminal size
        int cols, height;
        get_term_size( &cols, &height );

        // 2. Filter
        filtered_count = 0;
        for ( int i = 0; i < ITEMS_COUNT; i++ ) {
            if ( contains_icase( items[i], query ) )
                filtered[filtered_count++] = items[i];
        }

        // 3. Preserve selected item across query changes; fall back to top
        selected_index = 0;
        for ( int i = 0; i < filtered_count; i++ ) {
            if ( strcmp( filtered[i], selected_item ) == 0 ) {
                selected_index = i;
                break;
            }
        }
        if ( filtered_count > 0 )
            strncpy( selected_item, filtered[selected_index],
                     sizeof( selected_item ) - 1 );
        else
            selected_item[0] = '\0';

        // 4. Render (bottom-up)
        printf( "\033[2J" );  // clear screen

        int num_lines   = filtered_count;
        int max_visible = height - 2;
        if ( num_lines > max_visible ) num_lines = max_visible;

        for ( int i = 0; i < num_lines; i++ ) {
            int row = height - 2 - ( num_lines - 1 - i );
            printf( "\033[%d;1H", row );
            if ( i == selected_index )
                printf( "%s> %s%s\r", COLOR_CYAN, filtered[i], COLOR_RESET );
            else
                printf( "  %s\r", filtered[i] );
        }

        // Separator
        printf( "\033[%d;1H------------------\r", height - 1 );

        // Query line
        printf( "\033[%d;1H%s> %s%s\r", height, COLOR_CYAN, query,
                COLOR_RESET );

        // Cursor at end of query
        printf( "\033[%d;%dH", height, 3 + query_len );
        fflush( stdout );

        // 5. Read input
        unsigned char buf[3] = { 0 };
        int           n      = (int)read( STDIN_FILENO, buf, sizeof( buf ) );

        if ( n == 1 ) {
            unsigned char c = buf[0];
            if ( c == 3 ) {  // Ctrl+C
                break;
            } else if ( c == 13 ) {  // Enter
                restore_term( );
                if ( filtered_count > 0 )
                    printf( "\nSelected: %s\n", filtered[selected_index] );
                return 0;
            } else if ( c == 127 ) {  // Backspace
                if ( query_len > 0 ) query[--query_len] = '\0';
            } else if ( c >= 32 && c <= 126 ) {
                if ( query_len < (int)sizeof( query ) - 1 ) {
                    query[query_len++] = (char)c;
                    query[query_len]   = '\0';
                }
            }
        } else if ( n == 3 && buf[0] == 27 && buf[1] == 91 ) {
            if ( buf[2] == 65 ) {  // Up arrow
                if ( selected_index > 0 ) {
                    selected_index--;
                    strncpy( selected_item, filtered[selected_index],
                             sizeof( selected_item ) - 1 );
                }
            } else if ( buf[2] == 66 ) {  // Down arrow
                if ( selected_index < filtered_count - 1 ) {
                    selected_index++;
                    strncpy( selected_item, filtered[selected_index],
                             sizeof( selected_item ) - 1 );
                }
            }
        }
    }

    restore_term( );
    return 0;
}
