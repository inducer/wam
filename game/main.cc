// ----------------------------------------------------------------------------
//  Description      : WAM main game lib registry
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "engine/codemanager.hh"
#include "engine/gamebase.hh"
#include "background.hh"
#include "credits.hh"
#include "hero.hh"
#include "weapon.hh"
#include "environment.hh"
#include "supervisor.hh"
#include "main.hh"




static wGameject *createHero( wGame &g )
{
  return new wHero( g );
}




static wGameject *createBazooka( wGame &g )
{
  return new wBazooka( g );
}




void wamInitializeModule( wGame &g, const char *libname )
{
  registerEnvironmentManager( g );
  registerBackground( g );
  registerCredits( g );
  registerSupervisor( g );
  g.getGameCodeManager().registerClass( GJFEATURESET_HERO, libname, createHero );
  g.getGameCodeManager().registerClass( GJFEATURESET_BAZOOKA, libname, createBazooka );
  wamPrintDebug( "main library initializing" );
}
