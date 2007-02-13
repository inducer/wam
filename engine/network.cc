// ----------------------------------------------------------------------------
//  Description      : WAM network exchange
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------



#include <algorithm>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ixlib_re.hh>
#include "utility/debug.hh"
#include "utility/manager_impl.hh"
#include "engine/commandmanager.hh"
#include "engine/network_internal.hh"




/*
-------------------------------------------------------------------------------
WAM uses the following network protocol: 
-------------------------------------------------------------------------------
* [u32] something = something is an unsigned 32 bit field
* all values are transmitted in network byte order.
* "remote" and "local" terminology is written with respect to the
  recipient of the packet.
* "generation" numbers mean the number of safe packets that should have
  been received before this packet should be processed
* sequence numbers start out at 0 for every generation
 
the following defines the transport media for the safe connection.
<safe_packet>:
  [u16] packet size excluding this value
  <initiate_packet>|<response_packet>|<safe_data_packet>
 
<initiate_packet>:
  [u8] reserved == 0
  [u32] WAM_NET_MAGIC_NUMBER
  [u8] protocol revision (increment for incompatible changes)
  [u8] minor protocol revision
 
<response_packet>:
  [u8] 0 == connection acknowledge, everything else fail
  [u32] WAM_NET_MAGIC_NUMBER
  [u8] protocol revision (increment for incompatible changes)
  [u8] minor protocol revision
 
<safe_data_packet>:
  (<creation>|<message>|<parent_message>)*
  [u8] WAM_NET_END
 
<creation>
  [u8] WAM_NET_CREATE
  [u8] create mode
  [u32] remote gameject id
  [string] gameject class in internal representation
 
<message>
  [u8] WAM_NET_MESSAGE
  [u32] remote gameject id
  <message>
 
<parent_message>
  [u8] WAM_NET_PARENT_MESSAGE
  [u32] local gameject id
  <message>
 
-------------------------------------------------------------------------------
*/




static const TUnsigned32 WAM_NET_MAGIC_NUMBER = 0xfadebabeL;
static const TUnsigned8 WAM_NET_PROTOCOL_REVISION = 2;
static const TUnsigned8 WAM_NET_PROTOCOL_MINOR_REVISION = 0;

static const TUnsigned8 WAM_NET_END = 0;
static const TUnsigned8 WAM_NET_CREATE = 1;
static const TUnsigned8 WAM_NET_MESSAGE = 2;
static const TUnsigned8 WAM_NET_PARENT_MESSAGE = 3;

static const TUnsigned8 WAM_NET_CREATE_MODE_NORMAL = 0;
static const TUnsigned8 WAM_NET_CREATE_MODE_SINGLETON = 1;

#define WAM_DEFAULT_PORT    24197
#define WAM_NET_QUEUE_SIZE    50

#define WAM_NET_FFLAG_CONTINUED    1




#ifdef WIN32

