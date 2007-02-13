// ----------------------------------------------------------------------------
//  Description      : Environment management
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <cmath>
#include <fstream>
#include <memory>
#include <ixlib_random.hh>
#include <ixlib_xml.hh>
#include "utility/debug.hh"
#include "utility/string_pack.hh"
#include "utility/manager_impl.hh"
#include "utility/filemanager.hh"
#include "engine/names.hh"
#include "engine/gamebase.hh"
#include "engine/codemanager.hh"
#include "engine/instancemanager.hh"
#include "engine/scrolling.hh"
#include "engine/console.hh"
#include "engine/commandmanager.hh"
#include "environment.hh"
#include "fx.hh"




#define XML_ATTR_WIND                   "wind"
#define XML_ATTR_GRAVITY                "gravity"
#define XML_ATTR_LANDSCAPE  "landscape"




// wEnvironmentManager -------------------------------------------------------
wEnvironmentManager::wEnvironmentManager( wGame &g )
  : wGameject( g ), wTickReceiver( g ), wDrawable( g ), wShaped( g ),
  wPersistentGameject( g ), wNetworkedGameject( g ), 
  wMessageReceiver( g ),
  Wind( 0 ), Gravity( 0 ),
  GroundTile( Game.getImageManager() ),
  HoleTile( Game.getImageManager() ),
  Floor( Game.getImageManager() ),
  Ceiling( Game.getImageManager() ),
  HoleFloor( Game.getImageManager() ),
  HoleCeiling( Game.getImageManager() )
{
  IdExplosion = Game.getMessageManager().registerMessage(
      MSG_EXPLOSION );
}




void wEnvironmentManager::registerMe()
{
  wShaped::registerMe();
  wDrawable::registerMe();
  wTickReceiver::registerMe();
  wPersistentGameject::registerMe();
  wNetworkedGameject::registerMe();
  wMessageReceiver::registerMe();
}




void wEnvironmentManager::startUp()
{
  wShaped::startUp();
  wDrawable::startUp();
  wTickReceiver::startUp();
  wPersistentGameject::startUp();
  wNetworkedGameject::startUp();
  wMessageReceiver::startUp();

  show();
}




void wEnvironmentManager::prepareToDie()
{
  wShaped::prepareToDie();
  wDrawable::prepareToDie();
  wTickReceiver::prepareToDie();
  wPersistentGameject::prepareToDie();
  wNetworkedGameject::prepareToDie();
  wMessageReceiver::prepareToDie();
}




void wEnvironmentManager::unregisterMe()
{
  wShaped::unregisterMe();
  wDrawable::unregisterMe();
  wPersistentGameject::unregisterMe();
  wTickReceiver::unregisterMe();
  wNetworkedGameject::unregisterMe();
  wMessageReceiver::unregisterMe();
}




wDisplayExtent wEnvironmentManager::getDisplayExtent()
{
  return Foreground.extent() - Game.getScrollManager().getPosition();
}




void wEnvironmentManager::draw( drawable &dpy )
{
  wScreenVector pos = -Game.getScrollManager().getPosition();
  Foreground.blit( dpy, pos[ 0 ], pos[ 1 ] );
}




void wEnvironmentManager::tick( float seconds )
{
  Wind += seconds * RandomNumGen( 1 );
}




ref<wEnvironmentManager::wMessageReturn>
wEnvironmentManager::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == IdExplosion )
  {
    wExplosion *exp = reinterpret_cast<wExplosion *>( par2 );
    wGameVector pos = round( exp->position() );
    punchHole( pos[0], pos[1], (int) exp->radius() );
  }
  return NULL;
}




void wEnvironmentManager::loadLandscape( string const &name )
{
  LandscapeBuildInstructions.clear();
  wStreamWriter writer( LandscapeBuildInstructions );
  writer << LOADED << name;
  writer.commit();

  wStreamReader reader( LandscapeBuildInstructions );
  executeLandscapeBuildInstructions( reader );

  if ( atMaster() )
    sendDump( NULL, NULL );
}




