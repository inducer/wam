// ----------------------------------------------------------------------------
//  Description      : Console
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <iomanip>
#include <iostream>
#include <sys/time.h>
#include <SDL.h>
#include <ixlib_numconv.hh>
#include "utility/debug.hh"
#include "engine/gamebase.hh"
#include "engine/codemanager.hh"
#include "engine/console.hh"




// global data ----------------------------------------------------------------
wNewLine newl;
wClearLine clearl;




// registration ---------------------------------------------------------------
wGameject *createConsole( wGame &s )
{
  return new wConsole( s );
}

void registerConsole( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_CONSOLE, "", createConsole );
}




// wSimpleConsoleDisplay -----------------------------------------------------
class wSimpleConsoleDisplay : public wConsoleDisplay
{
    void clearLine()
    {
      cout << '\r' << string( 79, ' ' ) << '\r' << ::flush;
    }
    void output( string const &txt )
    {
      cout << txt;
    }
    void newLine()
    {
      cout << endl;
    }
    void flush()
    {
      cout << ::flush;
    }
};




// scripting interface --------------------------------------------------------
WAM_DECLARE_COMMAND( console_enable, "CoEn", "set whether console processes events", "[boolean]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "console_enable", 0, 1 )

  if ( parameters.size() == 1 )
  {
    Game.getConsole().enableConsole( parameters[ 0 ] ->toBoolean() );
    return makeNull();
  }
  else
    return makeConstant( Game.getConsole().isConsoleEnabled() );
}




WAM_DECLARE_COMMAND( console_keywait, "CoKD", "set keyboard delay", "[seconds]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "console_keywait", 0, 1 )

  if ( parameters.size() == 1 )
  {
    Game.getConsole().setKeyWait( parameters[ 0 ] ->toFloat() );
    return makeNull();
  }
  else
    return makeConstant( Game.getConsole().getKeyWait() );
}




WAM_DECLARE_COMMAND( console_keyrepeat, "CoKR", "set keyboard repeat", "[seconds]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "console_keyrepeat", 0, 1 )

  if ( parameters.size() == 1 )
  {
    Game.getConsole().setKeyRepeat( parameters[ 0 ] ->toFloat() );
    return makeNull();
  }
  else
    return makeConstant( Game.getConsole().getKeyRepeat() );
}




// wConsole -------------------------------------------------------------------
wConsole const &wConsole::operator<<( string const &what ) const
{
  if ( Display )
    Display->output( what );
  return *this;
}




wConsole const &wConsole::operator<<( char const *what ) const
{
  if ( Display )
    Display->output( what );
  return *this;
}




wConsole const &wConsole::operator<<( double what ) const
{
  if ( Display )
    Display->output( float2dec( what ) );
  return *this;
}




wConsole const &wConsole::operator<<( unsigned int what ) const
{
  if ( Display )
    Display->output( unsigned2dec( what ) );
  return *this;
}




wConsole const &wConsole::operator<<( int what ) const
{
  if ( Display )
    Display->output( signed2dec( what ) );
  return *this;
}




wConsole const &wConsole::operator<<( char what ) const
{
  if ( Display )
    Display->output( string( 1, what ) );
  return *this;
}




wConsole const &wConsole::operator<<( wNewLine const &newl ) const
{
  if ( Display )
    Display->newLine();
  return *this;
}




wConsole const &wConsole::operator<<( wClearLine const &clearl ) const
{
  if ( Display )
    Display->clearLine();
  return *this;
}




wConsole::wConsole( wGame &g )
    : wGameject( g ), wKeyboardInputListener( g ), wTickReceiver( g ), 
    KeyRepeat( 0.02 ), KeyWait( 0.2 ),
    KeyRepeating( false ), WaitTillNextRepetition( 0 ),
    CursorPosition( 0 ), CursorInOnPhase( true ), CursorBlinkCounter( CursorBlinkSpeed ),
    PromptInhibit( 1 ), Enabled( true ),
    CommandManager( g ), Display( NULL )
{
  wSimpleConsoleDisplay * dpy = new wSimpleConsoleDisplay;
  setDisplay( dpy );

  HistoryPosition = History.end();
  wamPrintDebug( "console online" );
}




wConsole::~wConsole()
{
  if ( Display )
    delete Display;
}




void wConsole::registerMe()
{
  wKeyboardInputListener::registerMe();
  wTickReceiver::registerMe();
  Game.setConsole( this );

  register_console_enable( Game );
  register_console_keywait( Game );
  register_console_keyrepeat( Game );
}




void wConsole::startUp()
{
  wKeyboardInputListener::startUp();
  wTickReceiver::startUp();

  printPrompt();
}




void wConsole::prepareToDie()
{
  wKeyboardInputListener::prepareToDie();
  wTickReceiver::prepareToDie();
  clearPrompt();
}




void wConsole::unregisterMe()
{
  Game.setConsole();

  wKeyboardInputListener::unregisterMe();
  wTickReceiver::unregisterMe();
}




void wConsole::printDebug( string const &what )
{
  clearPrompt();
  *this << "[debug] " << what << newl;
  printPrompt();
}




void wConsole::setDisplay( wConsoleDisplay *disp )
{
  clearPrompt();
  Display = disp;
  printPrompt();
}




void wConsole::clearPrompt()
{
  if ( PromptInhibit == 0 )
    * this << clearl;
  PromptInhibit++;
}




