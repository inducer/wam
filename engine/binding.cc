// ----------------------------------------------------------------------------
//  Description      : Binding manager
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include <SDL.h>
#include <ixlib_re.hh>
#include "engine/gamebase.hh"
#include "engine/codemanager.hh"
#include "engine/binding.hh"




// console interface ----------------------------------------------------------
#define FIND_BIND \
  wBindingManager *bm = dynamic_cast<wBindingManager *>( \
    Game.getInstanceManager().getObjectByFeatureSet(GJFEATURESET_BINDINGMGR));
// throws exception if not found




WAM_DECLARE_COMMAND( list_bindings, "LBnd", "list bound keys", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_bindings", 0, 0 )

  FIND_BIND

  auto_ptr<js_array> arr( new js_array() );

  TIndex idx = 0;
  FOREACH_CONST( first, *bm, wBindingManager )
  ( *arr ) [ idx++ ] = makeConstant( bm->keys2text( first->Keys ) );

  return arr.release();
}




WAM_DECLARE_COMMAND( bind, "Bind", "bind command to keys", "keylist onpress [onrelease]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "bind", 2, 3 )

  FIND_BIND

  string keys = parameters[ 0 ] ->toString();
  if ( parameters.size() == 3 )
    bm->bind( bm->text2keys( keys ), parameters[ 1 ], parameters[ 2 ] );
  else
    bm->bind( bm->text2keys( keys ), parameters[ 1 ] );
  return makeNull();
}




WAM_DECLARE_COMMAND( unbind, "UBnd", "unbind keys", "keylist" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "unbind", 1, 1 )

  FIND_BIND

  string keys = parameters[ 0 ] ->toString();

  bm->unbind( bm->text2keys( keys ) );
  return makeNull();
}




WAM_DECLARE_COMMAND( bind_clear, "UBnd", "clear all bindings", "keylist" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "bind_clear", 0, 0 )

  FIND_BIND

  bm->clear();
  return makeNull();
}




// registration ---------------------------------------------------------------
static wGameject *createBindingManager( wGame &g )
{
  return new wBindingManager( g );
}




void registerBindingManager( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_BINDINGMGR, "", createBindingManager );
  register_list_bindings( g );
  register_bind( g );
  register_unbind( g );
  register_bind_clear( g );
}




// service functions ----------------------------------------------------------
bool operator<( wKey const &key1, wKey const &key2 )
{
  if ( key1.Mouse != key2.Mouse )
    return key1.Mouse;
  if ( key1.Mouse )
    return key1.Button < key2.Button;
  else
    return key1.Key < key2.Key;
}




bool operator==( wKey const &key1, wKey const &key2 )
{
  bool result = key1.Mouse == key2.Mouse;
  if ( result )
  {
    if ( key1.Mouse )
      result = key1.Button == key2.Button;
    else
      result = key1.Key == key2.Key;
  }
  return result;
}




bool operator!=( wKey const &key1, wKey const &key2 )
{
  return !( key1 == key2 );
}




bool operator<=( wKeyList const &keys1, wKeyList const &keys2 )
{
  wKeyList::const_iterator first1 = keys1.begin(), last1 = keys1.end();
  wKeyList::const_iterator first2 = keys2.begin(), last2 = keys2.end();

  while ( first1 != last1 )
  {
    while ( *first2 < *first1 && first2 != last2 )
      first2++;
    if ( first2 == last2 )
      return false;
    if ( *first1 != *first2 )
      return false;
    first1++;
    first2++;
  }
  return true;
}




bool operator==( wKeyList const &keys1, wKeyList const &keys2 )
{
  return keys1 <= keys2 && keys2 <= keys1;
}




// wBindingManager -----------------------------------------------------------
wBindingManager::wBindingManager( wGame &g )
    : wKeyboardInputListener( g ), wMouseInputListener( g ), wGameject( g )
{}




