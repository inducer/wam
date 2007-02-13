// ----------------------------------------------------------------------------
//  Description      : WAM supervisor
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_random.hh>
#include "utility/debug.hh"
#include "utility/shapebitmap.hh"
#include "engine/collision.hh"
#include "engine/instancemanager.hh"
#include "engine/commandmanager.hh"
#include "hero.hh"
#include "supervisor.hh"
#include "game_names.hh"




/*
-------------------------------------------------------------------------------
network protocol used by the supervisor
-------------------------------------------------------------------------------
 
local/remote written with respect to recipient.
 
general message format:
  [u8] MSGKIND_XXXX
  rest of message
 
[ATTACH_]RESPONSE:
  [u32] request_id
  [u8] ack (0/1)
  [str] reason
 
ATTACH:
  [u32] request_id
  [str] game_pwd
 
END_GAME:
  <none>
 
ADD_PLAYER_REQUEST:
  [u32] request_id
  [str] player_id
  [str] player_pwd
  [str] team_id
  [str] team_pwd
 
ADD_PLAYER_PROPAGATE:
  [str] player_id
 
DEACTIVATE_PLAYER:
  [str] player_id  
 
INSTANTIATE_PLAYER:
  [str] player_id
  [s32] player_x
 
GIVE_FOCUS:
  [str] player_id
 
TAKE_FOCUS:
  [str] player_id
 
RETURN_FOCUS:
  <none>
  
CYCLE_FOCUS:
  <none>
*/




wGameSupervisor::wGameSupervisor( wGame &g )
    : wGameject( g ), wMessageReceiver( g ), wNetworkedGameject( g ),
    Controller( false ), NextRequestId( 0 ), ControlConnection( NULL ), Attached( false )
{
  registerMessages();
}




wGameSupervisor::wGameSupervisor( wGame &g, string const &game_pwd, bool allow_join )
    : wGameject( g ), wMessageReceiver( g ), wNetworkedGameject( g ),
    Controller( true ), NextRequestId( 0 ),
    State( PREGAME ), AllowJoinAfterStart( allow_join ), GamePassword( game_pwd )
{
  registerMessages();
}




void wGameSupervisor::registerMe()
{
  wMessageReceiver::registerMe();
  wNetworkedGameject::registerMe();
}




void wGameSupervisor::unregisterMe()
{
  wNetworkedGameject::unregisterMe();
  wMessageReceiver::unregisterMe();
}




ref<wGameSupervisor::wMessageReturn>
wGameSupervisor::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == ReturnFocusMessageId )
  {
    if ( !Controller )
    {
      wStreamStore store;
      {
        wStreamWriter writer( store );
        writer << MSGKIND_RETURN_FOCUS;
      }
      sendNetMessage( ControlConnection, NULL, store );
    }
    else
      returnFocusHook( NULL );
  }
  else if ( id == CycleFocusMessageId )
  {
    if ( !Controller )
    {
      wStreamStore store;
      {
        wStreamWriter writer( store );
        writer << MSGKIND_CYCLE_FOCUS;
      }
      sendNetMessage( ControlConnection, NULL, store );
    }
    else
      cycleFocusHook( NULL );
  }
  else if ( id == AvatarBeginDeathMessageId )
  {
    wPlayer *ply = &*getPlayer( par1 );
    avatarBeginDeathHook( ply );
  }
  else if ( id == AvatarRequestDeathMessageId )
  {
    wPlayer *ply = &*getPlayer( par1 );
    avatarRequestDeathHook( ply );
  }

  return NULL;
}




void wGameSupervisor::notifyNewConnection( wNetworkConnection *cnx )
{
  requestChildCreationAt( cnx, true );

  if ( !Controller && !ControlConnection )
    ControlConnection = cnx;
}




void wGameSupervisor::notifyEndedConnection( wNetworkConnection *cnx )
{
  if ( cnx == ControlConnection )
    ControlConnection = NULL;

  // *** CODEME deactivate all Players associated to that connection
  // propagate that deactivation
  // *** FIXME what to do if control connection goes away?
}




void wGameSupervisor::notifyRemoteInitiation( wNetworkConnection *cnx, wObject::TId remote_id )
{
  registerAsChildOf( cnx, remote_id );

  if ( !Controller && !ControlConnection )
    ControlConnection = cnx;
}




