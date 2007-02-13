// ----------------------------------------------------------------------------
//  Description      : WAM weapons
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_WEAPON
#define WAM_WEAPON




#include <ixlib_random.hpp>
#include "engine/animator.hh"
#include "engine/message.hh"
#include "engine/drawable.hh"
#include "engine/collision.hh"
#include "engine/tick.hh"
#include "engine/resource.hh"
#include "engine/network.hh"
#include "engine/input.hh"
#include "game_names.hh"
#include "physics.hh"




// wWeapon -------------------------------------------------------------------
/** A weapon handles all the UI issues of firing a weapon.
 */
class wWeapon : public wMessageReceiver
{
  protected:
    bool Enabled;

    static TMessageId IdWeaponEnumerate, IdWeaponLookup;

  public:
    wWeapon( wGame &g );
    ref<wMessageReturn> receiveMessage( TMessageId id, string const &par1, void *par2 );

    virtual char const *getWeaponId() const = 0;
    virtual TIndex getWeaponClass() const = 0;

    virtual void startInteraction() = 0;
    virtual void stopInteraction() = 0;
    virtual void setFiringPosition( const wGameVector &pos ) = 0;
    /// Sets the firing direction in degrees counterclockwise.
    virtual void setFiringDirection( float direction ) = 0;
    /// Gets the firing direction in degrees counterclockwise.
    virtual float getFiringDirection() = 0;
};



/*
// wWeaponMenu ---------------------------------------------------------------
class wWeaponMenu : public wDrawable,public wMouseInputListener,
  public wMessageReceiver {
    typedef vector<wWeapon> wWeaponList;
    
  public:
    wWeaponMenu(wGame &g);
 
    wFeatureSet const getFeatureSet() const;
    
    void registerMe();
    void unregisterMe();
    
    void draw(wImage &screen, wRectangle &cliprect);
    wRectangle getDisplayExtent();
    TPriority getDrawPriority() const;
    TPriority getMouseInputPriority() const;
    bool processMouseEvent(wMouseEvent const &event);
    ref<wMessageReturn> receiveMessage(TMessageId id,string const &par1,void *par2);
  };
*/



// wBazooka -------------------------------------------------------------------
class wBazooka : public wWeapon, public wDrawable, public wTickReceiver
{
    bool Interacting;
    wImageHolder CrossHair;
    bool SteeringUp,SteeringDown,Firing;

    float Direction;
    wGameVector Position;
    wGameVector CenterOffset;

    TMessageId IdUpPress,IdUpRelease,IdDownPress,IdDownRelease;
    TMessageId IdFirePress,IdFireRelease;

  public:
    wBazooka( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_BAZOOKA;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_SPRITE;
    }

    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    ref<wMessageReturn> receiveMessage( TMessageId id, string const &par1, void *par2 );

    void tick( float seconds );

    char const *getWeaponId() const;
    TIndex getWeaponClass() const;
    void startInteraction();
    void stopInteraction();
    void setFiringPosition( const wGameVector &pos );
    void setFiringDirection( float direction );
    float getFiringDirection();
};



// wRocket --------------------------------------------------------------------
class wRocket : public wDrawable, public wTickReceiver, public wCollidable,
  public wPhysicalGameject, public wAutoReplicatingNetworkedGameject
{
    wPhysicsVector Position;
    wAnimator Animator;
    wShapeBitmapHolder Shape;

    wImage *CurrentImage;

  public:
    wRocket( wGame &g, const wPhysicsVector &position, wPhysicsVector &speed );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_ROCKET;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    double getMass() const
    {
      return 0.5;
    }
    double getAirFrictionConstant() const
    {
      return 1e-6;
    }
    double getElasticity() const
    {
      return 0.5;
    }

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_SPRITE;
    }

    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float seconds );

    void collideWith( TCollisionCode code, wShaped *target, wGameVector const &pos, wNormalVector const &normal );

    // network
    void writeStateDump( wStreamWriter &writer );
    void writeUpdate( wStreamWriter &writer );
    void readStateDump( wStreamReader &reader );
    void readUpdate( wStreamReader &reader );
};




#endif
