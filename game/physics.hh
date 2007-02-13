// ----------------------------------------------------------------------------
//  Description      : WAM game physics
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_PHYSICS
#define WAM_PHYSICS




#include "engine/gamebase.hh"
#include "engine/gameject.hh"




class wEnvironmentManager;




typedef coord_vector<double, 2> wPhysicsVector;




class wPhysicalGameject : virtual public wGameject
{
  protected:
    wPhysicsVector Speed;
    wEnvironmentManager *EnvironmentManager;

  public:
    wPhysicalGameject( wGame &g );

    virtual double getMass() const = 0;
    virtual double getAirFrictionConstant() const = 0; // 0 < x < 1
    virtual double getElasticity() const = 0; // 0 < x < 1

    wPhysicsVector getImpulse() const;

    void applyAirStep( float seconds );
    void processIdealCollision( wGameject *opponent, wPhysicsVector const &normal );
};




#endif
