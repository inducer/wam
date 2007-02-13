// ----------------------------------------------------------------------------
//  Description      : Tick manager
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_GAME_TICK
#define WAM_GAME_TICK




#include "utility/manager.hh"
#include "engine/gameject.hh"




// wTickReceiver -------------------------------------------------------------
class wTickReceiver : public virtual wGameject
{
  public:
    wTickReceiver( wGame &g );

    virtual void tick( float seconds ) = 0;
    virtual void registerMe();
    virtual void unregisterMe();
};




// wTickManager --------------------------------------------------------------
struct wTickManager : public wRegisteringManager<wTickReceiver>
{
  void run( float seconds );
};




#endif
