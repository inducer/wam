// ----------------------------------------------------------------------------
//  Description      : WAM principal avatar :-)
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include <cmath>
#include "utility/debug.hh"
#include "utility/string_pack.hh"
#include "engine/scrolling.hh"
#include "engine/instancemanager.hh"
#include "hero.hh"
#include "fx.hh"




#define XML_ATTR_WEAPON   "weapon"
#define XML_ATTR_DIRECTION  "direction"
#define XML_ATTR_STATE   "state"
#define XML_ATTR_SPEED   "speed"
#define XML_ATTR_POSITION  "position"




// wHero ----------------------------------------------------------------------
wHero::wHero( wGame &g )
    : wGameject( g ), wTickReceiver( g ), wCollidable( g ), wDrawable( g ),
    wMessageReceiver( g ), wPhysicalGameject( g ),
    wAutoReplicatingNetworkedGameject( g ),
    wPersistentGameject( g ),
    Weapon( NULL ),
    Dying( false ),
    Position( 0, 0 ),
    InstantiationComplete( false ),
    HaveInputFocus( false ), 
    TrackTime( 0 ), 
    Health( 0 ), 
    HealthDisplay( NULL ),
    Animator( g.getImageManager(), *g.getFileManager().openFile( WAM_FT_DATA, "hero.ani" ) ),
    JumpSound( g, "humph.wav" ),
    DieSound( g, "die.wav" ),
    SteeringLeft( false ), SteeringRight( false ), CurrentImage( NULL ),
    Shape( Game.getShapeBitmapManager(), "mole.png" )
{
  registerMessages();

  auto_ptr<wHeroHealthDisplay> health_disp( new wHeroHealthDisplay( Game ) );
  HealthDisplay = health_disp.get();
  Game.getInstanceManager().insertObject( health_disp.release() );
}




void wHero::setupHero( string const &identifier, TGameCoordinate start_x, int health )
{
  Position[0] = start_x;
  Identifier = identifier;
  
  Health = health;

  notifyInstantiationCompleted();
}




void wHero::registerMe()
{
  wTickReceiver::registerMe();
  wMessageReceiver::registerMe();
  wCollidable::registerMe();
  wDrawable::registerMe();
  wAutoReplicatingNetworkedGameject::registerMe();
  wPersistentGameject::registerMe();
}




void wHero::startUp()
{
  wTickReceiver::startUp();
  wMessageReceiver::startUp();
  wCollidable::startUp();
  wDrawable::startUp();
  wAutoReplicatingNetworkedGameject::startUp();
  wPersistentGameject::startUp();
}




void wHero::prepareToDie()
{
  Game.getInstanceManager().requestDestruction( HealthDisplay );

  wTickReceiver::prepareToDie();
  wMessageReceiver::prepareToDie();
  wCollidable::prepareToDie();
  wDrawable::prepareToDie();
  wAutoReplicatingNetworkedGameject::prepareToDie();
  wPersistentGameject::prepareToDie();
}




void wHero::unregisterMe()
{
  wTickReceiver::unregisterMe();
  wMessageReceiver::unregisterMe();
  wCollidable::unregisterMe();
  wDrawable::unregisterMe();
  wAutoReplicatingNetworkedGameject::unregisterMe();
  wPersistentGameject::unregisterMe();
}




void wHero::notifyInstantiationCompleted()
{
  wamPrintDebug( "hero instantiation completed" );
  setShapeBitmap( Shape.get() );
  setShapePosition( round( Position ) );

  setWeapon( lookupWeapon( "bazooka" ) );
  setDirection( 90 );
  setState( FLY );

  show();
  InstantiationComplete = true;

  HealthDisplay->setIdentifier( Identifier );
  HealthDisplay->setHealth( Health );
}




wDisplayExtent wHero::getDisplayExtent()
{
  wDisplayExtent result;

  if ( CurrentImage )
    result.unite( CurrentImage->extent() + 
	( round( Position ) - Game.getScrollManager().getPosition() ) );
  return result;
}




void wHero::draw( drawable &dpy )
{
  if ( CurrentImage )
  {
    wScreenVector pos = round( Position ) - Game.getScrollManager().getPosition();
    CurrentImage->blit( dpy, pos[ 0 ], pos[ 1 ] );
  }
}