namespace
{
void CloseSocket( int socket )
{
  closesocket( socket );
}

int getLastSocketError()
{
  return WSAGetLastError();
}

string stringifyLastSocketError()
{
  int error = getLastSocketError();
  switch ( error )
  {
  case WSAEINTR:
    return "WSAEINTR";
  case WSAEBADF:
    return "WSAEBADF";
  case WSAEACCES:
    return "WSAEACCES";
  case WSAEFAULT:
    return "WSAEFAULT";
  case WSAEINVAL:
    return "WSAEINVAL";
  case WSAEMFILE:
    return "WSAEMFILE";
  case WSAEWOULDBLOCK:
    return "WSAEWOULDBLOCK";
  case WSAEINPROGRESS:
    return "WSAEINPROGRESS";
  case WSAEALREADY:
    return "WSAEALREADY";
  case WSAENOTSOCK:
    return "WSAENOTSOCK";
  case WSAEDESTADDRREQ:
    return "WSAEDESTADDRREQ";
  case WSAEMSGSIZE:
    return "WSAEMSGSIZE";
  case WSAEPROTOTYPE:
    return "WSAEPROTOTYPE";
  case WSAENOPROTOOPT:
    return "WSAENOPROTOOPT";
  case WSAEPROTONOSUPPORT:
    return "WSAEPROTONOSUPPORT";
  case WSAESOCKTNOSUPPORT:
    return "WSAESOCKTNOSUPPORT";
  case WSAEOPNOTSUPP:
    return "WSAEOPNOTSUPP";
  case WSAEPFNOSUPPORT:
    return "WSAEPFNOSUPPORT";
  case WSAEAFNOSUPPORT:
    return "WSAEAFNOSUPPORT";
  case WSAEADDRINUSE:
    return "WSAEADDRINUSE";
  case WSAEADDRNOTAVAIL:
    return "WSAEADDRNOTAVAIL";
  case WSAENETDOWN:
    return "WSAENETDOWN";
  case WSAENETUNREACH:
    return "WSAENETUNREACH";
  case WSAENETRESET:
    return "WSAENETRESET";
  case WSAECONNABORTED:
    return "WSAECONNABORTED";
  case WSAECONNRESET:
    return "WSAECONNRESET";
  case WSAENOBUFS:
    return "WSAENOBUFS";
  case WSAEISCONN:
    return "WSAEISCONN";
  case WSAENOTCONN:
    return "WSAENOTCONN";
  case WSAESHUTDOWN:
    return "WSAESHUTDOWN";
  case WSAETOOMANYREFS:
    return "WSAETOOMANYREFS";
  case WSAETIMEDOUT:
    return "WSAETIMEDOUT";
  case WSAECONNREFUSED:
    return "WSAECONNREFUSED";
  case WSAELOOP:
    return "WSAELOOP";
  case WSAENAMETOOLONG:
    return "WSAENAMETOOLONG";
  case WSAEHOSTDOWN:
    return "WSAEHOSTDOWN";
  case WSAEHOSTUNREACH:
    return "WSAEHOSTUNREACH";
  case WSAENOTEMPTY:
    return "WSAENOTEMPTY";
  case WSAEPROCLIM:
    return "WSAEPROCLIM";
  case WSAEUSERS:
    return "WSAEUSERS";
  case WSAEDQUOT:
    return "WSAEDQUOT";
  case WSAESTALE:
    return "WSAESTALE";
  case WSAEREMOTE:
    return "WSAEREMOTE";
  case WSAEDISCON:
    return "WSAEDISCON";
  default:
    return "unknown winsock error, id " + signed2dec( error );
  }
}
}

#define socklen_t  int
 #define sockopt_data_t char
#else
namespace
{
void CloseSocket( int socket )
{
  close( socket );
}

int getLastSocketError()
{
  return errno;
}

string stringifyLastSocketError()
{
  return strerror( getLastSocketError() );
}
}

#define sockopt_data_t int
 #define WSAEADDRINUSE  EADDRINUSE
 #define WSAEWOULDBLOCK EWOULDBLOCK
 #define WSAECONNRESET  ECONNRESET
#endif




#define THROW_NET_ERRNO(MESSAGE) \
  EXGAME_THROWINFO(ECGAME_NET,(string(MESSAGE ": ")+stringifyLastSocketError()).c_str())




// tool functions -------------------------------------------------------------
namespace
{
string sockaddr2string( sockaddr const &name )
{
  in_addr * addr = &( ( ( sockaddr_in * ) & name ) ->sin_addr );
  return inet_ntoa( *addr );
}

void enableNagle( int socket, bool nagle_enabled )
{
  int nodelay_enabled = !nagle_enabled;
  setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, ( sockopt_data_t * ) & nodelay_enabled, sizeof( nodelay_enabled ) );
}

void setNonBlockMode( int fd, bool enable )
{
#ifdef WIN32
  u_long enable_int = enable;
  ioctlsocket( fd, FIONBIO, &enable_int );
#else

  int oldflags = fcntl( fd, F_GETFL, 0 );
  if ( enable )
    fcntl( fd, F_SETFL, oldflags | O_NONBLOCK );
  else
    fcntl( fd, F_SETFL, oldflags & ~O_NONBLOCK );
#endif

}
}




// wNetworkedGameject ---------------------------------------------------------
wNetworkedGameject::wNetworkedGameject( wGame &g )
    : wGameject( g )
{}




void wNetworkedGameject::registerMe()
{
  Game.getNetworkManager().registerObject( this );
}




void wNetworkedGameject::unregisterMe()
{
  Game.getNetworkManager().unregisterObject( this );
}




bool wNetworkedGameject::atMaster() const
{
  return Game.getNetworkManager().isMaster();
}