void wGameSupervisor::readMessage( wNetworkConnection *from, wStreamReader &reader )
{
  TMessageKind mkind;
  reader >> mkind;

  switch ( mkind )
  {
  case MSGKIND_ATTACH_RESPONSE:
  case MSGKIND_RESPONSE:
    {
      TRequestId rid;
      TUnsigned8 ack;
      string reason;
      reader >> rid >> ack >> reason;
      wamPrintDebugLevel( ( "sup: received response rid:" + unsigned2dec( rid ) + " ack:" + unsigned2dec( ack ) + " reason:" + reason ).c_str(), WAM_DEBUG_VERBOSE );

      if ( ack )
      {
        requestAck( rid );
        if ( mkind == MSGKIND_ATTACH_RESPONSE )
        {
          wamPrintDebugLevel( "sup: attached", WAM_DEBUG_VERBOSE );
          if ( Attached )
            EXGAME_THROWINFO( ECGAME_NET, "sup: tried to double-attach" );
	  Attached = true;
        }
      }
      else
        requestNAck( rid, reason );
      break;
    }

  case MSGKIND_ATTACH:
    {
      if ( !Controller )
        EXGAME_THROWINFO( ECGAME_NET, "sup: attach to non-controller" );

      TRequestId rid;
      string supposed_game_pwd;

      reader >> rid >> supposed_game_pwd;
      wamPrintDebugLevel( ( "sup: received attach rid:" + unsigned2dec( rid ) + " gpwd:" + supposed_game_pwd ).c_str(), WAM_DEBUG_VERBOSE );

      string reason;
      TUnsigned8 ack = 1;
      if ( supposed_game_pwd != GamePassword )
      {
        ack = 0;
        reason = "game password incorrect";
      }

      wStreamStore store;
      {
        wStreamWriter writer( store );
        writer << MSGKIND_ATTACH_RESPONSE << rid << ack << reason;
      }
      sendNetMessage( from, NULL, store );
      break;
    }

  case MSGKIND_ADD_PLAYER_REQUEST:
    {
      TRequestId rid;
      string player_id, player_pwd, team_id, team_pwd;
      reader >> rid >> player_id >> player_pwd >> team_id >> team_pwd;
      wamPrintDebugLevel( ( "sup: received add_player rid:" + unsigned2dec( rid ) + " player:" + player_id ).c_str(), WAM_DEBUG_VERBOSE );

      TUnsigned8 ack = 1;
      string reason;

      try
      {
        addPlayer( player_id, player_pwd, team_id, team_pwd, from );
      }
      catch ( string const & rsn )
      {
        ack = 0;
        reason = rsn;
      }

      wStreamStore store;
      {
        wStreamWriter writer( store );
        writer << MSGKIND_RESPONSE << rid << ack << reason;
      }
      sendNetMessage( from, NULL, store );
      break;
    }

  case MSGKIND_ADD_PLAYER_PROPAGATE:
    {
      string player_id;
      reader >> player_id;
      wamPrintDebugLevel( ( "sup: received add_player_propagate player_id:" + player_id ).c_str(), WAM_DEBUG_VERBOSE );

      propagateAddPlayer( player_id );
      break;
    }

  case MSGKIND_INSTANTIATE_PLAYER:
    {
      string player_id;
      TSigned32 player_x;
      reader >> player_id >> player_x;
      wamPrintDebugLevel( ( "sup: received instantiate_player player_id:" + player_id ).c_str(), WAM_DEBUG_VERBOSE );

      wPlayerList::iterator ply;
      try
      {
	ply = getPlayer( player_id );
	instantiatePlayerLocally( ply, player_x );
      }
      catch ( wGameException )
      {
        EXGAME_THROWINFO( ECGAME_NET, ( "sup: can't instantiate unknown player " + player_id ).c_str() )
      }
      break;
    }

  case MSGKIND_GIVE_FOCUS:
    {
      string player_id;
      reader >> player_id;
      wamPrintDebugLevel( "sup: received give_focus " + player_id, WAM_DEBUG_VERBOSE );
      giveFocusLocally( getPlayer( player_id ) );
      break;
    }

  case MSGKIND_TAKE_FOCUS:
    {
      string player_id;
      reader >> player_id;
      wamPrintDebugLevel( "sup: received take_focus " + player_id, WAM_DEBUG_VERBOSE );
      takeFocusLocally( getPlayer( player_id ) );
      break;
    }

  case MSGKIND_RETURN_FOCUS:
    wamPrintDebugLevel( "sup: received return_focus", WAM_DEBUG_VERBOSE );
    returnFocusHook( from );
    break;

  case MSGKIND_CYCLE_FOCUS:
    wamPrintDebugLevel( "sup: received cycle_focus", WAM_DEBUG_VERBOSE );
    cycleFocusHook( from );
    break;

  default:
    // *** FIXME potential DOS
    EXGAME_THROWINFO( ECGAME_NET, "sup: protocol error" );
  }
}




