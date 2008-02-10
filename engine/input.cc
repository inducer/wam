// ----------------------------------------------------------------------------
//  Description      : WAM server-side input manager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "utility/manager_impl.hh"
#include "engine/gamebase.hh"
#include "engine/console.hh"
#include "engine/commandmanager.hh"
#include "engine/input.hh"




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( list_keyboard_listeners, "LiKb", "list objects registered to KeyboardInputManager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_keyboard_listeners", 0, 0 )

  auto_ptr<js_array> arr( new js_array() );

  TIndex idx = 0;

  wInputManager::keyboard_const_iterator
  first = Game.getInputManager().keyboard_begin(),
          last = Game.getInputManager().keyboard_end();
  while ( first != last )
  {
    ( *arr ) [ idx++ ] = makeConstant( ( *first ) ->id() );
    first++;
  }

  return arr.release();
}




WAM_DECLARE_COMMAND( list_mouse_listeners, "LiMo", "list objects registered to MouseInputManager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_mouse_listeners", 0, 0 )

  auto_ptr<js_array> arr( new js_array() );

  TIndex idx = 0;

  wInputManager::mouse_const_iterator
  first = Game.getInputManager().mouse_begin(),
          last = Game.getInputManager().mouse_end();
  while ( first != last )
  {
    ( *arr ) [ idx++ ] = makeConstant( ( *first ) ->id() );
    first++;
  }

  return arr.release();
}




void registerInputManagerCommands( wGame &g )
{
  register_list_keyboard_listeners( g );
  register_list_mouse_listeners( g );
}




// wKeyboardEvent -------------------------------------------------------------
wKeyboardEvent::wKeyboardEvent( SDL_KeyboardEvent const &event )
{
  if ( event.type == SDL_KEYDOWN )
    Type = KEY_PRESS;
  else
    Type = KEY_RELEASE;

  Key = event.keysym.sym;
  Modifier = event.keysym.mod;
  Unicode = event.keysym.unicode;
}




bool wKeyboardEvent::isPrintable()
{
  return Unicode != 0 && Unicode >= ' ';
}




TUnsigned16 wKeyboardEvent::getUnicode() const
{
  return Unicode;
}




char wKeyboardEvent::getASCII() const
{
  if ( Unicode & 0xff00 )
    EXGAME_THROWINFO( ECGAME_GENERAL, "Not a UTF-8 character, unable to convert" )
    if ( Unicode == 0 )
      EXGAME_THROWINFO( ECGAME_GENERAL, "Not a printable character" )
      return char( Unicode );
}




// wMouseEvent ----------------------------------------------------------------
wMouseEvent::wMouseEvent( SDL_MouseMotionEvent const &event )
{
  Type = MOTION;
  Position.set( event.x, event.y );
  RelativeMovement.set( event.xrel, event.yrel );
}




wMouseEvent::wMouseEvent( SDL_MouseButtonEvent const &event )
{
  if ( event.type == SDL_MOUSEBUTTONDOWN )
    Type = BUTTON_PRESS;
  else
    Type = BUTTON_RELEASE;
  Position.set( event.x, event.y );
  Button = event.button;
}




// Template instantiations ----------------------------------------------------
template class wRegisteringManager<wKeyboardInputListener>;
template class wRegisteringManager<wMouseInputListener>;
template class wPriorityManager<wKeyboardInputListener>;
template class wPriorityManager<wMouseInputListener>;




// wKeyboardInputListener -----------------------------------------------------
void wKeyboardInputListener::registerMe()
{
  Game.getInputManager().registerObject( this );
}




void wKeyboardInputListener::unregisterMe()
{
  Game.getInputManager().unregisterObject( this );
}




// wMouseInputListener -----------------------------------------------------
void wMouseInputListener::registerMe()
{
  Game.getInputManager().registerObject( this );
}




void wMouseInputListener::unregisterMe()
{
  Game.getInputManager().unregisterObject( this );
}




// wInputManager --------------------------------------------------------------
wInputManager::wInputManager()
{
  SDL_EnableUNICODE( 1 );
}




void wInputManager::registerObject( wKeyboardInputListener *obj )
{
  KeyboardSuper::registerObject( obj );
}




void wInputManager::unregisterObject( wKeyboardInputListener *obj )
{
  KeyboardSuper::unregisterObject( obj );
}




void wInputManager::registerObject( wMouseInputListener *obj )
{
  MouseSuper::registerObject( obj );
}




void wInputManager::unregisterObject( wMouseInputListener *obj )
{
  MouseSuper::unregisterObject( obj );
}




bool wInputManager::run()
{
  SDL_Event event;
  while ( SDL_PollEvent( &event ) )
    switch ( event.type )
    {
    case SDL_KEYDOWN:
      processEvent( event.key );
      break;
    case SDL_KEYUP:
      processEvent( event.key );
      break;
    case SDL_MOUSEMOTION:
      processEvent( event.motion );
      break;
    case SDL_MOUSEBUTTONDOWN:
      processEvent( event.button );
      break;
    case SDL_MOUSEBUTTONUP:
      processEvent( event.button );
      break;
    case SDL_QUIT:
      return false;
    default:
      break;
    }
  return true;
}




void wInputManager::processEvent( wKeyboardEvent const &event )
{
  wKeyboardEvent copy( event );

  keyboard_const_iterator first = keyboard_begin(), last = keyboard_end();
  while ( first != last )
  {
    if ( ( *first ) ->processKeyEvent( copy ) )
      break;
    first++;
  }
}




void wInputManager::processEvent( wMouseEvent const &event )
{
  wMouseEvent copy( event );

  mouse_const_iterator first = mouse_begin(), last = mouse_end();
  while ( first != last )
  {
    if ( ( *first ) ->processMouseEvent( copy ) )
      break;
    first++;
  }
}