void wNetworkedGameject::requestChildCreationAt( wNetworkConnection *at, bool singleton )
{
  at->requestChildCreation( this, singleton );
}




void wNetworkedGameject::requestChildCreation( wNetworkConnection *except, bool singleton )
{
  Game.getNetworkManager().requestChildCreation( this, except, singleton );
}




void wNetworkedGameject::registerAsChildOf( wNetworkConnection *cnx, wObject::TId remote_id )
{
  cnx->registerAsChild( this, remote_id );
}




void wNetworkedGameject::sendNetMessage( wNetworkConnection *to, wNetworkConnection *except, wStreamStore const &msg )
{
  if ( to )
  {
    if ( to != except )
      to->sendMessage( this, msg );
  }
  else
    Game.getNetworkManager().sendMessage( this, except, msg );
}




// wNetworkConnection ---------------------------------------------------------
wNetworkConnection::wNetworkConnection( wGame &game, wNetworkManager &mgr, int socket,
                                        sockaddr const &name, bool master )
    : Game( game ), Manager( mgr ), SafeSocket( socket ), Name( name ),
    SafeSendPointer( NULL )
{

  setNonBlockMode( SafeSocket, true );
  enableNagle( SafeSocket, false );

  if ( master )
    State = NEGOTIATING_MASTER;
  else
    State = NEGOTIATING_SLAVE;

  wamPrintDebug( ( "net: connection with " + getName() ).c_str() );

  if ( State == NEGOTIATING_SLAVE )
    sendNegotiatePacket();
}




wNetworkConnection::~wNetworkConnection()
{
  FOREACH( first, Manager, wNetworkManager )
  ( *first ) ->notifyEndedConnection( this );

  CloseSocket( SafeSocket );
  wamPrintDebug( ( "net: ended connection with " + getName() ).c_str() );

  while ( SafeTxQueue.size() )
  {
    delete SafeTxQueue.front();
    SafeTxQueue.pop();
  }
}




string wNetworkConnection::getName() const
{
  return sockaddr2string( Name );
}




void wNetworkConnection::runInbound()
{
  readSafe();
}




void wNetworkConnection::runOutbound()
{
  if ( State == CONNECTED )
    sendUpdate();
  writeSafe();
}




bool wNetworkConnection::isManagedByThisConnection( wNetworkedGameject *gj )
{
  wNetworkedGamejectList::iterator it =
    find( ManagedGamejects.begin(), ManagedGamejects.end(), gj );
  return it != ManagedGamejects.end();
}




wNetworkedGameject *wNetworkConnection::getObject( wObject::TId id )
{
  FOREACH_CONST( first, ManagedGamejects, wNetworkedGamejectList )
  if ( ( *first ) ->id() == id )
    return * first;
  EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "managed gameject having id " + unsigned2dec( id ) ).c_str() )
}




void wNetworkConnection::registerObject( wNetworkedGameject *gj )
{
  if ( State != CONNECTED )
    return ;

  // if it's managed by this connection, very well, there it is.
  // if it's not managed by this connection, tell it that we exist.
  if ( !isManagedByThisConnection( gj ) )
    gj->notifyNewConnection( this );
}




void wNetworkConnection::unregisterObject( wNetworkedGameject *gj )
{
  if ( State != CONNECTED )
    return ;

  wNetworkedGamejectList::iterator it =
    find( ManagedGamejects.begin(), ManagedGamejects.end(), gj );

  // if it was managed by this connection, very well, kill its traces.
  if ( it != ManagedGamejects.end() )
  {
    ManagedGamejects.erase( it );
    LocalToRemoteIdLookupTable.erase( gj->id() );

    FOREACH( first, RemoteToLocalIdLookupTable, wIdLookupTable )
    if ( first->second == gj->id() )
    {
      RemoteToLocalIdLookupTable.erase( first );
      break;
    }
  }
}




void wNetworkConnection::requestChildCreation( wNetworkedGameject *gj, bool singleton )
{
  if ( State != CONNECTED )
    return ;
  wStreamWriter writer( SafeTransmitDataPacketAssembly );
  writer
  << WAM_NET_CREATE
  << ( TUnsigned8 ) ( singleton ? WAM_NET_CREATE_MODE_SINGLETON : WAM_NET_CREATE_MODE_NORMAL )
  << ( TUnsigned32 ) gj->id()
  << string( gj->getFeatureSet() );
}




