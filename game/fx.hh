// ----------------------------------------------------------------------------
//  Description      : WAM effects
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2003 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_FX
#define WAM_FX




#include <ixlib_random.hpp>
#include "engine/drawable.hh"
#include "engine/tick.hh"
#include "engine/resource.hh"
#include "engine/input.hh"
#include "game_names.hh"
#include "physics.hh"




// wExplosion -----------------------------------------------------------------
class wExplosion : public wDrawable, public wTickReceiver
{
    wImageHolder		GrayBall, DarkBall;
    wSoundHolder		BoomSound;

    struct wExplosionPiece {
      wImage		*Image;
      wPhysicsVector	Position;
      wPhysicsVector	OutwardVector;

      float		ScaleX,ScaleY;
      float		MoveUpSpeed;
      double		OutwardFactor;
    };

    wPhysicsVector	Position;
    float		Radius;
    float		Intensity;

    typedef vector<wExplosionPiece *>	wPiecesList;
    wPiecesList		PiecesList;

    static float_random	RNG;
    static bool		RNGInitialized;

    double		RespewTimeout,TotalRespewTimeout;
    double		LastBoomSoundSecondsAgo;

  public:
    wExplosion( wGame &g, const wPhysicsVector &position, float radius, float intensity );
    ~wExplosion();

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_EXPLOSION;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    const wPhysicsVector &position() const
    {
      return Position;
    }
    float radius() const
    {
      return Radius;
    }
    float intensity() const
    {
      return Intensity;
    }

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_FX;
    }

    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    void tick( float seconds );

    void addBubbles( int count );
};




#endif