wGameSupervisor::TRequestId wGameSupervisor::requestAttach( string const &game_pwd )
{
  if ( Controller )
    EXGAME_THROWINFO( ECGAME_NET, "sup: controller can't attach" );
  if ( ControlConnection == NULL )
    EXGAME_THROWINFO( ECGAME_NET, "sup: can't attach without net connection" );

  TRequestId rid = nextRequestId();

  wStreamStore store;
  {
    wStreamWriter writer( store );
    writer
    << MSGKIND_ATTACH
    << rid
    << game_pwd;
  }
  sendNetMessage( ControlConnection, NULL, store );
  return rid;
}




wGameSupervisor::TRequestId wGameSupervisor::requestAddLocalPlayer(
  string const &identifier,
  string const &pwd,
  string const &team_id,
  string const &team_pwd )
{
  if ( Controller )
  {
    if ( State != PREGAME && State != GAME )
      EXGAME_THROWINFO( ECGAME_NET, "sup: can only add players in pregame and game" );

    try
    {
      addPlayer( identifier, pwd, team_id, team_pwd, NULL );
      TRequestId rid = nextRequestId();
      requestAck( rid );
      return rid;
    }
    catch ( string const & reason )
    {
      TRequestId rid = nextRequestId();
      requestNAck( rid, reason );
      return rid;
    }
  }
  else
  {
    if ( !Attached )
      EXGAME_THROWINFO( ECGAME_NET, "sup: can only add players if attached to controller" );

    TRequestId rid = nextRequestId();

    wStreamStore store;
    {
      wStreamWriter writer( store );
      writer
	<< MSGKIND_ADD_PLAYER_REQUEST
	<< rid
	<< identifier
	<< pwd
	<< team_id
	<< team_pwd;
    }

    sendNetMessage( ControlConnection, NULL, store );
    return rid;
  }
}




void wGameSupervisor::startGame()
{
  if ( !Controller )
    EXGAME_THROWINFO( ECGAME_NET, "sup: non-controller can't start game" );
  if ( State != PREGAME )
    EXGAME_THROWINFO( ECGAME_NET, "sup: can't start game unless in pregame" );
  if ( PlayerList.size() == 0 )
    EXGAME_THROWINFO( ECGAME_NET, "sup: can't start game unless there is a player present" );

  vector<TGameCoordinate> x_positions;
  generateXPositions( x_positions, PlayerList.size() );
  vector<TGameCoordinate>::iterator first_p = x_positions.begin();

  State = GAME;
  FOREACH( first, PlayerList, wPlayerList )
  {
    // don't instantiate inactive Players
    if ( !first->Active )
    {
      first_p++;
      continue;
    }

    instantiatePlayerController( first, *first_p );
    first_p++;
  }

  startGameHook();
}




void wGameSupervisor::addPlayer( string const &identifier,
    string const &pwd,
    string const &team_id,
    string const &team_pwd,
    wNetworkConnection *cnx )
{
  if ( State != PREGAME && State != GAME )
    throw string( "can only add players in pregame and game" );

  if ( identifier == "" )
    throw string( "cannot add players with empty name" );

  if ( !Controller )
    throw string( "only controller can decide player addition" );

  // check passwords

  wPlayerList::iterator ply;
  bool player_present = false;

  try
  {
    ply = getPlayer( identifier );
    player_present = true;

    wamPrintDebug( "found existing player " + identifier );
    if ( ply->Active )
      throw string( "player with that name has already joined" );

    if ( pwd != ply->Password )
      throw string( "player password incorrect" );
  }
  catch ( wGameException )
  {
    if ( State == GAME && !AllowJoinAfterStart )
      throw string( "not allowed to join after game has started" );
  }

  wTeamList::iterator team;
  try
  {
    team = getTeam( team_id );

    if ( team_pwd != team->Password )
      throw string( "team password incorrect" );
  }
  catch ( wGameException ) 
  {
    wTeam new_team;

    new_team.Identifier = team_id;
    new_team.Password = team_pwd;

    TeamList.push_back( new_team );
    team = TeamList.end() - 1;
  }

  if ( !player_present )
  {
    PlayerList.push_back( wPlayer( identifier ) );
    ply = PlayerList.end() - 1;

    ply->Connection = cnx;
    ply->Password = pwd;
    ply->Team = &*team;
  }
  else
  {
    ply->Connection = cnx;
    ply->Active = true;
  }

  // propagate everywhere except originating connection
  wStreamStore store;
  {
    wStreamWriter writer( store );
    writer << MSGKIND_ADD_PLAYER_PROPAGATE << ply->Identifier;
  }
  sendNetMessage( NULL, NULL, store );

  // instantiate
  if ( State == GAME )
  {
    vector<TGameCoordinate> x_positions;
    generateXPositions( x_positions, 1 );
    instantiatePlayerController( ply, x_positions[ 0 ] );
  }
}




