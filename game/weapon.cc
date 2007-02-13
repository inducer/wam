// ----------------------------------------------------------------------------
//  Description      : WAM weapons
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/string_pack.hh"
#include "engine/gamebase.hh"
#include "engine/scrolling.hh"
#include "engine/instancemanager.hh"
#include "utility/debug.hh"
#include "weapon.hh"
#include "fx.hh"




#define XML_ATTR_ENABLED  "enabled"




// wWeapon --------------------------------------------------------------------
TMessageId wWeapon::IdWeaponEnumerate, wWeapon::IdWeaponLookup;




wWeapon::wWeapon( wGame &g )
    : wGameject( g ), wMessageReceiver( g ), Enabled( true )
{
  IdWeaponEnumerate = g.getMessageManager().registerMessage( MSG_WEAPON_ENUMERATE );
  IdWeaponLookup = g.getMessageManager().registerMessage( MSG_WEAPON_LOOKUP );
}




ref<wWeapon::wMessageReturn>
wWeapon::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == IdWeaponEnumerate || ( id == IdWeaponLookup && par1 == getWeaponId() ) )
  {
    ref<wMessageReturn> res = new wMessageReturn;
    res->second = this;
    return res;
  }
  return ref<wMessageReturn>( NULL );
}




// wBazooka -------------------------------------------------------------------
wBazooka::wBazooka( wGame &g )
: wGameject( g ), wWeapon( g ), wDrawable( g ), wTickReceiver( g ),
  Interacting( false ),
  CrossHair( g.getImageManager(), string( "cross.png" ) ),
  SteeringUp( false ), SteeringDown( false ), Firing( false ),
  CenterOffset( 0, -15 )
{
  wMessageManager & mm = Game.getMessageManager();

  IdUpPress = mm.registerMessage( "+" MSG_STEER_UP );
  IdUpRelease = mm.registerMessage( "-" MSG_STEER_UP );
  IdDownPress = mm.registerMessage( "+" MSG_STEER_DOWN );
  IdDownRelease = mm.registerMessage( "-" MSG_STEER_DOWN );
  IdFirePress = mm.registerMessage( "+" MSG_FIRE );
  IdFireRelease = mm.registerMessage( "-" MSG_FIRE );
}




char const *wBazooka::getWeaponId() const
{
  return "bazooka";
}




TIndex wBazooka::getWeaponClass() const
{
  return WC_ROCKET;
}




void wBazooka::registerMe()
{
  wWeapon::registerMe();
  wDrawable::registerMe();
  wTickReceiver::registerMe();
}




void wBazooka::startUp()
{
  wWeapon::startUp();
  wDrawable::startUp();
  wTickReceiver::startUp();
}




void wBazooka::prepareToDie()
{
  wTickReceiver::prepareToDie();
  wDrawable::prepareToDie();
  wWeapon::prepareToDie();
}




void wBazooka::unregisterMe()
{
  wTickReceiver::unregisterMe();
  wDrawable::unregisterMe();
  wWeapon::unregisterMe();
}




wDisplayExtent wBazooka::getDisplayExtent()
{
  double rad = Direction / 180 * M_PI;

  affine_transformation tx;
  tx.identity();
  tx.rotate( rad );

  wImage rotated;
  rotated.transformFrom( *CrossHair, tx );

  wScreenVector pos = 
    Position - Game.getScrollManager().getPosition() + CenterOffset;
  pos[0] += (int) (-sin( rad ) * 60);
  pos[1] += (int) (-cos( rad ) * 60);

  return rotated.extent() + pos;
}




void wBazooka::draw( drawable &dpy )
{
  double rad = Direction / 180 * M_PI;

  affine_transformation tx;
  tx.identity();
  tx.rotate( rad );

  wImage rotated;
  rotated.transformFrom( *CrossHair, tx );

  wScreenVector pos = 
    Position - Game.getScrollManager().getPosition() + CenterOffset;
  pos[0] += (int) (-sin( rad ) * 60);
  pos[1] += (int) (-cos( rad ) * 60);

  rotated.blit( dpy, pos[0], pos[1] );
}




ref<wBazooka::wMessageReturn> 
wBazooka::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == IdUpPress )
    SteeringUp = true;
  else if ( id == IdUpRelease )
    SteeringUp = false;
  else if ( id == IdDownPress )
    SteeringDown = true;
  else if ( id == IdDownRelease )
    SteeringDown = false;
  else if ( id == IdFirePress )
    Firing = true;
  else if ( id == IdFireRelease )
  {
    Firing = false;
    if ( !Interacting )
      return NULL;

    wPhysicsVector speed;
    double rad = Direction / 180 * M_PI;
    speed[0] = -sin( rad );
    speed[1] = -cos( rad );

    wPhysicsVector float_pos = Position + CenterOffset;
    float_pos += speed * 30;

    speed *= 200;

    auto_ptr<wGameject> rocket( new wRocket( Game, float_pos, speed ) );
    Game.getInstanceManager().insertObject( rocket.get() );
    rocket.release();
  }
  else
    return wWeapon::receiveMessage( id, par1, par2 );

  return NULL;
}




