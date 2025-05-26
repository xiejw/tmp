/* A demo about pcre2 library.
 *
 * - To install dependency:
 *
 *       sudo apt install libpcre2-dev
 *
 * - To see usage:
 *
 *       make
 *
 * See: https://www.pcre.org/current/doc/html/pcre2demo.html
 */
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>
#include <stdio.h>
#include <string.h>

int
main( int argc, char **argv )
{
    int i;

    if ( argc != 3 ) {
        printf(
            "Exactly two arguments required: a regex and a subject string\n" );
        return 1;
    }

    /* Pattern and subject are char arguments, so they can be straightforwardly
    cast to PCRE2_SPTR because we are working in 8-bit code units. The subject
    length is cast to PCRE2_SIZE for completeness, though PCRE2_SIZE is in fact
    defined to be size_t. */

    PCRE2_SPTR pattern; /* PCRE2_SPTR is a pointer to unsigned code units of */
    PCRE2_SPTR subject; /* the appropriate width (in this case, 8 bits). */
    PCRE2_SIZE subject_length;

    pattern        = (PCRE2_SPTR)argv[1];
    subject        = (PCRE2_SPTR)argv[2];
    subject_length = (PCRE2_SIZE)strlen( (char *)subject );

    PCRE2_SIZE  erroroffset;
    int         errornumber;
    pcre2_code *re = pcre2_compile(
        pattern,               /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
        PCRE2_UTF,             /* default options */
        &errornumber,          /* for error number */
        &erroroffset,          /* for error offset */
        NULL );                /* use default compile context */

    /* Compilation failed: print the error message and exit. */

    if ( re == NULL ) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message( errornumber, buffer, sizeof( buffer ) );
        printf( "PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
                buffer );
        return 1;
    }

    /*************************************************************************
     * If the compilation succeeded, we call PCRE2 again, in order to do a    *
     * pattern match against the subject string. This does just ONE match. If *
     * further matching is needed, it will be done below. Before running the  *
     * match we must set up a match_data block for holding the result. Using  *
     * pcre2_match_data_create_from_pattern() ensures that the block is       *
     * exactly the right size for the number of capturing parentheses in the  *
     * pattern. If you need to know the actual size of a match_data block as  *
     * a number of bytes, you can find it like this:                          *
     *                                                                        *
     * PCRE2_SIZE match_data_size = pcre2_get_match_data_size(match_data);    *
     *************************************************************************/

    pcre2_match_data *match_data =
        pcre2_match_data_create_from_pattern( re, NULL );

    /* Now run the match. */

    int rc = pcre2_match( re,             /* the compiled pattern */
                          subject,        /* the subject string */
                          subject_length, /* the length of the subject */
                          0,              /* start at offset 0 in the subject */
                          0,              /* default options */
                          match_data,     /* block for storing the result */
                          NULL );         /* use default match context */

    /* Matching failed: handle error cases */

    if ( rc < 0 ) {
        switch ( rc ) {
        case PCRE2_ERROR_NOMATCH:
            printf( "No match\n" );
            break;
        /*
        Handle other special cases if you like
        */
        default:
            printf( "Matching error %d\n", rc );
            break;
        }
        pcre2_match_data_free(
            match_data );      /* Release memory used for the match */
        pcre2_code_free( re ); /*   data and the compiled pattern. */
        return 1;
    }

    /* Match succeeded. Get a pointer to the output vector, where string offsets
    are stored. */

    PCRE2_SIZE *ovector;
    ovector = pcre2_get_ovector_pointer( match_data );
    printf( "Match succeeded at offset %d\n", (int)ovector[0] );

    /*************************************************************************
     * We have found the first match within the subject string. If the output *
     * vector wasn't big enough, say so. Then output any substrings that were *
     * captured.                                                              *
     *************************************************************************/

    /* The output vector wasn't big enough. This should not happen, because we
    used pcre2_match_data_create_from_pattern() above. */

    if ( rc == 0 )
        printf(
            "ovector was not big enough for all the captured substrings\n" );

    /* Show substrings stored in the output vector by number. Obviously, in a
    real application you might want to do things other than print them. */

    for ( i = 0; i < rc; i++ ) {
        PCRE2_SPTR substring_start  = subject + ovector[2 * i];
        PCRE2_SIZE substring_length = ovector[2 * i + 1] - ovector[2 * i];
        printf( "%2d: %.*s\n", i, (int)substring_length,
                (char *)substring_start );
    }

    return 0;
}