void wHero::collideWith( TCollisionCode code, wShaped *target, wGameVector const &pos, wNormalVector const &normal )
{
  if ( NUM_ABS( Speed[ 1 ] ) > NUM_ABS( Speed[ 0 ] ) && NUM_ABS( Speed[ 1 ] ) > 120 )
    Animator.change( "state", "land" );
  if ( State != WALK )
  {
    processIdealCollision( target, normal );
    // stand there if:
    // * post hit speed small enough
    // * normal does not point down
    if ( sqrt( Speed * Speed ) < 5 && normal[ 1 ] < 0 )
      setState( WALK );
  }
}




void wHero::tick( float seconds )
{
  if ( Game.getPauseMode() || !InstantiationComplete )
    return;

  double steer_x = 0;
  if ( SteeringLeft && HaveInputFocus )
    steer_x += -1;
  if ( SteeringRight && HaveInputFocus )
    steer_x += 1;

  if ( HaveInputFocus && Weapon )
    setDirection( Weapon->getFiringDirection() );

  Animator.step( seconds );
  if ( Animator.isCurrentFrameChanged() )
  {
    requestRedraw();
    CurrentImage = Animator.getCurrentFrame();
    requestRedraw();
  }

  if ( Animator.countPendingSignals() )
  {
    string sig = Animator.getSignal();
    if ( sig == "jump" )
    {
      double const amount = 150;
      setState( FLY );

      Speed = wPhysicsVector( steer_x, -2 ) * amount;
      JumpSound.play();
    }
    if ( sig == "die" )
      Game.getMessageManager().post( 
	  Game.getMessageManager().registerMessage( MSG_SUPERVISOR_AVATAR_REQUEST_DEATH ),
	  Identifier );
  }

  if ( State == WALK || State == STAND )
  {
    if ( steer_x == 0 )
      setState( STAND );
    else
      setState( WALK );

    wPhysicsVector newpos = Position + wPhysicsVector( steer_x, 0 ) * 50.0 * seconds;
    wGameVector rounded_newpos = round( newpos );
    wGameVector rounded_position = round( Position );

    TSize dist = NUM_MIN( 5, TSize( NUM_ABS( rounded_newpos[ 0 ] - rounded_position[ 0 ] ) * 3 ) );
    switch ( findGround( rounded_newpos, dist ) )
    {
      case FOUND:
	newpos[ 1 ] = rounded_newpos[ 1 ];
	setHeroPosition( newpos );
	break;
      case NOTFOUND_UPWARDS:
	break;
      case NOTFOUND_DOWNWARDS:
	newpos[ 1 ] = Position[ 1 ];
	setHeroPosition( newpos );
	setState( FLY );
	break;
    }
  }
  else if ( State == FLY )
  {
    applyAirStep( seconds );

    wPhysicsVector newpos = Position + Speed * seconds;
    if ( tryMove( round( newpos ) ) )
      setHeroPosition( newpos );
  }

  // TrackTime is the time for which we'll want to track the hero
  if ( TrackTime > 0 || steer_x !=0 || State == PREJUMP )
  {
    if ( TrackTime > 0 )
    {
      TrackTime -= seconds;
      if ( TrackTime < 0 )
	TrackTime = 0;
    }

    if ( HaveInputFocus )
    {
      Game.getScrollManager().makeVisible( getGameExtent(), SCROLL_AVATAR );
    }
  }

  // check if hero went out of bounds
  const int out_margin = 300;
  if ( Position[ 0 ] < -out_margin || Position[ 0 ] > Game.getScrollManager().getWorldWidth() + out_margin 
      || Position[ 1 ] < -out_margin || Position[ 1 ] > Game.getScrollManager().getWorldHeight() + out_margin )
    startToDie();
}




