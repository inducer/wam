// ----------------------------------------------------------------------------
//  Description      : WAM mouse pointer
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "engine/instancemanager.hh"
#include "engine/codemanager.hh"
#include "engine/resource.hh"
#include "engine/input.hh"
#include "engine/drawable.hh"
#include "gamejects/mouse_pointer.hh"




#define GJFEATURESET_MOUSEPOINTER GJFEATURE_DISPLAYED "MPtr"




// wMousePointer -------------------------------------------------------------
class wMousePointer : public wDrawable, public wMouseInputListener
{
    wImageHolder Image;
    wMouseEvent::wMouseVector Position;
    bool First;
    SDL_Cursor *PrevCursor;
    TByte CursorData;

  public:
    wMousePointer( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MOUSEPOINTER;
    }
    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_ABOVEALL;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    TPriority getMouseInputPriority() const
    {
      return WAM_INPUTPRIO_POINTER;
    }
    bool processMouseEvent( wMouseEvent &event );
};




// wMousePointer -------------------------------------------------------------
wMousePointer::wMousePointer( wGame &g )
    : wGameject( g ), wDrawable( g ), wMouseInputListener( g ),
    Image( Game.getImageManager(), wImageDescriptor( "mouse.png" ) ),
    Position( 0, 0 ), First( true )
{}




void wMousePointer::registerMe()
{
  wDrawable::registerMe();
  wMouseInputListener::registerMe();
}




void wMousePointer::startUp()
{
  PrevCursor = SDL_GetCursor();
  CursorData = 0;
  SDL_Cursor *mycursor = SDL_CreateCursor( &CursorData, &CursorData, 8, 1, 0, 0 );
  SDL_SetCursor( mycursor );
}




void wMousePointer::prepareToDie()
{
  SDL_Cursor * mycursor = SDL_GetCursor();
  SDL_SetCursor( PrevCursor );
  SDL_FreeCursor( mycursor );
}




void wMousePointer::unregisterMe()
{
  wDrawable::unregisterMe();
  wMouseInputListener::unregisterMe();
}




void wMousePointer::draw( drawable &dpy )
{
  Image->blit( dpy, Position[ 0 ], Position[ 1 ] );
}




wDisplayExtent wMousePointer::getDisplayExtent()
{
  return Image->extent() + Position;
}




bool wMousePointer::processMouseEvent( wMouseEvent &event )
{
  if ( First )
  {
    show();
    First = false;
  }
  requestRedraw();
  Position = event.getPosition();
  requestRedraw();
  return false;
}



// registration ---------------------------------------------------------------
namespace
{
wGameject *createMousePointer( wGame &g )
{
  return new wMousePointer( g );
}
}




void registerMousePointer( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_MOUSEPOINTER, "", createMousePointer );
}
