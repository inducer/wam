// ----------------------------------------------------------------------------
//  Description      : WAM image class
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_numconv.hh>
#include "utility/png_info.hh"
#include "output/image.hh"




// wImage ---------------------------------------------------------------------
wImage::wImage()
    : ReferencePoint( 0, 0 )
{}




wImage::wImage( SDL_PixelFormat const &fmt, TSize width, TSize height,
                Uint32 flags )
    : super( fmt, width, height, flags ), ReferencePoint( 0, 0 )
{}





wImage::wImage( wImage const &src )
    : super( src ), ReferencePoint( src.ReferencePoint )
{}





coordinate_vector wImage::referencePoint() const
{
  return ReferencePoint;
}




void wImage::referencePoint( coordinate_vector const &refp )
{
  ReferencePoint = refp;
}




coordinate_rectangle wImage::extent() const
{
  return super::extent() - ReferencePoint;
}




void wImage::loadPNG( std::istream &datastrm )
{
  png_meta_data meta;
  super::loadPNG( datastrm, &meta );

  ReferencePoint[ 0 ] = 0;
  ReferencePoint[ 1 ] = 0;
  try
  {
    ReferencePoint[ 0 ] = evalSigned( meta[ WAM_PNG_REFPOINTX ] );
    ReferencePoint[ 1 ] = evalSigned( meta[ WAM_PNG_REFPOINTY ] );
  }
  catch ( ... )
  { }
}




void wImage::savePNG( std::ostream &datastrm )
{
  png_meta_data meta;
  meta[ WAM_PNG_REFPOINTX ] = signed2dec( ReferencePoint[ 0 ] );
  meta[ WAM_PNG_REFPOINTY ] = signed2dec( ReferencePoint[ 1 ] );
  super::savePNG( datastrm, &meta );
}




void wImage::create( SDL_PixelFormat const &fmt, TSize width, TSize height,
                     Uint32 flags )
{
  super::create( fmt, width, height, flags );
  ReferencePoint.set( 0, 0 );
}




void wImage::copyFrom( wImage const &src )
{
  super::copyFrom( src );
  ReferencePoint = src.ReferencePoint;
}




void wImage::convertFrom( wImage const &src, SDL_PixelFormat const &fmt, Uint32 flags )
{
  super::convertFrom( src, fmt, flags );
  ReferencePoint = src.ReferencePoint;
}





void wImage::convertForAcceleratedBlitFrom( wImage const &src )
{
  super::convertForAcceleratedBlitFrom( src );
  ReferencePoint = src.ReferencePoint;
}




void wImage::stretchFrom( wImage const &src, double stretch_x, double stretch_y )
{
  super::stretchFrom( src, stretch_x, stretch_y );
  ReferencePoint[ 0 ] = TCoordinate( src.ReferencePoint[ 0 ] * stretch_x );
  ReferencePoint[ 1 ] = TCoordinate( src.ReferencePoint[ 1 ] * stretch_y );
  if ( ReferencePoint[ 0 ] < 0 )
    ReferencePoint[ 0 ] += width();
  if ( ReferencePoint[ 1 ] < 0 )
    ReferencePoint[ 1 ] += height();
}




void wImage::transformFrom( wImage const &src, affine_transformation const &trans )
{
  coordinate_vector origin_shift;
  super::transformFrom( src, trans, &origin_shift );
  double ref_x, ref_y;
  trans.transform( ref_x, ref_y, src.ReferencePoint[ 0 ], src.ReferencePoint[ 1 ] );
  ReferencePoint[ 0 ] = TCoordinate( ref_x ) + origin_shift[ 0 ];
  ReferencePoint[ 1 ] = TCoordinate( ref_y ) + origin_shift[ 1 ];
}




void wImage::blit( drawable &dest, TCoordinate x, TCoordinate y )
{
  super::blit( dest, x - ReferencePoint[ 0 ], y - ReferencePoint[ 1 ] );
}




void wImage::blit( drawable &dest, TCoordinate x, TCoordinate y,
                   coordinate_rectangle const &source )
{
  super::blit( dest, x - ReferencePoint[ 0 ], y - ReferencePoint[ 1 ] );
}

