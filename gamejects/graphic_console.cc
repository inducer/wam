// ----------------------------------------------------------------------------
//  Description      : Graphic console
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
#include "engine/input.hh"
#include "engine/drawable.hh"
#include "gamejects/graphic_console.hh"




// constants ------------------------------------------------------------------
#define WAM_CONSOLEBACKGROUND  "console_background.png"
#define WAM_STANDARD_FONT  "standard.fnt"

#define GJFEATURESET_CONSOLEDISPLAY GJFEATURE_DISPLAYED "Cons"




// wGraphicConsoleDisplay -----------------------------------------------------
class wGraphicConsoleDisplay : public wDrawable, public wTickReceiver, public wConsoleDisplay,
      public wMessageReceiver
{
    wFontHolder Font;
    wImageHolder Background;

    typedef vector<string> wContents;
    wContents Contents;
    bool Enabled;
    TSize OmitLines;
    float LinearYPosition;

    wConsoleDisplay *PreviousDisplay;

    static TSize const SpaceToConsoleBottom = 10;

    static TMessageId ScrollUpMessage, ScrollDownMessage;

  public:
    wGraphicConsoleDisplay( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_CONSOLEDISPLAY;
    }

    void registerMe();
    void unregisterMe();
    void startUp();
    void prepareToDie();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_CONSOLE;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float seconds );

    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );

    float getYPosition() const;

    void redrawLastLine();

    void clearLine();
    void output( string const &txt );
    void newLine();
    void flush()
    {}

    void enable();
    void disable();
};




// wGraphicConsoleDisplay -----------------------------------------------------
TMessageId
wGraphicConsoleDisplay::ScrollUpMessage,
wGraphicConsoleDisplay::ScrollDownMessage;




wGraphicConsoleDisplay::wGraphicConsoleDisplay( wGame &g )
    : wGameject( g ), wDrawable( g ), wTickReceiver( g ), wMessageReceiver( g ),
    Font( g.getFontManager(), WAM_STANDARD_FONT ),
    Background( g.getImageManager(), string( WAM_CONSOLEBACKGROUND ) ),
    Enabled( g.getConsole().isConsoleEnabled() ),
    OmitLines( 0 ),
    LinearYPosition( 0 )
{
  Background.set( wImageDescriptor( WAM_CONSOLEBACKGROUND,
                                    float( Game.getDrawableManager().getDisplay().width() ) / Background->width(),
                                    float( Game.getDrawableManager().getDisplay().height() / 2 ) / Background->height() ) );
}




void wGraphicConsoleDisplay::registerMe()
{
  wDrawable::registerMe();
  wTickReceiver::registerMe();
  wMessageReceiver::registerMe();
}




void wGraphicConsoleDisplay::unregisterMe()
{
  wTickReceiver::unregisterMe();
  wDrawable::unregisterMe();
  wMessageReceiver::unregisterMe();
}




void wGraphicConsoleDisplay::startUp()
{
  wDrawable::startUp();
  wTickReceiver::startUp();
  wMessageReceiver::startUp();

  ScrollUpMessage = Game.getMessageManager().registerMessage( WAM_MSG_SCROLL_UP_CONSOLE );
  ScrollDownMessage = Game.getMessageManager().registerMessage( WAM_MSG_SCROLL_DOWN_CONSOLE );

  if ( Enabled )
    show();

  PreviousDisplay = Game.getConsole().getDisplay();
  Game.getConsole().setDisplay( this );
}




void wGraphicConsoleDisplay::prepareToDie()
{
  wDrawable::prepareToDie();
  wTickReceiver::prepareToDie();
  wMessageReceiver::prepareToDie();

  if ( Enabled || LinearYPosition != 0 )
    hide();

  Game.getConsole().setDisplay( PreviousDisplay );
}




wDisplayExtent
wGraphicConsoleDisplay::getDisplayExtent()
{
  return wDisplayExtent( 0, 0, Game.getDrawableManager().getDisplay().width(), int( getYPosition() + 1 ) );
}




