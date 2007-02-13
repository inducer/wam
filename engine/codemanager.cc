// ----------------------------------------------------------------------------
//  Description      : WAM server-side codemanager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/codemanager_impl.hh"
#include "engine/commandmanager.hh"
#include "engine/console.hh"
#include "engine/gamebase.hh"
#include "engine/codemanager.hh"




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( cm_loadall, "LAll", "load all available modules", "[mpack]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "cm_loadall", 0, 1 )

  if ( parameters.size() == 1 )
    Game.getGameCodeManager().loadAllCode( parameters[ 0 ] ->toString() );
  else
    Game.getGameCodeManager().loadAllCode();
  return makeNull();
}




WAM_DECLARE_COMMAND( cm_load, "LMod", "load single module", "module [mpack]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "cm_load", 1, 2 )

  if ( parameters.size() == 1 )
    Game.getGameCodeManager().loadCode( parameters[ 0 ] ->toString() );
  else
    Game.getGameCodeManager().loadCode( parameters[ 0 ] ->toString(), parameters[ 1 ] ->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( list_classes, "LCls", "list classes having features", "[feature,feature,...]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_classes", 0, 1 )

  wFeatureSetProcessor fp;
  if ( parameters.size() == 1 )
    fp.parseDisplayRep( parameters[ 0 ] ->toString() );

  auto_ptr<js_array> arr( new js_array( Game.getTickManager().size() ) );

  TIndex idx = 0;

  wGameCodeManager::class_const_iterator
  first = Game.getGameCodeManager().class_begin(), last = Game.getGameCodeManager().class_end();
  while ( first != last )
  {
    if ( first->Features.hasFeatures( fp.c_str() ) )
      ( *arr ) [ idx++ ] = makeConstant( first->Features.getDisplayRep() );
    first++;
  }
  return arr.release();
}





void registerGameCodeManagerCommands( wGame &g )
{
  register_cm_loadall( g );
  register_cm_load( g );
  register_list_classes( g );
}




// template instantiation -----------------------------------------------------
template wCodeManager<wGameject, wGame>
;




// wGameCodeManager -----------------------------------------------------------
wGameCodeManager::wGameCodeManager( wGame &g )
    : wCodeManager<wGameject, wGame>( g )
{}
