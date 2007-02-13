// ----------------------------------------------------------------------------
//  Description      : WAM supervisor
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_SUPERVISOR
#define WAM_SUPERVISOR




#include <vector>
#include "engine/message.hh"
#include "engine/tick.hh"
#include "engine/network.hh"




class wGameSupervisor : public wMessageReceiver, public wNetworkedGameject
{
  protected:
    typedef TUnsigned8 TMessageKind;
    static const TMessageKind MSGKIND_RESPONSE = 0; // controller -> client
    static const TMessageKind MSGKIND_ATTACH = 1; // client -> controller
    static const TMessageKind MSGKIND_ATTACH_RESPONSE = 2; // controller -> client

    static const TMessageKind MSGKIND_ADD_PLAYER_REQUEST = 10; // client -> controller
    static const TMessageKind MSGKIND_ADD_PLAYER_PROPAGATE = 11; // controller -> clients
    static const TMessageKind MSGKIND_DEACTIVATE_PLAYER = 12; // controller -> client
    static const TMessageKind MSGKIND_INSTANTIATE_PLAYER = 13; // controller -> client

    static const TMessageKind MSGKIND_GIVE_FOCUS = 20; // controller -> client
    static const TMessageKind MSGKIND_TAKE_FOCUS = 21; // controller -> client
    static const TMessageKind MSGKIND_RETURN_FOCUS = 22; // client -> controller
    static const TMessageKind MSGKIND_CYCLE_FOCUS = 23; // client -> controller

    typedef TUnsigned8 TSupervisorState;
    static const TSupervisorState PREGAME = 0;
    static const TSupervisorState GAME = 1;

    TMessageId AckMessageId, NAckMessageId;
    TMessageId GetFocusMessageId, LoseFocusMessageId, 
      ReturnFocusMessageId, CycleFocusMessageId,
      AvatarBeginDeathMessageId, AvatarRequestDeathMessageId,
      AvatarDieMessageId;

  public:
    typedef TUnsigned32 TRequestId;

  protected:
    struct wTeam
    {
      string Identifier;
      string Password;
    };

    struct wPlayer
    {
      string Identifier;
      wMessageReceiver *Gameject;

      wNetworkConnection *Connection; // controller only
      string Password; // controller only

      typedef hash_map<string,int> wKillsMap;
      wKillsMap KillsMap; // controller only
      bool CurrentlyDead,Active; // controller only
      wTeam *Team; // controller only

      wPlayer( string const &id )
          : Identifier( id ), Gameject( NULL ), Connection( NULL ),
          CurrentlyDead( false ), Active( true ), Team( NULL )
      {}
    };

    typedef vector<wTeam> wTeamList;
    typedef vector<wPlayer> wPlayerList;

    bool Controller;
    wPlayerList PlayerList;
    TRequestId NextRequestId;

    TSupervisorState State; // controller only
    wTeamList TeamList; // controller only
    bool AllowJoinAfterStart; // controller only
    string GamePassword; // controller only

    wNetworkConnection *ControlConnection; // client only
    bool Attached; // client only

  public:
    // creates a client
    wGameSupervisor( wGame &g );
    // creates a controller
    wGameSupervisor( wGame &g, string const &game_pwd, bool allow_join );

    void registerMe();
    void unregisterMe();

    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );

    void notifyNewConnection( wNetworkConnection *cnx );
    void notifyEndedConnection( wNetworkConnection *cnx );
    void notifyRemoteInitiation( wNetworkConnection *cnx, wObject::TId remote_id );

    void readMessage( wNetworkConnection *from, wStreamReader &reader );

    TRequestId requestAttach( string const &game_pwd );
    /** Request addition of a player/hero object that is controlled
     * from this (local) console.
     */
    TRequestId requestAddLocalPlayer(
      string const &identifier,
      string const &pwd,
      string const &team_id,
      string const &team_pwd );
    /** Start the game.
     */
    void startGame();

  protected:
    /** Adds a player to the list of players.
     * May only be called at the controller.
     */
    void addPlayer( string const &identifier,
                              string const &pwd,
                              string const &team_id,
                              string const &team_pwd, wNetworkConnection *cnx );
    /** Adds a player to the list of players.
     * May only be called at a client.
     */
    void propagateAddPlayer( string const &identifier );

    /** Tool function for the controller to instantiate a
     * player regardless of where the instantiation actually
     * takes place.
     */
    void instantiatePlayerController( wPlayerList::iterator ply, TGameCoordinate player_x );

    /** Create the player gameject in this instance.
     */
    void instantiatePlayerLocally( wPlayerList::iterator ply, TGameCoordinate player_x );

    void giveFocus( wPlayerList::iterator ply );
    void takeFocus( wPlayerList::iterator ply );
    void giveFocusLocally( wPlayerList::iterator ply );
    void takeFocusLocally( wPlayerList::iterator ply );

    wPlayerList::iterator getPlayer( string const &identifier );
    wPlayerList::iterator getPlayer( TId id );
    wTeamList::iterator getTeam( string const &identifier );
    TRequestId nextRequestId();
    void requestAck( TRequestId rid );
    void requestNAck( TRequestId rid, string const &reason );
    void registerMessages();
    void generateXPositions( vector<TGameCoordinate> &positions, TSize count );

    virtual void startGameHook() = 0;
    virtual void returnFocusHook( wNetworkConnection *cnx ) = 0;
    virtual void cycleFocusHook( wNetworkConnection *cnx ) = 0;
    virtual void avatarBeginDeathHook( wPlayer *ply ) = 0;
    virtual void avatarRequestDeathHook( wPlayer *ply ) = 0;
};




class wRoundsGameSupervisor : public wGameSupervisor, public wTickReceiver
{
    typedef wGameSupervisor super;

  protected:
    string CurrentTeamId;
    string CurrentPlayerId;
    float CurrentTurnSecondsRemaining;

  public:
    // creates a client
    wRoundsGameSupervisor( wGame &g );
    // creates a controller
    wRoundsGameSupervisor( wGame &g, string const &game_pwd, bool allow_join );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_ROUNDSGAME_SUPERVISOR;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    void tick( float seconds );

    void startGameHook();
    void returnFocusHook( wNetworkConnection *cnx );
    void cycleFocusHook( wNetworkConnection *cnx );
    void avatarBeginDeathHook( wPlayer *ply );
    void avatarRequestDeathHook( wPlayer *ply );

    void nextTeam();
    void cycleFocus();
};


/*
 
class wSimultaneousGameSupervisor : public wGameSupervisor {
  public:
    wSimultaneousGameSupervisor(string const &game_pwd,bool allow_join);
    
    wFeatureSet const getFeatureSet() const {
      return GJFEATURESET_SIMULTANEOUSGAME_SUPERVISOR;
      }
  };
*/



// public interface -----------------------------------------------------------
void registerSupervisor( wGame &g );




#endif