void wGameSupervisor::propagateAddPlayer( string const &identifier )
{
  if ( Controller )
    EXGAME_THROWINFO( ECGAME_NET, "sup: propagateAddPlayer called on controller" );

  try
  {
    getPlayer( identifier );
  }
  catch ( wGameException )
  {
    PlayerList.push_back( wPlayer( identifier ) );
  }
}




void wGameSupervisor::instantiatePlayerController( wPlayerList::iterator ply, TGameCoordinate player_x )
{
  if ( ply->Connection )
  {
    // *** create the remote gameject

    wStreamStore store;
    {
      wStreamWriter writer( store );
      writer
      << MSGKIND_INSTANTIATE_PLAYER
      << ply->Identifier
      << ( TSigned32 ) player_x;
    }
    sendNetMessage( ply->Connection, NULL, store );
  }
  else
  {
    // *** create the gameject locally
    instantiatePlayerLocally( ply, player_x );
  }
}




void wGameSupervisor::instantiatePlayerLocally( wPlayerList::iterator ply, TGameCoordinate player_x )
{
  if ( ply->Gameject )
    EXGAME_THROWINFO( ECGAME_NET, "sup: player already instantiated" );

  auto_ptr<wHero> hero( new wHero( Game ) );
  hero->setupHero( ply->Identifier, player_x, 100 );
  Game.getInstanceManager().insertObject( hero.get() );
  ply->Gameject = hero.release();
}




void wGameSupervisor::giveFocus( wPlayerList::iterator ply )
{
  if ( Controller && ply->Connection )
  {
    wStreamStore store;
    {
      wStreamWriter writer( store );
      writer << MSGKIND_GIVE_FOCUS << ply->Identifier;
    }
    sendNetMessage( ply->Connection, NULL, store );
  }
  else
    giveFocusLocally( ply );
}




void wGameSupervisor::takeFocus( wPlayerList::iterator ply )
{
  if ( Controller && ply->Connection )
  {
    wStreamStore store;
    {
      wStreamWriter writer( store );
      writer << MSGKIND_TAKE_FOCUS << ply->Identifier;
    }
    sendNetMessage( ply->Connection, NULL, store );
  }
  else
    takeFocusLocally( ply );
}




void wGameSupervisor::giveFocusLocally( wPlayerList::iterator ply )
{
  Game.getMessageManager().postTo( ply->Gameject, GetFocusMessageId );
}




void wGameSupervisor::takeFocusLocally( wPlayerList::iterator ply )
{
  Game.getMessageManager().postTo( ply->Gameject, LoseFocusMessageId );
}




wGameSupervisor::wPlayerList::iterator
wGameSupervisor::getPlayer( string const &identifier )
{
  FOREACH( first, PlayerList, wPlayerList )
    if ( identifier == first->Identifier )
      return first;
  EXGAME_THROWINFO( ECGAME_NET, "sup: invalid player id " + identifier );
}




wGameSupervisor::wPlayerList::iterator
wGameSupervisor::getPlayer( TId id )
{
  FOREACH( first, PlayerList, wPlayerList )
    if ( id == first->Gameject->id() )
      return first;
  EXGAME_THROWINFO( ECGAME_NET, "sup: invalid player id " + unsigned2dec( id ) );
}




