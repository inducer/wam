// ----------------------------------------------------------------------------
//  Description      : WAM game object
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "engine/names.hh"
#include "engine/binding.hh"
#include "engine/scrolling.hh"
#include "engine/commands.hh"
#include "engine/audio_commands.hh"
#include "engine/game.hh"




// wGameFinal ----------------------------------------------------------------
wGameFinal::wGameFinal( string const &basedir, display &dpy, audio_manager *amgr )
    : IsMaster( true ), Quitflag( false ), PauseMode( false ), DebugMode( false ),
    GameTime( 0 ), LastTickStart( SDL_GetTicks() ),
    InstanceManager( *this ), GameCodeManager( *this ), NetworkManager( *this ), PersistenceManager( *this ),
    DrawableManager( dpy ),
    ScrollManager( NULL ), Console( NULL ),
    FileManager( basedir ),
    ImageManager( FileManager ), FontManager( FileManager ), ShapeBitmapManager( FileManager ),
    SoundManager( FileManager ),
    AudioManager( amgr )
{}




void wGameFinal::startUp()
{
  registerConsole( *this );
  InstanceManager.createObject( GJFEATURESET_CONSOLE );
  InstanceManager.commit();

  registerInstanceManagerCommands( *this );
  registerGameCodeManagerCommands( *this );
  registerCollisionManagerCommands( *this );
  registerMessageManagerCommands( *this );
  registerNetworkCommands( *this );
  registerResourceManagerCommands( *this );
  registerInputManagerCommands( *this );
  registerDrawableManagerCommands( *this );
  registerPersistenceCommands( *this );
  startupAudioCommands( *this );

  registerScrollManager( *this );
  registerBindingManager( *this );
  registerCommands( *this );
  InstanceManager.commit();
}




wGameFinal::~wGameFinal()
{
  InstanceManager.destroyAll();

  shutdownAudioCommands( *this );
}




void wGameFinal::cleanUp()
{
  wamPrintDebug( "cleaning up resources" );
  ImageManager.cleanUp();
  FontManager.cleanUp();
  ShapeBitmapManager.cleanUp();
  SoundManager.cleanUp();
}




void wGameFinal::resetManagers()
{
  InstanceManager.commit();
  CollisionManager.reset();
  DrawableManager.requestRedrawEverything();
}




namespace
{
struct wMainLoopExit
{
  int u;
};
#define MLE if (RestartMainLoop) throw wMainLoopExit();

}




bool wGameFinal::loopIteration()
{
  if ( SDL_GetTicks() - LastTickStart < 5 )
    SDL_Delay( 5 - ( SDL_GetTicks() - LastTickStart ) );

  unsigned long tick_start = SDL_GetTicks();
  float seconds = float( tick_start - LastTickStart ) / 1000;
  if ( DebugMode )
    seconds = 0.01;
  if ( seconds > 0.3 )
    seconds = 0.3;

  GameTime += seconds;

  try
  {
    if ( AudioManager )
      AudioManager->tick();

    DrawableManager.run();
    MLE
    NetworkManager.run();
    MLE
    if ( !InputManager.run() )
      setQuitflag();
    MLE
    MessageManager.run();
    MLE
    TickManager.run( seconds );
    MLE
    CollisionManager.run();
    MLE
    InstanceManager.commit();
  }
  catch ( wMainLoopExit & ex )
  {
    resetManagers();
    RestartMainLoop = false;
  }

  LastTickStart = tick_start;
  return ( !Quitflag );
}




void wGameFinal::addMissionPack( string const &name )
{
  FileManager.addMissionPack( name );
  try
  {
    getConsole().getCommandManager().executeScript( WAM_SCRIPT_MPACKINIT, name );
  }
  catch ( ... )
  {
    FileManager.removeMissionPack( name );
    throw;
  }
  wamPrintDebug( "added mission pack: " + name );
}




void wGameFinal::removeMissionPack( string const &name )
{
  FileManager.removeMissionPack( name );
  try
  {
    getConsole().getCommandManager().executeScript( WAM_SCRIPT_MPACKDONE, name );
  }
  catch ( ... )
  {
    FileManager.addMissionPack( name );
    throw;
  }
  wamPrintDebug( "removed mission pack: " + name );
}




void wGameFinal::setPauseMode( bool value )
{
  if ( PauseMode != value )
  {
    if ( value )
      getMessageManager().post( getMessageManager().registerMessage( WAM_MSG_PAUSE ) );
    else
      getMessageManager().post( getMessageManager().registerMessage( WAM_MSG_UNPAUSE ) );
  }
  PauseMode = value;
}