void wEnvironmentManager::generateLandscape( string const &description, int seed )
{
  LandscapeBuildInstructions.clear();
  {
    wStreamWriter writer( LandscapeBuildInstructions );
    generateLandscapeBuildInstructions( writer, description, seed );
  }

  wStreamStore append_end_store;
  {
    wStreamWriter writer( append_end_store );
    insert( writer, LandscapeBuildInstructions );
    writer << END;
  }
  wStreamReader reader( append_end_store );
  executeLandscapeBuildInstructions( reader );

  if ( atMaster() )
    sendDump( NULL, NULL );
}




void wEnvironmentManager::applyGravity( wPhysicsVector &speed, float seconds )
{
  speed[ 1 ] += seconds * Gravity;
}




void wEnvironmentManager::applyWind( wPhysicsVector &speed, float seconds, float friction )
{
  speed[ 0 ] += seconds * Wind * friction;
}




void wEnvironmentManager::load( xml_file::tag &tag )
{
  tag.Attributes[ XML_ATTR_WIND ] >>= Wind;
  tag.Attributes[ XML_ATTR_GRAVITY ] >>= Gravity;

  wStreamStore compressed;
  tag.Attributes[ XML_ATTR_LANDSCAPE ] >>= compressed;
  decompress( LandscapeBuildInstructions, compressed );

  {
    wStreamReader reader( LandscapeBuildInstructions );
    executeLandscapeBuildInstructions( reader );
  }
}




void wEnvironmentManager::save( xml_file::tag &tag )
{
  tag.Attributes[ XML_ATTR_WIND ] <<= Wind;
  tag.Attributes[ XML_ATTR_GRAVITY ] <<= Gravity;

  wStreamStore compressed;
  compress( compressed, LandscapeBuildInstructions );

  tag.Attributes[ XML_ATTR_LANDSCAPE ] <<= compressed;
}




// network --------------------------------------------------------------------
void wEnvironmentManager::notifyNewConnection( wNetworkConnection *cnx )
{
  requestChildCreationAt( cnx, true );
  if ( atMaster() )
    sendDump( cnx, NULL );
}




void wEnvironmentManager::notifyEndedConnection( wNetworkConnection *cnx )
{}




void wEnvironmentManager::notifyRemoteInitiation( wNetworkConnection *cnx, wObject::TId remote_id )
{
  registerAsChildOf( cnx, remote_id );
}




void wEnvironmentManager::readMessage( wNetworkConnection *from, wStreamReader &reader )
{
  TMessageKind mkind;
  reader >> mkind;

  if ( mkind == NET_MKIND_DUMP )
  {
    LandscapeBuildInstructions.clear();
    wStreamWriter writer( LandscapeBuildInstructions );
    decompress( writer, reader );
    writer.commit();

    wStreamReader reader( LandscapeBuildInstructions );
    executeLandscapeBuildInstructions( reader );
  }
}




void wEnvironmentManager::sendDump( wNetworkConnection *to, wNetworkConnection *except )
{
  wStreamStore append_end_store;
  {
    wStreamWriter writer( append_end_store );
    insert( writer, LandscapeBuildInstructions );
    writer << END;
  }

  wStreamStore send_store;
  {
    wStreamWriter writer( send_store );
    writer << NET_MKIND_DUMP;

    wStreamReader reader( append_end_store );
    compress( writer, reader );
  }

  sendNetMessage( to, except, send_store );
}




// internal stuff -------------------------------------------------------------
double wEnvironmentManager::deviate( float_random &rng, double value, double max_stepsize, double min, double max )
{
  value += rng( 2 * max_stepsize ) - max_stepsize;
  if ( value < min )
    return min;
  if ( value > max )
    return max;
  return value;
}




