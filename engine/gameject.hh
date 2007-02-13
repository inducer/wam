// ----------------------------------------------------------------------------
//  Description      : Gameject base
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_GAMEJECT
#define WAM_GAMEJECT




#include <list>
#include <ixlib_numconv.hh>
#include <ixlib_xml.hh>
#include "utility/basic_types.hh"
#include "utility/object.hh"
#include "utility/exgame.hh"



class wGame;




// gameject features ----------------------------------------------------------
#define GJFEATURE_DISPLAYED             "Disp" // is displayed on screen
#define GJFEATURE_MANAGER  "Mngr" // is manager
#define GJFEATURE_GAMECOORD  "GCoo" // exists in game coordinates




// wGameject -----------------------------------------------------------------
class wGameject : public wObject
{
  protected:
    wGame &Game;

  public:
    wGameject( wGame &g );

  protected:
    void requestDestruction();
};




#endif
