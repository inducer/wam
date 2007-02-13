// ----------------------------------------------------------------------------
//  Description      : Gameject base
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "engine/gameject.hh"
#include "engine/gamebase.hh"
#include "engine/instancemanager.hh"




// wGameject ------------------------------------------------------------------
wGameject::wGameject( wGame &g )
    : Game( g )
{}




void wGameject::requestDestruction()
{
  Game.getInstanceManager().requestDestruction( this );
}
