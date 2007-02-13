// ----------------------------------------------------------------------------
//  Description      : Scripting notifier object
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "engine/input.hh"
#include "engine/console.hh"
#include "engine/scrolling.hh"
#include "engine/message.hh"
#include "engine/gamebase.hh"
#include "gamejects/cron.hh"




#define GJFEATURESET_SCRIPT_NOTIFIER "ScNo"




// wScriptNotifier ------------------------------------------------------------------
class wScriptNotifier : public wMessageReceiver
{
  protected:
    ref<value> Function;

  public:
    wScriptNotifier( wGame &g, ref<value> function );
    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_SCRIPT_NOTIFIER;
    }
    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );
};




// wScriptNotifier ------------------------------------------------------------------
wScriptNotifier::wScriptNotifier( wGame &g, ref<value> function )
    : wGameject( g ), wMessageReceiver( g ), Function( function )
{}




ref<wScriptNotifier::wMessageReturn>
wScriptNotifier::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  value::parameter_list pl;
  pl.push_back( makeConstant( Game.getMessageManager().getMessageName( id ) ) );
  pl.push_back( makeConstant( par1 ) );
  Function->call( pl );

  return NULL;
}




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( create_notifier, "CnNf", "create script notifier that calls function", "function" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "create_notifier", 1, 1 )

  wGameject * gj = new wScriptNotifier( Game, parameters[ 0 ] );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




// registration ---------------------------------------------------------------
void registerScriptNotifier( wGame &g )
{
  register_create_notifier( g );
}
