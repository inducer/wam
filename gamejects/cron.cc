// ----------------------------------------------------------------------------
//  Description      : Cron object
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "engine/input.hh"
#include "engine/console.hh"
#include "engine/scrolling.hh"
#include "engine/tick.hh"
#include "engine/gamebase.hh"
#include "gamejects/cron.hh"




#define GJFEATURESET_CRON "Cron"




// wCronJob ------------------------------------------------------------------
class wCronJob : public wTickReceiver
{
  protected:
    float Every, Elapsed;
    ref<value> Notification;
    bool Once;

  public:
    wCronJob( wGame &g, float every, ref<value> notification, bool once = false );
    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_CRON;
    }
    void tick( float seconds );
};




// wCronJob ------------------------------------------------------------------
wCronJob::wCronJob( wGame &g, float every, ref<value> notification, bool once )
    : wGameject( g ), wTickReceiver( g ), Every( every ), Elapsed( 0 ), Notification( notification ), Once( once )
{}




void wCronJob::tick( float seconds )
{
  Elapsed += seconds;
  if ( Elapsed >= Every )
  {
    Notification->call( value::parameter_list() );
    if ( Once )
      Game.getInstanceManager().requestDestruction( this );
    else
      Elapsed = 0;
  }
}




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( cron, "Cron", "create cron job", "seconds command" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "cron", 2, 2 )

  wGameject * gj = new wCronJob( Game, parameters[ 0 ] ->toFloat(),
                                 parameters[ 1 ], false );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




WAM_DECLARE_COMMAND( timeout, "TimO", "create timeout job", "seconds command" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "timeout", 2, 2 )

  wGameject * gj = new wCronJob( Game, parameters[ 0 ] ->toFloat(),
                                 parameters[ 1 ], true );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




// registration ---------------------------------------------------------------
void registerCron( wGame &g )
{
  register_cron( g );
  register_timeout( g );
}
