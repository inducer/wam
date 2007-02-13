// ----------------------------------------------------------------------------
//  Description      : Servject instance manager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/instancemanager_impl.hh"
#include "utility/manager_impl.hh"
#include "engine/instancemanager.hh"
#include "engine/console.hh"
#include "engine/commandmanager.hh"
#include "engine/gamebase.hh"




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( list_gamejects, "LIns", "list instances", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_gamejects", 0, 0 )

  auto_ptr<js_array> arr( new js_array( Game.getInstanceManager().size() ) );

  TIndex idx = 0;

  FOREACH_CONST( first, Game.getInstanceManager(), wGameInstanceManager )
  ( *arr ) [ idx++ ] = makeConstant( ( *first ) ->id() );

  return arr.release();
}




WAM_DECLARE_COMMAND( im_create, "CrIn", "create an instance", "class" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "im_create", 1, 1 )

  wFeatureSetProcessor fp;
  wObject *object;
  fp.parseDisplayRep( parameters[ 0 ] ->toString() );
  object = Game.getInstanceManager().createObject( fp.c_str() );
  return makeConstant( object->id() );
}




WAM_DECLARE_COMMAND( im_kill, "KiIn", "kill an instance", "id" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "im_kill", 1, 1 )

  Game.getInstanceManager().requestDestruction(
    Game.getInstanceManager().getObject( parameters[ 0 ] ->toInt() )
  );
  return makeNull();
}




WAM_DECLARE_COMMAND( im_getfeatures, "GeFe", "find features of instance", "id" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "im_getfeatures", 1, 1 )

  wGameject * gj = Game.getInstanceManager().getObject( parameters[ 0 ] ->toInt() );
  wFeatureSetProcessor fsp = gj->getFeatureSet();
  return makeConstant( fsp.getDisplayRep() );
}




WAM_DECLARE_COMMAND( im_hasfeatures, "HaFe", "determine whether instance has certain feature", "id feature_set" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "im_hasfeatures", 2, 2 )

  wGameject * gj = Game.getInstanceManager().getObject( parameters[ 0 ] ->toInt() );
  wFeatureSetProcessor fsp;
  fsp.parseDisplayRep( parameters[ 1 ] ->toString() );
  return makeConstant( gj->hasFeatures( fsp.c_str() ) );
}




WAM_DECLARE_COMMAND( im_commit, "InCm", "commit changes to instance manager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "im_commit", 0, 0 )

  Game.getInstanceManager().commit();
  return makeNull();
}




void registerInstanceManagerCommands( wGame &g )
{
  register_list_gamejects( g );
  register_im_create( g );
  register_im_kill( g );
  register_im_getfeatures( g );
  register_im_hasfeatures( g );
  register_im_commit( g );
}




// Template instantiation -----------------------------------------------------
template wRegisteringManager<wGameject>
;
template wInstanceManager<wGameject, wGameCodeManager>
;




// wGameInstanceManager -----------------------------------------------------------
wGameInstanceManager::wGameInstanceManager( wGame &g )
    : wInstanceManager<wGameject, wGameCodeManager>( g.getGameCodeManager() )
{
  wamPrintDebug( "gameject instance management online" );
}