bool wBindingManager::processKeyEvent( wKeyboardEvent &event )
{
  if ( event.getType() == wKeyboardEvent::KEY_PRESS )
  {
    wKey key;
    key.Mouse = false;
    key.Key = event.getKey();
    ActiveKeys.insert( key );
  }
  if ( event.getType() == wKeyboardEvent::KEY_RELEASE )
  {
    wKey key;
    key.Mouse = false;
    key.Key = event.getKey();
    ActiveKeys.erase( key );
  }
  update();
  return false;
}




bool wBindingManager::processMouseEvent( wMouseEvent &event )
{
  if ( event.getType() == wMouseEvent::BUTTON_PRESS )
  {
    wKey key;
    key.Mouse = true;
    key.Button = event.getButton();
    ActiveKeys.insert( key );
  }
  if ( event.getType() == wMouseEvent::BUTTON_RELEASE )
  {
    wKey key;
    key.Mouse = true;
    key.Button = event.getButton();
    ActiveKeys.erase( key );
  }
  update();
  return false;
}




void wBindingManager::registerMe()
{
  wKeyboardInputListener::registerMe();
  wMouseInputListener::registerMe();
}




void wBindingManager::unregisterMe()
{
  wMouseInputListener::unregisterMe();
  wKeyboardInputListener::unregisterMe();
}




void wBindingManager::bind( wKeyList const &keys, ref<value> onpress, ref<value> onrelease )
{
  internalUnbind( keys );
  wKeyBinding binding;
  binding.Keys = keys;
  binding.OnPress = onpress;
  binding.OnRelease = onrelease;
  binding.Active = ActiveKeys <= keys;
  Bindings.push_back( binding );
  BindingsChanged = true;
}





void wBindingManager::unbind( wKeyList const &keys )
{
  if ( !internalUnbind( keys ) )
    EXGAME_THROWINFO( ECGAME_NOTFOUND, keys2text( keys ).c_str() )
  }




void wBindingManager::clear()
{
  FOREACH_CONST( first, Bindings, wBindingList )
  if ( first->Active )
  {
    if ( first->OnRelease.get() )
      first->OnRelease->call( value::parameter_list() );
  }
  Bindings.clear();
  BindingsChanged = true;
}




wBindingManager::wKeyBinding const &
wBindingManager::getBinding( wKeyList const &keys ) const
{
  FOREACH_CONST( first, Bindings, wBindingList )
  if ( first->Keys == keys )
    return * first;
  EXGAME_THROWINFO( ECGAME_NOTFOUND, keys2text( keys ).c_str() )
}




