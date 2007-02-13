// ----------------------------------------------------------------------------
//  Description      : WAM menu background
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_random.hh>
#include "engine/drawable.hh"
#include "engine/tick.hh"
#include "engine/resource.hh"
#include "engine/codemanager.hh"
#include "game_names.hh"
#include "background.hh"




#define GJFEATURESET_MENU_BACKGROUND GJFEATURE_DISPLAYED GJFEATURE_MPACK_WAM "Back"




// wMenuBackground ------------------------------------------------------------
class wMenuBackground : public wDrawable
{
    wImageHolder Sky;

  public:
    wMenuBackground( wGame &g );

    void startUp();

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MENU_BACKGROUND;
    }

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_BACKGROUND_1;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );
};




// wMenuBackground ------------------------------------------------------------
wMenuBackground::wMenuBackground( wGame &g )
    : wGameject( g ), wDrawable( g ),
    Sky( Game.getImageManager(), wImageDescriptor( "sky.png" ) )
{}




void wMenuBackground::startUp()
{
  wDrawable::startUp();

  show();
}




wDisplayExtent wMenuBackground::getDisplayExtent()
{
  return Game.getDrawableManager().getDisplay().extent();
}




void wMenuBackground::draw( drawable &dpy )
{
  dpy.drawingTile( Sky.get() );
  dpy.fillBox( 0, 0, dpy.width(), dpy.height() );
}




// registry -------------------------------------------------------------------
namespace
{
wGameject *createMenuBackground( wGame &g )
{
  return new wMenuBackground( g );
}
}




void registerBackground( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_MENU_BACKGROUND, "", createMenuBackground );
}




