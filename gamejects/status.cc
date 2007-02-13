// ----------------------------------------------------------------------------
//  Description      : Status gamejects
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "engine/names.hh"
#include "engine/gamebase.hh"
#include "engine/instancemanager.hh"
#include "engine/codemanager.hh"
#include "engine/console.hh"
#include "engine/resource.hh"
#include "engine/message.hh"
#include "engine/scrolling.hh"
#include "engine/input.hh"
#include "engine/drawable.hh"
#include "engine/commandmanager.hh"
#include "gamejects/status.hh"
#include "utility/config.hh"




// constants ------------------------------------------------------------------
#define WAM_STANDARD_FONT  "standard.fnt"

#define GJFEATURESET_VERSIONDISPLAY GJFEATURE_DISPLAYED "VerD"
#define GJFEATURESET_FPSDISPLAY  GJFEATURE_DISPLAYED "FpsD"




// wFPSDisplay ----------------------------------------------------------------
class wFPSDisplay : public wDrawable, public wTickReceiver
{
    wFontHolder Font;
    float LastFPS;

  public:
    wFPSDisplay( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_FPSDISPLAY;
    }
    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_STATUS;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float seconds );
};




// wVersionDisplay ------------------------------------------------------------
class wVersionDisplay : public wDrawable
{
    wFontHolder Font;

  public:
    wVersionDisplay( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_VERSIONDISPLAY;
    }
    void startUp();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_STATUS;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );
};




// wFPSDisplay ----------------------------------------------------------------
wFPSDisplay::wFPSDisplay( wGame &g )
    : wGameject( g ), wDrawable( g ), wTickReceiver( g ),
    Font( Game.getFontManager(), WAM_STANDARD_FONT ), LastFPS( 0 )
{}




void wFPSDisplay::registerMe()
{
  wDrawable::registerMe();
  wTickReceiver::registerMe();
}




void wFPSDisplay::startUp()
{
  wDrawable::startUp();
  wTickReceiver::startUp();

  show();
}




void wFPSDisplay::prepareToDie()
{
  wDrawable::prepareToDie();
  wTickReceiver::prepareToDie();
}




void wFPSDisplay::unregisterMe()
{
  wDrawable::unregisterMe();
  wTickReceiver::unregisterMe();
}




wDisplayExtent wFPSDisplay::getDisplayExtent()
{
  wDisplayExtent ext = Font->extent( "XXXXXX.X fps" );
  TSize width = ext.B[ 0 ] - ext.A[ 0 ];
  return wDisplayExtent(
           10,
           Game.getDrawableManager().getDisplay().height() - 10 - Font->lineHeight(),
           10 + width,
           Game.getDrawableManager().getDisplay().height() - 10 );
}




void wFPSDisplay::draw( drawable &dpy )
{
  string fps = float2dec( LastFPS, 1 ) + " fps";
  Font->print( dpy, 10, dpy.height() - 10, fps, font::ALIGN_BOTTOM );
}




void wFPSDisplay::tick( float seconds )
{
  requestRedraw();
  LastFPS = 1 / seconds;
}




// wVersionDisplay ------------------------------------------------------------
wVersionDisplay::wVersionDisplay( wGame &g )
    : wGameject( g ), wDrawable( g ), Font( Game.getFontManager(), WAM_STANDARD_FONT )
{}




void wVersionDisplay::startUp()
{
  wDrawable::startUp();

  show();
}




wDisplayExtent wVersionDisplay::getDisplayExtent()
{
  wDisplayExtent ext = Font->extent( "Version XX.XXX" );
  return wDisplayExtent(
           Game.getDrawableManager().getDisplay().width() - 10 - ext.width(),
           Game.getDrawableManager().getDisplay().height() - 10 - Font->lineHeight(),
           Game.getDrawableManager().getDisplay().width() - 10,
           Game.getDrawableManager().getDisplay().height() - 10 );
}




void wVersionDisplay::draw( drawable &dpy )
{
  string ver = string( "Version " ) + WAM_VERSION;
  wDisplayExtent ext = Font->extent( ver );
  Font->print( dpy, dpy.width() - 10 - ext.width(), dpy.height() - 10, ver, font::ALIGN_BOTTOM );
}




// registration ---------------------------------------------------------------
namespace
{
wGameject *createFPSDisplay( wGame &g )
{
  return new wFPSDisplay( g );
}

wGameject *createVersionDisplay( wGame &g )
{
  return new wVersionDisplay( g );
}
}




void registerStatus( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_FPSDISPLAY, "", createFPSDisplay );
  g.getGameCodeManager().registerClass( GJFEATURESET_VERSIONDISPLAY, "", createVersionDisplay );
}
