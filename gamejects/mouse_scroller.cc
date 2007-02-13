// ----------------------------------------------------------------------------
//  Description      : WAM mouse scroller
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <sdlucid_video.hh>
#include "engine/gamebase.hh"
#include "engine/instancemanager.hh"
#include "engine/codemanager.hh"
#include "engine/console.hh"
#include "engine/drawable.hh"
#include "engine/scrolling.hh"
#include "engine/input.hh"
#include "engine/commandmanager.hh"
#include "gamejects/mouse_scroller.hh"




#define GJFEATURESET_MOUSESCROLLER "MScl"




// wMouseScroller -------------------------------------------------------------
class wMouseScroller : public wMouseInputListener
{
    bool GrabMouse;

  public:
    wMouseScroller( wGame &g )
        : wGameject( g ), wMouseInputListener( g ), GrabMouse( false )
    {}
    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MOUSESCROLLER;
    }
    TPriority getMouseInputPriority() const
    {
      return WAM_INPUTPRIO_SCROLLER;
    }
    bool processMouseEvent( wMouseEvent &event );

    bool getGrabMouse() const
    {
      return GrabMouse;
    }
    void setGrabMouse( bool grabmouse );
};




// wMouseScroller ------------------------------------------------------------
#define WAM_MOUSESCROLLER_SPEEDFACTOR 5

bool wMouseScroller::processMouseEvent( wMouseEvent &event )
{
  if ( event.getType() != wMouseEvent::MOTION )
  {
    return false;
  }

  TSize screen_width = Game.getDrawableManager().getDisplay().width();
  TSize screen_height = Game.getDrawableManager().getDisplay().height();

  wMouseEvent::wMouseVector mouse_pos = event.getPosition();

  if ( GrabMouse )
  {
    wGameVector scroll_pos = Game.getScrollManager().getTarget();

    int const scroll_rim = 16;
    int const amplification = 4;

    bool x_changed = false;
    bool y_changed = false;

    if ( mouse_pos[ 0 ] < scroll_rim )
    {
      scroll_pos[ 0 ] -= amplification * ( scroll_rim - mouse_pos[ 0 ] );
      mouse_pos[ 0 ] = scroll_rim;
      x_changed = true;
    }

    if ( mouse_pos[ 1 ] < scroll_rim )
    {
      scroll_pos[ 1 ] -= amplification * ( scroll_rim - mouse_pos[ 1 ] );
      mouse_pos[ 1 ] = scroll_rim;
      y_changed = true;
    }

    if ( mouse_pos[ 0 ] > int( screen_width - scroll_rim ) )
    {
      scroll_pos[ 0 ] += amplification * ( mouse_pos[ 0 ] - ( screen_width - scroll_rim ) );
      mouse_pos[ 0 ] = screen_width - scroll_rim;
      x_changed = true;
    }

    if ( mouse_pos[ 1 ] > int( screen_height - scroll_rim ) )
    {
      scroll_pos[ 1 ] += amplification * ( mouse_pos[ 1 ] - ( screen_height - scroll_rim ) );
      mouse_pos[ 1 ] = screen_height - scroll_rim;
      y_changed = true;
    }

    if ( x_changed || y_changed )
    {
      int new_mouse_x, new_mouse_y;
      SDL_GetMouseState( &new_mouse_x, &new_mouse_y );
      if ( x_changed )
        new_mouse_x = int( mouse_pos[ 0 ] );
      if ( y_changed )
        new_mouse_y = int( mouse_pos[ 1 ] );
      SDL_WarpMouse( new_mouse_x, new_mouse_y );
      Game.getScrollManager().scrollTo( scroll_pos, SCROLL_ALWAYS );

      // consume this event, because a new one will be generated through
      // SDL_WarpMouse()
      return true;
    }
  }
  else
  {
    coord_vector<double, 2> precise_mouse_pos = mouse_pos;
    double scroll_width = Game.getScrollManager().getWorldWidth();
    double scroll_height = Game.getScrollManager().getWorldHeight();

    precise_mouse_pos[ 0 ] /= screen_width;
    precise_mouse_pos[ 1 ] /= screen_height;

    precise_mouse_pos[ 0 ] = precise_mouse_pos[ 0 ] * 1.2 - 0.1;
    precise_mouse_pos[ 1 ] = precise_mouse_pos[ 1 ] * 1.2 - 0.1;

    if ( precise_mouse_pos[ 0 ] < 0 )
      precise_mouse_pos[ 0 ] = 0;
    if ( precise_mouse_pos[ 1 ] < 0 )
      precise_mouse_pos[ 1 ] = 0;
    if ( precise_mouse_pos[ 0 ] > 1 )
      precise_mouse_pos[ 0 ] = 1;
    if ( precise_mouse_pos[ 1 ] > 1 )
      precise_mouse_pos[ 1 ] = 1;

    coord_vector<double, 2> newpos( precise_mouse_pos[ 0 ] * ( scroll_width - screen_width ),
                                    precise_mouse_pos[ 1 ] * ( scroll_height - screen_height ) );
    Game.getScrollManager().scrollTo( round( newpos ), SCROLL_ALWAYS );
  }

  return false;
}




void wMouseScroller::setGrabMouse( bool grabmouse )
{
  if ( grabmouse != GrabMouse )
  {
    if ( grabmouse )
      SDL_WM_GrabInput( SDL_GRAB_ON );
    else
      SDL_WM_GrabInput( SDL_GRAB_OFF );
    GrabMouse = grabmouse;
  }
}




// console interface ----------------------------------------------------------
#define FIND_MOUSE_SCROLLER \
  wMouseScroller *mouse_scroller= dynamic_cast<wMouseScroller *>( \
    Game.getInstanceManager().getObjectByFeatureSet(GJFEATURESET_MOUSESCROLLER));




WAM_DECLARE_COMMAND( grabmouse, "GrMo", "whether we should limit the mouse pointer to our window", "[on|off]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "grabmouse", 0, 1 )
  FIND_MOUSE_SCROLLER

  if ( parameters.size() == 1 )
  {
    mouse_scroller->setGrabMouse( parameters[ 0 ] ->toBoolean() );
    return makeNull();
  }
  else
    return makeConstant( mouse_scroller->getGrabMouse() );
}




// registration ---------------------------------------------------------------
namespace
{
wGameject *createMouseScroller( wGame &g )
{
  return new wMouseScroller( g );
}
}




void registerMouseScroller( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_MOUSESCROLLER, "", createMouseScroller );

  register_grabmouse( g );
}
