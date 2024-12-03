/*
 * ===----- USAGE -----------------------------------------------------------===
 * This script demostrates how to use One-Step Query Execution Interface
 * (sqlite3_exec).
 *
 * It runs all statements in the SQL string (no binding). Caller is responsible
 * for escaping for safety. But very convenient.
 *
 * References:
 * - https://www.sqlite.org/quickstart.html
 * - https://www.sqlite.org/c3ref/open.html
 * - https://www.sqlite.org/cli.html
 *
 * ===-----------------------------------------------------------------------===
 */
#include <sqlite3.h>
#include <stdio.h>

static int
callback( void *NotUsed, int argc, char **argv, char **azColName )
{
    int i;
    printf( "->-> {{{\n" );
    for ( i = 0; i < argc; i++ ) {
        printf( "\t\t%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL" );
    }
    printf( "->-> }}}\n" );
    return 0;
}

int
main( int argc, char **argv )
{
    sqlite3    *db      = NULL;
    char       *zErrMsg = 0;
    int         rc;
    const char *zDBName = ""; /* In memory. Destroy during close. */

    if ( argc != 2 ) {
        fprintf( stderr, "Usage: %s SQL-STATEMENT\n", argv[0] );
        return ( 1 );
    }

    rc = sqlite3_open( zDBName, &db );
    if ( rc ) {
        fprintf( stderr, "Can't open database: %s\n", sqlite3_errmsg( db ) );
        goto cleanup_with_error;
    }

    rc = sqlite3_exec( db, argv[1], callback, 0, &zErrMsg );
    if ( rc != SQLITE_OK ) {
        fprintf( stderr, "SQL error: %s\n", zErrMsg );
        sqlite3_free( zErrMsg );
        goto cleanup_with_error;
    }

    sqlite3_close( db );
    return 0;

cleanup_with_error:
    sqlite3_close( db );
    return 1;
}
