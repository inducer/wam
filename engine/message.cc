// ----------------------------------------------------------------------------
//  Description      : WAM message manager
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "utility/manager_impl.hh"
#include "engine/gamebase.hh"
#include "engine/commandmanager.hh"
#include "engine/console.hh"
#include "engine/message.hh"




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( msg_send, "MSnd", "send gameject broadcast", "messageid [parameter]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "msg_send", 1, 2 )

  wMessageManager::wResultSet result;
  if ( parameters.size() == 1 )
    Game.getMessageManager().send(
      Game.getMessageManager().registerMessage( parameters[ 0 ] ->toString() ),
      &result );
  else
    Game.getMessageManager().send(
      Game.getMessageManager().registerMessage( parameters[ 0 ] ->toString() ),
      &result, parameters[ 1 ] ->toString() );
  if ( result.size() )
    return makeConstant( result.front() ->first );
  else
    return makeNull();
}




WAM_DECLARE_COMMAND( list_message_ids, "LMsg", "list registered gameject broadcast types", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_message_ids", 0, 0 )

  wMessageManager & msgman = Game.getMessageManager();

  auto_ptr<js_array> arr( new js_array( msgman.getFirstUnusedMessageId() ) );

  TIndex idx = 0;

  TMessageId first = msgman.getFirstMessageId(), last = msgman.getFirstUnusedMessageId();
  while ( first != last )
  {
    ( *arr ) [ idx++ ] = makeConstant( msgman.getMessageName( first ) );
    first++;
  }
  return arr.release();
}




void registerMessageManagerCommands( wGame &g )
{
  register_msg_send( g );
  register_list_message_ids( g );
}




// wMessageReceiver -----------------------------------------------------------
wMessageReceiver::wMessageReceiver( wGame &g )
    : wGameject( g )
{}




void wMessageReceiver::registerMe()
{
  wGameject::registerMe();
  Game.getMessageManager().registerObject( this );
}




void wMessageReceiver::unregisterMe()
{
  Game.getMessageManager().unregisterObject( this );
  wGameject::unregisterMe();
}




IXLIB_GARBAGE_DECLARE_MANAGER( wMessageReceiver::wMessageReturn )




// template instantiation -----------------------------------------------------
template wRegisteringManager<wMessageReceiver>
;




// wMessageManager -----------------------------------------------------------
TMessageId wMessageManager::registerMessage( wMessageName const &name )
{
  FOREACH_CONST( first, MessageRegister, wMessageRegister )
  if ( *first == name )
    return first -MessageRegister.begin();
  TMessageId msgid = MessageRegister.size();
  MessageRegister.push_back( name );
  return msgid;
}




wMessageName wMessageManager::getMessageName( TMessageId msg_id ) const
{
  return MessageRegister[ msg_id ];
}




void wMessageManager::run()
{
  while ( MessageQueue.size() )
  {
    wMessage msg = MessageQueue.front();
    MessageQueue.pop();
    if ( msg.Destination )
      sendTo( msg.Destination, msg.Id, msg.Parameter1, msg.Parameter2 );
    else
      send( msg.Id, NULL, msg.Parameter1, msg.Parameter2 );
  }
}




void wMessageManager::post( TMessageId msg_id, string const &par1, void *par2 )
{
  wMessage msg = { NULL, msg_id, par1, par2 };
  MessageQueue.push( msg );
}




void wMessageManager::postTo( wGameject *rec, TMessageId msg_id, string const &par1, void *par2 )
{
  wMessage msg = { castFromGameject( rec ), msg_id, par1, par2 };
  MessageQueue.push( msg );
}




void wMessageManager::send( TMessageId msg_id, wResultSet *res, string const &par1, void *par2 )
{
  wamPrintDebugLevel( ( "broadcasting '" + getMessageName( msg_id ) + "' with parameter '" + par1 + "'" ).c_str(), WAM_DEBUG_VERBOSE );
  FOREACH_CONST( first, *this, wMessageManager )
  {
    ref<wMessageReceiver::wMessageReturn> ret = ( *first ) ->receiveMessage( msg_id, par1, par2 );
    if ( res && ret.get() )
      res->push_back( ret );
  }
}




ref<wMessageReceiver::wMessageReturn>
wMessageManager::sendTo( wGameject *rec, TMessageId msg_id, string const &par1, void *par2 )
{
  wamPrintDebugLevel( ( "sending '" + getMessageName( msg_id ) + "' with parameter '" + par1 + "' to " +
                        wFeatureSetProcessor( rec->getFeatureSet() ).getDisplayRep() + " " + unsigned2dec( rec->id() ) ).c_str(), WAM_DEBUG_VERBOSE );
  return castFromGameject( rec ) ->receiveMessage( msg_id, par1, par2 );
}




ref<wMessageReceiver::wMessageReturn> wMessageManager::sendTo( wObject::TId id, TMessageId msg_id, string const &par1, void *par2 )
{
  FOREACH_CONST( first, *this, wMessageManager )
  {
    if ( ( *first ) ->id() == id )
    {
      wamPrintDebugLevel( ( "sending '" + getMessageName( msg_id ) + "' with parameter '" + par1 + "' to " +
                            wFeatureSetProcessor( ( *first ) ->getFeatureSet() ).getDisplayRep() + " " + unsigned2dec( ( *first ) ->id() ) ).c_str(), WAM_DEBUG_PERIODIC );
      return ( *first ) ->receiveMessage( msg_id, par1, par2 );
    }
  }
  EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "message receiver having id " + unsigned2dec( id ) ).c_str() );
}




wMessageReceiver *wMessageManager::castFromGameject( wGameject *gj )
{
  wMessageReceiver *result = dynamic_cast< wMessageReceiver * >( gj );
  if ( result )
    return result;
  else
    EXGAME_THROWINFO( ECGAME_NOTFOUND, "message receiver from gameject pointer" );
}