ref<wHero::wMessageReturn>
wHero::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == IdAvatarDie )
  {
    actuallyDie();
  }
  else if ( id == IdExplosion && InstantiationComplete )
  {
    wExplosion *expl = reinterpret_cast<wExplosion *>( par2 );
    wPhysicsVector offset = Position - expl->position();
    double dist = sqrt( offset * offset );
    double dist_offset = NUM_MAX( dist - expl->radius(), 0 );
    double falloff_factor;
    if ( dist_offset == 0 )
      falloff_factor = 1;
    else
      falloff_factor = pow( dist_offset, -dist_offset / 300 );

    double lost_health = expl->intensity() * 100 * falloff_factor;
    loseHealth( (int) lost_health );

    if ( falloff_factor > 0.05 )
    {
      setState( FLY );
      Speed += offset * expl->intensity() * falloff_factor * 5;
    }
  }
  else if ( id == IdGetFocus && InstantiationComplete )
  {
    HaveInputFocus = true;
    TrackTime = 6;
    HealthDisplay->startShowing();
    if ( Weapon )
    {
      Weapon->setFiringPosition( round( Position ) );
      Weapon->setFiringDirection( Direction );
      Weapon->startInteraction();
    }
  }
  else if ( id == IdLoseFocus )
  {
    HaveInputFocus = false;
    if ( Weapon )
    {
      Weapon->stopInteraction();
      Direction = Weapon->getFiringDirection();
    }
  }
  else if ( id == IdLeftPress )
    SteeringLeft = true;
  else if ( id == IdLeftRelease )
    SteeringLeft = false;
  else if ( id == IdRightPress )
    SteeringRight = true;
  else if ( id == IdRightRelease )
    SteeringRight = false;

  if ( !Game.getPauseMode() && HaveInputFocus )
  {
    float dir = Weapon->getFiringDirection();
    if ( 0 < dir && dir < 180 && SteeringRight )
      setDirection( 360 - dir );
    if ( 180 < dir && dir < 360 && SteeringLeft )
      setDirection( 360 - dir );
    Weapon->setFiringDirection( Direction );

    if ( id == IdJumpPress && ( State == WALK || State == STAND ) )
    {
      Animator.change( "state", "stand" );
      setState( PREJUMP );
      TrackTime = 4;
    }

    if ( id == IdJumpRelease && State == PREJUMP )
      Animator.change( "state", "jump" );

    if ( State == FLY )
    {
      double const amount = 20;
      if ( id == IdLeftPress )
        Speed += wPhysicsVector( -amount, 0 );
      if ( id == IdRightPress )
        Speed += wPhysicsVector( amount, 0 );
    }
  }

  return ref<wHero::wMessageReturn>( NULL );
}




void wHero::load( xml_file::tag &tag )
{
  tag.Attributes[ XML_ATTR_POSITION ] >>= Position;
  tag.Attributes[ XML_ATTR_SPEED ] >>= Speed;

  int state;
  tag.Attributes[ XML_ATTR_STATE ] >>= state;
  State = ( TState ) state;

  double direction;
  tag.Attributes[ XML_ATTR_DIRECTION ] >>= direction;
  setDirection( direction );

  string weapon_id;
  tag.Attributes[ XML_ATTR_WEAPON ] >>= weapon_id;
  if ( weapon_id.size() )
    setWeapon( lookupWeapon( weapon_id ) );
}




void wHero::save( xml_file::tag &tag )
{
  tag.Attributes[ XML_ATTR_POSITION ] <<= Position;
  tag.Attributes[ XML_ATTR_SPEED ] <<= Speed;

  tag.Attributes[ XML_ATTR_STATE ] <<= ( int ) State;
  tag.Attributes[ XML_ATTR_DIRECTION ] <<= Direction;
  if ( Weapon )
    tag.Attributes[ XML_ATTR_WEAPON ] <<= Weapon->getWeaponId();
}




// network --------------------------------------------------------------------
void wHero::writeStateDump( wStreamWriter &writer )
{
}




void wHero::writeUpdate( wStreamWriter &writer )
{
}




void wHero::readStateDump( wStreamReader &reader )
{
  notifyInstantiationCompleted();
}




void wHero::readUpdate( wStreamReader &reader )
{
}




// private --------------------------------------------------------------------
wWeapon *wHero::lookupWeapon( string const &weapon_id )
{
  wMessageManager::wResultSet res;
  Game.getMessageManager().send(
    Game.getMessageManager().registerMessage( MSG_WEAPON_LOOKUP ),
    &res, weapon_id );

  if ( res.size() == 0 )
    EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "weapon with id " + weapon_id ).c_str() )
    return reinterpret_cast<wWeapon *>( res[ 0 ] ->second );
}




