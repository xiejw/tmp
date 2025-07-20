/*
 * ===----- USAGE -----------------------------------------------------------===
 * This script demostrates how to use prepare, step and finalize to run SQL
 * queries. Particularly, it takes a multi sql statements string and process it
 * one each time.
 *
 * It runs all statements in the SQL string (no binding in this example).
 *
 * References:
 * - https://www.sqlite.org/c3ref/open.html
 * - https://www.sqlite.org/c3ref/prepare.html
 * - https://www.sqlite.org/c3ref/step.html
 * - https://www.sqlite.org/c3ref/column_count.html
 * - https://www.sqlite.org/c3ref/column_blob.html
 * - https://www.sqlite.org/c3ref/errcode.html
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
handle_sqlite_row( sqlite3_stmt *pStmt )
{
    int colNum = sqlite3_column_count( pStmt );
    for ( int i = 0; i < colNum; i++ ) {
        /* No matter the data type, column_text is ok to use */
        printf( "\t-> value (id:%d): %s\n", i,
                sqlite3_column_text( pStmt, i ) );
    }
}

int
handle_sqlite_statement( sqlite3 *db, sqlite3_stmt *pStmt )
{
    while ( 1 ) {
        int rc = sqlite3_step( pStmt );
        switch ( rc ) {
        case SQLITE_DONE:
            /* We are really done here. */
            return 0;
        case SQLITE_ROW:
            handle_sqlite_row( pStmt );
            continue;
        case SQLITE_MISUSE:
            /* According to the official doc [1], seems only SQLITE_MISUSE is
             * not associated with sqlite3_errmsg. This is confirmed in [2] as
             * well. All other error codes can result in printing some error
             * messages with sqlite3_errmsg.
             *
             * [1]: https://www.sqlite.org/c3ref/step.html
             * [2]: https://www.sqlite.org/c3ref/errcode.html
             */
            fprintf( stderr, "Can't step statement (rc: %d) %s\n", rc,
                     "misuse" );
        default:
            /* Print error message via sqlite3_errmsg. */
            fprintf( stderr, "Can't step statement (rc: %d) %s\n", rc,
                     sqlite3_errmsg( db ) );
            return 1;
        }
    }
}

int
is_str_empty( const char *z )
{
    if ( z == NULL ) return 1;
    while ( z[0] == ' ' || z[0] == '\n' || z[0] == '\r' || z[0] == '\t' ) z++;
    return z[0] == 0;
}

int
main( int argc, char **argv )
{
    sqlite3    *db = NULL;
    int         rc;
    const char *zDBName  = ""; /* In memory. Destroy during close. */
    int         exitCode = 0;

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
        sqlite3_stmt *pStmt = NULL;
        rc =
            sqlite3_prepare_v2( db, zSql, (int)strlen( zSql ), &pStmt, &zTail );
        if ( rc != SQLITE_OK ) {
            fprintf( stderr, "SQL prepare error: %s\n", sqlite3_errmsg( db ) );
            sqlite3_finalize( pStmt );
            goto cleanup_with_error;
        }

        rc = handle_sqlite_statement( db, pStmt );
        if ( rc ) break;

        if ( is_str_empty( zTail ) ) {
            printf( "-> Tail sql (empty)\n" );
            sqlite3_finalize( pStmt );
            break;
        }

        sqlite3_finalize( pStmt );
        printf( "-> Tail sql (offset %zu): \"%s\"\n", zTail - zSql, zTail );
        zSql  = zTail;
        zTail = NULL;
        pStmt = NULL;
    }

    goto cleanup;

cleanup_with_error:
    exitCode = 1;

cleanup:
    sqlite3_close( db );
    return exitCode;
}
