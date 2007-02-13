// ----------------------------------------------------------------------------
//  Description      : WAM base-class-of-everything
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_OBJECT
#define WAM_OBJECT




#include <vector>
#include "utility/basic_types.hh"




// constants ------------------------------------------------------------------
#define WAM_DELORDER_PLAIN      -50
#define WAM_DELORDER_MANAGER  -30
#define WAM_DELORDER_BASIC_MANAGER -10
#define WAM_DELORDER_CONSOLE  0




// wObject --------------------------------------------------------------------
class wObject
{
  public:
    typedef unsigned int TId;

  private:
    TId Id;

  public:
    virtual ~wObject()
    {}

    virtual wFeatureSet const getFeatureSet() const = 0;
    bool hasFeatures( wFeatureSet const &feature ) const;
    virtual TPriority getDeleteOrder() const
    {
      return WAM_DELORDER_PLAIN;
    }

    // these are meant as default implementations. no need to call these
    // from subclasses
    virtual void registerMe()
    {}
    virtual void startUp()
    {}
    virtual void prepareToDie()
    {}
    virtual void unregisterMe()
    {}

    TId id() const
    {
      return Id;
    }

    // do not call, internal
    void setId( TId id )
    {
      Id = id;
    }
};




#endif
