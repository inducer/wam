// ----------------------------------------------------------------------------
//  Description      : WAM game physics
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/string_pack.hh"
#include "engine/instancemanager.hh"
#include "environment.hh"
#include "physics.hh"




#define XML_ATTR_SPEED            "speed"




// wPhysicalGameject ----------------------------------------------------------
wPhysicalGameject::wPhysicalGameject( wGame &g )
    : wGameject( g ), Speed( 0, 0 )
{
  EnvironmentManager = dynamic_cast<wEnvironmentManager *>(
                         Game.getInstanceManager().getObjectByFeatureSet( GJFEATURE_MANAGER
                             GJFEATURE_ENVIRONMENT ) );
}




wPhysicsVector wPhysicalGameject::getImpulse() const
{
  return Speed * getMass();
}




void wPhysicalGameject::applyAirStep( float seconds )
{
  if ( EnvironmentManager )
    EnvironmentManager->applyGravity( Speed, seconds );
  double speed_abs_sqr = Speed * Speed;
  double speed_abs = sqrt( speed_abs_sqr );

  wPhysicsVector friction_force = -Speed * getAirFrictionConstant() * speed_abs;
  Speed += friction_force * seconds;
}




void wPhysicalGameject::processIdealCollision( wGameject *opponent, wPhysicsVector const &normal )
{
  Speed += normal * 2 * NUM_ABS( normal * Speed );
  Speed *= getElasticity();

  if ( opponent )
  {
    // use impulse conservation
    // *** FIXME CODEME
  }
}
