// ----------------------------------------------------------------------------
//  Description      : WAM collision manager
// ----------------------------------------------------------------------------
//  Remarks          :
//    One important thing to note here is that there are two kinds of 
//    collisions.
//      I) Bounce-off collisions. Here, a collision state is never 
//         sustained. This works using the tryMove() idiom.
//      II) Sustained collisions. Here, a gameject moves into another
//          one's shape and actually stays there. This works
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_COLLISIONMANAGER
#define WAM_COLLISIONMANAGER




#include <ixlib_garbage.hh>
#include "utility/manager.hh"
#include "utility/shapebitmap.hh"
#include "engine/gameject.hh"




// wCollisionInfo -------------------------------------------------------------
typedef wShapeBitmap::TPreciseVector wNormalVector;
class wShaped;
class wCollidable;

enum TCollisionCode 
{
  COLLISION_START,
  COLLISION_END,
  COLLISION_BOUNCE_OFF
};

struct wCollisionInfo
{
  TCollisionCode Code;
  wCollidable *Me; 
  wShaped *Opponent;
  wGameVector Position;
  wGameVector RimPoint; // in game coordinates
  wNormalVector Normal;
};

typedef list<wCollisionInfo> wCollisionList;




// wShaped -------------------------------------------------------------------
class wShaped : virtual public wGameject
{
    wGameVector		ShapePosition;
    const wShapeBitmap	*ShapeBitmap;

  public:
    wShaped( wGame &g );

    void registerMe();
    void prepareToDie();
    void unregisterMe();

    void setShapePosition( const wGameVector &shapepos );
    void setShapeBitmap( const wShapeBitmap *shapebitmap );

    wGameExtent getGameExtent() const;

  friend class wCollidable;
  friend class wCollisionManager;
};




// wCollidable ---------------------------------------------------------------
class wCollidable : public wShaped
{
  public:


    wCollidable( wGame &g );

    /** Queries for collision information for a move to newpos.
     * \return true if a collision would take place
    */
    bool queryMoveCollisions( wGameVector const &newpos, wCollisionList *collisions = NULL );
    /** Implements bounce-off collision.
     * If a move without a collision to \c newpos is possible,
     * execute it.
     * Otherwise, file collision info and don't move.
     * \return whether the move succeeded.
     */
    bool tryMove( wGameVector const &newpos );

    virtual void collideWith( TCollisionCode code, wShaped *target, wGameVector const &pos, wNormalVector const &normal )
    { }

    void registerMe();
    void unregisterMe();

    enum TFindGroundResult { FOUND, NOTFOUND_UPWARDS, NOTFOUND_DOWNWARDS };
    TFindGroundResult findGround( wGameVector &speculative_pos, TSize max_search,
                                  wGameVector const &up = wGameVector( 0, -1 ) );
};




// wCollisionManager ---------------------------------------------------------
class wShaped;
class wCollidable;
class wCollisionManager : public wRegisteringManager<wShaped>,
      public wRegisteringManager<wCollidable>
{
    typedef wRegisteringManager<wShaped> ShapedSuper;
    typedef wRegisteringManager<wCollidable> CollidableSuper;

    wCollisionList NotificationList;

    typedef list<wShaped *> wShapedList;
    wShapedList ChangeList;

  public:
    void run();

    typedef CollidableSuper::iterator iterator;
    typedef CollidableSuper::const_iterator const_iterator;

    void reset();

    iterator begin()
    {
      return CollidableSuper::begin();
    }
    const_iterator begin() const
    {
      return CollidableSuper::begin();
    }
    iterator end()
    {
      return CollidableSuper::end();
    }
    const_iterator end() const
    {
      return CollidableSuper::end();
    }

    void registerObject( wShaped *o );
    void unregisterObject( wShaped *o );
    void registerObject( wCollidable *o );
    void unregisterObject( wCollidable *o );
    void dumpBitmap( wShapeBitmap &game_bits );

    void notifyChange( wShaped *o );
    void queryCollisions( wCollidable *from, wGameVector const &newpos, wCollisionList &collisions );
    void fileCollisions( wCollisionList const &collisions );

  private:
    bool gatherCollisionInfo( wCollidable *from, wShaped *partner,
        wGameVector const &newpos, wGameVector const &speed, wCollisionInfo &info );
};





// console interface ----------------------------------------------------------
void registerCollisionManagerCommands( wGame &g );




#endif
