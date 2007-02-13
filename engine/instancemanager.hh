// ----------------------------------------------------------------------------
//  Description      : Gameject instance manager
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_GAMEJECTMANAGER
#define WAM_GAMEJECTMANAGER




#include "utility/instancemanager.hh"
#include "engine/gameject.hh"
#include "engine/codemanager.hh"




void registerInstanceManagerCommands( wGame &g );




class wGameInstanceManager : public wInstanceManager<wGameject, wGameCodeManager>
{
  public:
    wGameInstanceManager( wGame &g );
};




#endif
