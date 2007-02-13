// ----------------------------------------------------------------------------
//  Description      : WAM server-side codemanager
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_SERVER_CODEMANAGER
#define WAM_SERVER_CODEMANAGER




#include "utility/codemanager.hh"
#include "engine/gameject.hh"




void registerGameCodeManagerCommands( wGame &g );




// wGameCodeManager -----------------------------------------------------------
class wGameCodeManager : public wCodeManager<wGameject, wGame>
{
  public:
    wGameCodeManager( wGame &g );
};




#endif