void wNetworkConnection::registerAsChild( wNetworkedGameject *gj, wObject::TId remote_id )
{
  RemoteToLocalIdLookupTable[ remote_id ] = gj->id();
  LocalToRemoteIdLookupTable[ gj->id() ] = remote_id;

  ManagedGamejects.push_back( gj );
}




void wNetworkConnection::sendMessage( wNetworkedGameject *gj, wStreamStore const &msg )
{
  if ( State != CONNECTED )
    return ;

  if ( isManagedByThisConnection( gj ) )
  {
    wStreamWriter writer( SafeTransmitDataPacketAssembly );

    writer
    << WAM_NET_PARENT_MESSAGE
    << ( TUnsigned32 ) LocalToRemoteIdLookupTable[ gj->id() ];
    insert( writer, msg );
  }
  else
  {
    wStreamWriter writer( SafeTransmitDataPacketAssembly );

    writer
    << WAM_NET_MESSAGE
    << ( TUnsigned32 ) gj->id();
    insert( writer, msg );
  }
}




void wNetworkConnection::readSafe()
{
  int const proposed_read_size = 4096;
  int read_size;

  // read as much as possible off the wire
  do
  {
    SafeReceivePacketAssembly.enlargeCapacity(
      SafeReceivePacketAssembly.usedSize() + proposed_read_size );

    read_size = recv( SafeSocket, ( char * ) SafeReceivePacketAssembly.end(), proposed_read_size, 0 );

    // did recv result in an error?
    if ( read_size == -1 )
    {
      if ( getLastSocketError() == WSAEWOULDBLOCK )
        break;
      if ( getLastSocketError() == WSAECONNRESET )
      {
        wamPrintDebug( ( "net: connection reset by peer " + getName() ).c_str() );
        Manager.requestDeletion( this );
        State = CLOSING;
        return ;
      }
      THROW_NET_ERRNO( "error while reading from socket" )
    }

    // did we get an eof?
    if ( read_size == 0 )
    {
      wamPrintDebug( ( "net: peer " + getName() + " closed connection" ).c_str() );
      Manager.requestDeletion( this );
      State = CLOSING;
      return ;
    }

    SafeReceivePacketAssembly.setUsedSize(
      SafeReceivePacketAssembly.usedSize() + read_size );
  }
  while ( read_size == proposed_read_size );

  // process packets
  while ( true )
  {
    // extract packet size
    TSize recv_packet_size;
    if ( SafeReceivePacketAssembly.usedSize() >= sizeof( TUnsigned16 ) )
      recv_packet_size = ntohs( *( ( TUnsigned16 * ) SafeReceivePacketAssembly.get() ) );
    else
      return ;

    // do we have a complete packet?
    if ( SafeReceivePacketAssembly.usedSize() - sizeof( TUnsigned16 ) >= recv_packet_size )
    {
      wStreamReader reader( SafeReceivePacketAssembly, sizeof( TUnsigned16 ) );
      processInboundPacket( reader );
      memmove( SafeReceivePacketAssembly.get(),
               SafeReceivePacketAssembly.get() + sizeof( TUnsigned16 ) + recv_packet_size,
               SafeReceivePacketAssembly.usedSize() - recv_packet_size );

      SafeReceivePacketAssembly.setUsedSize(
        SafeReceivePacketAssembly.usedSize() - ( sizeof( TUnsigned16 ) + recv_packet_size )
      );
    }
    else
      return ;
  }
}




void wNetworkConnection::writeSafe()
{
  int written_size;
  do
  {
    if ( !SafeSendPointer )
    {
      if ( SafeTxQueue.size() == 0 )
        return ;
      SafeSendPointer = SafeTxQueue.front() ->Data.get();
      SafeSendPacketEnd = SafeSendPointer + SafeTxQueue.front() ->Size;
    }

    written_size = send( SafeSocket, ( char * ) SafeSendPointer, SafeSendPacketEnd - SafeSendPointer, 0 );

    // error? retry later.
    if ( written_size == -1 )
    {
      if ( getLastSocketError() == WSAEWOULDBLOCK )
        return ;
      THROW_NET_ERRNO( "error while writing to socket" )
    }

    SafeSendPointer += written_size;
    if ( SafeSendPointer == SafeSendPacketEnd )
    {
      SafeSendPointer = NULL;
      SafeTxQueue.pop();
    }
  }
  while ( written_size );
}




