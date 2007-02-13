// ----------------------------------------------------------------------------
//  Description      : WAM main program.
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM, all rights reserved.
// ----------------------------------------------------------------------------




#include <cmath>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <ixlib_exbase.hh>
#include <ixlib_numconv.hh>
#include <ixlib_cmdline.hh>
#include "utility/debug.hh"
#include "engine/game.hh"
#include "engine/names.hh"
#include "engine/console.hh"
#include "gamejects/mouse_scroller.hh"
#include "gamejects/graphic_console.hh"
#include "gamejects/mouse_pointer.hh"
#include "gamejects/parallax.hh"
#include "gamejects/pause_monitor.hh"
#include "gamejects/status.hh"
#include "gamejects/cron.hh"
#include "gamejects/menu.hh"
#include "gamejects/script_notifier.hh"

#ifndef HAVE_DYNAMIC_LINKING
 #include "game/main.hh"
#endif




// needed for win32 compatibility ---------------------------------------------
extern "C"
{
  int main( int argc, char **argv );
}




// variables ------------------------------------------------------------------
#define WAM_DEFAULT_WIDTH               640
#define WAM_DEFAULT_HEIGHT              480




static wGame *wamGame = NULL;
static wDebugLevel wamDebugLevel = WAM_DEBUG_INFO;




// Tools ----------------------------------------------------------------------
#ifdef WAM_DEBUG
void wamPrintDebug( string const &what )
{
  wamPrintDebugLevel( what, WAM_DEBUG_INFO );
}




void wamPrintDebugLevel( string const &what, wDebugLevel level )
{
  if ( level > wamDebugLevel )
    return ;
  try
  {
    if ( !wamGame )
      throw 5;
    wamGame->getConsole().printDebug( what );
  }
  catch ( ... )
  {
    cerr << "[debug (no console)] " << what << endl;
  }
}
#endif



wDebugLevel wamGetDebugLevel()
{
  return wamDebugLevel;
}




void wamSetDebugLevel( wDebugLevel level )
{
  wamDebugLevel = level;
}




// Main program ---------------------------------------------------------------
void main_loop( string const &basedir, display &dpy, audio_manager *amgr )
{
  try
  {
    wGameFinal game( basedir, dpy, amgr );
    wamGame = &game;

#ifdef WAM_DEBUG

    try
    {
#endif // WAM_DEBUG
      game.startUp();

      registerMouseScroller( game );
      registerGraphicConsole( game );
      registerMousePointer( game );
      registerParallax( game );
      registerPauseMonitor( game );
      registerStatus( game );
      registerCron( game );
      registerScriptNotifier( game );
      registerMenuCommands( game );

#ifndef HAVE_DYNAMIC_LINKING

      game.getGameCodeManager().registerLibrary( "libwam.so", wamInitializeModule );
#endif

      game.addMissionPack( WAM_MPACK_INITIAL );

      if ( amgr )
      {
        wamPrintDebug( "audio online" );
        amgr->play();
      }

      wamPrintDebug( "entering main loop" );
      while ( game.loopIteration() )
        ;

#ifdef WAM_DEBUG

    }
    catch ( exception & ex )
    {
      wamPrintDebug( string( "pre-cleanup exception dump: " ) + ex.what() );
      throw;
    }
#endif // WAM_DEBUG

  }
  catch ( ... )
  {
    wamGame = NULL;
    throw;
  }
  wamGame = NULL;
}




void inner_main( string const &basedir, TSize width, TSize height, bool fullscreen, bool sound, TSize srate )
{
  try
  {
    wamPrintDebug( "initializing..." );

    Uint32 init_flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;
    if ( sound )
      init_flags |= SDL_INIT_AUDIO;
    SDL_Init( init_flags );

    Uint32 dpy_flags = SDL_HWPALETTE | SDL_HWSURFACE | SDL_DOUBLEBUF;
    if ( fullscreen )
      dpy_flags |= SDL_FULLSCREEN;

    display dpy( width, height, dpy_flags );
    dpy.caption( "WAM " + unsigned2dec( WAM_MAJOR_VERSION ) + "." +
                 unsigned2dec( WAM_MINOR_VERSION ) );

    auto_ptr<audio_manager> amgr( NULL );
    if ( sound )
    {
      try
      {
        wamPrintDebug( "audio init (" + unsigned2dec( srate ) + " hz)" );
        auto_ptr<audio_manager> newamgr( new audio_manager( srate ) );
        amgr = newamgr;
      }
      catch ( sdl_exception & ex )
      {
        cerr
        << "*** could not initialize audio:" << endl
        << "*** " << ex.what() << endl;
      }
    }
    main_loop( basedir, dpy, amgr.get() );

    SDL_Quit();
  }
  catch ( ... )
  {
    SDL_Quit();
    throw;
  }
}




int main( int argc, char **argv )
{
  try
  {
    cout << endl;
    cout << "============================================" << endl;
    cout << "WAM Release " << ( int ) WAM_MAJOR_VERSION << '.'
    << ( int ) WAM_MINOR_VERSION << " (C) 2001 Team WAM" << endl;
    cout << "============================================" << endl << endl;

    // parse command line
    command_line cmdline( argc, argv );

    string basedir = ".";
    bool fullscreen = false, sound = true;
    TSize width = WAM_DEFAULT_WIDTH, height = WAM_DEFAULT_HEIGHT;
    TSize srate = 22050;

    if ( cmdline.count( "--help" ) || cmdline.count( "-h" ) )
    {
      cout << endl
      << "  --width=<num>        make window <num> pixels wide" << endl
      << "  --height=<num>       make window <num> pixels high" << endl
      << "  --fullscreen         try to run in fullscreen mode" << endl
      << "  --nosound            do not use sound engine" << endl
      << "  --rate=<num>         use <num> Hz sampling rate" << endl
      << endl;
      return 0;
    }

    if ( cmdline.count( "--base=" ) )
      basedir = cmdline.get( "--base=" );
    if ( cmdline.count( "--width=" ) )
      width = evalUnsigned( cmdline.get( "--width=" ) );
    if ( cmdline.count( "--height=" ) )
      height = evalUnsigned( cmdline.get( "--height=" ) );
    if ( cmdline.count( "--fullscreen" ) )
      fullscreen = true;
    if ( cmdline.count( "--nosound" ) )
      sound = false;
    if ( cmdline.count( "--rate=" ) )
      srate = evalUnsigned( cmdline.get( "--rate=" ) );

    inner_main( basedir, width, height, fullscreen, sound, srate );
  }
  catch ( exception & ex )
  {
    cerr
    << "*** fatal exception:" << endl
    << "*** " << ex.what() << endl;
    return 1;
  }
  catch ( ... )
  {
    cerr << "*** WAM terminated by unknown exception. Sorry." << endl
    << "Please report this along with a procedure to reproduce this bug (if known)" << endl
    << "to <wam@ixion.net>. Thank you." << endl;
    return 1;
  }
  cout << endl;
  return 0;
}

