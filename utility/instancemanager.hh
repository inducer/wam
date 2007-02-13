// ----------------------------------------------------------------------------
//  Description      : Instance manager
// ----------------------------------------------------------------------------
//  Remarks          :
//    Registration with the Instance manager is neither intended nor
//    necessary.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_INSTANCEMANAGER
#define WAM_INSTANCEMANAGER




#include <vector>
#include <list>
#include "utility/object.hh"
#include "utility/manager.hh"




template <class Object, class CodeManagerType>
struct wInstanceManager : protected wRegisteringManager<Object>
{
  typedef wRegisteringManager<Object> Super;

protected:
  wObject::TId NextId;
  CodeManagerType &CodeManager;

  typedef list<Object *> wObjectQueue;
  wObjectQueue InsertionRequests, DestructionRequests;

public:
  using Super::size;
  using Super::iterator;
  using Super::const_iterator;
  using Super::begin;
  using Super::end;
  using Super::getObject;

  wInstanceManager( CodeManagerType &classmgr )
      : NextId( WAMID_FIRSTVALID ), CodeManager( classmgr )
  {}
  void destroyAll();

  Object *createObject( wFeatureSet const &features );
  void insertObject( Object *obj );
  void insertObjectImmediate( Object *obj );
  void requestDestruction( wFeatureSet const &features );
  void requestDestruction( Object *o );

  void commit();

  Object *getObjectByFeatureSet( wFeatureSet const &features );
};




#endif
