// ----------------------------------------------------------------------------
//  Description      : WAM effects
// ----------------------------------------------------------------------------
//  (c) Copyright 2003 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/string_pack.hh"
#include "engine/gamebase.hh"
#include "engine/scrolling.hh"
#include "engine/message.hh"
#include "engine/instancemanager.hh"
#include "utility/debug.hh"
#include "fx.hh"




// wExplosion -----------------------------------------------------------------
float_random  		wExplosion::RNG;
bool  			wExplosion::RNGInitialized = false;




wExplosion::wExplosion( wGame &g, const wPhysicsVector &position, float radius, float intensity )
  : wGameject( g ), wDrawable( g ), wTickReceiver( g ), 
  GrayBall( g.getImageManager(), string( "exp_ball_gray.png" ) ),
  DarkBall( g.getImageManager(), string( "exp_ball_dark.png" ) ),
  BoomSound( g, "explode.wav" ),
  Position( position ), Radius( radius ), Intensity( intensity )
{
  if ( !RNGInitialized )
    RNG.init();

  RespewTimeout = Intensity;
  TotalRespewTimeout = RespewTimeout;

  Game.getMessageManager().send( 
      Game.getMessageManager().registerMessage( MSG_EXPLOSION ),
	NULL, "", this );
}




wExplosion::~wExplosion()
{
  FOREACH( first, PiecesList, wPiecesList )
    delete *first;
}




void wExplosion::registerMe()
{
  wDrawable::registerMe();
  wTickReceiver::registerMe();
}




void wExplosion::startUp()
{
  wDrawable::startUp();
  wTickReceiver::startUp();

  Radius *= 1.5;
  addBubbles( int( Radius * Radius * M_PI * 4e-3 * Intensity ) );
  BoomSound.play();
  LastBoomSoundSecondsAgo = 0;
  show();
}




void wExplosion::prepareToDie()
{
  wTickReceiver::prepareToDie();
  wDrawable::prepareToDie();
}




void wExplosion::unregisterMe()
{
  wTickReceiver::unregisterMe();
  wDrawable::unregisterMe();
}




wDisplayExtent wExplosion::getDisplayExtent()
{
  wDisplayExtent result;

  // no scale exceeds 1,1
  FOREACH( first, PiecesList, wPiecesList )
    result.unite( (*first)->Image->extent() + 
	round( (*first)->Position ) - 
	Game.getScrollManager().getPosition() );
  return result;
}




void wExplosion::draw( drawable &dpy )
{
  FOREACH( first, PiecesList, wPiecesList )
  {
    wImage scaled;
    scaled.stretchFrom( *(*first)->Image, (*first)->ScaleX, (*first)->ScaleY );

    wGameVector pos = round( (*first)->Position ) - Game.getScrollManager().getPosition();
    scaled.blit( dpy, pos[0], pos[1] );
  }
}




void wExplosion::tick( float seconds )
{
  wPiecesList::iterator first = PiecesList.begin(), last = PiecesList.end();

  requestRedraw();
  while ( first != last )
  {
    if ( (*first)->OutwardFactor > 0 )
    {
      (*first)->Position += (*first)->OutwardVector * 
	NUM_MIN( (*first)->OutwardFactor, seconds * 10 );
      (*first)->OutwardFactor = 
	NUM_MAX( 0, (*first)->OutwardFactor - seconds * 10 );
    }

    double aspect_ratio = (*first)->ScaleY / (*first)->ScaleX;
    (*first)->ScaleX -= 0.1 * seconds;
    (*first)->ScaleY -= 0.1 * seconds * aspect_ratio;

    if ( (*first)->ScaleX < 0.001 || (*first)->ScaleY < 0.001 )
    {
      first = PiecesList.erase( first );
      last = PiecesList.end();
      continue;
    }

    (*first)->Position[1] -= seconds * (*first)->MoveUpSpeed;

    first++;
  }
  requestRedraw();

  LastBoomSoundSecondsAgo += seconds;
  if ( RespewTimeout >= 0 && RNG( TotalRespewTimeout ) <= seconds + RespewTimeout * 0.25 )
  {
    addBubbles( int( Radius * Radius * M_PI * 2e-3 * Intensity ) );
    if ( LastBoomSoundSecondsAgo > 0.3 )
    {
      BoomSound.play();
      LastBoomSoundSecondsAgo = 0;
    }
  }
  RespewTimeout -= seconds;

  if ( PiecesList.size() == 0 )
    Game.getInstanceManager().requestDestruction( this );
}




void wExplosion::addBubbles( int count )
{
  while ( count-- )
  {
    float angle = RNG( M_PI * 2 );
    float radius = RNG( Radius );
    auto_ptr<wExplosionPiece> piece( new wExplosionPiece );

    if ( RNG( 1 ) > 0.6 )
      piece->Image = DarkBall.get();
    else
      piece->Image = GrayBall.get();

    piece->Position = Position;
    piece->OutwardVector[0] = radius * cos( angle );
    piece->OutwardVector[1] = radius * sin( angle );
    piece->OutwardFactor = 0.1 + RNG( 0.9 );
    piece->ScaleX = 0.05 + RNG( 0.05 );
    piece->ScaleY = 0.05 + RNG( 0.05 );

    piece->MoveUpSpeed = RNG( 40 );

    PiecesList.push_back( piece.release() );
  }
}
