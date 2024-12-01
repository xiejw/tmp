#include <gallery/app.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Window.H>
#include <FL/names.h>

#include <eve/base/error.h>
#include <eve/base/log.h>

namespace {
using gallery::Store;
class MyClass : public Fl_Box {
  private:
    Store &Store;

  public:
    MyClass( struct Store &Store, int x, int y, int w, int h )
        : Fl_Box( x, y, w, h ), Store( Store )
    {
    }

  private:
    Fl_Image *loadImage( const char *path )
    {
        Fl_Shared_Image *img = Fl_Shared_Image::get( path );

        int w  = img->w( );
        int h  = img->h( );
        int bw = this->w( );
        int bh = this->h( );

        if ( w > bw || h > bh ) {
            // Fl_Image *temp;
            if ( w > bw ) {
                h = int( h * 1.0 / w * bw );
                w = bw;
            }
            if ( h > bh ) {
                w = int( w * 1.0 / h * bh );
                h = bh;
            }
            img->scale( w, h, 1 );
            // temp = img->copy( w, h );
            // img->release( );
            // img = (Fl_Shared_Image *)temp;
        }
        return img;
    }

  public:
    int handle( int e )
    {
        if ( Fl::event_key( ) == FL_Escape ) {
            logInfo( "deteced escape key. quit" );
            return 0;
        }

        if ( e == FL_KEYDOWN && Fl::event_key( ) == FL_Up ) {
            std::optional<std::string> PrevImage = Store.getPrev( );
            if ( !PrevImage )
                logInfo( "No more prev image" );
            else {
                auto *path = PrevImage.value( ).c_str( );
                logInfo( "Load prev image: %s", path );
                ( (Fl_Shared_Image *)this->image( ) )->release( );
                this->image( loadImage( path ) );
                this->parent( )->redraw( );
            }
        }
        return 1;
    }
};
}  // namespace

namespace gallery {
int
App::Run( )
{
    return RunFltk( );
}

int
App::RunFltk( )
{
    auto PhotoPathOpt = Store.getLast( );
    if ( !PhotoPathOpt ) {
        panic( "no last photo found" );
    }
    logInfo( "Open File with: %s", PhotoPathOpt.value( ).c_str( ) );

    fl_register_images( );
    Fl_Window        win( 720, 486 );
    MyClass          box( Store, 10, 10, 720 - 20, 486 - 20 );
    Fl_Shared_Image *img =
        Fl_Shared_Image::get( PhotoPathOpt.value( ).c_str( ) );

    if ( img->w( ) > box.w( ) || img->h( ) > box.h( ) ) {
        Fl_Image *temp;
        if ( img->w( ) > img->h( ) ) {
            temp = img->copy( box.w( ), box.h( ) * img->h( ) / img->w( ) );
        } else {
            temp = img->copy( box.w( ) * img->w( ) / img->h( ), box.h( ) );
        }
        img->release( );
        img = (Fl_Shared_Image *)temp;
    }
    box.image( *img );
    box.align( FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_CLIP );
    win.resizable( &win );
    win.show( );
    return Fl::run( );
}
}  // namespace gallery