void wHero::setWeapon( wWeapon *weapon )
{
  if ( Weapon && HaveInputFocus )
    Weapon->stopInteraction();
  Weapon = weapon;
  if ( Weapon )
  {
    if ( HaveInputFocus )
      Weapon->startInteraction();
    Animator.change( "weapon", Weapon->getWeaponId() );
  }
  else
    Animator.change( "weapon", "none" );
}




void wHero::setState( TState state )
{
  if ( State == state )
    return ;

  if ( state == STAND )
  {
    Animator.change( "state", "stand" );
  }
  else if ( state == WALK )
  {
    Animator.change( "state", "walk" );
  }
  else if ( state == PREJUMP )
  {
    Animator.change( "state", "prejump" );
  }
  else
  {
    Animator.change( "state", "fly" );
  }

  State = state;
}




void wHero::setHeroPosition( wPhysicsVector &pos )
{
  wGameVector old_rounded_pos = round( Position );
  wGameVector rounded_pos = round( pos );

  if ( old_rounded_pos != rounded_pos )
    requestRedraw();
  Position = pos;
  if ( old_rounded_pos != rounded_pos )
  {
    requestRedraw();
    setShapePosition( rounded_pos );
    if ( Weapon && HaveInputFocus )
      Weapon->setFiringPosition( rounded_pos );
    HealthDisplay->setPosition( rounded_pos );
  }
  if ( networkRole() == ORIGINAL )
    requestSendUpdate();
}




void wHero::setDirection( double dir )
{
  double fullcirc = floor( dir / 360 );
  dir -= fullcirc * 360;
  if ( dir < 0 )
    dir += 360;

  unsigned previous_ani_dir = unsigned( Direction * 20 / 360 );
  Direction = dir;
  unsigned ani_dir = unsigned( Direction * 20 / 360 );

  if ( ani_dir != previous_ani_dir )
    Animator.change( "direction", unsigned2dec( ani_dir, 2 ) );
}




void wHero::registerMessages()
{
  wMessageManager & mm = Game.getMessageManager();

  IdJumpPress = mm.registerMessage( "+" MSG_STEER_JUMP );
  IdJumpRelease = mm.registerMessage( "-" MSG_STEER_JUMP );
  IdLeftPress = mm.registerMessage( "+" MSG_STEER_LEFT );
  IdLeftRelease = mm.registerMessage( "-" MSG_STEER_LEFT );
  IdRightPress = mm.registerMessage( "+" MSG_STEER_RIGHT );
  IdRightRelease = mm.registerMessage( "-" MSG_STEER_RIGHT );
  IdGetFocus = mm.registerMessage( MSG_SUPERVISOR_GET_FOCUS );
  IdLoseFocus = mm.registerMessage( MSG_SUPERVISOR_LOSE_FOCUS );
  IdExplosion = mm.registerMessage( MSG_EXPLOSION );
  IdAvatarDie = mm.registerMessage( MSG_SUPERVISOR_AVATAR_DIE );
}




void wHero::loseHealth( int amount )
{
  Health -= amount;
  HealthDisplay->loseHealth( amount );

  if ( Health <= 0 )
    startToDie();
}




void wHero::startToDie()
{
  if ( Dying )
    return;

  DieSound.play();

  // stay visible for a while
  Game.getScrollManager().makeVisible( getGameExtent(), SCROLL_ACTION, 4 );

  // notify supervisor of death
  Game.getMessageManager().send( 
      Game.getMessageManager().registerMessage( MSG_SUPERVISOR_AVATAR_BEGIN_DEATH ),
      NULL,
      Identifier );

  // begin death
  Dying = true;
  Animator.change( "state", "die" );
}




void wHero::actuallyDie()
{
  // self-destruct
  Game.getInstanceManager().requestDestruction( this );

  // spawn a grave
  auto_ptr<wGrave> grave( new wGrave( Game, Position ) );
  Game.getInstanceManager().insertObject( grave.release() );
}