void wEnvironmentManager::generateLandscapePolygon( float_random &rng, string const &type,
    polygon<int> &poly, int x1, int y1, int x2, int y2 )
{
  auto_ptr<polygon_segment<int> > seg( new polygon_segment<int> );

  int width = x2 - x1;
  int height = y2 - y1;

  bool mess = true, smooth = true;

  if ( type == "ground" )
  {
    double level = rng( height );
    double dlevel = rng( 10 ) - 5;

    for ( int x = int( 19.0 * width / 20 );x > width / 20;x -= 10 )
    {
      seg->push_back_c( x + x1, int( y2 - level * ( 1 - pow( 2.0 * x / width - 1, 8 ) * 0.7 ) ) );
      dlevel = deviate( rng, dlevel, 10, -20, 20 );

      level += dlevel;
      if ( level < height / 10 )
      {
        level = height / 10;
        dlevel = rng( 5 );
      }
      if ( level > height )
      {
        level = height;
        dlevel = -rng( 5 );
      }
    }
    seg->push_back_c( x1, y2 );
    seg->push_back_c( x1, y2 + 20 );
    seg->push_back_c( x1 + 10, y2 + 30 );
    seg->push_back_c( x2 - 10, y2 + 30 );
    seg->push_back_c( x2, y2 + 20 );
    seg->push_back_c( x2, y2 );

    mess = false;
    smooth = false;
  }
  else if ( type == "ceiling" )
  {
    double level = rng( height );
    double dlevel = rng( 10 ) - 5;

    for ( int x = int( width / 20.0 );x < 19*width / 20;x += 10 )
    {
      seg->push_back_c( x + x1, int( y1 + level * ( 1 - pow( 2.0 * x / width - 1, 8 ) * 0.7 ) ) );
      dlevel = deviate( rng, dlevel, 10, -20, 20 );

      level += dlevel;
      if ( level < height / 10 )
      {
        level = height / 10;
        dlevel = rng( 5 );
      }
      if ( level > height )
      {
        level = height;
        dlevel = -rng( 5 );
      }
    }
    seg->push_back_c( x2, y1 );
    seg->push_back_c( x2, y1 - 20 );
    seg->push_back_c( x2 - 10, y1 - 30 );
    seg->push_back_c( x1 + 10, y1 - 30 );
    seg->push_back_c( x1, y1 - 20 );
    seg->push_back_c( x1, y1 );

    mess = false;
    smooth = false;
  }
  else if ( type == "rectangular" )
  {
    TSize points = int( double( width ) * height / 1e4 );

    polygon_segment<int> src;

    for ( TIndex i = 0;i < points;i++ )
      src.push_back_c(
        int( x1 + rng( width ) ),
        int( y1 + rng( height ) ) );
    src.makeConvexHull( *seg );
    messWithConvexPolygonSegment( rng, *seg, x1, y1, x2, y2 );
  }
  else if ( type == "elliptic" )
  {
    double
    radius_x = width / 2,
               radius_y = height / 2,
                          center_x = x1 + radius_x,
                                     center_y = y1 + radius_y;

    TSize points = int( double( width ) * height / 1e4 );

    polygon_segment<int> src;

    for ( TIndex i = 0;i < points;i++ )
    {
      double phi = rng( 2 * Pi );
      src.push_back_c(
        int( center_x + rng( radius_x ) * cos( phi ) ),
        int( center_y + rng( radius_y ) * sin( phi ) ) );
    }
    src.makeConvexHull( *seg );
    messWithConvexPolygonSegment( rng, *seg, x1, y1, x2, y2 );
  }
  else if ( type == "rectangle" )
  {
    seg->push_back_c( x1, y1 );
    seg->push_back_c( x1, y2 );
    seg->push_back_c( x2, y2 );
    seg->push_back_c( x2, y1 );
    mess = false;
  }
  else if ( type == "ellipse" )
  {
    double
    radius_x = width / 2,
               radius_y = height / 2,
                          center_x = x1 + radius_x,
                                     center_y = y1 + radius_y;

    for ( double phi = 0;phi < 2*Pi;phi += Pi / 16 )
      seg->push_back_c(
        int( center_x + radius_x * cos( phi ) ),
        int( center_y - radius_y * sin( phi ) ) );

    mess = false;
    smooth = false;
  }
  else
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, ( "invalid chunk type: " + type ).c_str() )

    poly.push_back( seg.release() );
  if ( smooth )
    poly.subdivide();
  if ( mess )
    messWithConvexPolygonSegment( rng, *poly.front(), x1, y1, x2, y2, 0.1 );
  if ( smooth )
  {
    poly.smooth();
    poly.smooth();
  }
}




