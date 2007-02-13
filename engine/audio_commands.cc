// ----------------------------------------------------------------------------
//  Description      : Audio console commands
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_garbage.hh>
#include <sdlucid_audio.hh>
#include "utility/filemanager.hh"
#include "engine/console.hh"
#include "engine/commandmanager.hh"
#include "engine/gamebase.hh"
#include "engine/audio_commands.hh"

#ifdef SDLUCID_HAS_SMPEG
#include <sdlucid_smpeg_stream.hh>
#endif

#ifdef SDLUCID_HAS_LIBMIKMOD
#include <sdlucid_mikmod_stream.hh>
#endif




#define CHECK_FOR_AUDIO_MANAGER \
  if (!Game.getAudioManager()) { \
    EXGAME_THROWINFO(ECGAME_CONSOLE,"audio manager not present"); \
    }





WAM_DECLARE_COMMAND( audio_isactive, "IsAu", "return whether audio manager is active", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_list", 0, 0 )

  return makeConstant( Game.getAudioManager() != NULL );
}




WAM_DECLARE_COMMAND( list_audio_streams, "CoAu", "return ids of current audio streams", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_audio_streams", 0, 0 )
  CHECK_FOR_AUDIO_MANAGER

  auto_ptr<js_array> arr( new js_array( Game.getAudioManager() ->size() ) );

  TIndex idx = 0;
  FOREACH_CONST( first, *Game.getAudioManager(), audio_manager )
  ( *arr ) [ idx++ ] = makeConstant( first->Id );
  return arr.release();
}




WAM_DECLARE_COMMAND( audio_describe, "DeAu", "describe audio stream with id", "id" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_describe", 1, 1 )
  CHECK_FOR_AUDIO_MANAGER

  TIndex id = parameters[ 0 ] ->toInt();
  ref<audio_stream> strm = Game.getAudioManager() ->getStream( id );
  return makeConstant( strm->describe() );
}




WAM_DECLARE_COMMAND( audio_kill, "KiAu", "kill audio stream", "id" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_kill", 1, 1 )
  CHECK_FOR_AUDIO_MANAGER

  TIndex id = parameters[ 0 ] ->toInt();
  Game.getAudioManager() ->removeStream( id );

  return makeNull();
}




WAM_DECLARE_COMMAND( audio_setvolume, "SeVo", "set audio stream volume and panning", "stream_id volume [panning]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_setvolume", 2, 3 )
  CHECK_FOR_AUDIO_MANAGER

  TIndex id = parameters[ 0 ] ->toInt();
  ref<audio_stream> strm = Game.getAudioManager() ->getStream( id );
  if ( parameters.size() == 2 )
    strm->setVolume( parameters[ 1 ] ->toFloat() );
  else
    strm->setVolume( parameters[ 1 ] ->toFloat(), parameters[ 2 ] ->toFloat() );

  return makeNull();
}




WAM_DECLARE_COMMAND( audio_getvolume, "GeVo", "get audio stream volume", "stream_id" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_getvolume", 1, 1 )
  CHECK_FOR_AUDIO_MANAGER

  TIndex id = parameters[ 0 ] ->toInt();
  ref<audio_stream> strm = Game.getAudioManager() ->getStream( id );
  return makeConstant( strm->getVolume() );
}




WAM_DECLARE_COMMAND( audio_getpanning, "GePa", "get audio stream panning", "stream_id" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_getpanning", 1, 1 )
  CHECK_FOR_AUDIO_MANAGER

  TIndex id = parameters[ 0 ] ->toInt();
  ref<audio_stream> strm = Game.getAudioManager() ->getStream( id );
  return makeConstant( strm->getPanning() );
}




#ifdef SDLUCID_HAS_SMPEG
WAM_DECLARE_COMMAND( audio_playmp3, "pMP3", "play mp3 file", "file" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_playmp3", 1, 1 )
  CHECK_FOR_AUDIO_MANAGER

  ref<wFileManager::wStream> file = Game.getFileManager().openFile( WAM_FT_MUSIC, parameters[ 0 ] ->toString() );
  ref<audio_stream> mp3 = new audio_mp3_stream( file.releaseFromGCArena(), true );
  return makeConstant( Game.getAudioManager() ->addStream( mp3 ) );
}
#endif // SDLUCID_HAS_SMPEG