// wHeroHealthDisplay ---------------------------------------------------------
wHeroHealthDisplay::wHeroHealthDisplay( wGame &g )
  : wGameject( g ), wTickReceiver( g ), wDrawable( g ), Health( 0 ),
    RecentlyLostHealth( 0 ), LostHealthOffset( 0 ),
    ShowHealthTimer( 0 ),
    Font( Game.getFontManager(), "ingame.fnt" )
{
}




void wHeroHealthDisplay::registerMe()
{
  wTickReceiver::registerMe();
  wDrawable::registerMe();
}




void wHeroHealthDisplay::startUp()
{
  wTickReceiver::startUp();
  wDrawable::startUp();
}




void wHeroHealthDisplay::prepareToDie()
{
  wTickReceiver::prepareToDie();
  wDrawable::prepareToDie();
}




void wHeroHealthDisplay::unregisterMe()
{
  wTickReceiver::unregisterMe();
  wDrawable::unregisterMe();
}




void wHeroHealthDisplay::setPosition( wGameVector &position )
{
  requestRedraw();
  Position = position;
  requestRedraw();
}




void wHeroHealthDisplay::startShowing()
{
  if ( ShowHealthTimer <= 0 )
    show();
  ShowHealthTimer = 6;
}




void wHeroHealthDisplay::setIdentifier( const string &id )
{
  requestRedraw();
  Identifier = id;
  requestRedraw();
}




void wHeroHealthDisplay::setHealth( int amount )
{
  requestRedraw();
  Health = amount;
  requestRedraw();
}




void wHeroHealthDisplay::loseHealth( int amount )
{
  if ( amount == 0 )
    return;

  Health -= amount;
  RecentlyLostHealth += amount;
  LostHealthOffset = 0;
  startShowing();
}




wDisplayExtent wHeroHealthDisplay::getDisplayExtent()
{
  wDisplayExtent result;

  wDisplayExtent total_ext( boxExtent( Identifier, signed2dec( Health ) ) );
  result.unite( total_ext
      + wGameVector( - total_ext.width() / 2, - 40 - total_ext.height() ) 
      + Position
      - Game.getScrollManager().getPosition() );

  if ( RecentlyLostHealth != 0 )
  {
    wDisplayExtent total_ext( boxExtent( signed2dec( (int) RecentlyLostHealth ), "" ) );
    result.unite( total_ext
	+ wGameVector( - total_ext.width() / 2, - 40 - total_ext.height() - (int) LostHealthOffset ) 
	+ Position
	- Game.getScrollManager().getPosition() );
  }

  return result;
}




void wHeroHealthDisplay::draw( drawable &dpy )
{
  wImage img;
  renderToBox( Identifier, signed2dec( Health ), img );
  wScreenVector pos = Position - Game.getScrollManager().getPosition();
  img.blit( dpy, pos[0] - img.width() / 2, pos[1] - 40 - img.height() );

  if ( RecentlyLostHealth != 0 )
  {
    wImage img;
    renderToBox( signed2dec( (int) RecentlyLostHealth ), "" , img );
    img.blit( dpy, pos[0] - img.width() / 2, pos[1] - 40 - img.height() - (int) LostHealthOffset );
  }

}



void wHeroHealthDisplay::tick( float seconds )
{
  if ( ShowHealthTimer > 0 )
  {
    ShowHealthTimer -= seconds;
    if ( ShowHealthTimer < 0 )
    {
      requestRedraw();
      ShowHealthTimer = 0;
      hide();
    }
  }


  if ( RecentlyLostHealth != 0 )
  {
    requestRedraw();
    LostHealthOffset += seconds * 30;
    if ( LostHealthOffset > 50 )
      RecentlyLostHealth = 0;
    requestRedraw();
  }
}




wDisplayExtent wHeroHealthDisplay::boxExtent( const string &line1, const string &line2 )
{
  int lines = 1;
  if ( line2.size() )
    lines = 2;

  int font_height = Font->BottomLine - Font->TopLine - 3;
  wDisplayExtent ext = Font->extent( line1 );
  int width_line1 = ext.width();
  ext = Font->extent( line2 );
  int width_line2 = ext.width();
  int width = NUM_MAX( width_line1, width_line2 );

  return wDisplayExtent( 0, 0, 10 + width, 8 + font_height * lines );
}