void wEnvironmentManager::digHoles( float_random &rng, wShapeBitmap &holemap, int x1, int y1, int x2, int y2 )
{
  if ( x1 < 0 )
    x1 = 0;
  if ( y1 < 0 )
    y1 = 0;
  if ( x2 > int( holemap.width() ) )
    x2 = holemap.width();
  if ( y2 > int( holemap.height() ) )
    y2 = holemap.height();

  int width = x2 - x1;
  int height = y2 - y1;

  const double
  aspect_ratio = double( width ) / height,
                 min_radius_x = 30,
                                min_radius_y = 30,
                                               max_radius_x = 40,
                                                              max_radius_y = 40 / aspect_ratio;

  double
  radius_x = min_radius_x + rng( max_radius_x - min_radius_x ),
             radius_y = min_radius_x + rng( max_radius_x - min_radius_x ),
                        speed_x = rng( 1 ) - 0.5,
                                  speed_y = rng( 1 ) - 0.5,
                                            x = rng( width ),
                                                y = rng( height );

  TSize paint_count = TSize( double( width ) * height / 3000 );

  while ( paint_count-- )
  {
    holemap.fillEllipse( x1 + int( x ), y1 + int( y ), int( radius_x ), int( radius_y ) );

    radius_x = deviate( rng, radius_x, 5, min_radius_x, max_radius_x );
    radius_y = deviate( rng, radius_y, 5, min_radius_y, max_radius_y );

    // mutate speed
    { speed_y *= aspect_ratio;

      double direction = atan2( speed_y, speed_x );
      double abs = NUM_MIN( radius_x, radius_y ) * 0.3;

      direction += rng( 0.8 ) - 0.4;

      speed_x = abs * cos( direction );
      speed_y = abs * sin( direction );

      // generate a tendency away from the edges
      speed_x -= ( 20 + rng( 20 ) ) * pow( 2 * x / width - 1, 5 );
      speed_y -= ( 20 + rng( 20 ) ) * pow( 2 * y / height - 1, 5 );

      speed_y /= aspect_ratio;
    }

    x += speed_x;
    y += speed_y;

    // make caves
    if ( rng( 200 ) <= 1 )
      speed_x = -speed_x;
  }
}




void wEnvironmentManager::messWithConvexPolygonSegment( float_random &rng, polygon_segment<int> &poly, int x1, int y1, int x2, int y2, double intensity )
{
  int width = x2 - x1;
  int height = y2 - y1;

  FOREACH( first, poly, polygon_segment<int> )
  {
    if ( rng( 2 ) <= 1 )
    {
      ( *first ) [ 0 ] += int( rng( width * intensity ) );
      ( *first ) [ 1 ] += int( rng( height * intensity ) );
    }
  }
  poly.removeCrossings();
}