void wNetworkConnection::sendSafePacket( wStreamStore const &store )
{
  TSize packet_size = store.usedSize() + sizeof( TUnsigned16 );
  auto_ptr<TByte> packet_data( new TByte[ packet_size ] );

  TUnsigned16 size = htons( store.usedSize() );
  memcpy( packet_data.get(), &size, sizeof( size ) );
  memcpy( packet_data.get() + sizeof( size ), store.get(), store.usedSize() );

  auto_ptr<wPacket> packet( new wPacket( packet_data.get(), packet_size ) );
  packet_data.release();

  SafeTxQueue.push( packet.get() );
  packet.release();
}




void wNetworkConnection::sendNegotiatePacket()
{
  wStreamStore store( 200 );
  wStreamWriter writer( store );

  writer << ( TUnsigned8 ) 0;
  writer << WAM_NET_MAGIC_NUMBER;
  writer << WAM_NET_PROTOCOL_REVISION;
  writer << WAM_NET_PROTOCOL_MINOR_REVISION;
  writer.commit();

  sendSafePacket( store );
}




void wNetworkConnection::sendUpdate()
{
  if ( SafeTransmitDataPacketAssembly.usedSize() != 0 )
  {
    {
      wStreamWriter writer( SafeTransmitDataPacketAssembly );
      writer << WAM_NET_END;
    }

    sendSafePacket( SafeTransmitDataPacketAssembly );
    SafeTransmitDataPacketAssembly.clear();
  }
}




void wNetworkConnection::processInboundPacket( wStreamReader &reader )
{
  if ( State == NEGOTIATING_MASTER || State == NEGOTIATING_SLAVE )
    processNegotiatePacket( reader );
  else if ( State == CONNECTED )
    processDataPacket( reader );
  else
    wamPrintDebug( ( "net: ignored packet from " + getName() ).c_str() );
}





void wNetworkConnection::processNegotiatePacket( wStreamReader &reader )
{
  TUnsigned8 dummy, revision_major, revision_minor;
  TUnsigned32 magic;
  reader >> dummy;
  reader >> magic;
  reader >> revision_major;
  reader >> revision_minor;

  if ( magic != WAM_NET_MAGIC_NUMBER ||
       revision_major != WAM_NET_PROTOCOL_REVISION )
  {
    wamPrintDebug( ( "net: protocol error with peer " + getName() ).c_str() );
    Manager.requestDeletion( this );
    State = CLOSING;
    return ;
  }

  wamPrintDebug( ( "net: negotiated connection with " + getName() + " protocol version " +
                   unsigned2dec( revision_major ) + "." + unsigned2dec( revision_minor ) ).c_str() );

  if ( State == NEGOTIATING_MASTER )
    sendNegotiatePacket();

  State = CONNECTED;

  FOREACH_CONST( first, Manager, wNetworkManager )
  ( *first ) ->notifyNewConnection( this );
}




void wNetworkConnection::processDataPacket( wStreamReader &reader )
{
  bool quitflag = false;
  while ( !quitflag )
  {
    TUnsigned8 type;
    reader >> type;

    switch ( type )
    {
    case WAM_NET_END:
      quitflag = true;
      break;

    case WAM_NET_CREATE:
      {
        TUnsigned8 create_mode;
        TUnsigned32 remote_id;
        string features;
        reader >> create_mode >> remote_id >> features;

        bool managed = true;
        wGameject *gj;
        if ( create_mode == WAM_NET_CREATE_MODE_NORMAL )
        {
          gj = Game.getInstanceManager().createObject( features.c_str() );
        }
        else if ( create_mode == WAM_NET_CREATE_MODE_SINGLETON )
        {
          try
          {
            gj = Game.getInstanceManager().getObjectByFeatureSet( features.c_str() );
            managed = false;
          }
          catch ( ... )
          {
            gj = Game.getInstanceManager().createObject( features.c_str() );
          }
        }
        wNetworkedGameject *ngj = dynamic_cast<wNetworkedGameject *>( gj );
        if ( ngj == NULL )
          EXGAME_THROWINFO( ECGAME_GENERAL, "cast to networked gj failed" )
          ngj->notifyRemoteInitiation( this, remote_id );
        break;
      }

    case WAM_NET_MESSAGE:
      {
        TUnsigned32 remote_id;
        reader >> remote_id;
        getObject( RemoteToLocalIdLookupTable[ remote_id ] ) ->readMessage( this, reader );
        break;
      }

    case WAM_NET_PARENT_MESSAGE:
      {
        TUnsigned32 local_id;
        reader >> local_id;

        // note that the parent is most likely not managed by this connection
        Manager.getObject( local_id ) ->readMessage( this, reader );
        break;
      }

    default:
      wamPrintDebug( ( "net: protocol error with peer " + getName() ).c_str() );
      Manager.requestDeletion( this );
      State = CLOSING;
      return ;
    }
  }
}