void wHeroHealthDisplay::renderToBox( const string &line1, const string &line2, wImage &img )
{
  int lines = 1;
  if ( line2.size() )
    lines = 2;

  int font_height = Font->BottomLine - Font->TopLine - 3;
  wDisplayExtent ext = Font->extent( line1 );
  int width_line1 = ext.width();
  ext = Font->extent( line2 );
  int width_line2 = ext.width();
  int width = NUM_MAX( width_line1, width_line2 );

  img.create( Game.getDrawableManager().getDisplay().format(),
      10 + width, 8 + font_height * lines );

  TColor colorkey = mapColor( Game.getDrawableManager().getDisplay().format(), 255, 0, 255 );
  TColor black = mapColor( Game.getDrawableManager().getDisplay().format(), 0, 0, 0 ); 
  TColor white = mapColor( Game.getDrawableManager().getDisplay().format(), 255, 255, 255 ); 
  img.colorKey( colorkey, SDL_SRCCOLORKEY | SDL_RLEACCEL );
  img.drawingColor( black );
  img.fillBox( 0, 0, img.width(), img.height() );

  img.drawingColor( white );
  img.drawBox( 0, 0, img.width() - 1, img.height() - 1 );
  img.drawBox( 1, 1, img.width() - 2, img.height() - 2 );

  img.drawingColor( colorkey );
  img.setPixel( 0, 0 );
  img.setPixel( 0, img.height() - 1 );
  img.setPixel( img.width() - 1, img.height() - 1 );
  img.setPixel( img.width() - 1, 0 );

  Font->print( img, img.width() / 2 - width_line1 / 2, 3, line1 );
  Font->print( img, img.width() / 2 - width_line2 / 2, 3 + font_height, line2 ) ;
}




// wGrave ---------------------------------------------------------------------
wGrave::wGrave( wGame &g, wPhysicsVector &pos )
  : wGameject( g ), wDrawable( g ), wTickReceiver( g ), 
  wCollidable( g ),wPhysicalGameject( g ),
  Position( pos ),
  Grave( g.getImageManager(), string( "grave.png" ) ),
  Shape( g.getShapeBitmapManager(), "grave.png" )
{
}




void wGrave::registerMe()
{
  wDrawable::registerMe();
  wTickReceiver::registerMe();
  wCollidable::registerMe();
  wPhysicalGameject::registerMe();
}




void wGrave::startUp()
{
  wDrawable::startUp();
  wTickReceiver::startUp();
  wCollidable::startUp();
  wPhysicalGameject::startUp();

  show();
  setShapeBitmap( Shape.get() );
}




void wGrave::prepareToDie()
{
  wDrawable::prepareToDie();
  wTickReceiver::prepareToDie();
  wCollidable::prepareToDie();
  wPhysicalGameject::prepareToDie();
}




void wGrave::unregisterMe()
{
  wDrawable::unregisterMe();
  wTickReceiver::unregisterMe();
  wCollidable::unregisterMe();
  wPhysicalGameject::unregisterMe();
}




wDisplayExtent wGrave::getDisplayExtent()
{
  return Grave->extent() + round( Position )
    - Game.getScrollManager().getPosition();
}




void wGrave::draw( drawable &dpy )
{
  wGameVector drawpos = round( Position ) - Game.getScrollManager().getPosition();
  Grave->blit( dpy, drawpos[0], drawpos[1] );
}




void wGrave::tick( float seconds )
{
  if ( Game.getPauseMode() )
    return;

  applyAirStep( seconds );

  wPhysicsVector newpos = Position + Speed * seconds;
  if ( tryMove( round( newpos ) ) )
  {
    requestRedraw();
    Position = newpos;
    requestRedraw();
  }
  else
    Speed.set( 0, 0 );

  // check if grave went out of bounds
  const int out_margin = 300;
  if ( Position[ 0 ] < -out_margin || Position[ 0 ] > Game.getScrollManager().getWorldWidth() + out_margin 
      || Position[ 1 ] < -out_margin || Position[ 1 ] > Game.getScrollManager().getWorldHeight() + out_margin )
    Game.getInstanceManager().requestDestruction( this );
}

