// ----------------------------------------------------------------------------
//  Description      : WAM manager base
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_MANAGER_IMPL_BASE
#define WAM_MANAGER_IMPL_BASE




#include <algorithm>
#include <string.h>
#include "utility/exgame.hh"
#include "utility/manager.hh"




// wRegisteringManager --------------------------------------------------------
template <class Object>
void wRegisteringManager<Object>::registerObject( Object *o )
{
  typename vector<Object *>::iterator inspos = getLowerBound( o->id() );
  if ( inspos != ObjectList.end() && ( *inspos ) ->id() == o->id() )
    EXGAME_THROWINFO( ECGAME_DOUBLEID, "wRegisteringManager::registerObject" );
  ObjectList.insert( inspos, o );
}




template <class Object>
void wRegisteringManager<Object>::unregisterObject( Object *o )
{
  ObjectList.erase( getIterator( o->id() ) );
}




template <class Object>
Object *
wRegisteringManager<Object>::getObject( wObject::TId id )
{
  return * getIterator( id );
}




template <class Object>
typename vector<Object *>::iterator
wRegisteringManager<Object>::getLowerBound( wObject::TId id )
{
  if ( ObjectList.size() == 0 )
    return ObjectList.begin();

  TIndex first = 0, last = ObjectList.size();

  // binary search between first and last
  while ( last - first > 1 )
  {
    int mid = ( last + first ) / 2;
    if ( ObjectList[ mid ] ->id() < id )
      first = mid;
    else
      last = mid;
  }

  if ( ObjectList[ first ] ->id() >= id )
    return ObjectList.begin() + first;
  else
    return ObjectList.begin() + last;
}




template <class Object>
typename vector<Object *>::iterator
wRegisteringManager<Object>::getIterator( wObject::TId id )
{
  typename vector<Object *>::iterator pos = getLowerBound( id );

  if ( ( pos == ObjectList.end() ) || ( ( *pos ) ->id() != id ) )
    EXGAME_THROWINFO( ECGAME_NOSUCHID, "wRegisteringManager::getIterator" );

  return pos;
}




template <class Object>
bool wRegisteringManager<Object>::existID( wObject::TId id )
{
  typename vector<Object *>::iterator pos = getLowerBound( id );

  if ( ( pos == ObjectList.end() ) || ( ( *pos ) ->id() != id ) )
    return false;
  else
    return true;
}




// wPriorityManager -----------------------------------------------------------
template <class Object>
void wPriorityManager<Object>::registerObject( Object *o )
{
#ifdef WAM_DEBUG
  bool exists = true;
  try
  {
    getIterator( o->id() );
  }
  catch ( ... )
  {
    exists = false;
  }
  if ( exists )
    EXGAME_THROWINFO( ECGAME_DOUBLEID, "wPriorityManager::registerObject" );
#endif

  typename vector<Object *>::iterator inspos = getLowerBound( getPriority( o ) );
  ObjectList.insert( inspos, o );
}




template <class Object>
void wPriorityManager<Object>::unregisterObject( Object *o )
{
  ObjectList.erase( getIterator( o->id() ) );
}




template <class Object>
Object *
wPriorityManager<Object>::getObject( wObject::TId id )
{
  return * getIterator( id );
}




template <class Object>
typename vector<Object *>::iterator
wPriorityManager<Object>::getLowerBound( signed int prio )
{
  if ( ObjectList.size() == 0 )
    return ObjectList.begin();

  TIndex first = 0, last = ObjectList.size();

  // binary search between first and last
  while ( last - first > 1 )
  {
    int mid = ( last + first ) / 2;
    if ( getPriority( ObjectList[ mid ] ) < prio )
      first = mid;
    else
      last = mid;
  }

  if ( getPriority( ObjectList[ first ] ) >= prio )
    return ObjectList.begin() + first;
  else
    return ObjectList.begin() + last;
}




template <class Object>
typename vector<Object *>::iterator
wPriorityManager<Object>::getIterator( wObject::TId id )
{
  typename vector<Object *>::iterator
  first = ObjectList.begin(),
          last = ObjectList.end();
  while ( first != last )
  {
    if ( id == ( *first ) ->id() )
      return first;
    first++;
  }

  EXGAME_THROWINFO( ECGAME_NOSUCHID, "wPriorityManager::getIterator" )
}




template <class Object>
bool wPriorityManager<Object>::existID( wObject::TId id )
{
  typename vector<Object *>::iterator pos = getLowerBound( id );

  if ( ( pos == ObjectList.end() ) || ( ( *pos ) ->id() != id ) )
    return false;
  else
    return true;
}



template <class Object>
void wPriorityManager<Object>::changeObject( Object *o )
{
  unregisterObject( o );
  registerObject( o );
}




#endif