void wEnvironmentManager::generateLandscapeBuildInstructions( wStreamWriter &writer, string const &description, int seed )
{
  wamPrintDebug( "environment: generating landscape" );

  float_random rng;
  if ( seed == 0 )
    rng.init();
  else
    rng.init( seed );

  // parse xml file
  ref<wFileManager::wStream> stream = Game.getFileManager().openFile( WAM_FT_DATA, description );
  xml_file landscape;
  landscape.read( *stream );

  xml_file::tag *root_tag = landscape.getRootTag();
  if ( root_tag->getName() != "landscape" )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "'landscape' expected" )

    xml_file::tag::const_iterator
    first = root_tag->begin(), last = root_tag->end();

  writer << BUILT;

  // theme section
  if ( first == last || ( *first ) ->getName() != "theme" )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "'theme' expected" )

    FOREACH_CONST( first_pic, **first, xml_file::tag )
    writer << THEME << ( *first_pic ) ->getName() << ( *first_pic ) ->Attributes[ "src" ];

  // read layout section
  first++;

  if ( first == last || ( *first ) ->getName() != "layout" )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "'layout' expected" )

    TSize width = evalUnsigned( ( *first ) ->Attributes[ "width" ] );
  TSize height = evalUnsigned( ( *first ) ->Attributes[ "height" ] );

  auto_ptr<wShapeBitmap> shape( new wShapeBitmap( width, height ) );
  shape->wipe( false );

  auto_ptr<wShapeBitmap> holemap( new wShapeBitmap( width, height ) );
  holemap->wipe( false );

  FOREACH_CONST( first_block, **first, xml_file::tag )
  {
    if ( ( *first_block ) ->getName() == "chunk" )
    {
      int x1 = int( evalFloat( ( *first_block ) ->Attributes[ "left" ] ) * width );
      int x2 = int( evalFloat( ( *first_block ) ->Attributes[ "right" ] ) * width );
      int y1 = int( evalFloat( ( *first_block ) ->Attributes[ "top" ] ) * height );
      int y2 = int( evalFloat( ( *first_block ) ->Attributes[ "bottom" ] ) * height );
      bool hole = ( *first_block ) ->Attributes.count( "hole" ) != 0;

      polygon<int> landscape_poly;
      generateLandscapePolygon( rng, ( *first_block ) ->Attributes[ "type" ],
                                landscape_poly, x1, y1, x2, y2 );
      shape->setDrawMode( wShapeBitmap::OR );
      shape->fillPolygon( landscape_poly );

      if ( hole )
        digHoles( rng, *holemap.get(), x1, y1, x2, y2 );
    }
  }

  // limit holes to landscape
  holemap->setDrawMode( wShapeBitmap::AND );
  holemap->drawBitmap( 0, 0, *shape );

  // dig holes into landscape
  shape->setDrawMode( wShapeBitmap::AND_NOT );
  shape->drawBitmap( 0, 0, *holemap );

  // add the shape to the holemap
  holemap->setDrawMode( wShapeBitmap::OR );
  holemap->drawBitmap( 0, 0, *shape );

  writer << SHAPE << *shape << HOLEMAP << *holemap;

  // span bridges
  first++;

  if ( first != last && ( *first ) ->getName() == "bridges" )
  {
    FOREACH_CONST( first_bridge, **first, xml_file::tag )
    {
      if ( ( *first_bridge ) ->getName() != "bridge" )
        EXGAME_THROWINFO( ECGAME_FILEFORMAT, "'bridge' expected" )

        TSigned32 left_end = int( width * evalFloat( ( *first_bridge ) ->Attributes[ "start_x" ] ) );
      TSigned32 right_end = left_end;
      TSigned32 y = int( height * evalFloat( ( *first_bridge ) ->Attributes[ "y" ] ) );

      if ( shape->getPixel( left_end, y ) )
        continue;

      while ( !shape->getPixel( left_end, y ) && left_end > 0 )
        left_end--;
      while ( !shape->getPixel( right_end, y ) && right_end < ( int ) shape->width() )
        right_end++;

      writer << BRIDGE << y << left_end << right_end;
    }

    first++;
  }

  // draw gadgets
  if ( first != last && ( *first ) ->getName() == "gadgets" )
  {
    FOREACH_CONST( first_gadget, **first, xml_file::tag )
    {
      if ( ( *first_gadget ) ->getName() != "gadget" )
        EXGAME_THROWINFO( ECGAME_FILEFORMAT, "'gadget' expected" )

        double low_x = width * evalFloat( ( *first_gadget ) ->Attributes[ "low_x" ] );
      double high_x = width * evalFloat( ( *first_gadget ) ->Attributes[ "high_x" ] );
      int start_y = int( height * evalFloat( ( *first_gadget ) ->Attributes[ "start_y" ] ) );
      double probability = evalFloat( ( *first_gadget ) ->Attributes[ "probability" ] );

      TSize count = 1;
      if ( ( *first_gadget ) ->Attributes.count( "count" ) )
        count = evalUnsigned( ( *first_gadget ) ->Attributes[ "count" ] );

      while ( count-- )
      {
        if ( rng( 1 ) > probability )
          continue;

        TSigned32 x = int( low_x + ( high_x - low_x ) * rng( 1 ) );
        TSigned32 y = start_y;

        if ( shape->getPixel( x, y ) )
          continue;
        while ( !shape->getPixel( x, y ) && y < int( shape->height() ) )
          y++;
        if ( y == int( shape->height() ) )
          continue;
        // use last unoccupied pixel
        y--;

        if ( holemap->getPixel( x, y ) )
          continue;

        writer << GADGET << ( *first_gadget ) ->Attributes[ "src" ] << x << y;
      }
    }
    first++;
  }
}