#ifdef SDLUCID_HAS_LIBMIKMOD
WAM_DECLARE_COMMAND( audio_playmod, "pMOD", "play mod file", "file" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_playmod", 1, 1 )
  CHECK_FOR_AUDIO_MANAGER

  ref<wFileManager::wStream> file = Game.getFileManager().openFile( WAM_FT_MUSIC, parameters[ 0 ] ->toString() );
  ref<audio_stream> mod = new audio_mod_stream( *file );
  return makeConstant( Game.getAudioManager() ->addStream( mod ) );
}
#endif // SDLUCID_HAS_LIBMIKMOD




WAM_DECLARE_COMMAND( audio_playwav, "pWAV", "play mod file", "file [repeatcount]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_playwav", 1, 2 )
  CHECK_FOR_AUDIO_MANAGER

  ref<wFileManager::wStream> file = Game.getFileManager().openFile( WAM_FT_SOUND, parameters[ 0 ] ->toString() );
  auto_ptr<audio_data> data( new audio_data );
  data->loadWAV( *file );
  ref<audio_stream> stream;
  if ( parameters.size() == 2 )
    stream = new audio_data_stream( data.release(), parameters[ 1 ] ->toInt() );
  else
    stream = new audio_data_stream( data.release() );
  return makeConstant( Game.getAudioManager() ->addStream( stream ) );
}




// Notifier -------------------------------------------------------------------
static class wJavascriptNotificationHandler : public audio_manager::notification_handler
{
    wCommandManager *Manager;
    ref<value> NotificationFunction;

  public:
    wJavascriptNotificationHandler()
        : Manager( NULL ), NotificationFunction( NULL )
    {}
    void setManager( wCommandManager *manager )
    {
      Manager = manager;
    }
    void setNotificationFunction( ref<value> notif_func )
    {
      NotificationFunction = notif_func;
    }
    void operator() ( TNotificationType type,audio_manager::TStreamId id );
}
JavascriptNotificationHandler;




void wJavascriptNotificationHandler::operator() ( TNotificationType type,audio_manager::TStreamId id )
{
  if ( NotificationFunction.get() == NULL )
    return ;
  string type_string;
  switch ( type )
  {
  case STARTED:
    type_string = "STARTED";
    break;
  case ENDED:
    type_string = "ENDED";
    break;
  case REMOVED:
    type_string = "REMOVED";
    break;
  }
  value::parameter_list plist;
  plist.push_back( makeConstant( type_string ) );
  plist.push_back( makeConstant( id ) );
  NotificationFunction->call( plist );
}





WAM_DECLARE_COMMAND( audio_setnotification, "ASNo", "set stream notification function", "function(type,id)" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "audio_setnotification", 1, 1 )
  CHECK_FOR_AUDIO_MANAGER

  JavascriptNotificationHandler.setNotificationFunction( parameters[ 0 ] );
  return makeNull();
}




// Registration ---------------------------------------------------------------
void startupAudioCommands( wGame &g )
{
  register_audio_isactive( g );
  register_list_audio_streams( g );
  register_audio_describe( g );
  register_audio_kill( g );
  register_audio_setvolume( g );
  register_audio_getvolume( g );
  register_audio_getpanning( g );
#ifdef SDLUCID_HAS_SMPEG

  register_audio_playmp3( g );
#endif
 #ifdef SDLUCID_HAS_LIBMIKMOD

  register_audio_playmod( g );
#endif

  register_audio_playwav( g );
  register_audio_setnotification( g );

  JavascriptNotificationHandler.setManager( &g.getConsole().getCommandManager() );
  audio_manager *am = g.getAudioManager();
  if ( am )
    am->addNotificationHandler( &JavascriptNotificationHandler );
}




void shutdownAudioCommands( wGame &g )
{
  JavascriptNotificationHandler.setManager( NULL );

  audio_manager *am = g.getAudioManager();
  if ( am )
    am->removeNotificationHandler( &JavascriptNotificationHandler );
}
