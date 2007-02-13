// ----------------------------------------------------------------------------
//  Description      : WAM network exchange, internal header
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_NETWORK_INTERNAL
#define WAM_NETWORK_INTERNAL




#ifdef WIN32
 #include <winsock.h>
#else
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <netinet/tcp.h>
 #include <netdb.h>
#endif

#include <queue>
#include "utility/hash_map.hh"
#include "utility/manager.hh"
#include "utility/stream.hh"
#include "engine/network.hh"




// wNetworkConnection ---------------------------------------------------------
class wNetworkManager;
class wNetworkConnection
{
    typedef vector<wObject::TId> wGamejectIdList;
    typedef vector<wNetworkedGameject *> wNetworkedGamejectList;

    struct wPacket
    {
      auto_ptr<TByte> Data;
      TSize Size;

      wPacket( TByte *data, TSize size )
          : Data( data ), Size( size )
      {}
    }
    ;

    enum wConnectionState {
      NEGOTIATING_MASTER,
      NEGOTIATING_SLAVE,
      CONNECTED,

      // error conditions
      CLOSING
  };

    wConnectionState State;

    wGame &Game;
    wNetworkManager &Manager;

    int SafeSocket;
    sockaddr Name;

    queue<wPacket *> SafeTxQueue;

    typedef hash_map<wObject::TId, wObject::TId> wIdLookupTable;
    wIdLookupTable RemoteToLocalIdLookupTable;
    wIdLookupTable LocalToRemoteIdLookupTable;

    wNetworkedGamejectList ManagedGamejects;

    wStreamStore SafeReceivePacketAssembly;
    TByte *SafeSendPointer, *SafeSendPacketEnd;
    wStreamStore SafeTransmitDataPacketAssembly;

  public:
    wNetworkConnection( wGame &game, wNetworkManager &mgr, int socket,
                        sockaddr const &name, bool master );
    ~wNetworkConnection();

    string getName() const;
    void runInbound();
    void runOutbound();

    bool isManagedByThisConnection( wNetworkedGameject *gj );
    wNetworkedGameject *getObject( wObject::TId id );

    void registerObject( wNetworkedGameject *gj );
    void unregisterObject( wNetworkedGameject *gj );
    void requestChildCreation( wNetworkedGameject *gj, bool singleton );
    void registerAsChild( wNetworkedGameject *gj, wObject::TId remote_id );
    void sendMessage( wNetworkedGameject *gj, wStreamStore const &msg );

  private:
    void readSafe();
    void writeSafe();

    void sendSafePacket( wStreamStore const &store );
    void sendNegotiatePacket();
    void sendUpdate();

    void processInboundPacket( wStreamReader &reader );
    void processNegotiatePacket( wStreamReader &reader );
    void processDataPacket( wStreamReader &reader );
};





// wNetworkManager ------------------------------------------------------------
class wNetworkManager : public wRegisteringManager<wNetworkedGameject>
{
    typedef TUnsigned16 wConnectionId;
    typedef wRegisteringManager<wNetworkedGameject> Super;
    typedef vector<wNetworkedGameject *> wNetworkedGamejectList;

    wGame &Game;

    typedef vector<wNetworkConnection *> wConnectionList;
    wConnectionList ConnectionList, DeletionRequests;

    // data needed when master
    bool Master;
    int ListenSocket;

  public:
    wNetworkManager( wGame &g );
    ~wNetworkManager();
    void run();
    void connect( string const &url ); // "wam://host:port"
    void disconnect();
    void startMaster( string const &port = "" ); // "port"
    void stopMaster();

    bool isMaster() const
    {
      return Master;
    }
    TSize getConnectionCount()
    {
      return ConnectionList.size();
    }

    void requestDeletion( wNetworkConnection *connection );

    void registerObject( wNetworkedGameject *gj );
    void unregisterObject( wNetworkedGameject *gj );
    void requestChildCreation( wNetworkedGameject *gj, wNetworkConnection *except, bool singleton );
    void sendMessage( wNetworkedGameject *gj, wNetworkConnection *except,
                      wStreamStore const &msg );

  private:
    void acceptConnections();
    void purgeConnections();
};




void registerNetworkCommands( wGame &g );




#endif
