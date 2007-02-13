// ----------------------------------------------------------------------------
//  Description      : WAM image class
// ----------------------------------------------------------------------------
//  Remarks          :
//    The reference point only comes into play when a bitmap is placed
//    at a certain position in another one. (and even then, only the
//    reference point of the placed-in bitmap is used)
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_IMAGE
#define WAM_IMAGE




#include <sdlucid_video.hh>
#include "utility/base.hh"




class wImage : public bitmap
{
    typedef bitmap super;
    sdl::coordinate_vector ReferencePoint;

  public:
    wImage();
    wImage( SDL_PixelFormat const &fmt, TSize width, TSize height,
            Uint32 flags = SDL_SWSURFACE );
    wImage( wImage const &src );

    coordinate_vector referencePoint() const;
    void referencePoint( coordinate_vector const &refp );
    coordinate_rectangle extent() const;

    void loadPNG( std::istream &datastrm );
    void savePNG( std::ostream &datastrm );

    void create( SDL_PixelFormat const &fmt, TSize width, TSize height,
                 Uint32 flags = SDL_SWSURFACE );
    void copyFrom( wImage const &src );
    void convertFrom( wImage const &src, SDL_PixelFormat const &fmt, Uint32 flags );
    void convertForAcceleratedBlitFrom( wImage const &src );
    void stretchFrom( wImage const &src, double stretch_x, double stretch_y );
    void transformFrom( wImage const &src, affine_transformation const &trans );

    void blit( drawable &dest, TCoordinate x, TCoordinate y );
    void blit( drawable &dest, TCoordinate x, TCoordinate y,
               coordinate_rectangle const &source );
};




#endif