wGameSupervisor::wTeamList::iterator
wGameSupervisor::getTeam( string const &identifier )
{
  FOREACH( first, TeamList, wTeamList )
  if ( identifier == first->Identifier )
    return first;
  EXGAME_THROWINFO( ECGAME_NET, "sup: invalid team id " + identifier );
}




wGameSupervisor::TRequestId wGameSupervisor::nextRequestId()
{
  return NextRequestId++;
}




void wGameSupervisor::requestAck( TRequestId rid )
{
  Game.getMessageManager().post( AckMessageId, unsigned2dec( rid ) );
}




void wGameSupervisor::requestNAck( TRequestId rid, string const &reason )
{
  Game.getMessageManager().post( NAckMessageId, unsigned2dec( rid ) + ":" + reason );
}





void wGameSupervisor::registerMessages()
{
  AckMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_ACKNOWLEDGE );
  NAckMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_REJECT );
  GetFocusMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_GET_FOCUS );
  LoseFocusMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_LOSE_FOCUS );
  ReturnFocusMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_RETURN_FOCUS );
  CycleFocusMessageId = Game.getMessageManager().registerMessage( MSG_CYCLE_FOCUS );
  AvatarBeginDeathMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_AVATAR_BEGIN_DEATH );
  AvatarRequestDeathMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_AVATAR_REQUEST_DEATH );
  AvatarDieMessageId = Game.getMessageManager().registerMessage( MSG_SUPERVISOR_AVATAR_DIE );
}




void wGameSupervisor::generateXPositions( vector<TGameCoordinate> &positions, TSize count )
{
  positions.clear();
  wShapeBitmap game_bits;
  Game.getCollisionManager().dumpBitmap( game_bits );

  int_random rng;
  rng.init();

  game_bits.setDrawMode( wShapeBitmap::DETECT );
  TGameCoordinate width = game_bits.width();
  if ( width == 0 )
    EXGAME_THROWINFO( ECGAME_NET, "sup: cannot generate x positions for zero-width playing field" );

  while ( count-- )
  {
    int attempts = 2000;

    TGameCoordinate x;
    while ( true )
    {
      // make sure there's some form of floor here, so players don't just
      // "drop out".

      x = rng( game_bits.width() );
      game_bits.resetDetector();
      game_bits.drawVLine( x, 0, game_bits.height() );
      if ( !game_bits.getDetector() )
        continue;

      // make sure players have a minimum distance to each other.
      bool invalid = false;
      FOREACH_CONST( first, positions, vector<TGameCoordinate> )
      {
        if ( NUM_ABS( *first - x ) < 15 )
        {
          invalid = true;
          break;
        }
      }
      if ( !invalid )
        break;

      if ( --attempts == 0 )
	EXGAME_THROWINFO( ECGAME_NET, "sup: could not find a valid player distribution" );
    }

    positions.push_back( x - game_bits.referencePointX() );
  }
}




// wRoundsGameSupervisor ------------------------------------------------------
wRoundsGameSupervisor::wRoundsGameSupervisor( wGame &g )
  : wGameject( g ), super( g ), wTickReceiver( g )
{}




wRoundsGameSupervisor::wRoundsGameSupervisor( wGame &g, string const &game_pwd, bool allow_join )
    : wGameject( g ), super( g, game_pwd, allow_join ), wTickReceiver( g )
{}




void wRoundsGameSupervisor::registerMe()
{
  super::registerMe();
  wTickReceiver::registerMe();
}




void wRoundsGameSupervisor::startUp()
{
  super::startUp();
  wTickReceiver::startUp();
}




void wRoundsGameSupervisor::prepareToDie()
{
  wTickReceiver::prepareToDie();
  super::prepareToDie();
}




void wRoundsGameSupervisor::unregisterMe()
{
  wTickReceiver::unregisterMe();
  super::unregisterMe();
}




void wRoundsGameSupervisor::tick( float seconds )
{
}




void wRoundsGameSupervisor::startGameHook()
{
  CurrentTeamId = TeamList[0].Identifier;
  cycleFocus();
}




void wRoundsGameSupervisor::returnFocusHook( wNetworkConnection *cnx )
{
  nextTeam();
}




void wRoundsGameSupervisor::cycleFocusHook( wNetworkConnection *cnx )
{
  if ( State != GAME )
    return;

  // determine if the cycleFocus is legitimate
  try
  {
    wPlayerList::iterator it;
    it = getPlayer( CurrentPlayerId );
    if ( it->Active && it->Connection == cnx )
      cycleFocus();
    else
      return;
  }
  catch ( wGameException )
  {
    cycleFocus();
  }
}