void wBazooka::tick( float seconds )
{
  if ( Game.getPauseMode() || !Interacting )
    return;

  float speed = 0;
  if ( SteeringUp )
    speed = -90;
  if ( SteeringDown )
    speed = 90;

  if ( speed != 0 )
  {
    requestRedraw();

    if ( 0 <= Direction && Direction < 180 )
    {
      float newdir = Direction + seconds * speed;

      if ( 0 <= newdir && newdir < 180 )
	Direction = newdir;
    }

    if ( 180 <= Direction && Direction < 360 )
    {
      float newdir = Direction - seconds * speed;

      if ( 180 <= newdir && newdir < 360 )
	Direction = newdir;
    }
    requestRedraw();
  }
}




void wBazooka::startInteraction()
{
  Interacting = true;
  show();
}




void wBazooka::stopInteraction()
{
  Interacting = false;
  hide();
}




void wBazooka::setFiringPosition( const wGameVector &pos )
{
  requestRedraw();
  Position = pos;
  requestRedraw();
}




void wBazooka::setFiringDirection( float direction )
{
  requestRedraw();
  Direction = direction;
  requestRedraw();
}




float wBazooka::getFiringDirection()
{
  return Direction;
}





// wRocket --------------------------------------------------------------------
wRocket::wRocket( wGame &g, const wPhysicsVector &position, wPhysicsVector &speed )
  : wGameject( g ), wDrawable( g ), wTickReceiver( g ), wCollidable( g ),
  wPhysicalGameject( g ), wAutoReplicatingNetworkedGameject( g ),
  Position( position ),
  Animator( g.getImageManager(), *g.getFileManager().openFile( WAM_FT_DATA, "rocket.ani" ) ),
  Shape( Game.getShapeBitmapManager(), "rocket.png" ),
  CurrentImage( NULL )
{
  Speed = speed;
}




void wRocket::registerMe()
{
  wDrawable::registerMe();
  wTickReceiver::registerMe();
  wCollidable::registerMe();
  wPhysicalGameject::registerMe();
  wAutoReplicatingNetworkedGameject::registerMe();
}




void wRocket::startUp()
{
  wDrawable::startUp();
  wTickReceiver::startUp();
  wCollidable::startUp();
  wPhysicalGameject::startUp();
  wAutoReplicatingNetworkedGameject::startUp();

  show();
  setShapeBitmap( Shape.get() );
}




void wRocket::prepareToDie()
{
  wAutoReplicatingNetworkedGameject::prepareToDie();
  wPhysicalGameject::prepareToDie();
  wCollidable::prepareToDie();
  wTickReceiver::prepareToDie();
  wDrawable::prepareToDie();
}




void wRocket::unregisterMe()
{
  wAutoReplicatingNetworkedGameject::unregisterMe();
  wPhysicalGameject::unregisterMe();
  wCollidable::unregisterMe();
  wTickReceiver::unregisterMe();
  wDrawable::unregisterMe();
}




wDisplayExtent wRocket::getDisplayExtent()
{
  if ( CurrentImage )
    return CurrentImage->extent() + ( round( Position ) - Game.getScrollManager().getPosition() );
  else
    return wDisplayExtent();
}




void wRocket::draw( drawable &dpy )
{
  if ( CurrentImage )
  {
    wScreenVector pos = round( Position ) - Game.getScrollManager().getPosition();
    CurrentImage->blit( dpy, pos[ 0 ], pos[ 1 ] );
  }
}




void wRocket::tick( float seconds )
{
  if ( Game.getPauseMode() )
    return;

  requestRedraw();
  Speed += seconds * 500. * Speed * 1/(sqrt( Speed * Speed ));
  applyAirStep( seconds );
  Position = Position + Speed * seconds;
  tryMove( round( Position ) );

  float deg = atan2( -Speed[ 1 ], Speed[ 0 ] ) / M_PI * 180;
  Animator.change( "direction", float2dec( deg ) );

  Animator.step( seconds );
  if ( Animator.isCurrentFrameChanged() )
  {
    requestRedraw();
    CurrentImage = Animator.getCurrentFrame();
    requestRedraw();
  }

  // check if rocket went out of bounds
  const int out_margin = 300;
  if ( Position[ 0 ] < -out_margin || Position[ 0 ] > Game.getScrollManager().getWorldWidth() + out_margin 
      || Position[ 1 ] < -out_margin || Position[ 1 ] > Game.getScrollManager().getWorldHeight() + out_margin )
    Game.getInstanceManager().requestDestruction( this );
}




void wRocket::collideWith( TCollisionCode code, wShaped *target, wGameVector const &pos, wNormalVector const &normal )
{
  wFeatureSetProcessor fp( target->getFeatureSet() );
  if ( fp.hasFeature( GJFEATURE_SOLID ) )
  {
    Game.getInstanceManager().requestDestruction( this );

    auto_ptr<wGameject> explosion( new wExplosion( Game, Position, 50, 0.5 ) );
    Game.getInstanceManager().insertObject( explosion.get() );
    explosion.release();
  }
}




void wRocket::writeStateDump( wStreamWriter &writer )
{
}




void wRocket::writeUpdate( wStreamWriter &writer )
{
}




void wRocket::readStateDump( wStreamReader &reader )
{
}




void wRocket::readUpdate( wStreamReader &reader )
{
}

