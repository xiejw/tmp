#include <gallery/app.h>
#include <gallery/store.h>

int
main( )
{
    gallery::Store Store{ "/opt/photos" };
    gallery::App   App{ Store };
    return App.Run( );
}
