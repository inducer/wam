// ----------------------------------------------------------------------------
//  Description      : Console Commands
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "utility/filemanager.hh"
#include "engine/gamebase.hh"
#include "engine/console.hh"
#include "engine/instancemanager.hh"
#include "engine/commandmanager.hh"
#include "engine/commands.hh"




WAM_DECLARE_COMMAND( stringify, "Stfy", "stringify javascript object", "obj" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "stringify", 1, 1 )

  return makeConstant( parameters[ 0 ] ->stringify() );
}



WAM_DECLARE_COMMAND( execute, "Exec", "execute script", "script [mpack]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "execute", 1, 2 )

  if ( parameters.size() == 1 )
    return Game.getConsole().getCommandManager().executeScript( parameters[ 0 ] ->toString() );
  else
    return Game.getConsole().getCommandManager().executeScript( parameters[ 0 ] ->toString(), parameters[ 1 ] ->toString() );
}




WAM_DECLARE_COMMAND( help, "Help", "get help", "[command]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "help", 0, 1 )

  if ( parameters.size() == 1 )
  {
    wCommand * command;
    command = Game.getConsole().getCommandManager().getCommand( parameters[ 0 ] ->toString() );
    Game.getConsole()
    << command->getCommandLine() << newl
    << command->getHelpText() << newl;
  }
  else
  {
    list<wCommand *> l;
    Game.getConsole().getCommandManager().getRegisteredObjects( l );

    FOREACH( first, l, list<wCommand *> )
    {
      Game.getConsole()
      << ( *first ) ->getCommandString() << ": "
      << ( *first ) ->getHelpText() << newl;
    }
  }
  return makeNull();
}



WAM_DECLARE_COMMAND( echo, "Echo", "echo to console / set script echo on/off", "text|on|off" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "echo", 1, 1 )

  Game.getConsole() << parameters[ 0 ] ->toString() << newl;
  return makeNull();
}




WAM_DECLARE_COMMAND( failsafe, "Fail", "protect one command against failures, return (true,return_value) or (false,error_msg)", "command" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "failsafe", 1, 1 )

  auto_ptr<js_array> arr( new js_array( 2 ) );

  try
  {
    ( *arr ) [ 1 ] = Game.getConsole().getCommandManager().execute( parameters[ 0 ] ->toString() );
    ( *arr ) [ 0 ] = makeConstant( true );
  }
  catch ( exception & e )
  {
    ( *arr ) [ 0 ] = makeConstant( false );
    ( *arr ) [ 1 ] = makeConstant( string( e.what() ) );
  }
  return arr.release();
}




WAM_DECLARE_COMMAND( debuglevel, "DbgL", "set or get debug level", "[off|warn|normal|info|verbose|periodic|all]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "debuglevel", 0, 1 )

  if ( parameters.size() == 1 )
  {
    string par = parameters[ 0 ] ->toString();
    if ( par == "off" )
      wamSetDebugLevel( WAM_DEBUG_OFF );
    else if ( par == "warn" )
      wamSetDebugLevel( WAM_DEBUG_WARN );
    else if ( par == "normal" )
      wamSetDebugLevel( WAM_DEBUG_NORMAL );
    else if ( par == "info" )
      wamSetDebugLevel( WAM_DEBUG_INFO );
    else if ( par == "verbose" )
      wamSetDebugLevel( WAM_DEBUG_VERBOSE );
    else if ( par == "periodic" )
      wamSetDebugLevel( WAM_DEBUG_PERIODIC );
    else if ( par == "all" )
      wamSetDebugLevel( WAM_DEBUG_ALL );
    else
      EXGAME_THROWINFO( ECGAME_CONSOLE, ( "invalid parameter: " + par ).c_str() );
    return makeNull();
  }
  else
  {
    switch ( wamGetDebugLevel() )
    {
    case WAM_DEBUG_OFF:
      return makeConstant( "off" );
    case WAM_DEBUG_WARN:
      return makeConstant( "warn" );
    case WAM_DEBUG_NORMAL:
      return makeConstant( "normal" );
    case WAM_DEBUG_INFO:
      return makeConstant( "info" );
    case WAM_DEBUG_VERBOSE:
      return makeConstant( "verbose" );
    case WAM_DEBUG_PERIODIC:
      return makeConstant( "periodic" );
    case WAM_DEBUG_ALL:
      return makeConstant( "all" );
    default:
      return makeConstant( "--invalid--" );
    }
  }
}




WAM_DECLARE_COMMAND( debugmode, "DebM", "set/get game ticks uniform mode", "[boolean]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "debugmode", 0, 1 )

  if ( parameters.size() == 1 )
  {
    Game.setDebugMode( parameters[ 0 ] ->toBoolean() );
    return makeNull();
  }
  return makeConstant( Game.getDebugMode() );
}