void wConsole::printPrompt()
{
  PromptInhibit--;
  if ( PromptInhibit == 0 )
  {
    string editbuffer_with_cursor = EditBuffer;
    if ( CursorInOnPhase )
      editbuffer_with_cursor.insert( CursorPosition, "|" );
    else
      editbuffer_with_cursor.insert( CursorPosition, " " );

    if ( CommandBuffer.size() )
      * this << "[...] " << editbuffer_with_cursor;
    else
      *this << "[wam] " << editbuffer_with_cursor;
    if ( Display )
      Display->flush();
  }
}




void wConsole::enableConsole( bool enabled )
{
  if ( enabled != Enabled )
  {
    if ( enabled )
      printPrompt();
    else
      clearPrompt();
    Enabled = enabled;
    if ( Display )
    {
      if ( enabled )
        Display->enable();
      else
        Display->disable();
    }
  }
}




void wConsole::tick( float seconds )
{
  if ( Enabled && KeyRepeating )
  {
    WaitTillNextRepetition -= seconds;
    if ( WaitTillNextRepetition <= 0 )
    {
      WaitTillNextRepetition += KeyRepeat;

      processKeyEvent( LastKey );

      if ( WaitTillNextRepetition <= 0 )
        WaitTillNextRepetition = 1;
    }
  }
  CursorBlinkCounter -= seconds;
  if ( CursorBlinkCounter < 0 )
  {
    CursorInOnPhase = !CursorInOnPhase;
    CursorBlinkCounter += CursorBlinkSpeed;
    if ( CursorBlinkCounter < 0 )
      CursorBlinkCounter = CursorBlinkSpeed;
    clearPrompt();
    printPrompt();
  }
}




bool wConsole::processKeyEvent( wKeyboardEvent &event )
{
  if ( !Enabled )
    return false;

  if ( event.getType() != wKeyboardEvent::KEY_PRESS )
  {
    KeyRepeating = false;
    return false;
  }

  if ( !KeyRepeating )
  {
    KeyRepeating = true;
    LastKey = event;
    WaitTillNextRepetition = KeyWait;
  }

  CursorInOnPhase = true;
  CursorBlinkCounter = CursorBlinkSpeed;

  int held_modifiers = event.getModifier() & ( KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_META );
  wConsoleLocker locker( *this );
  if ( held_modifiers == 0 && event.getKey() == SDLK_RETURN )
  {
    if ( EditBuffer.size() == 0 )
      return true;
    if ( CommandBuffer.size() )
      * this << "...> " << EditBuffer << newl;
    else
      *this << "> " << EditBuffer << newl;
    if ( CommandBuffer.size() )
      CommandBuffer += " " + EditBuffer;
    else
      CommandBuffer += EditBuffer;
    EditBuffer = "";
    CursorPosition = 0;

    try
    {
      try
      {
        ref<value> val = CommandManager.execute( CommandBuffer );
        if ( val.get() && val->getType() != value::VT_NULL )
          * this << val->stringify() << newl;
        addHistoryCommand( CommandBuffer );
        CommandBuffer = "";
      }
      EX_CATCHCODE( javascript, ECJS_UNEXPECTED_EOF, )
    }
    catch ( exception & e )
    {
      *this << "FAILED." << ' ' << e.what() << newl;
      addHistoryCommand( CommandBuffer );
      CommandBuffer = "";
    }
    return true;
  }
  else if ( held_modifiers == 0 && event.getKey() == SDLK_TAB )
  {
    CommandManager.completeCommandLine( EditBuffer );
    CursorPosition = EditBuffer.size();
    return true;
  }
  else if ( held_modifiers == 0 && event.getKey() == SDLK_UP )
  {
    getPreviousHistoryCommand( EditBuffer );
    CursorPosition = EditBuffer.size();
    return true;
  }
  else if ( held_modifiers == 0 && event.getKey() == SDLK_DOWN )
  {
    getNextHistoryCommand( EditBuffer );
    CursorPosition = EditBuffer.size();
    return true;
  }
  else if ( held_modifiers == 0 && event.getKey() == SDLK_LEFT )
  {
    if ( CursorPosition > 0 )
      CursorPosition--;
    return true;
  }
  else if ( held_modifiers == 0 && event.getKey() == SDLK_RIGHT )
  {
    if ( CursorPosition < EditBuffer.size() )
      CursorPosition++;
    return true;
  }
  else if ( held_modifiers == 0 && event.getKey() == SDLK_BACKSPACE )
  {
    if ( CursorPosition > 0 )
    {
      CursorPosition--;
      EditBuffer.erase( CursorPosition, 1 );
    }
    return true;
  }
  else if ( event.isPrintable() )
  {
    EditBuffer.insert( CursorPosition++, string( 1, event.getASCII() ) );
    return true;
  }

  return false;
}




bool wConsole::getPreviousHistoryCommand( string &cmd )
{
  if ( ( History.size() > 0 ) && ( HistoryPosition != History.begin() ) )
  {
    HistoryPosition--;
    cmd = *HistoryPosition;
    return true;
  }
  return false;
}




bool wConsole::getNextHistoryCommand( string &cmd )
{
  if ( ( History.size() > 0 ) && ( HistoryPosition != History.end() ) )
  {
    HistoryPosition++;
    if ( HistoryPosition == History.end() )
      cmd = "";
    else
      cmd = *HistoryPosition;
    return true;
  }
  return false;
}




void wConsole::addHistoryCommand( string const &cmd )
{
  if ( History.size() == 0 || ( History.size() && History.back() != cmd ) )
    History.push_back( cmd );
  HistoryPosition = History.end();
}
