// ----------------------------------------------------------------------------
//  Description      : WAM principal avatar :-)
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_HERO
#define WAM_HERO




#include <ixlib_xml.hh>
#include "engine/gamebase.hh"
#include "engine/tick.hh"
#include "engine/drawable.hh"
#include "engine/collision.hh"
#include "engine/message.hh"
#include "engine/resource.hh"
#include "engine/animator.hh"
#include "engine/network.hh"
#include "engine/persistence.hh"
#include "game_names.hh"
#include "weapon.hh"
#include "physics.hh"




class wHeroHealthDisplay;

class wHero : public wTickReceiver, public wCollidable, public wDrawable,
      public wMessageReceiver, public wPhysicalGameject,
      public wAutoReplicatingNetworkedGameject, public wPersistentGameject
{
    string Model;
    string Identifier;

    wWeapon *Weapon;

    typedef TUnsigned8 TState;
    static const TState STAND = 0;
    static const TState WALK = 1;
    static const TState PREJUMP = 2;
    static const TState FLY = 3;
    TState State;

    bool Dying;

    double Direction; // degrees ccw, 0 at top
    wPhysicsVector Position;

    bool InstantiationComplete,HaveInputFocus;
    double TrackTime;

    int Health;
    wHeroHealthDisplay *HealthDisplay;

    wAnimator Animator;

    TMessageId IdJumpPress, IdJumpRelease;
    TMessageId IdLeftPress, IdLeftRelease;
    TMessageId IdRightPress, IdRightRelease;
    TMessageId IdGetFocus, IdLoseFocus;
    TMessageId IdExplosion, IdAvatarDie;

    wSoundHolder JumpSound, DieSound;

    bool SteeringLeft, SteeringRight;

    wImage *CurrentImage;
    wShapeBitmapHolder Shape;

  public:
    wHero( wGame &g );

    void setupHero( string const &identifier, TGameCoordinate start_x, int health );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_HERO;
    }
    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_SPRITE;
    }
    double getMass() const
    {
      return 50;
    }
    double getAirFrictionConstant() const
    {
      return 1e-4;
    }
    double getElasticity() const
    {
      return 0.1;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    void notifyInstantiationCompleted();

    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void collideWith( TCollisionCode code, wShaped *target, wGameVector const &pos, wNormalVector const &normal );
    void tick( float seconds );
    ref<wMessageReturn> receiveMessage( TMessageId id, string const &par1, void *par2 );

    TPersistenceMode getPersistenceMode() const
    {
      return PERSISTENCE_TRANSIENT;
    }
    TPriority getSaveOrder() const
    {
      return GJSAVE_DEFAULT;
    }
    void load( xml_file::tag &tag );
    void save( xml_file::tag &tag );

    // network
    void writeStateDump( wStreamWriter &writer );
    void writeUpdate( wStreamWriter &writer );
    void readStateDump( wStreamReader &reader );
    void readUpdate( wStreamReader &reader );

  private:
    wWeapon *lookupWeapon( string const &weapon_id );
    void setModel( string const &model );
    void setWeapon( wWeapon *weapon );
    void setState( TState state );
    void setHeroPosition( wPhysicsVector &pos );
    void setDirection( double dir );
    void registerMessages();
    void loseHealth( int amount );
    void startToDie();
    void actuallyDie();
};




class wHeroHealthDisplay : public wTickReceiver, public wDrawable
{
    int Health, RecentlyLostHealth;
    double LostHealthOffset;
    double ShowHealthTimer;

    string Identifier;
    wGameVector Position;

    wFontHolder Font;

  public:
    wHeroHealthDisplay( wGame &g );

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_HERO_HEALTH;
    }
    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_STATUS;
    }

    void setPosition( wGameVector &vector );
    void startShowing();
    void setIdentifier( const string &id );
    void setHealth( int amount );
    void loseHealth( int amount );

    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float seconds );

    wDisplayExtent boxExtent( const string &line1, const string &line2 );
    void renderToBox( const string &line1, const string &line2, wImage &img );
};




class wGrave : public wDrawable, public wTickReceiver, 
  public wCollidable, public wPhysicalGameject
{
    wPhysicsVector Position;
    wImageHolder Grave;
    wShapeBitmapHolder Shape;

  public:
    wGrave( wGame &g, wPhysicsVector &pos );

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    double getMass() const
    {
      return 50;

    }
    double getAirFrictionConstant() const 
    {
      return 1e-4;
    }
    double getElasticity() const 
    {
      return 0.2;
    }

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_HERO_GRAVE;
    }

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_SPRITE;
    }

    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float seconds );
};




#endif
