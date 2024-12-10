/*
 * ===----- USAGE -----------------------------------------------------------===
 * This script demostrates how to use One-Step Query Execution Interface
 * (sqlite3_exec).
 *
 * It runs all statements in the SQL string (no binding). Caller is responsible
 * for escaping for safety. But very convenient.
 *
 * References:
 * - https://www.sqlite.org/c3ref/open.html
 * - https://www.sqlite.org/c3ref/prepare.html
 * - https://www.sqlite.org/c3ref/step.html
 * - https://www.sqlite.org/c3ref/column_count.html
 * - https://www.sqlite.org/c3ref/column_blob.html
 *
 * See also for binding values in prepared statement (not demo'ed in this code).
 * - https://www.sqlite.org/c3ref/bind_blob.html
 * - https://www.sqlite.org/c3ref/reset.html
 *
 * ===-----------------------------------------------------------------------===
 */
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

void
handleRow( sqlite3_stmt *pStmt )
{
    int colNum = sqlite3_column_count( pStmt );
    for ( int i = 0; i < colNum; i++ ) {
        printf( "-> value (id:%d): %s\n", i, sqlite3_column_text( pStmt, i ) );
    }
}

int
isEmptyStr( const char *z )
{
    if ( z == NULL ) return 1;
    while ( z[0] == ' ' ) z++;
    return z[0] == 0;
}

int
main( int argc, char **argv )
{
    sqlite3      *db      = NULL;
    sqlite3_stmt *pStmt   = NULL;
    char         *zErrMsg = 0;
    int           rc;
    const char   *zDBName  = ""; /* In memory. Destroy during close. */
    int           exitCode = 0;

    if ( argc != 2 ) {
        fprintf( stderr, "Usage: %s SQL-STATEMENT\n", argv[0] );
        return ( 1 );
    }

    rc = sqlite3_open( zDBName, &db );
    if ( rc ) {
        fprintf( stderr, "Can't open database: %s\n", sqlite3_errmsg( db ) );
        goto cleanup_with_error;
    }

    const char *zSql  = argv[1];
    const char *zTail = NULL;

    while ( 1 ) {
        rc =
            sqlite3_prepare_v2( db, zSql, (int)strlen( zSql ), &pStmt, &zTail );
        if ( rc != SQLITE_OK ) {
            fprintf( stderr, "SQL error: %s\n", zErrMsg );
            sqlite3_free( zErrMsg );
            goto cleanup_with_error;
        }

        rc = sqlite3_step( pStmt );

        switch ( rc ) {
        case SQLITE_DONE:
            break;
        case SQLITE_ROW:
            handleRow( pStmt );
            break;
        default:
            fprintf( stderr, "Can't step statement (rc: %d) %s\n", rc,
                     sqlite3_errmsg( db ) );
            goto cleanup_with_error;
        }

        if ( isEmptyStr( zTail ) ) {
            printf( "-> Tail sql (empty)" );
            break;
        }
        printf( "-> Tail sql (offset %zu): \"%s\"\n", zTail - zSql, zTail );

        sqlite3_finalize( pStmt );
        zSql  = zTail;
        zTail = NULL;
        pStmt = NULL;
    }

    goto cleanup;

cleanup_with_error:
    exitCode = 1;

cleanup:
    sqlite3_finalize( pStmt );
    sqlite3_close( db );
    return exitCode;
}
