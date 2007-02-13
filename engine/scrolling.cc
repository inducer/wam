// ----------------------------------------------------------------------------
//  Description      : ScrollManager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <cmath>
#include <ixlib_numeric.hh>
#include "utility/debug.hh"
#include "utility/string_pack.hh"
#include "engine/gamebase.hh"
#include "engine/codemanager.hh"
#include "engine/commandmanager.hh"
#include "engine/console.hh"
#include "engine/scrolling.hh"
#include "engine/drawable.hh"




#define XML_ATTR_POS  "position"
#define XML_ATTR_VELOCITY "velocity"
#define SCROLLER_STOPATDISTANCE 1




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( sm_scrollto, "ScTo", "scroll to position", "x y" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "sm_scrollto", 2, 2 )

  Game.getScrollManager().scrollTo( wGameVector( parameters[ 0 ] ->toInt(), parameters[ 1 ] ->toInt() ), SCROLL_ALWAYS );
  return makeNull();
}




WAM_DECLARE_COMMAND( sm_velocity, "ScrV", "set/get scroll velocity", "[velocity]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "sm_velocity", 0, 1 )

  if ( parameters.size() == 1 )
  {
    Game.getScrollManager().setVelocity( parameters[ 0 ] ->toFloat() );
    return makeNull();
  }
  else
    return makeConstant( Game.getScrollManager().getVelocity() );
}




// registration ---------------------------------------------------------------
wGameject *createScrollManager( wGame &g )
{
  return ( wGameject * ) ( new wScrollManager( g ) );
}

void registerScrollManager( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_SCROLLMGR, "", createScrollManager );
  register_sm_scrollto( g );
  register_sm_velocity( g );
}




// wScrollManager -------------------------------------------------------------
wScrollManager::wScrollManager( wGame &g )
    : wGameject( g ), wTickReceiver( g ), wPersistentGameject( g ),
    Position( 0, 0 ), Target( 0, 0 ), Velocity( 0.2 ), StayForSeconds( 0 ), Priority( 0 ),
    WorldWidth( 0 ), WorldHeight( 0 )
{
}




void wScrollManager::registerMe()
{
  wTickReceiver::registerMe();
  wPersistentGameject::registerMe();
  Game.setScrollManager( this );
}




void wScrollManager::unregisterMe()
{
  Game.setScrollManager();

  wPersistentGameject::unregisterMe();
  wTickReceiver::unregisterMe();
}




wGameVector wScrollManager::getPosition() const
{
  // this is necessary to prevent rounding from making drawables
  // shake relative to each other
  return wGameVector( ( int ) Position[ 0 ], ( int ) Position[ 1 ] );
}




wGameVector wScrollManager::getTarget() const
{
  return Target;
}




void wScrollManager::tick( float seconds )
{
  wGameVector sp_before = getPosition();

  // read as weighted average
  coord_vector<double, 2> target = Target;
  Position = ( Position * Velocity + target * seconds ) / ( seconds + Velocity );

  bool xreached = NUM_ABS( Position[ 0 ] - target[ 0 ] ) < SCROLLER_STOPATDISTANCE,
                  yreached = NUM_ABS( Position[ 1 ] - target[ 1 ] ) < SCROLLER_STOPATDISTANCE;
  if ( xreached )
    Position[ 0 ] = target[ 0 ];
  if ( yreached )
    Position[ 1 ] = target[ 1 ];
  if ( xreached && yreached )
  {
    StayForSeconds -= seconds;
    if ( StayForSeconds <= 0 )
    {
      Priority = SCROLL_NEVER;
      StayForSeconds = 0;
    }
  }

  if ( sp_before != getPosition() )
  {
    Game.getDrawableManager().requestRedrawEverything();
  }
}




void wScrollManager::load( xml_file::tag &tag )
{
  tag.Attributes[ XML_ATTR_POS ] >>= Position;
  tag.Attributes[ XML_ATTR_VELOCITY ] >>= Velocity;
  Target = round( Position );
}




void wScrollManager::save( xml_file::tag &tag )
{
  tag.Attributes[ XML_ATTR_POS ] <<= Position;
  tag.Attributes[ XML_ATTR_VELOCITY ] <<= Velocity;
}




void wScrollManager::scrollTo( wGameVector const &pos, TPriority priority, double stay_for_seconds )
{
  if ( priority >= Priority )
  {
    Target = pos;
    if ( Target[ 0 ] > int( WorldWidth - Game.getDrawableManager().getDisplay().width() ) )
      Target[ 0 ] = WorldWidth - Game.getDrawableManager().getDisplay().width();
    if ( Target[ 1 ] > int( WorldHeight - Game.getDrawableManager().getDisplay().height() ) )
      Target[ 1 ] = WorldHeight - Game.getDrawableManager().getDisplay().height();
    if ( Target[ 0 ] < 0 )
      Target[ 0 ] = 0;
    if ( Target[ 1 ] < 0 )
      Target[ 1 ] = 0;
    Priority = priority;
    StayForSeconds = stay_for_seconds;
  }
}




void wScrollManager::makeVisible( wGameExtent const &ext, TPriority priority, double stay_for_seconds )
{
  wGameVector const excess( 45, 45 );
  wGameExtent screen( getPosition(),
                      getPosition() + wGameVector( Game.getDrawableManager().getDisplay().width(), Game.getDrawableManager().getDisplay().height() ) );
  screen.A += excess;
  screen.B -= excess;

  wGameVector newpos = round( Position );
  bool doscroll = false;

  if ( ext.A[ 0 ] < screen.A[ 0 ] )
  {
    doscroll = true;
    newpos[ 0 ] -= screen.A[ 0 ] - ext.A[ 0 ];
  }
  if ( ext.A[ 1 ] < screen.A[ 1 ] )
  {
    doscroll = true;
    newpos[ 1 ] -= screen.A[ 1 ] - ext.A[ 1 ];
  }

  if ( ext.B[ 0 ] > screen.B[ 0 ] )
  {
    doscroll = true;
    newpos[ 0 ] += ext.B[ 0 ] - screen.B[ 0 ];
  }
  if ( ext.B[ 1 ] > screen.B[ 1 ] )
  {
    doscroll = true;
    newpos[ 1 ] += ext.B[ 1 ] - screen.B[ 1 ];
  }

  if ( doscroll )
    scrollTo( newpos, priority, stay_for_seconds );
  else
  {
    if ( stay_for_seconds > 0 )
      scrollTo( getPosition(), priority, stay_for_seconds );
  }
}




void wScrollManager::setWorldSize( TSize width, TSize height )
{
  WorldWidth = width;
  WorldHeight = height;
}




wGameVector wScrollManager::getMaxPosition() const
{
  return wGameVector( NUM_MAX( 0, WorldWidth - Game.getDrawableManager().getDisplay().width() ),
                      NUM_MAX( 0, WorldHeight - Game.getDrawableManager().getDisplay().height() ) );
}
