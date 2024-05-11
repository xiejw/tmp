#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Window.H>
#include <FL/names.h>

#include <eve/fs.h>

class MyClass : public Fl_Box {
  public:
    MyClass( int x, int y, int w, int h ) : Fl_Box( x, y, w, h ) {}
    int handle( int e )
    {
        if ( Fl::event_key( ) == FL_Escape ) {
            fprintf( stderr, "escape: %d", e );
            return 0;
        }

        if ( e == FL_KEYDOWN && Fl::event_key( ) == FL_Up ) {
            printf( "up\n" );
            Fl_Shared_Image *img = Fl_Shared_Image::get( "test1.jpg" );

            if ( img->w( ) > this->w( ) || img->h( ) > this->h( ) ) {
                Fl_Image *temp;
                if ( img->w( ) > img->h( ) ) {
                    temp = img->copy( this->w( ),
                                      this->h( ) * img->h( ) / img->w( ) );
                } else {
                    temp = img->copy( this->w( ) * img->w( ) / img->h( ),
                                      this->h( ) );
                }
                img->release( );
                img = (Fl_Shared_Image *)temp;
            }
            this->image( *img );
            this->redraw( );
        }
        return 1;
    }
    //         fprintf(stderr, "EVENT: %s(%d)\n", fl_eventnames[e], e);
    //         if (e==FL_KEYDOWN) {
    //         fprintf(stderr, "Key: %s(%d)\n", Fl::event_text() ,
    //         Fl::event_key());
    //         }
    //         	if (e == FL_KEYDOWN && Fl::event_key() == FL_Escape) {
    //            fprintf(stderr, "escape");
    //            return 0;
    //          }
    //         	if (e == FL_KEYUP && Fl::event_key() == FL_Escape) {
    //            fprintf(stderr, "escape_up");
    //            return 0;
    //          }

    //         return 1;
    //}
};

int
main( )
{
    fl_register_images( );
    Fl_Window win( 720, 486 );
    MyClass   box( 10, 10, 720 - 20, 486 - 20 );
    // Fl_JPEG_Image  png("test.jpg");
    // Fl_Shared_Image *img  = &png;
    Fl_Shared_Image *img = Fl_Shared_Image::get( "test.jpg" );

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
    win.show( );
    return ( Fl::run( ) );
}