wKey wBindingManager::text2key( string const &str )
{
  wKey result;
  result.Mouse = false;
  bool found = false;
  if ( str == "backspace" )
  {
    found = true;
    result.Key = SDLK_BACKSPACE;
  }
  if ( str == "tab" )
  {
    found = true;
    result.Key = SDLK_TAB;
  }
  if ( str == "clear" )
  {
    found = true;
    result.Key = SDLK_CLEAR;
  }
  if ( str == "return" )
  {
    found = true;
    result.Key = SDLK_RETURN;
  }
  if ( str == "pause" )
  {
    found = true;
    result.Key = SDLK_PAUSE;
  }
  if ( str == "escape" )
  {
    found = true;
    result.Key = SDLK_ESCAPE;
  }
  if ( str == "space" )
  {
    found = true;
    result.Key = SDLK_SPACE;
  }
  if ( str == "!" )
  {
    found = true;
    result.Key = SDLK_EXCLAIM;
  }
  if ( str == "\"" )
  {
    found = true;
    result.Key = SDLK_QUOTEDBL;
  }
  if ( str == "#" )
  {
    found = true;
    result.Key = SDLK_HASH;
  }
  if ( str == "$" )
  {
    found = true;
    result.Key = SDLK_DOLLAR;
  }
  if ( str == "&" )
  {
    found = true;
    result.Key = SDLK_AMPERSAND;
  }
  if ( str == "'" )
  {
    found = true;
    result.Key = SDLK_QUOTE;
  }
  if ( str == "(" )
  {
    found = true;
    result.Key = SDLK_LEFTPAREN;
  }
  if ( str == ")" )
  {
    found = true;
    result.Key = SDLK_RIGHTPAREN;
  }
  if ( str == "*" )
  {
    found = true;
    result.Key = SDLK_ASTERISK;
  }
  if ( str == "+" )
  {
    found = true;
    result.Key = SDLK_PLUS;
  }
  if ( str == "," )
  {
    found = true;
    result.Key = SDLK_COMMA;
  }
  if ( str == "-" )
  {
    found = true;
    result.Key = SDLK_MINUS;
  }
  if ( str == "." )
  {
    found = true;
    result.Key = SDLK_PERIOD;
  }
  if ( str == "/" )
  {
    found = true;
    result.Key = SDLK_SLASH;
  }
  if ( str == "0" )
  {
    found = true;
    result.Key = SDLK_0;
  }
  if ( str == "1" )
  {
    found = true;
    result.Key = SDLK_1;
  }
  if ( str == "2" )
  {
    found = true;
    result.Key = SDLK_2;
  }
  if ( str == "3" )
  {
    found = true;
    result.Key = SDLK_3;
  }
  if ( str == "4" )
  {
    found = true;
    result.Key = SDLK_4;
  }
  if ( str == "5" )
  {
    found = true;
    result.Key = SDLK_5;
  }
  if ( str == "6" )
  {
    found = true;
    result.Key = SDLK_6;
  }
  if ( str == "7" )
  {
    found = true;
    result.Key = SDLK_7;
  }
  if ( str == "8" )
  {
    found = true;
    result.Key = SDLK_8;
  }
  if ( str == "9" )
  {
    found = true;
    result.Key = SDLK_9;
  }
  if ( str == ":" )
  {
    found = true;
    result.Key = SDLK_COLON;
  }
  if ( str == ";" )
  {
    found = true;
    result.Key = SDLK_SEMICOLON;
  }
  if ( str == "<" )
  {
    found = true;
    result.Key = SDLK_LESS;
  }
  if ( str == "=" )
  {
    found = true;
    result.Key = SDLK_EQUALS;
  }
  if ( str == ">" )
  {
    found = true;
    result.Key = SDLK_GREATER;
  }
  if ( str == "?" )
  {
    found = true;
    result.Key = SDLK_QUESTION;
  }
  if ( str == "@" )
  {
    found = true;
    result.Key = SDLK_AT;
  }
  if ( str == "[" )
  {
    found = true;
    result.Key = SDLK_LEFTBRACKET;
  }
  if ( str == "\\" )
  {
    found = true;
    result.Key = SDLK_BACKSLASH;
  }
  if ( str == "]" )
  {
    found = true;
    result.Key = SDLK_RIGHTBRACKET;
  }
  if ( str == "^" )
  {
    found = true;
    result.Key = SDLK_CARET;
  }
  if ( str == "_" )
  {
    found = true;
    result.Key = SDLK_UNDERSCORE;
  }
  if ( str == "`" )
  {
    found = true;
    result.Key = SDLK_BACKQUOTE;
  }
  if ( str == "a" )
  {
    found = true;
    result.Key = SDLK_a;
  }
  if ( str == "b" )
  {
    found = true;
    result.Key = SDLK_b;
  }
  if ( str == "c" )
  {
    found = true;
    result.Key = SDLK_c;
  }
  if ( str == "d" )
  {
    found = true;
    result.Key = SDLK_d;
  }
  if ( str == "e" )
  {
    found = true;
    result.Key = SDLK_e;
  }
  if ( str == "f" )
  {
    found = true;
    result.Key = SDLK_f;
  }
  if ( str == "g" )
  {
    found = true;
    result.Key = SDLK_g;
  }
  if ( str == "h" )
  {
    found = true;
    result.Key = SDLK_h;
  }
  if ( str == "i" )
  {
    found = true;
    result.Key = SDLK_i;
  }
  if ( str == "j" )
  {
    found = true;
    result.Key = SDLK_j;
  }
  if ( str == "k" )
  {
    found = true;
    result.Key = SDLK_k;
  }
  if ( str == "l" )
  {
    found = true;
    result.Key = SDLK_l;
  }
  if ( str == "m" )
  {
    found = true;
    result.Key = SDLK_m;
  }
  if ( str == "n" )
  {
    found = true;
    result.Key = SDLK_n;
  }
  if ( str == "o" )
  {
    found = true;
    result.Key = SDLK_o;
  }
  if ( str == "p" )
  {
    found = true;
    result.Key = SDLK_p;
  }
  if ( str == "q" )
  {
    found = true;
    result.Key = SDLK_q;
  }
  if ( str == "r" )
  {
    found = true;
    result.Key = SDLK_r;
  }
  if ( str == "s" )
  {
    found = true;
    result.Key = SDLK_s;
  }
  if ( str == "t" )
  {
    found = true;
    result.Key = SDLK_t;
  }
  if ( str == "u" )
  {
    found = true;
    result.Key = SDLK_u;
  }
  if ( str == "v" )
  {
    found = true;
    result.Key = SDLK_v;
  }
  if ( str == "w" )
  {
    found = true;
    result.Key = SDLK_w;
  }
  if ( str == "x" )
  {
    found = true;
    result.Key = SDLK_x;
  }
  if ( str == "y" )
  {
    found = true;
    result.Key = SDLK_y;
  }
  if ( str == "z" )
  {
    found = true;
    result.Key = SDLK_z;
  }
  if ( str == "delete" )
  {
    found = true;
    result.Key = SDLK_DELETE;
  }
  if ( str == "kp0" )
  {
    found = true;
    result.Key = SDLK_KP0;
  }
  if ( str == "kp1" )
  {
    found = true;
    result.Key = SDLK_KP1;
  }
  if ( str == "kp2" )
  {
    found = true;
    result.Key = SDLK_KP2;
  }
  if ( str == "kp3" )
  {
    found = true;
    result.Key = SDLK_KP3;
  }
  if ( str == "kp4" )
  {
    found = true;
    result.Key = SDLK_KP4;
  }
  if ( str == "kp5" )
  {
    found = true;
    result.Key = SDLK_KP5;
  }
  if ( str == "kp6" )
  {
    found = true;
    result.Key = SDLK_KP6;
  }
  if ( str == "kp7" )
  {
    found = true;
    result.Key = SDLK_KP7;
  }
  if ( str == "kp8" )
  {
    found = true;
    result.Key = SDLK_KP8;
  }
  if ( str == "kp9" )
  {
    found = true;
    result.Key = SDLK_KP9;
  }
  if ( str == "kp." )
  {
    found = true;
    result.Key = SDLK_KP_PERIOD;
  }
  if ( str == "kp/" )
  {
    found = true;
    result.Key = SDLK_KP_DIVIDE;
  }
  if ( str == "kp*" )
  {
    found = true;
    result.Key = SDLK_KP_MULTIPLY;
  }
  if ( str == "kp-" )
  {
    found = true;
    result.Key = SDLK_KP_MINUS;
  }
  if ( str == "kp+" )
  {
    found = true;
    result.Key = SDLK_KP_PLUS;
  }
  if ( str == "up" )
  {
    found = true;
    result.Key = SDLK_UP;
  }
  if ( str == "down" )
  {
    found = true;
    result.Key = SDLK_DOWN;
  }
  if ( str == "right" )
  {
    found = true;
    result.Key = SDLK_RIGHT;
  }
  if ( str == "left" )
  {
    found = true;
    result.Key = SDLK_LEFT;
  }
  if ( str == "insert" )
  {
    found = true;
    result.Key = SDLK_INSERT;
  }
  if ( str == "home" )
  {
    found = true;
    result.Key = SDLK_HOME;
  }
  if ( str == "end" )
  {
    found = true;
    result.Key = SDLK_END;
  }
  if ( str == "pageup" )
  {
    found = true;
    result.Key = SDLK_PAGEUP;
  }
  if ( str == "pagedown" )
  {
    found = true;
    result.Key = SDLK_PAGEDOWN;
  }
  if ( str == "f1" )
  {
    found = true;
    result.Key = SDLK_F1;
  }
  if ( str == "f2" )
  {
    found = true;
    result.Key = SDLK_F2;
  }
  if ( str == "f3" )
  {
    found = true;
    result.Key = SDLK_F3;
  }
  if ( str == "f4" )
  {
    found = true;
    result.Key = SDLK_F4;
  }
  if ( str == "f5" )
  {
    found = true;
    result.Key = SDLK_F5;
  }
  if ( str == "f6" )
  {
    found = true;
    result.Key = SDLK_F6;
  }
  if ( str == "f7" )
  {
    found = true;
    result.Key = SDLK_F7;
  }
  if ( str == "f8" )
  {
    found = true;
    result.Key = SDLK_F8;
  }
  if ( str == "f9" )
  {
    found = true;
    result.Key = SDLK_F9;
  }
  if ( str == "f10" )
  {
    found = true;
    result.Key = SDLK_F10;
  }
  if ( str == "f11" )
  {
    found = true;
    result.Key = SDLK_F11;
  }
  if ( str == "f12" )
  {
    found = true;
    result.Key = SDLK_F12;
  }
  if ( str == "f13" )
  {
    found = true;
    result.Key = SDLK_F13;
  }
  if ( str == "f14" )
  {
    found = true;
    result.Key = SDLK_F14;
  }
  if ( str == "f15" )
  {
    found = true;
    result.Key = SDLK_F15;
  }
  if ( str == "numlock" )
  {
    found = true;
    result.Key = SDLK_NUMLOCK;
  }
  if ( str == "capslock" )
  {
    found = true;
    result.Key = SDLK_CAPSLOCK;
  }
  if ( str == "scrollock" )
  {
    found = true;
    result.Key = SDLK_SCROLLOCK;
  }
  if ( str == "rshift" )
  {
    found = true;
    result.Key = SDLK_RSHIFT;
  }
  if ( str == "lshift" )
  {
    found = true;
    result.Key = SDLK_LSHIFT;
  }
  if ( str == "rctrl" )
  {
    found = true;
    result.Key = SDLK_RCTRL;
  }
  if ( str == "lctrl" )
  {
    found = true;
    result.Key = SDLK_LCTRL;
  }
  if ( str == "ralt" )
  {
    found = true;
    result.Key = SDLK_RALT;
  }
  if ( str == "lalt" )
  {
    found = true;
    result.Key = SDLK_LALT;
  }
  if ( str == "rmeta" )
  {
    found = true;
    result.Key = SDLK_RMETA;
  }
  if ( str == "lmeta" )
  {
    found = true;
    result.Key = SDLK_LMETA;
  }
  if ( str == "lwin" )
  {
    found = true;
    result.Key = SDLK_LSUPER;
  }
  if ( str == "rwin" )
  {
    found = true;
    result.Key = SDLK_RSUPER;
  }
  if ( str == "mode" )
  {
    found = true;
    result.Key = SDLK_MODE;
  }
  if ( str == "help" )
  {
    found = true;
    result.Key = SDLK_HELP;
  }
  if ( str == "print" )
  {
    found = true;
    result.Key = SDLK_PRINT;
  }
  if ( str == "sysreq" )
  {
    found = true;
    result.Key = SDLK_SYSREQ;
  }
  if ( str == "break" )
  {
    found = true;
    result.Key = SDLK_BREAK;
  }
  if ( str == "menu" )
  {
    found = true;
    result.Key = SDLK_MENU;
  }
  if ( str == "power" )
  {
    found = true;
    result.Key = SDLK_POWER;
  }
  if ( str == "euro" )
  {
    found = true;
    result.Key = SDLK_EURO;
  }
  if ( str.substr( 0, 5 ) == "mouse" )
  {
    result.Mouse = true;
    result.Button = evalUnsigned( str.substr( 5 ) );
    found = true;
  }
  if ( !found )
    EXGAME_THROWINFO( ECGAME_GENERAL, ( "not a valid key descriptor: " + str ).c_str() );
  return result;
}