// wNetworkManager ------------------------------------------------------------
wNetworkManager::wNetworkManager( wGame &g )
    : Game( g ), Master( false )
{

#ifdef WIN32
  { WSADATA data;
    if ( WSAStartup( MAKEWORD( 2, 0 ), &data ) != 0 )
      EXGAME_THROWINFO( ECGAME_NET, "win32 sockets init failed." )
    }
#endif
 #ifndef WIN32
  signal( SIGPIPE, SIG_IGN );
#endif

}




wNetworkManager::~wNetworkManager()
{
  if ( Master )
    stopMaster();
  disconnect();

#ifdef WIN32

  WSACleanup();
#endif

}




void wNetworkManager::run()
{
  if ( Master )
    acceptConnections();

  if ( ConnectionList.size() )
  {
    FOREACH_CONST( first, ConnectionList, wConnectionList )
    ( *first ) ->runInbound();
    FOREACH_CONST( first, ConnectionList, wConnectionList )
    ( *first ) ->runOutbound();

    purgeConnections();
  }
}




void wNetworkManager::connect( string const &url )
{
  if ( isMaster() )
    EXGAME_THROWINFO( ECGAME_NET, "network master cannot connect." )
    regex_string url_parser( "wam\\:\\/\\/([^:]*)(\\:(.*))?" );
  if ( !url_parser.match( url ) )
    EXGAME_THROWINFO( ECGAME_NET, ( "invalid url: " + url ).c_str() )

    string host_str = url_parser.getBackref( 0 );
  string port_str;
  if ( url_parser.countBackrefs() == 3 )
    port_str = url_parser.getBackref( 2 );

  sockaddr_in name;
  name.sin_family = AF_INET;
  name.sin_port = htons( WAM_DEFAULT_PORT );
  if ( port_str.size() )
    name.sin_port = htons( evalUnsigned( port_str ) );

  name.sin_addr.s_addr = inet_addr( host_str.c_str() );
  if ( name.sin_addr.s_addr == INADDR_NONE )
  {
    wamPrintDebug( ( "net: resolving " + host_str ).c_str() );
    hostent *entry = gethostbyname( host_str.c_str() );
    if ( !entry )
      EXGAME_THROWINFO( ECGAME_NET, ( "invalid host: " + host_str ).c_str() )
      memcpy( &name.sin_addr, entry->h_addr, sizeof( name.sin_addr ) );
  }

  int sock;
  if ( ( sock = socket( PF_INET, SOCK_STREAM, 0 ) ) < 0 )
    THROW_NET_ERRNO( "could not create outbound socket" )

    wamPrintDebug( ( "net: connecting to " + sockaddr2string( *( ( sockaddr * ) & name ) ) ).c_str() );
  if ( ::connect( sock, ( sockaddr * ) & name, sizeof( name ) ) < 0 )
    THROW_NET_ERRNO( "unable to connect" )

    auto_ptr<wNetworkConnection>
    cnx( new wNetworkConnection( Game, *this, sock, *( ( sockaddr * ) & name ), false ) );

  ConnectionList.push_back( cnx.release() );
}




void wNetworkManager::disconnect()
{
  FOREACH( first, ConnectionList, wConnectionList )
  delete * first;
  ConnectionList.clear();
}