WAM_DECLARE_COMMAND( getfilename, "GeFN", "resolve mission pack lookup", "type basename" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "getfilename", 2, 2 )

  wFileType ft = WAM_FT_SAVEGAME;
  string par = parameters[ 0 ] ->toString();
  if ( par == "bitmap" )
    ft = WAM_FT_IMAGE;
  if ( par == "mask" )
    ft = WAM_FT_SHAPE;
  if ( par == "font" )
    ft = WAM_FT_FONT;
  if ( par == "script" )
    ft = WAM_FT_SCRIPT;
  if ( par == "code" )
    ft = WAM_FT_CODE;
  if ( par == "sound" )
    ft = WAM_FT_SOUND;
  if ( par == "music" )
    ft = WAM_FT_MUSIC;

  return makeConstant( Game.getFileManager().getFilename( ft, parameters[ 1 ] ->toString() ) );
}




WAM_DECLARE_COMMAND( printdebug, "PDbg", "print debug message", "[off|warn|normal|info|periodic|all] message" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "printdebug", 1, 2 )

  bool dlfound = false;
  wDebugLevel dl = WAM_DEBUG_INFO;
  string par = parameters[ 0 ] ->toString();

  if ( par == "off" )
  {
    dl = WAM_DEBUG_OFF;
    dlfound = true;
  }
  else if ( par == "warn" )
  {
    dl = WAM_DEBUG_WARN;
    dlfound = true;
  }
  else if ( par == "normal" )
  {
    dl = WAM_DEBUG_NORMAL;
    dlfound = true;
  }
  else if ( par == "info" )
  {
    dl = WAM_DEBUG_INFO;
    dlfound = true;
  }
  else if ( par == "periodic" )
  {
    dl = WAM_DEBUG_PERIODIC;
    dlfound = true;
  }
  else if ( par == "all" )
  {
    dl = WAM_DEBUG_ALL;
    dlfound = true;
  }

  if ( dlfound )
    wamPrintDebugLevel( parameters[ 1 ] ->toString(), dl );
  else
    wamPrintDebug( parameters[ 0 ] ->toString() );
  return makeNull();
}




// game stuff -----------------------------------------------------------------
WAM_DECLARE_COMMAND( res_cleanup, "Clup", "clean up various resource heaps", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "res_cleanup", 0, 0 )

  Game.cleanUp();
  return makeNull();
}




WAM_DECLARE_COMMAND( game_addpack, "AdPk", "add mission pack to game", "mpack" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_addpack", 1, 1 )
  Game.addMissionPack( parameters[ 0 ] ->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( game_removepack, "RmPk", "remove mission pack from game", "mpack" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "rgame_emovepack", 1, 1 )
  Game.removeMissionPack( parameters[ 0 ] ->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( game_version, "Vers", "get program version", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_version", 0, 0 )
  return makeConstant( VERSION );
}




WAM_DECLARE_COMMAND( game_about, "Abou", "about this game", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_about", 0, 0 )

  Game.getConsole()
  << "WAM version " << VERSION << newl
  << "by" << newl
  << "Hendrik Dahlkamp" << newl
  << "Hardy Kahl" << newl
  << "Andreas Kloeckner" << newl
  << "Christian Plagemann" << newl;
  return makeNull();
}




WAM_DECLARE_COMMAND( game_quit, "Quit", "quit the game", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_quit", 0, 0 )

  Game.setQuitflag();
  return makeNull();
}




WAM_DECLARE_COMMAND( game_time, "GeGT", "get game time", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "getgametime", 0, 0 )
  return makeConstant( Game.getGameTime() );
}




WAM_DECLARE_COMMAND( list_packs, "LiPk", "list mission packs in game", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_packs", 0, 0 )

  wFileManager::wMissionPackList mpl;
  Game.getFileManager().listMissionPacks( mpl );

  auto_ptr<js_array> arr( new js_array( mpl.size() ) );

  TIndex idx = 0;
  FOREACH_CONST( first, mpl, wFileManager::wMissionPackList )
  ( *arr ) [ idx++ ] = makeConstant( *first );
  return arr.release();
}




WAM_DECLARE_COMMAND( game_pause, "Paus", "get/set pause mode", "[boolean]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_pause", 0, 1 )
  if ( parameters.size() == 1 )
  {
    Game.setPauseMode( parameters[ 0 ] ->toBoolean() );
    return makeNull();
  }
  else
    return makeConstant( Game.getPauseMode() );
}




// manager stuff --------------------------------------------------------------
WAM_DECLARE_COMMAND( list_tick_receivers, "LiTi", "list objects registered to TickManager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_tick_receivers", 0, 0 )

  auto_ptr<js_array> arr( new js_array( Game.getTickManager().size() ) );

  TIndex idx = 0;
  FOREACH_CONST( first, Game.getTickManager(), wTickManager )
  ( *arr ) [ idx++ ] = makeConstant( ( *first ) ->id() );
  return arr.release();
}




// console stuff --------------------------------------------------------------
void registerCommands( wGame &g )
{
  register_stringify( g );
  register_execute( g );
  register_help( g );
  register_echo( g );
  register_failsafe( g );
  register_debuglevel( g );
  register_debugmode( g );
  register_getfilename( g );
  register_printdebug( g );

  // game stuff
  register_res_cleanup( g );
  register_list_packs( g );
  register_game_addpack( g );
  register_game_removepack( g );
  register_game_version( g );
  register_game_about( g );
  register_game_quit( g );
  register_game_time( g );
  register_game_pause( g );

  // manager stuff
  register_list_tick_receivers( g );
}