string wBindingManager::key2text( wKey const &key )
{
  if ( key.Mouse )
    return "mouse" + unsigned2dec( key.Button );
  else
    switch ( key.Key )
    {
    case SDLK_BACKSPACE:
      return "backspace";
    case SDLK_TAB:
      return "tab";
    case SDLK_CLEAR:
      return "clear";
    case SDLK_RETURN:
      return "return";
    case SDLK_PAUSE:
      return "pause";
    case SDLK_ESCAPE:
      return "escape";
    case SDLK_SPACE:
      return "space";
    case SDLK_EXCLAIM:
      return "!";
    case SDLK_QUOTEDBL:
      return "\"";
    case SDLK_HASH:
      return "#";
    case SDLK_DOLLAR:
      return "$";
    case SDLK_AMPERSAND:
      return "&";
    case SDLK_QUOTE:
      return "'";
    case SDLK_LEFTPAREN:
      return "(";
    case SDLK_RIGHTPAREN:
      return ")";
    case SDLK_ASTERISK:
      return "*";
    case SDLK_PLUS:
      return "+";
    case SDLK_COMMA:
      return ",";
    case SDLK_MINUS:
      return "-";
    case SDLK_PERIOD:
      return ".";
    case SDLK_SLASH:
      return "/";
    case SDLK_0:
      return "0";
    case SDLK_1:
      return "1";
    case SDLK_2:
      return "2";
    case SDLK_3:
      return "3";
    case SDLK_4:
      return "4";
    case SDLK_5:
      return "5";
    case SDLK_6:
      return "6";
    case SDLK_7:
      return "7";
    case SDLK_8:
      return "8";
    case SDLK_9:
      return "9";
    case SDLK_COLON:
      return ":";
    case SDLK_SEMICOLON:
      return ";";
    case SDLK_LESS:
      return "<";
    case SDLK_EQUALS:
      return "=";
    case SDLK_GREATER:
      return ">";
    case SDLK_QUESTION:
      return "?";
    case SDLK_AT:
      return "@";
    case SDLK_LEFTBRACKET:
      return "[";
    case SDLK_BACKSLASH:
      return "\\";
    case SDLK_RIGHTBRACKET:
      return "]";
    case SDLK_CARET:
      return "^";
    case SDLK_UNDERSCORE:
      return "_";
    case SDLK_BACKQUOTE:
      return "`";
    case SDLK_a:
      return "a";
    case SDLK_b:
      return "b";
    case SDLK_c:
      return "c";
    case SDLK_d:
      return "d";
    case SDLK_e:
      return "e";
    case SDLK_f:
      return "f";
    case SDLK_g:
      return "g";
    case SDLK_h:
      return "h";
    case SDLK_i:
      return "i";
    case SDLK_j:
      return "j";
    case SDLK_k:
      return "k";
    case SDLK_l:
      return "l";
    case SDLK_m:
      return "m";
    case SDLK_n:
      return "n";
    case SDLK_o:
      return "o";
    case SDLK_p:
      return "p";
    case SDLK_q:
      return "q";
    case SDLK_r:
      return "r";
    case SDLK_s:
      return "s";
    case SDLK_t:
      return "t";
    case SDLK_u:
      return "u";
    case SDLK_v:
      return "v";
    case SDLK_w:
      return "w";
    case SDLK_x:
      return "x";
    case SDLK_y:
      return "y";
    case SDLK_z:
      return "z";
    case SDLK_DELETE:
      return "delete";
    case SDLK_KP0:
      return "kp0";
    case SDLK_KP1:
      return "kp1";
    case SDLK_KP2:
      return "kp2";
    case SDLK_KP3:
      return "kp3";
    case SDLK_KP4:
      return "kp4";
    case SDLK_KP5:
      return "kp5";
    case SDLK_KP6:
      return "kp6";
    case SDLK_KP7:
      return "kp7";
    case SDLK_KP8:
      return "kp8";
    case SDLK_KP9:
      return "kp9";
    case SDLK_KP_PERIOD:
      return "kp.";
    case SDLK_KP_DIVIDE:
      return "kp/";
    case SDLK_KP_MULTIPLY:
      return "kp*";
    case SDLK_KP_MINUS:
      return "kp-";
    case SDLK_KP_PLUS:
      return "kp+";
    case SDLK_UP:
      return "up";
    case SDLK_DOWN:
      return "down";
    case SDLK_RIGHT:
      return "right";
    case SDLK_LEFT:
      return "left";
    case SDLK_INSERT:
      return "insert";
    case SDLK_HOME:
      return "home";
    case SDLK_END:
      return "end";
    case SDLK_PAGEUP:
      return "pageup";
    case SDLK_PAGEDOWN:
      return "pagedown";
    case SDLK_F1:
      return "f1";
    case SDLK_F2:
      return "f2";
    case SDLK_F3:
      return "f3";
    case SDLK_F4:
      return "f4";
    case SDLK_F5:
      return "f5";
    case SDLK_F6:
      return "f6";
    case SDLK_F7:
      return "f7";
    case SDLK_F8:
      return "f8";
    case SDLK_F9:
      return "f9";
    case SDLK_F10:
      return "f10";
    case SDLK_F11:
      return "f11";
    case SDLK_F12:
      return "f12";
    case SDLK_F13:
      return "f13";
    case SDLK_F14:
      return "f14";
    case SDLK_F15:
      return "f15";
    case SDLK_NUMLOCK:
      return "numlock";
    case SDLK_CAPSLOCK:
      return "capslock";
    case SDLK_SCROLLOCK:
      return "scrollock";
    case SDLK_RSHIFT:
      return "rshift";
    case SDLK_LSHIFT:
      return "lshift";
    case SDLK_RCTRL:
      return "rctrl";
    case SDLK_LCTRL:
      return "lctrl";
    case SDLK_RALT:
      return "ralt";
    case SDLK_LALT:
      return "lalt";
    case SDLK_RMETA:
      return "rmeta";
    case SDLK_LMETA:
      return "lmeta";
    case SDLK_LSUPER:
      return "lwin";
    case SDLK_RSUPER:
      return "rwin";
    case SDLK_MODE:
      return "mode";
    case SDLK_HELP:
      return "help";
    case SDLK_PRINT:
      return "print";
    case SDLK_SYSREQ:
      return "sysreq";
    case SDLK_BREAK:
      return "break";
    case SDLK_MENU:
      return "menu";
    case SDLK_POWER:
      return "power";
    case SDLK_EURO:
      return "euro";
    default:
      EXGAME_THROWINFO( ECGAME_GENERAL, "no text representation for key" );
    }
}