void wEnvironmentManager::executeLandscapeBuildInstructions( wStreamReader &reader )
{
  if ( reader.getRemainingSize() == 0 )
    return ;

  TUnsigned32 id;
  reader >> id;
  if ( id == LOADED )
  {
    string name;
    reader >> name;

    wImageHolder img( Game.getImageManager(), name );
    Foreground.copyFrom( *img.get() );

    wShapeBitmapHolder shape( Game.getShapeBitmapManager(), name );
    Shape.copyFrom( *shape );
  }
  else
  {
    // id == BUILT

    executeLandscapeBuildInstructions_BUILT( reader );
  }
  updateManagers();
}




void wEnvironmentManager::executeLandscapeBuildInstructions_BUILT( wStreamReader &reader )
{
  if ( reader.getRemainingSize() == 0 )
    return ;

  wamPrintDebug( "environment: painting landscape" );

  float_random rng;
  rng.init();

  wImageHolder
  bridge( Game.getImageManager() ),
  bridgehead_left( Game.getImageManager() ),
  bridgehead_right( Game.getImageManager() );

  wShapeBitmapHolder
  bridge_shape( Game.getShapeBitmapManager() ),
  bridgehead_left_shape( Game.getShapeBitmapManager() ),
  bridgehead_right_shape( Game.getShapeBitmapManager() );

  string background = "";

  TUnsigned32 id;

  reader >> id;
  while ( id == THEME )
  {
    string what, value;
    reader >> what >> value;

    if ( what == "ground_tile" )
      GroundTile.set( wImageDescriptor( value ) );
    if ( what == "floor" )
      Floor.set( wImageDescriptor( value ) );
    if ( what == "ceiling" )
      Ceiling.set( wImageDescriptor( value ) );
    if ( what == "hole_tile" )
      HoleTile.set( wImageDescriptor( value ) );
    if ( what == "hole_floor" )
      HoleFloor.set( wImageDescriptor( value ) );
    if ( what == "hole_ceiling" )
      HoleCeiling.set( wImageDescriptor( value ) );
    if ( what == "bridge" )
    {
      bridge.set( wImageDescriptor( value ) );
      bridge_shape.set( value );
    }
    if ( what == "bridgehead_left" )
    {
      bridgehead_left.set( wImageDescriptor( value ) );
      bridgehead_left_shape.set( value );
    }
    if ( what == "bridgehead_right" )
    {
      bridgehead_right.set( wImageDescriptor( value ) );
      bridgehead_right_shape.set( value );
    }
    if ( what == "background" )
      background = value;

    reader >> id;
  }

  // retrieve shapes
  if ( id != SHAPE )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "env: SHAPE expected" );
  reader >> Shape >> id;
  if ( id != HOLEMAP )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "env: HOLEMAP expected" );
  reader >> HoleMap >> id;

  // paint landscape
  Foreground.create( Game.getDrawableManager().getDisplay().format(), 
      Shape.width(), Shape.height() );

  TColor colorkey = mapColor( Game.getDrawableManager().getDisplay().format(), 255, 0, 255 );
  Foreground.colorKey( colorkey, SDL_SRCCOLORKEY );
  Foreground.drawingColor( colorkey );
  Foreground.fillBox( 0, 0, Foreground.width(), Foreground.height() );

  Foreground.fillBox( 0, 0, Foreground.width(), Foreground.height() );

  paintForeground( Foreground, 0, 0, Foreground.width(), Foreground.height() );

  // span bridges
  Shape.setDrawMode( wShapeBitmap::OR );
  while ( id == BRIDGE )
  {
    TSigned32 y, left_end, right_end;
    reader >> y >> left_end >> right_end;

    left_end += bridgehead_left->width() / 2;
    right_end -= bridgehead_right->width() / 2;

    bridgehead_left->blit( Foreground, left_end, y );
    Shape.drawBitmap( left_end, y, *bridgehead_left_shape.get() );

    while ( left_end + ( TSigned32 ) bridge->width() < right_end )
    {
      bridge->blit( Foreground, left_end, y );
      Shape.drawBitmap( left_end, y, *bridge_shape.get() );
      left_end += bridge_shape->width();
    }

    bridgehead_right->blit( Foreground, left_end, y );
    Shape.drawBitmap( left_end, y, *bridgehead_right_shape.get() );

    reader >> id;
  }

  // draw gadgets
  while ( id == GADGET )
  {
    string name;
    TSigned32 x, y;
    reader >> name >> x >> y;

    wImageHolder gadget( Game.getImageManager(), name );
    wShapeBitmapHolder gadget_shape( Game.getShapeBitmapManager(), name );

    gadget->blit( Foreground, x, y );
    Shape.drawBitmap( x, y, *gadget_shape.get() );

    reader >> id;
  }

  if ( id != END )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "env: END expected" );

  // finalize
  // OptimizedForeground.convertForAcceleratedBlitFrom( Foreground );

  if ( background.size() )
    Game.getConsole().getCommandManager().execute( "wam.prlx_setimage('" + background + "');" );
}




