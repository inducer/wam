// ----------------------------------------------------------------------------
//  Description      : scrolling manager
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_SCROLLMANAGER
#define WAM_SCROLLMANAGER




#include "engine/tick.hh"
#include "engine/persistence.hh"




// registration ---------------------------------------------------------------
#define GJFEATURESET_SCROLLMGR GJFEATURE_MANAGER "SclM"

void registerScrollManager( wGame &g );




// constants ------------------------------------------------------------------
#define SCROLL_NEVER  -1000
#define SCROLL_AVATAR  0
#define SCROLL_ACTION  50
#define SCROLL_ALWAYS           100




// wScrollManager -------------------------------------------------------------
class wScrollManager : public wTickReceiver, public wPersistentGameject
{
  protected:
    coord_vector<double, 2> Position;
    wGameVector Target;
    double Velocity,StayForSeconds;
    TPriority Priority;

    TSize WorldWidth, WorldHeight;

  public:
    wScrollManager( wGame &g );
    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_SCROLLMGR;
    }
    void tick( float seconds );

    TPersistenceMode getPersistenceMode() const
    {
      return PERSISTENCE_SINGLETON;
    }
    TPriority getSaveOrder() const
    {
      return GJSAVE_MANAGER;
    }
    void load( xml_file::tag &tag );
    void save( xml_file::tag &tag );

    TPriority getDeleteOrder() const
    {
      return WAM_DELORDER_MANAGER;
    }
    void registerMe();
    void unregisterMe();

    wGameVector getPosition() const;
    wGameVector getTarget() const;
    void scrollTo( wGameVector const &pos, TPriority priority, double stay_for_seconds = 0);
    void makeVisible( wGameExtent const &ext, TPriority priority, double stay_for_seconds = 0 );

    void setVelocity( double velocity )
    {
      Velocity = velocity;
    }
    double getVelocity() const
    {
      return Velocity;
    }

    void setWorldSize( TSize width, TSize height );
    TSize getWorldWidth() const
    {
      return WorldWidth;
    }
    TSize getWorldHeight() const
    {
      return WorldHeight;
    }

    wGameVector getMaxPosition() const;
};




#endif

