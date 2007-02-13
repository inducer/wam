// ----------------------------------------------------------------------------
//  Description      : Miscellaneous game objects
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "engine/names.hh"
#include "engine/gamebase.hh"
#include "engine/instancemanager.hh"
#include "engine/codemanager.hh"
#include "engine/console.hh"
#include "engine/resource.hh"
#include "engine/scrolling.hh"
#include "engine/drawable.hh"
#include "engine/commandmanager.hh"
#include "gamejects/parallax.hh"




// constants ------------------------------------------------------------------
#define GJFEATURESET_PARALLAX  GJFEATURE_DISPLAYED "Prlx"




// wParallaxBackground --------------------------------------------------------
class wParallaxBackground : public wDrawable
{
    wImageHolder Image;

  public:
    wParallaxBackground( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_PARALLAX;
    }

    void startUp();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_BACKGROUND_1;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void setImage( wImageDescriptor const &desc );
};




// wParallaxBackground --------------------------------------------------------
wParallaxBackground::wParallaxBackground( wGame &g )
    : wGameject( g ), wDrawable( g ), Image( Game.getImageManager() )
{}




void wParallaxBackground::startUp()
{
  show();
}



void wParallaxBackground::draw( drawable &dpy )
{
  if ( Image.get() == NULL )
    return ;
  wGameVector scroll_position = Game.getScrollManager().getPosition();
  wGameVector max_scroll_position = Game.getScrollManager().getMaxPosition();

  double scale_x, scale_y;

  if ( dpy.width() < Image->width() )
  {
    TSize image_excess = Image->width() - dpy.width();

    if ( max_scroll_position[ 0 ] == 0 )
      scale_x = 0;
    else
      scale_x = double( image_excess ) / max_scroll_position[ 0 ];
  }
  else
    scale_x = 0.5;

  if ( dpy.height() < Image->height() )
  {
    TSize image_excess = Image->height() - dpy.height();

    if ( max_scroll_position[ 1 ] == 0 )
      scale_y = 0;
    else
      scale_y = double( image_excess ) / max_scroll_position[ 1 ];
  }
  else
    scale_y = 0.5;

  dpy.drawingTile( Image.get(),
                   int( scroll_position[ 0 ] * scale_x ),
                   int( scroll_position[ 1 ] * scale_y ) );
  wDisplayExtent const &clip = dpy.clipping();
  dpy.fillBox( clip.A[ 0 ], clip.A[ 1 ], clip.B[ 0 ], clip.B[ 1 ] );
}




wDisplayExtent wParallaxBackground::wParallaxBackground::getDisplayExtent()
{
  return Game.getDrawableManager().getDisplay().extent();
}




void wParallaxBackground::setImage( wImageDescriptor const &desc )
{
  Image.set( desc );
}




// console interface ----------------------------------------------------------
#define FIND_PARALLAX \
  wParallaxBackground *parallax= dynamic_cast<wParallaxBackground *>( \
    Game.getInstanceManager().getObjectByFeatureSet(GJFEATURESET_PARALLAX));




WAM_DECLARE_COMMAND( prlx_setimage, "PxIm", "set parallax image", "file" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "prlx_setimage", 1, 1 )
  FIND_PARALLAX
  parallax->setImage( parameters[ 0 ] ->toString() );
  return makeNull();
}




// registration ---------------------------------------------------------------
namespace
{
wGameject *createParallaxBackground( wGame &g )
{
  return new wParallaxBackground( g );
}
}




void registerParallax( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_PARALLAX, "", createParallaxBackground );

  register_prlx_setimage( g );
}