void wRoundsGameSupervisor::avatarBeginDeathHook( wPlayer *ply )
{
  ply->CurrentlyDead = true;

  if ( ply->Identifier == CurrentPlayerId )
    nextTeam();
}




void wRoundsGameSupervisor::avatarRequestDeathHook( wPlayer *ply )
{
  Game.getMessageManager().sendTo( ply->Gameject, AvatarDieMessageId );
  ply->Gameject = NULL;
}




void wRoundsGameSupervisor::nextTeam()
{
  cycleFocus();
}




void wRoundsGameSupervisor::cycleFocus()
{
  if ( PlayerList.begin() == PlayerList.end() )
    EXGAME_THROWINFO( ECGAME_NET, "sup: can't cycle focus: no players" );

  wPlayerList::iterator last_focused;
  try
  {
    last_focused = getPlayer( CurrentPlayerId );
    takeFocus( last_focused );
  }
  catch ( wGameException )
  {
    last_focused = PlayerList.end() - 1;
  }

  wPlayerList::iterator it = last_focused + 1;
  if ( it == PlayerList.end() )
    it = PlayerList.begin();

  while ( it != last_focused )
  {
    if ( it->Active && !it->CurrentlyDead && it->Team->Identifier == CurrentTeamId )
      break;

    it++;
    if ( it == PlayerList.end() )
      it = PlayerList.begin();
  }

  if ( it->Active && !it->CurrentlyDead && it->Team->Identifier == CurrentTeamId )
  {
    giveFocus( it );
    CurrentPlayerId = it->Identifier;
  }
  else
  {
     // there was no one to give the focus to - game needs to end
  }
}




// scripting interface --------------------------------------------------------
#define FIND_SUP \
  wGameSupervisor *sup = dynamic_cast<wGameSupervisor *>( \
    Game.getInstanceManager().getObjectByFeatureSet(GJFEATURE_SUPERVISOR));
// throws exception if not found

WAM_DECLARE_COMMAND( sup_create_rounds, GJFEATURE_MPACK_WAM "SvCR", "create controller role rounds supvervisor", "game_pwd allow_join" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "sup_create_rounds", 2, 2 )

  wGameject * sv = new wRoundsGameSupervisor( Game, parameters[ 0 ] ->toString(), parameters[ 1 ] ->toBoolean() );
  Game.getInstanceManager().insertObject( sv );
  return makeConstant( sv->id() );
}




WAM_DECLARE_COMMAND( sup_attach, GJFEATURE_MPACK_WAM "SvAt", "try to attach to controller", "game_pwd" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "sup_attach", 1, 1 )

  FIND_SUP

  return makeConstant( sup->requestAttach( parameters[ 0 ] ->toString() ) );
}




WAM_DECLARE_COMMAND( sup_addplayer, GJFEATURE_MPACK_WAM "SvAP", "try to add a player", "player_id player_pwd team_id team_pwd" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "sup_addplayer", 4, 4 )

  FIND_SUP

  return makeConstant( sup->requestAddLocalPlayer(
                         parameters[ 0 ] ->toString(),
                         parameters[ 1 ] ->toString(),
                         parameters[ 2 ] ->toString(),
                         parameters[ 3 ] ->toString()
                       ) );
}




WAM_DECLARE_COMMAND( sup_start, GJFEATURE_MPACK_WAM "SvSt", "start the game", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "sup_start", 0, 0 )

  FIND_SUP
  sup->startGame();
  return makeNull();
}




// registration ---------------------------------------------------------------
static wGameject *createRoundsGameSupervisor( wGame &g )
{
  return new wRoundsGameSupervisor( g );
}




/*static wGameject *createSimultaneousGameSupervisor(wGame &g) {
  return new wSimultaneousGameSupervisor(g);
  }*/




void registerSupervisor( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_ROUNDSGAME_SUPERVISOR, "", createRoundsGameSupervisor );
  // g.getGameCodeManager().registerClass(GJFEATURESET_SIMULTANEOUSGAME_SUPERVISOR,"",createSimultaneousGameSupervisor);

  register_sup_create_rounds( g );
  register_sup_attach( g );
  register_sup_addplayer( g );
  register_sup_start( g );
}
