// ----------------------------------------------------------------------------
//  Description      : WAM manager base
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_MANAGER_BASE
#define WAM_MANAGER_BASE




#include <vector>
#include "utility/object.hh"




// wRegisteringManager --------------------------------------------------------
template <class Object>
class wRegisteringManager
{
  protected:
    typedef vector<Object *> wObjectList;
    wObjectList ObjectList;

  public:
    typedef typename wObjectList::iterator iterator;
    typedef typename wObjectList::const_iterator const_iterator;

    void registerObject( Object *o );
    void unregisterObject( Object *o );
    Object *getObject( wObject::TId id );
    bool existID( wObject::TId id );

    TSize size() const
    {
      return ObjectList.size();
    }
    iterator begin()
    {
      return ObjectList.begin();
    }
    const_iterator begin() const
    {
      return ObjectList.begin();
    }
    iterator end()
    {
      return ObjectList.end();
    }
    const_iterator end() const
    {
      return ObjectList.end();
    }

  protected:
    typename wObjectList::iterator getLowerBound( wObject::TId id );
    typename wObjectList::iterator getIterator( wObject::TId id );
};




// wPriorityManager -----------------------------------------------------------
template <class Object>
struct wPriorityManager
{
protected:
  typedef vector<Object *> wObjectList;
  wObjectList ObjectList;

public:
  typedef typename wObjectList::iterator iterator;
  typedef typename wObjectList::const_iterator const_iterator;

  virtual ~wPriorityManager()
  {}
  void registerObject( Object *o );
  void unregisterObject( Object *o );
  Object *getObject( wObject::TId id );
  bool existID( wObject::TId id );
  void changeObject( Object *o );

  virtual TPriority getPriority( Object const *o ) const = 0;

  iterator begin()
  {
    return ObjectList.begin();
  }
  const_iterator begin() const
  {
    return ObjectList.begin();
  }
  iterator end()
  {
    return ObjectList.end();
  }
  const_iterator end() const
  {
    return ObjectList.end();
  }

protected:
  typename vector<Object *>::iterator getLowerBound( signed int prio );
  typename vector<Object *>::iterator getIterator( wObject::TId id );
};




#endif
