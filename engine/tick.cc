// ----------------------------------------------------------------------------
//  Description      : Tick manager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "utility/manager_impl.hh"
#include "engine/tick.hh"
#include "engine/gamebase.hh"




// wTickReceiver ---------------------------------------------------------------
wTickReceiver::wTickReceiver( wGame &g )
    : wGameject( g )
{}




void wTickReceiver::registerMe()
{
  Game.getTickManager().registerObject( this );
}




void wTickReceiver::unregisterMe()
{
  Game.getTickManager().unregisterObject( this );
}




// Template instantiation -----------------------------------------------------
template class wRegisteringManager<wTickReceiver>;




// wTickManager ---------------------------------------------------------------
void wTickManager::run( float seconds )
{
  FOREACH( first, ObjectList, vector<wTickReceiver *> )
  ( *first ) ->tick( seconds );
}
