// ----------------------------------------------------------------------------
//  Description      : WAM server-side collision manager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <set>
#include <ixlib_numeric.hh>
#include "utility/debug.hh"
#include "utility/string_pack.hh"
#include "utility/manager_impl.hh"
#include "engine/names.hh"
#include "engine/console.hh"
#include "engine/commandmanager.hh"
#include "engine/resource.hh"
#include "engine/collision.hh"
#include "engine/gamebase.hh"




// wShaped -------------------------------------------------------------------
wShaped::wShaped( wGame &g )
: wGameject( g ),ShapePosition( 0, 0 ), ShapeBitmap( NULL )
{}




void wShaped::registerMe()
{
  Game.getCollisionManager().registerObject( this );
  wGameject::registerMe();
}




void wShaped::prepareToDie()
{
  wGameject::prepareToDie();
}




void wShaped::unregisterMe()
{
  wGameject::unregisterMe();
  Game.getCollisionManager().unregisterObject( this );
}




void wShaped::setShapePosition( const wGameVector &shapepos )
{
  ShapePosition = shapepos;
  Game.getCollisionManager().notifyChange( this );
}




void wShaped::setShapeBitmap( const wShapeBitmap *shapebitmap )
{
  ShapeBitmap = shapebitmap;
  Game.getCollisionManager().notifyChange( this );
}




wGameExtent wShaped::getGameExtent() const
{
  wGameExtent rect;

  if ( ShapeBitmap )
  {
    rect.A = ShapePosition - 
      wGameVector( ShapeBitmap->referencePointX(), ShapeBitmap->referencePointY() );
    rect.B = rect.A + wGameVector( ShapeBitmap->width(), ShapeBitmap->height() );
  }
  else
    rect.set( ShapePosition, ShapePosition );
  return rect;
}




// wCollidable ---------------------------------------------------------------
wCollidable::wCollidable( wGame &g )
    : wGameject( g ), wShaped( g )
{}




void wCollidable::registerMe()
{
  // bypass registration as shaped
  Game.getCollisionManager().registerObject( this );
}




void wCollidable::unregisterMe()
{
  // bypass registration as shaped
  Game.getCollisionManager().unregisterObject( this );
}




bool wCollidable::queryMoveCollisions( wGameVector const &newpos, wCollisionList *collisions )
{
  wCollisionList my_collisions;
  if ( !collisions )
    collisions = &my_collisions;

  Game.getCollisionManager().queryCollisions( this, newpos, *collisions );
  return collisions->size() != 0;
}




bool wCollidable::tryMove( const wGameVector &newpos )
{
  if ( newpos == ShapePosition )
    return true;

  wCollisionList collisions;
  bool result = !queryMoveCollisions( newpos, &collisions );
  if ( result )
    setShapePosition( newpos );
  else
  {
    FOREACH( first, collisions, wCollisionList )
      first->Code = COLLISION_BOUNCE_OFF;
    Game.getCollisionManager().fileCollisions( collisions );
  }

  return result;
}




wCollidable::TFindGroundResult wCollidable::findGround( wGameVector &speculative_pos, TSize max_search, wGameVector const &up )
{
  bool in_ground = queryMoveCollisions( speculative_pos );
  if ( in_ground )
  {
    // search upwards
    for ( unsigned offset = 1;offset <= max_search;offset += 1 )
    {
      if ( !queryMoveCollisions( speculative_pos + up * offset ) )
      {
        speculative_pos += up * offset;
        return FOUND;
      }
    }
    return NOTFOUND_UPWARDS;
  }
  else
  {
    // search downwards
    for ( unsigned offset = 1;offset <= max_search + 1;offset += 1 )
    {
      if ( queryMoveCollisions( speculative_pos - up * offset ) )
      {
        speculative_pos -= up * ( offset - 1 );
        return FOUND;
      }
    }
    return NOTFOUND_DOWNWARDS;
  }
}




// template instantiation -----------------------------------------------------
template wRegisteringManager<wCollidable>
;
template wRegisteringManager<wShaped>
;




// wCollisionManager ----------------------------------------------------------
void wCollisionManager::run()
{
  // *** FIXME CODEME sustained collision detection
  /*
  FOREACH_CONST( first, ChangeList, wShapedList )
  {

  }
  */
  ChangeList.clear();

  FOREACH_CONST( first, NotificationList, wCollisionList )
    first->Me->collideWith( first->Code, first->Opponent, first->RimPoint, first->Normal );
  NotificationList.clear();
}




void wCollisionManager::reset()
{
  NotificationList.clear();
}




void wCollisionManager::registerObject( wShaped *o )
{
  ShapedSuper::registerObject( o );
}




void wCollisionManager::unregisterObject( wShaped *o )
{
  ShapedSuper::unregisterObject( o );
}




void wCollisionManager::registerObject( wCollidable *o )
{
  CollidableSuper::registerObject( o );
}




void wCollisionManager::unregisterObject( wCollidable *o )
{
  CollidableSuper::unregisterObject( o );
}