void wEnvironmentManager::paintForeground( wImage &fg, int x1, int y1, int x2, int y2 )
{
  TColor colorkey = fg.colorKey();

  enum TPixelState { FREE_AIR, GROUND, HOLE };
  fg.drawingTile( GroundTile.get() );
  fg.fillBox( x1, y1, x2, y2 );

  for ( wShapeBitmap::TCoordinate x = x1; x < x2; x++ )
  {
    TPixelState current = FREE_AIR;
    wImage *upper_edge = NULL, *next_upper_edge = NULL;

#define DRAW_UPPER_EDGE \
      if (upper_edge) upper_edge->blit( fg,x,begin_y);

    bool shape_change_valid = false, hole_change_valid = false;
    wShapeBitmap::TCoordinate hole_change_y, shape_change_y;

    wShapeBitmap::TCoordinate y = 0, begin_y;
    while ( y < ( wShapeBitmap::TCoordinate ) fg.height() )
    {
      begin_y = y;

      if ( !shape_change_valid )
      {
        shape_change_y = y;
        if ( !Shape.getNextPixelOfColor( x, shape_change_y, 0, 1, !Shape.getPixel( x, y ) ) )
          shape_change_y = fg.height();
        shape_change_valid = true;
      }
      if ( !hole_change_valid )
      {
        hole_change_y = y;
        if ( !HoleMap.getNextPixelOfColor( x, hole_change_y, 0, 1, !HoleMap.getPixel( x, y ) ) )
          hole_change_y = fg.height();
        hole_change_valid = true;
      }

      enum TChangeType { HOLE_CHANGE, SHAPE_CHANGE } change;

      if ( hole_change_y < shape_change_y )
      {
        y = hole_change_y;
        change = HOLE_CHANGE;
        hole_change_valid = false;
      }
      else
      {
        y = shape_change_y;
        change = SHAPE_CHANGE;
        shape_change_valid = false;
        if ( hole_change_y == shape_change_y )
          hole_change_valid = false;
      }

      switch ( current )
      {
      case GROUND:
        {
          // assuming SHAPE_CHANGE
          if ( HoleMap.getPixelNoClip( x, y ) )
          {
            current = HOLE;
            next_upper_edge = HoleCeiling.get();
          }
          else
          {
            current = FREE_AIR;
            next_upper_edge = Ceiling.get();
          }
          DRAW_UPPER_EDGE
          break;
        }
      case HOLE:
        {
          if ( change == HOLE_CHANGE )
          {
            current = FREE_AIR;
            next_upper_edge = NULL;
          }
          else
          {
            current = GROUND;
            next_upper_edge = HoleFloor.get();
          }

          fg.drawingTile( HoleTile.get(), 0, 0 );
          fg.drawVLine( x, begin_y, y );
          DRAW_UPPER_EDGE
          break;
        }
      case FREE_AIR:
        {
          if ( change == HOLE_CHANGE )
          {
            current = HOLE;
            next_upper_edge = NULL;
          }
          else
          {
            current = GROUND;
            next_upper_edge = Floor.get();
          }

          fg.drawingColor( colorkey );
          fg.drawVLine( x, begin_y, y );
          DRAW_UPPER_EDGE
          break;
        }
      }
      upper_edge = next_upper_edge;
    }
  }
}