void wNetworkManager::startMaster( string const &port )
{
  if ( Master )
    EXGAME_THROWINFO( ECGAME_NET, "master already started" )

    sockaddr_in name;
  name.sin_family = AF_INET;
  name.sin_port = htons( WAM_DEFAULT_PORT );
  if ( port.size() )
    name.sin_port = htons( evalUnsigned( port ) );
  name.sin_addr.s_addr = htonl( INADDR_ANY );

  if ( ( ListenSocket = socket( PF_INET, SOCK_STREAM, 0 ) ) < 0 )
    THROW_NET_ERRNO( "could not create listen socket" )

    try
    {
      wamPrintDebug( "net: bound to port " + unsigned2dec( ntohs( name.sin_port ) ) );
      if ( bind( ListenSocket, ( sockaddr * ) & name, sizeof( name ) ) < 0 )
        THROW_NET_ERRNO( "could not bind socket" )

        if ( listen( ListenSocket, 10 ) < 0 )
          THROW_NET_ERRNO( "listen failed" )

          setNonBlockMode( ListenSocket, true );

      Master = true;

      wamPrintDebug( "net: listening started" );
    }
    catch ( ... )
    {
      close( ListenSocket );
      throw;
    }
}




void wNetworkManager::stopMaster()
{
  if ( !Master )
    EXGAME_THROWINFO( ECGAME_NET, "master not running" )

    close( ListenSocket );
  wamPrintDebug( "net: listening stopped" );
  Master = false;
}




// connection service functions -----------------------------------------------
void wNetworkManager::requestDeletion( wNetworkConnection *cnx )
{
  DeletionRequests.push_back( cnx );
}




// support routines -----------------------------------------------------------
void wNetworkManager::registerObject( wNetworkedGameject *gj )
{
  Super::registerObject( gj );

  FOREACH( first, ConnectionList, wConnectionList )
  ( *first ) ->registerObject( gj );
}




void wNetworkManager::unregisterObject( wNetworkedGameject *gj )
{
  FOREACH( first, ConnectionList, wConnectionList )
  ( *first ) ->unregisterObject( gj );

  Super::unregisterObject( gj );
}




void wNetworkManager::requestChildCreation( wNetworkedGameject *gj, wNetworkConnection *except, bool singleton )
{
  FOREACH( first, ConnectionList, wConnectionList )
  if ( *first != except )
    ( *first ) ->requestChildCreation( gj, singleton );
}




void wNetworkManager::sendMessage( wNetworkedGameject *gj, wNetworkConnection *except,
                                   wStreamStore const &msg )
{
  FOREACH( first, ConnectionList, wConnectionList )
  if ( *first != except )
    ( *first ) ->sendMessage( gj, msg );
}




// loop service functions -----------------------------------------------------
void wNetworkManager::acceptConnections()
{
  int socket;
  sockaddr name;
  socklen_t name_size = sizeof( name );

  if ( ( socket = accept( ListenSocket, &name, &name_size ) ) >= 0 )
  {
    auto_ptr<wNetworkConnection> cnx( new wNetworkConnection( Game, *this, socket, name, true ) );

    ConnectionList.push_back( cnx.release() );
  }
}




void wNetworkManager::purgeConnections()
{
  FOREACH_CONST( first, DeletionRequests, wConnectionList )
  {
    FOREACH( first_p, ConnectionList, wConnectionList )
    if ( *first_p == *first )
    {
      ConnectionList.erase( first_p );
      break;
    }

    delete *first;
  }
  DeletionRequests.clear();
}




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( net_connect, "NeCo", "connect to master", "url: wam://host:port" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "net_connect", 1, 1 )
  Game.getNetworkManager().connect( parameters[ 0 ] ->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( net_disconnect, "NeDi", "disconnect from master", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "net_disconnect", 0, 0 )
  Game.getNetworkManager().disconnect();
  return makeNull();
}




WAM_DECLARE_COMMAND( net_startmaster, "NeAc", "listen for network connections", "port" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "net_startmaster", 0, 1 )
  if ( parameters.size() )
    Game.getNetworkManager().startMaster( parameters[ 0 ] ->toString() );
  else
    Game.getNetworkManager().startMaster();
  return makeNull();
}




WAM_DECLARE_COMMAND( net_stopmaster, "NeRe", "stop master, close down all connections", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "net_stopmaster", 0, 0 )
  Game.getNetworkManager().stopMaster();
  return makeNull();
}




WAM_DECLARE_COMMAND( net_getconnectioncount, "NeRe", "get connection count", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "net_getconnectioncount", 0, 0 )

  return makeConstant( Game.getNetworkManager().getConnectionCount() );
}




void registerNetworkCommands( wGame &g )
{
  register_net_connect( g );
  register_net_disconnect( g );
  register_net_startmaster( g );
  register_net_stopmaster( g );
  register_net_getconnectioncount( g );
}




