#include <stdio.h>
#include <string.h>
#include <zmq.h>

int
main( )
{
    void *context    = zmq_ctx_new( );
    void *subscriber = zmq_socket( context, ZMQ_SUB );

    // Connect to publisher
    zmq_connect( subscriber, "tcp://localhost:5555" );

    // Subscribe to all messages
    zmq_setsockopt( subscriber, ZMQ_SUBSCRIBE, "", 0 );

    for ( int i = 0; i < 10; i++ ) {
        char buffer[50];
        int  size = zmq_recv( subscriber, buffer, 50, 0 );
        if ( size != -1 ) {
            buffer[size] = '\0';
            printf( "Received: %s\n", buffer );
        }
    }

    zmq_close( subscriber );
    zmq_ctx_destroy( context );
    return 0;
}