void wGraphicConsoleDisplay::draw( drawable &dpy )
{
  float y_position = getYPosition();
  Background->blit( dpy, 0, int( y_position - dpy.height() / 2 ) );

  TCoordinate y = TCoordinate( y_position - SpaceToConsoleBottom );

  if ( OmitLines )
  {
    Font->print( dpy, 10, y, "--- " + unsigned2dec( OmitLines ) + " more lines ---", font::ALIGN_BOTTOM );
    y -= Font->lineHeight();
  }

  TIndex i;
  if ( Contents.size() >= OmitLines )
    i = Contents.size() - OmitLines;
  else
    return ;

  bool quitflag = false;
  while ( !quitflag && i != 0 )
  {
    Font->print( dpy, 10, y, Contents[ --i ], font::ALIGN_BOTTOM );

    if ( y <= 0 )
      quitflag = true;

    y -= Font->lineHeight();
  }
}




void wGraphicConsoleDisplay::tick( float seconds )
{
  float const swing_duration = 0.5; // seconds
  bool redraw = false;
  float new_y_position = LinearYPosition;

  if ( Enabled )
  {
    if ( new_y_position < 1 )
    {
      new_y_position += seconds / swing_duration;
      if ( new_y_position > 1 )
        new_y_position = 1;
      redraw = true;
    }
  }
  else
  {
    if ( new_y_position > 0 )
    {
      new_y_position -= seconds / swing_duration;
      if ( new_y_position <= 0 )
      {
        hide();
        LinearYPosition = 0;
      }
      else
        redraw = true;
    }
  }

  if ( redraw )
  {
    requestRedraw();
    LinearYPosition = new_y_position;
    requestRedraw();
  }
}




ref<wGraphicConsoleDisplay::wMessageReturn>
wGraphicConsoleDisplay::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  TSize const scroll_by = 10;

  if ( id == ScrollUpMessage )
  {
    OmitLines += scroll_by;
    requestRedraw();
  }
  if ( id == ScrollDownMessage )
  {
    if ( OmitLines > scroll_by )
      OmitLines -= scroll_by;
    else
      OmitLines = 0;
    requestRedraw();
  }
  return ref<wGraphicConsoleDisplay::wMessageReturn>( NULL );
}




float wGraphicConsoleDisplay::getYPosition() const
{
  float screen_height = Game.getDrawableManager().getDisplay().height();
  float ypos = 1 - ( 1 - LinearYPosition ) * ( 1 - LinearYPosition );
  return ypos * screen_height / 2;
}




void wGraphicConsoleDisplay::redrawLastLine()
{
  float y_position = getYPosition();

  Game.getDrawableManager().requestRedraw(
    wDisplayExtent ( 0, TCoordinate( y_position - SpaceToConsoleBottom - Font->lineHeight() ),
                     Game.getDrawableManager().getDisplay().width(), TCoordinate( y_position - SpaceToConsoleBottom ) )
  );
}




void wGraphicConsoleDisplay::clearLine()
{
  if ( Contents.size() != 0 )
  {
    Contents.back() = "";
    redrawLastLine();
  }
}




void wGraphicConsoleDisplay::output( string const &txt )
{
  string::size_type start_index = 0,index = txt.find( "\n" );

  while ( index != string::npos )
  { 
    string chopped_text = txt.substr( start_index, index - start_index );
    if ( Contents.size() == 0 )
      Contents.push_back( chopped_text );
    else
      Contents.back() += chopped_text;
    newLine();
    start_index = index + 1;
    index = txt.find( "\n", start_index );
  }
  string chopped_text = txt.substr( start_index, index - start_index );
  if ( Contents.size() == 0 )
    Contents.push_back( chopped_text );
  else
    Contents.back() += chopped_text;
  redrawLastLine();
}




void wGraphicConsoleDisplay::newLine()
{
  Contents.push_back( "" );
  if ( OmitLines )
    OmitLines++;
  requestRedraw();
}




void wGraphicConsoleDisplay::enable()
{
  Enabled = true;
  if ( LinearYPosition == 0 )
    show();
}




void wGraphicConsoleDisplay::disable()
{
  Enabled = false;
}




// registration ---------------------------------------------------------------
namespace
{
wGameject *createGraphicConsoleDisplay( wGame &g )
{
  return new wGraphicConsoleDisplay( g );
}
}




void registerGraphicConsole( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_CONSOLEDISPLAY, "", createGraphicConsoleDisplay );
}