// wAutoReplicatingNetworkedGameject ------------------------------------------
wAutoReplicatingNetworkedGameject::wAutoReplicatingNetworkedGameject( wGame &s )
    : wGameject( s ), wNetworkedGameject( s ), Role( UNDETERMINED ), ParentConnection( NULL ), Dying( false )
{}




void
wAutoReplicatingNetworkedGameject::
notifyNewConnection( wNetworkConnection *cnx )
{
  switch ( Role )
  {
  case UNDETERMINED:
    EXGAME_THROWINFO( ECGAME_NET, "role undetermined at cnx notification" )

  case ORIGINAL:
    requestChildCreationAt( cnx, false );
    sendMessage( cnx, NULL, MSG_STATE_DUMP );
    break;

  case COPY:
    if ( atMaster() )
    {
      requestChildCreationAt( cnx, false );
      sendMessage( cnx, NULL, MSG_STATE_DUMP );
    }
    else
      EXGAME_THROWINFO( ECGAME_NET, "non-master copy detected illegal attachment" )
      break;
  }
}




void
wAutoReplicatingNetworkedGameject::
notifyEndedConnection( wNetworkConnection *cnx )
{
  switch ( Role )
  {
  case UNDETERMINED:
    EXGAME_THROWINFO( ECGAME_NET, "role undetermined at cnx notification" )

  case ORIGINAL:
    // nothing needs to be done here, we can't tell gamejects behind the
    // broken line anything.
    break;

  case COPY:
    if ( cnx == ParentConnection )
    {
      // whoops, the connection over which we're replicating is gone.
      // be sad. commit suicide.
      Dying = true;
      requestDestruction();

      if ( atMaster() )
      {
        // as we're potentially the leader of a cult of gamejects, tell
        // them all to commit suicide.
        sendMessage( NULL, ParentConnection, MSG_DELETE );
      }
    }
    break;
  }
}




void
wAutoReplicatingNetworkedGameject::
notifyRemoteInitiation( wNetworkConnection *cnx, wObject::TId remote_id )
{
  registerAsChildOf( cnx, remote_id );
  Role = COPY;
  ParentConnection = cnx;

  if ( atMaster() )
  {
    requestChildCreation( cnx, false );
    sendMessage( NULL, cnx, MSG_STATE_DUMP );
  }
}




void
wAutoReplicatingNetworkedGameject::registerMe()
{
  if ( Role == UNDETERMINED )
    Role = ORIGINAL;

  super::registerMe();
}




void
wAutoReplicatingNetworkedGameject::
prepareToDie()
{
  if ( Role == COPY && !Dying )
    wamPrintDebugLevel( "net: copy of gameject is dying. bad.", WAM_DEBUG_OFF );
  bool send_msg = false;

  send_msg = send_msg || Role == ORIGINAL;
  send_msg = send_msg || ( Role == COPY && atMaster() );
  if ( send_msg )
    sendMessage( NULL, ParentConnection, MSG_DELETE );
}




void
wAutoReplicatingNetworkedGameject::
readMessage( wNetworkConnection *from, wStreamReader &reader )
{
  TMessageKind msg_kind;
  reader >> msg_kind;

  if ( msg_kind == MSG_STATE_DUMP )
    readStateDump( reader );
  else if ( msg_kind == MSG_UPDATE )
  {
    readUpdate( reader );
    if ( atMaster() )
      requestSendUpdate();
  }
  else if ( msg_kind == MSG_DELETE )
  {
    Dying = true;
    requestDestruction();
  }
  else
    EXGAME_THROWINFO( ECGAME_NET, "auto_replicate protocol error" )
  }




void
wAutoReplicatingNetworkedGameject::
requestSendUpdate()
{
  sendMessage( NULL, ParentConnection, MSG_UPDATE );
}




void
wAutoReplicatingNetworkedGameject::
sendMessage( wNetworkConnection *to, wNetworkConnection *except, TMessageKind kind )
{
  wStreamStore store;
  {
    wStreamWriter writer( store );
    writer << kind;

    if ( kind == MSG_STATE_DUMP )
      writeStateDump( writer );
    else if ( kind == MSG_UPDATE )
      writeUpdate( writer );
  }
  sendNetMessage( to, except, store );
}