wKeyList wBindingManager::text2keys( string const &src_str )
{
  string str = src_str;
  wKeyList keys;
  regex_string re( "^([^ \t]*)[ \t]+(.+)$" );
  while ( re.match( str ) )
  {
    keys.insert( text2key( re.getBackref( 0 ) ) );
    str = re.getBackref( 1 );
  }
  keys.insert( text2key( str ) );
  return keys;
}




string wBindingManager::keys2text( wKeyList const &keys )
{
  string str;
  FOREACH_CONST( first, keys, wKeyList )
  {
    if ( str.size() )
      str += ' ';
    str += key2text( *first );
  }
  return str;
}




bool wBindingManager::internalUnbind( wKeyList const &keys )
{
  FOREACH( first, Bindings, wBindingList )
  if ( first->Keys == keys )
  {

    if ( first->Active && first->OnRelease.get() )
      first->OnRelease->call( value::parameter_list() );
    Bindings.erase( first );
    BindingsChanged = true;
    return true;
  }
  return false;
}




void wBindingManager::update()
{
  BindingsChanged = false;

  wBindingList::iterator first = Bindings.begin(), last = Bindings.end();
  while ( first != last )
  {
    if ( !( first->Keys <= ActiveKeys ) && first->Active )
    {
      first->Active = false;

      if ( first->OnRelease.get() )
        first->OnRelease->call( value::parameter_list() );
    }
    else if ( first->Keys <= ActiveKeys && !first->Active )
    {
      first->Active = true;
      if ( first->OnPress.get() )
        first->OnPress->call( value::parameter_list() );
    }
    if ( BindingsChanged )
    {
      BindingsChanged = false;
      first = Bindings.begin();
      last = Bindings.end();
    }
    else
      first++;
  }
}
