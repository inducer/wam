// ----------------------------------------------------------------------------
//  Description      : WAM menu background
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_textfile.hh>
#include "engine/drawable.hh"
#include "engine/tick.hh"
#include "engine/resource.hh"
#include "engine/codemanager.hh"
#include "game_names.hh"
#include "background.hh"




#define GJFEATURESET_CREDITS GJFEATURE_DISPLAYED GJFEATURE_MPACK_WAM "Cred"




// wCredits -------------------------------------------------------------------
class wCredits : public wDrawable, public wTickReceiver
{
    text_file Credits;
    text_file::iterator TopLine;
    float TopLinePosition;
    wFontHolder NormalFont, BoldFont;
    float Pause;

  public:
    wCredits( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_CREDITS;
    }
    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_FX;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float seconds );
};




// wCredits -------------------------------------------------------------------
wCredits::wCredits( wGame &g )
    : wGameject( g ), wDrawable( g ), wTickReceiver( g ),
    NormalFont( Game.getFontManager(), "credits.fnt" ),
    BoldFont( Game.getFontManager(), "credits_bold.fnt" ), Pause( 0 )
{
  *Game.getFileManager().openFile( WAM_FT_DATA, "credits.txt" ) >> Credits;
  TopLine = Credits.begin();
  TopLinePosition = 0;
}




void wCredits::registerMe()
{
  wDrawable::registerMe();
  wTickReceiver::registerMe();
}




void wCredits::startUp()
{
  wDrawable::startUp();
  wTickReceiver::startUp();

  show();
}




void wCredits::prepareToDie()
{
  wDrawable::prepareToDie();
  wTickReceiver::prepareToDie();
}




void wCredits::unregisterMe()
{
  wDrawable::unregisterMe();
  wTickReceiver::unregisterMe();
}




wDisplayExtent wCredits::getDisplayExtent()
{
  return Game.getDrawableManager().getDisplay().extent();
}




void wCredits::draw( drawable &dpy )
{
  text_file::const_iterator it = TopLine;
  TCoordinate y = ( int ) TopLinePosition;
  while ( y < ( TCoordinate ) dpy.height() )
  {
    font * fnt = NormalFont.get();
    TCoordinate x = 20;

    string str = *it++;
    if ( it == Credits.end() )
      it = Credits.begin();

    regex_string control_codes_re( "^\\[([a-z0-9]+)\\](.*)+" );
    if ( control_codes_re.match( str ) )
    {
      string control_codes = control_codes_re.getBackref( 0 );
      str = control_codes_re.getBackref( 1 );

      FOREACH_CONST( first, control_codes, string )
      {
        if ( *first == 'b' )
          fnt = BoldFont.get();
        else if ( *first == 'c' )
        {
          wDisplayExtent ext = fnt->extent( str );
          x = ( dpy.width() - ext.width() ) / 2;
        }
        else if ( *first == 'r' )
        {
          wDisplayExtent ext = fnt->extent( str );
          x = dpy.width() - 20 - ext.width();
        }
      }
    }

    fnt->print( dpy, x, y, str );

    y += BoldFont->lineHeight();
  }
}




void wCredits::tick( float seconds )
{
  TSize line_height = BoldFont->lineHeight();
  if ( Pause > 0 )
  {
    Pause -= seconds;
    if ( Pause < 0 )
      Pause = 0;
    return ;
  }
  TopLinePosition -= ( 0.8 * seconds * line_height );
  while ( TopLinePosition < - ( int ) line_height )
  {
    TopLine++;
    if ( TopLine == Credits.end() )
      TopLine = Credits.begin();
    TopLinePosition += line_height;

    regex_string control_codes_re( "^\\[([a-z0-9]+)\\](.*)+" );
    if ( control_codes_re.match( *TopLine ) )
    {
      string control_codes = control_codes_re.getBackref( 0 );

      FOREACH_CONST( first, control_codes, string )
      {
        if ( *first == 'p' )
          Pause += 1;
      }
    }
  }
  requestRedraw();
}




// registry -------------------------------------------------------------------
namespace
{
wGameject *createCredits( wGame &g )
{
  return new wCredits( g );
}
}




void registerCredits( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_CREDITS, "", createCredits );
}
