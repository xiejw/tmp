/*
 * Read the demo page to understand how to play with pcre2.
 * https://www.pcre.org/current/doc/html/pcre2demo.html
 *
 * ===----- INSTALL ---------------------------------------------------------===
 *
 * - On linux/macOs machine, pcre2 is pre-installed.
 * - To install pcre2 manually, look at homebrew formula [1] with releases [2].
 *   A example cmd pipeline looks like
 *
 *   ```
 *   ./configure                    \
 *       --prefix=<PREFIX>          \
 *       --disable-shared           \
 *       --enable-pcre2grep-libz    \
 *       --enable-pcre2grep-libbz2  \
 *       --enable-jit
 *
 *   make
 *   make install
 *   ```
 *
 * [1]:
 * https://github.com/Homebrew/homebrew-core/blob/0d67873ab0aca153eab22b32c0c4edacf4ae08e1/Formula/p/pcre2.rb
 * [2]: https://github.com/PCRE2Project/pcre2/releases
 *
 * ===-----------------------------------------------------------------------===
 *
 * ===----- COMPILATION and USAGE -------------------------------------------===
 *
 * See Makefile
 * ===-----------------------------------------------------------------------===
 */
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>
#include <stdio.h>
#include <string.h>

/**************************************************************************
 * Here is the program. The API includes the concept of "contexts" for     *
 * setting up unusual interface requirements for compiling and matching,   *
 * such as custom memory managers and non-standard newline definitions.    *
 * This program does not do any of this, so it makes no use of contexts,   *
 * always passing NULL where a context could be given.                     *
 **************************************************************************/

int
main( int argc, char **argv )
{
    pcre2_code *re;
    PCRE2_SPTR  pattern; /* PCRE2_SPTR is a pointer to unsigned code units of */
    PCRE2_SPTR  subject; /* the appropriate width (in this case, 8 bits). */
    PCRE2_SPTR  name_table;

    int errornumber;
    int i;
    int rc;

    uint32_t namecount;
    uint32_t name_entry_size;

    PCRE2_SIZE  erroroffset;
    PCRE2_SIZE *ovector;
    PCRE2_SIZE  subject_length;

    pcre2_match_data *match_data;

    /* We require exactly two arguments, which are the
       pattern, and the subject string. */

    if ( argc - 1 != 2 ) {
        printf(
            "Exactly two arguments required: a regex and a subject string\n" );
        return 1;
    }

    /* Pattern and subject are char arguments, so they can be straightforwardly
    cast to PCRE2_SPTR because we are working in 8-bit code units. The subject
    length is cast to PCRE2_SIZE for completeness, though PCRE2_SIZE is in fact
    defined to be size_t. */

    pattern        = (PCRE2_SPTR)argv[1];
    subject        = (PCRE2_SPTR)argv[1 + 1];
    subject_length = (PCRE2_SIZE)strlen( (char *)subject );

    /*************************************************************************
     * Now we are going to compile the regular expression pattern, and handle *
     * any errors that are detected.                                          *
     *************************************************************************/

    re = pcre2_compile(
        pattern,               /* the pattern */
        PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
        0,                     /* default options */
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

    match_data = pcre2_match_data_create_from_pattern( re, NULL );

    /* Now run the match. */

    rc = pcre2_match( re,             /* the compiled pattern */
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

    /* Since release 10.38 PCRE2 has locked out the use of \K in lookaround
    assertions. However, there is an option to re-enable the old behaviour. If
    that is set, it is possible to run patterns such as /(?=.\K)/ that use \K in
    an assertion to set the start of a match later than its end. In this
    demonstration program, we show how to detect this case, but it shouldn't
    arise because the option is never set. */

    if ( ovector[0] > ovector[1] ) {
        printf(
            "\\K was used in an assertion to set the match start after its "
            "end.\n"
            "From end to start the match was: %.*s\n",
            (int)( ovector[0] - ovector[1] ),
            (char *)( subject + ovector[1] ) );
        printf( "Run abandoned\n" );
        pcre2_match_data_free( match_data );
        pcre2_code_free( re );
        return 1;
    }

    /* Show substrings stored in the output vector by number. Obviously, in a
    real application you might want to do things other than print them. */

    for ( i = 0; i < rc; i++ ) {
        PCRE2_SPTR substring_start  = subject + ovector[2 * i];
        PCRE2_SIZE substring_length = ovector[2 * i + 1] - ovector[2 * i];
        printf( "%2d: %.*s\n", i, (int)substring_length,
                (char *)substring_start );
    }

    /**************************************************************************
     * That concludes the basic part of this demonstration program. We have    *
     * compiled a pattern, and performed a single match. The code that follows *
     * shows first how to access named substrings, and then how to code for    *
     * repeated matches on the same subject.                                   *
     **************************************************************************/

    /* See if there are any named substrings, and if so, show them by name.
    First we have to extract the count of named parentheses from the pattern. */

    (void)pcre2_pattern_info(
        re,                   /* the compiled pattern */
        PCRE2_INFO_NAMECOUNT, /* get the number of named substrings */
        &namecount );         /* where to put the answer */

    if ( namecount == 0 )
        printf( "No named substrings\n" );
    else {
        PCRE2_SPTR tabptr;
        printf( "Named substrings\n" );

        /* Before we can access the substrings, we must extract the table for
        translating names to numbers, and the size of each entry in the table.
      */

        (void)pcre2_pattern_info(
            re,                   /* the compiled pattern */
            PCRE2_INFO_NAMETABLE, /* address of the table */
            &name_table );        /* where to put the answer */

        (void)pcre2_pattern_info(
            re,                       /* the compiled pattern */
            PCRE2_INFO_NAMEENTRYSIZE, /* size of each entry in the table */
            &name_entry_size );       /* where to put the answer */

        /* Now we can scan the table and, for each entry, print the number, the
        name, and the substring itself. In the 8-bit library the number is held
        in two bytes, most significant first. */

        tabptr = name_table;
        for ( i = 0; i < namecount; i++ ) {
            int n = ( tabptr[0] << 8 ) | tabptr[1];
            printf( "(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
                    (int)( ovector[2 * n + 1] - ovector[2 * n] ),
                    subject + ovector[2 * n] );
            tabptr += name_entry_size;
        }
    }

    pcre2_match_data_free( match_data ); /* Release the memory that was used */
    pcre2_code_free( re ); /* for the match data and the pattern. */
    return 0;              /* Exit the program. */
}
