#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

int
main( )
{
    void *context   = zmq_ctx_new( );
    void *publisher = zmq_socket( context, ZMQ_PUB );

    // Bind to TCP port 5555
    zmq_bind( publisher, "tcp://*:5555" );

    for ( int i = 0; i < 10; i++ ) {
        char message[50];
        sprintf( message, "Hello %d", i );
        zmq_send( publisher, message, strlen( message ), 0 );
        printf( "Sent: %s\n", message );
        sleep( 1 );  // simulate work
    }

    zmq_close( publisher );
    zmq_ctx_destroy( context );
    return 0;
}