void wCollisionManager::dumpBitmap( wShapeBitmap &game_bits )
{
  wGameExtent game_ext;
  bool is_first = true;

  FOREACH_CONST( first, CollidableSuper::ObjectList, CollidableSuper::wObjectList )
  {
    wGameExtent ext = ( *first ) ->getGameExtent();
    if ( is_first )
    {
      game_ext = ext;
      is_first = false;
    }
    else
      game_ext.unite( ext );
  }

  FOREACH_CONST( first, ShapedSuper::ObjectList, ShapedSuper::wObjectList )
  {
    wGameExtent ext = ( *first ) ->getGameExtent();
    if ( is_first )
    {
      game_ext = ext;
      is_first = false;
    }
    else
      game_ext.unite( ext );
  }

  game_bits.create( game_ext.getSizeX(), game_ext.getSizeY() );
  game_bits.wipe();
  game_bits.setDrawMode( wShapeBitmap::XOR );
  game_bits.setReferencePoint( ( int ) - game_ext.A[ 0 ], ( int ) - game_ext.A[ 1 ] );

  FOREACH_CONST( first, CollidableSuper::ObjectList, CollidableSuper::wObjectList )
  {
    wGameVector pos = ( *first ) ->ShapePosition;
    wShapeBitmap const *sbmp = ( *first ) ->ShapeBitmap;
    if ( sbmp )
      game_bits.drawBitmap( pos[ 0 ], pos[ 1 ], *sbmp );
  }

  FOREACH_CONST( first, ShapedSuper::ObjectList, ShapedSuper::wObjectList )
  {
    wGameVector pos = ( *first ) ->ShapePosition;
    wShapeBitmap const *sbmp = ( *first ) ->ShapeBitmap;
    if ( sbmp )
      game_bits.drawBitmap( pos[ 0 ], pos[ 1 ], *sbmp );
  }
}




void wCollisionManager::notifyChange( wShaped *o )
{
  ChangeList.push_back( o );
}




void
wCollisionManager::queryCollisions( wCollidable *me, wGameVector const &newpos, wCollisionList &collisions)
{
  wGameVector speed = newpos - me->ShapePosition;
  wGameExtent my_ext = me->getGameExtent();

  wCollisionInfo info;

  FOREACH_CONST( first, CollidableSuper::ObjectList, CollidableSuper::wObjectList )
  {
    wGameExtent ext = ( *first ) ->getGameExtent();
    if ( ( *first ) != me && ext.doesIntersect( my_ext ) )
    {
      if ( gatherCollisionInfo( me, *first, newpos, speed, info ) )
	collisions.push_back( info );
    }
  }

  FOREACH_CONST( first, ShapedSuper::ObjectList, ShapedSuper::wObjectList )
  {
    wGameExtent ext = ( *first ) ->getGameExtent();
    if ( ( *first ) != me && ext.doesIntersect( my_ext ) )
    {
      if ( gatherCollisionInfo( me, *first, newpos, speed, info ) )
	collisions.push_back( info );
    }
  }
}




void wCollisionManager::fileCollisions( wCollisionList const &collisions )
{
  FOREACH_CONST( first, collisions, wCollisionList )
    NotificationList.push_back( *first );
}




bool
wCollisionManager::gatherCollisionInfo( wCollidable *me,
    wShaped *opponent, wGameVector const &newpos, wGameVector const &speed,
    wCollisionInfo &info )
{
  wShapeBitmap const * my_sb = me->ShapeBitmap;
  wShapeBitmap const *sb = opponent->ShapeBitmap;

  // *** FIXME we might also generate useful collision data in this case
  if ( !my_sb || !sb )
  {
    EXGEN_NYI
  }

  wShapeBitmap::TCoordVector my_refp( my_sb->referencePointX(), my_sb->referencePointY() );
  wShapeBitmap::TCoordVector refp( sb->referencePointX(), sb->referencePointY() );

  wGameVector displacement = opponent->ShapePosition - newpos;
  wShapeBitmap::TCoordVector where;

  if ( my_sb->doesCollide( my_refp + displacement, *sb, &where ) )
  {
    wShapeBitmap::TCoordVector rimpoint;
    wShapeBitmap::TPreciseVector normal;

    if ( speed[ 0 ] || speed[ 1 ] )
    {
      sb->getRimPoint( where - displacement + refp - my_refp, speed, rimpoint );
      sb->getNormal( rimpoint, speed, normal );
    }
    else
      normal.set( 0, 0 );

    info.Me = me;
    info.Opponent = opponent;
    info.Position = me->ShapePosition + where;
    info.RimPoint = opponent->ShapePosition + rimpoint;
    info.Normal = normal;
    return true;
  }
  return false;
}




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( dump_collision_bitmap, "CbDp", "dump collision bitmap to file", "file" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "dump_collision_bitmap", 1, 1 )

  ofstream file( parameters[ 0 ] ->toString().c_str() );
  if ( file.bad() )
    EXGAME_THROWINFO( ECGAME_IOERROR, parameters[ 0 ] ->toString().c_str() );
  wShapeBitmap game_bits;
  Game.getCollisionManager().dumpBitmap( game_bits );
  game_bits.save( file );
  return makeNull();
}




WAM_DECLARE_COMMAND( list_collidables, "LCol", "list objects registered to CollisionManager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_collidables", 0, 0 )

  auto_ptr<js_array> arr( new js_array() );

  TIndex idx = 0;

  FOREACH_CONST( first, Game.getCollisionManager(), wCollisionManager )
  ( *arr ) [ idx++ ] = makeConstant( ( *first ) ->id() );

  return arr.release();
}




void registerCollisionManagerCommands( wGame &g )
{
  register_dump_collision_bitmap( g );
  register_list_collidables( g );
}
