/*
 * ===----- USAGE -----------------------------------------------------------===
 * This script demostrates how to include and link sqlite3.
 *
 * In addition, this script also prints out the version, thread mode etc.
 *
 * Full function list can be found: https://www.sqlite.org/c3ref/funclist.html
 *
 * ===-----------------------------------------------------------------------===
 */
#include <sqlite3.h>
#include <stdio.h>

int
main( void )
{
    /* See https://www.sqlite.org/c3ref/libversion.html */
    printf( "sqlite3 version: %s\n", sqlite3_libversion( ) );

    /* See:
     * - https://www.sqlite.org/c3ref/threadsafe.html
     * - https://www.sqlite.org/compile.html#threadsafe
     */
    printf(
        "sqlite3 thread enabled: %s (%d/%s)\n",
        sqlite3_threadsafe( ) != 0 ? "yes" : "no", sqlite3_threadsafe( ),
        sqlite3_threadsafe( ) == 2
            ? "SQLITE_CONFIG_SERIALIZED"
            : ( sqlite3_threadsafe( ) == 1 ? "SQLITE_CONFIG_MULTITHREADl"
                                           : "SQLITE_CONFIG_SINGLETHREAD" ) );
    return 0;
}
