// ----------------------------------------------------------------------------
//  Description      : Instance manager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <algorithm>
#include "utility/debug.hh"
#include "utility/exgame.hh"
#include "utility/instancemanager.hh"




namespace
{
template <class Object>
struct less_DeleteOrder
{
  bool operator() ( Object *o1, Object *o2 )
  {
    return o1->getDeleteOrder() < o2->getDeleteOrder();
  }
};
}




template <class Object, class CodeManagerType>
void wInstanceManager<Object, CodeManagerType>::destroyAll()
{
  while ( InsertionRequests.size() )
  {
    delete InsertionRequests.front();
    InsertionRequests.pop_front();
  }

  FOREACH( first, ObjectList, typename wObjectList )
  requestDestruction( *first );

  commit();
}




template <class Object, class CodeManagerType>
Object *wInstanceManager<Object, CodeManagerType>::createObject( wFeatureSet const &features )
{
  Object * obj = CodeManager.createObject( features );
  obj->setId( NextId++ );
  InsertionRequests.push_back( obj );

  return obj;
}




template <class Object, class CodeManagerType>
void wInstanceManager<Object, CodeManagerType>::insertObject( Object *obj )
{
  obj->setId( NextId++ );
  InsertionRequests.push_back( obj );
}




template <class Object, class CodeManagerType>
void wInstanceManager<Object, CodeManagerType>::insertObjectImmediate( Object *obj )
{
  obj->setId( NextId++ );
  registerObject( obj );
  obj->registerMe();
  obj->startUp();

  wamPrintDebugLevel( "instance: immed inserted obj with " + wFeatureSetProcessor( obj->getFeatureSet() ).getDisplayRep() + " [id:" + unsigned2dec( obj->id() ) + "]", WAM_DEBUG_VERBOSE );
}




template <class Object, class CodeManagerType>
void wInstanceManager<Object, CodeManagerType>::requestDestruction( wFeatureSet const &features )
{
  FOREACH( first, ObjectList, typename wObjectList )
  if ( ( *first ) ->hasFeatures( features ) )
    requestDestruction( *first );
}




template <class Object, class CodeManagerType>
void wInstanceManager<Object, CodeManagerType>::requestDestruction( Object *o )
{
  typename wObjectQueue::iterator 
    it = find( DestructionRequests.begin(), DestructionRequests.end(), o );
  if ( it != DestructionRequests.end() )
    return ;

  o->prepareToDie();
  typename wObjectQueue::iterator bound =
    lower_bound( DestructionRequests.begin(), DestructionRequests.end(), o,
                 less_DeleteOrder<Object>() );
  DestructionRequests.insert( bound, o );
}




template <class Object, class CodeManagerType>
void wInstanceManager<Object, CodeManagerType>::commit()
{
  // an iterator construction is no use here because of
  // lack of exception safety
  while ( InsertionRequests.size() )
  {
    Object * obj = InsertionRequests.front();
    wamPrintDebugLevel( "instance: inserting " + unsigned2dec( obj->id() ) + " with " + wFeatureSetProcessor( obj->getFeatureSet() ).getDisplayRep(), WAM_DEBUG_VERBOSE );
    InsertionRequests.pop_front();
    registerObject( obj );
    obj->registerMe();
    obj->startUp();
  }
  while ( DestructionRequests.size() )
  {
    Object * obj = DestructionRequests.front();
    wamPrintDebugLevel( "instance: destroying " + unsigned2dec( obj->id() ) + " with " + wFeatureSetProcessor( obj->getFeatureSet() ).getDisplayRep(), WAM_DEBUG_VERBOSE );
    DestructionRequests.pop_front();
    unregisterObject( obj );
    obj->unregisterMe();
    delete obj;
  }
}




template <class Object, class CodeManagerType>
Object *wInstanceManager<Object, CodeManagerType>::getObjectByFeatureSet( wFeatureSet const &features )
{
  FOREACH_CONST( first, ObjectList, typename wObjectList )
  if ( ( *first ) ->hasFeatures( features ) )
    return * first;

  EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "existing object having " +
                                       wFeatureSetProcessor( features ).getDisplayRep() ).c_str() )
}