void wEnvironmentManager::punchHole( int center_x, int center_y, int radius )
{
  Shape.setDrawMode( wShapeBitmap::AND_NOT );
  Shape.fillCircle( center_x, center_y, radius );
  setShapeBitmap( &Shape );

  Foreground.drawingColor( Foreground.colorKey() );
  Foreground.fillCircle( center_x, center_y, radius );

  Foreground.drawingTile( HoleTile.get() );
  for ( int x = center_x - radius; x < center_x + radius; x++ )
  {
    bool last_hole = HoleMap.getPixel( x, center_y - radius - 1 );
    bool last_shape = Shape.getPixel( x, center_y - radius - 1 );
    bool last_hole_nshape = last_hole && !last_shape;

    int hole_begin_y = center_y - radius;

    int y;
    for ( y = center_y - radius; y < center_y + radius + 1; y++ )
    {
      bool this_hole = HoleMap.getPixel( x, y );
      bool this_shape = Shape.getPixel( x, y );
      bool this_hole_nshape = this_hole && !this_shape;

      if ( this_hole_nshape != last_hole_nshape )
      {
	if ( this_hole_nshape )
	  hole_begin_y = y;
	else
	{
	  Foreground.drawVLine( x, hole_begin_y, y );
	  HoleFloor->blit( Foreground, x, y );
	  if ( hole_begin_y != center_y - radius )
	    HoleCeiling->blit( Foreground, x, hole_begin_y );
	}
      }

      last_hole = this_hole;
      last_shape = this_shape;
      last_hole_nshape = this_hole_nshape;
    }

    if ( last_hole_nshape && hole_begin_y != -1 )
    {
      Foreground.drawVLine( x, hole_begin_y, y );

      if ( HoleMap.getPixel( x, y ) && Shape.getPixel( x, y ) )
	HoleFloor->blit( Foreground, x, y );
      if ( hole_begin_y != center_y - radius )
	HoleCeiling->blit( Foreground, x, hole_begin_y );
    }
  }

  // OptimizedForeground.copyFrom( Foreground );
  requestRedraw();
}




void wEnvironmentManager::updateManagers()
{
  Game.getScrollManager().setWorldSize( Shape.width(), Shape.height() );
  setShapeBitmap( &Shape );
}




// console interface ----------------------------------------------------------
#define FIND_ENVI \
  wEnvironmentManager *envi= dynamic_cast<wEnvironmentManager *>( \
    Game.getInstanceManager().getObjectByFeatureSet(GJFEATURE_MANAGER \
      GJFEATURE_ENVIRONMENT));
// throws exception if not found




WAM_DECLARE_COMMAND( em_wind, GJFEATURE_MPACK_WAM "GeWi", "get/set wind strength", "[value]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "em_wind", 0, 1 )

  FIND_ENVI
  if ( parameters.size() == 1 )
  {
    envi->setWind( parameters[ 0 ] ->toFloat() );
    return makeNull();
  }
  else
    return makeConstant( envi->getWind() );
}




WAM_DECLARE_COMMAND( em_gravity, GJFEATURE_MPACK_WAM "GeWi", "get/set gravity strength", "[value]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "em_gravity", 0, 1 )

  FIND_ENVI
  if ( parameters.size() == 1 )
  {
    envi->setGravity( parameters[ 0 ] ->toFloat() );
    return makeNull();
  }
  else
    return makeConstant( envi->getGravity() );
}




WAM_DECLARE_COMMAND( em_loadlandscape, GJFEATURE_MPACK_WAM "SeBg", "load landscape from files", "filename" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "em_loadlandscape", 1, 1 )

  FIND_ENVI
  envi->loadLandscape( parameters[ 0 ] ->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( em_genlandscape, GJFEATURE_MPACK_WAM "GnBg", "generate random landscape", "landscape_file [seed]" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "em_genlandscape", 1, 2 )

  FIND_ENVI

  if ( parameters.size() == 2 )
    envi->generateLandscape( parameters[ 0 ] ->toString(), parameters[ 1 ] ->toInt() );
  else if ( parameters.size() == 1 )
    envi->generateLandscape( parameters[ 0 ] ->toString() );
  return makeNull();
}




// registration ---------------------------------------------------------------
wGameject *createEnvironmentManager( wGame &s )
{
  return new wEnvironmentManager( s );
}




void registerEnvironmentManager( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_ENVIRONMENTMGR, "", createEnvironmentManager );

  register_em_wind( g );
  register_em_gravity( g );
  register_em_loadlandscape( g );
  register_em_genlandscape( g );
}
