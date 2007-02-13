// ----------------------------------------------------------------------------
//  Description      : WAM shape/mono bitmap type
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------
// bitwise alignment is like this: [CHANGED as of 2000-08-11]
// 31|30|....|0|31



#include <fstream>
#include <cmath>
#include <cstring>
#include <ixlib_exgen.hh>
#include <ixlib_numconv.hh>
#include <ixlib_numeric.hh>
#include <ixlib_drawing_functions.hh>
#include <ixlib_geometry_impl.hh>
#include <ixlib_polygon_impl.hh>
#include <sdlucid_png.hh>
#include "utility/debug.hh"
#include "utility/exgame.hh"
#include "utility/shapebitmap.hh"
#include "utility/png_info.hh"




using namespace png;




// template explicit instantiations -------------------------------------------
template rectangle<int>
;
template polygon_segment<int>
;
template polygon<int>
;




// Constants ------------------------------------------------------------------
#define BYTE_ALIGNMENT   4
#define BIT_ALIGNMENT   (BYTE_ALIGNMENT * 8)
#define FILLED_MASK   -1ul
#define LEFT_BIT   (1ul << (BIT_ALIGNMENT-1))




// wShapeBitmap ---------------------------------------------------------------
wShapeBitmap::wShapeBitmap()
    : Width( 0 ), Height( 0 ), ReferencePointX( 0 ), ReferencePointY( 0 ), DrawMode( OR )
{}




wShapeBitmap::wShapeBitmap( wShapeBitmap const &src )
{
  copyFrom( src );
}




wShapeBitmap::wShapeBitmap( int width, int height, TCoordinate refx, TCoordinate refy )
    : ReferencePointX( refx ), ReferencePointY( refy ), DrawMode( OR )
{
  create( width, height );
}




void wShapeBitmap::copyFrom( wShapeBitmap const & src )
{
  create( src.Width, src.Height );
  memcpy( Data.get(), src.Data.get(), PerLine * src.Height * sizeof( TData ) );

  ReferencePointX = src.ReferencePointX;
  ReferencePointY = src.ReferencePointY;
  DrawMode = src.DrawMode;
  Detector = src.Detector;
}




bool wShapeBitmap::doesCollide( rectangle<TCoordinate> const &rect, TCoordVector *where ) const
{
  TCoordinate x1 = rect.A[ 0 ];
  TCoordinate x2 = rect.B[ 0 ];
  TCoordinate y1 = rect.A[ 1 ];
  TCoordinate y2 = rect.B[ 1 ];

  if ( x1 < 0 )
    x1 = 0;
  if ( x2 > signed( Width ) )
    x2 = Width;
  if ( y1 < 0 )
    y1 = 0;
  if ( y2 > signed( Height ) )
    y2 = Height;

  if ( ( y1 > y2 ) || ( x1 > x2 ) )
    return false;

  // calculate masks for left and right end
  TData left_mask = leftBitsUnset( x1 % BIT_ALIGNMENT );
  TData right_mask = leftBitsSet( x2 % BIT_ALIGNMENT );

  TCoordinate x_start = x1 / BIT_ALIGNMENT;
  TCoordinate x_end = x2 / BIT_ALIGNMENT;

  TData coll_mask;

#define DETECT(X) \
    if (coll_mask != 0) { \
      if (where) \
        where->set((X * BIT_ALIGNMENT)+firstBit(coll_mask),y);\
      return true; \
      }

  for ( TCoordinate y = y1; y <= y2; y++ )
  {
    if ( x_start == x_end )
    {
      coll_mask = Data[ PerLine * y + x_start ] & left_mask & right_mask;
      DETECT( x_start )
    }
    else
    {
      coll_mask = Data[ PerLine * y + x_start ] & left_mask;
      DETECT( x_start )
      coll_mask = Data[ PerLine * y + x_end ] & right_mask;
      DETECT( x_end )
    }
    for ( TCoordinate x = x_start + 1; x < x_end; x++ )
    {
      coll_mask = Data[ PerLine * y + x ];
      DETECT( x )
    }
  }

#undef DETECT

  return false;
}




bool wShapeBitmap::doesCollide( TCoordVector const &pos, wShapeBitmap const &bmp, TCoordVector *where ) const
{
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // when updating this code, please also update drawBitmap if applicable
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // calculate rectangle that has to be checked in this bitmap
  TCoordinate x1 = pos[ 0 ] - bmp.ReferencePointX;
  TCoordinate x2 = x1 + bmp.Width;
  TCoordinate y1 = pos[ 1 ] - bmp.ReferencePointY;
  TCoordinate y2 = y1 + bmp.Height;

  TCoordinate unclipped_x1 = x1;
  TCoordinate unclipped_y1 = y1;
  bool negative = unclipped_x1 < 0;

  // clip to this bitmap's dimensions
  if ( x1 < 0 )
    x1 = 0;
  if ( x2 > signed( Width ) )
    x2 = Width;
  if ( y1 < 0 )
    y1 = 0;
  if ( y2 > signed( Height ) )
    y2 = Height;

  // nothing left from clipping?
  if ( y1 > y2 || x1 > x2 )
    return false;

  // calculate line offsets inside this bitmap
  TCoordinate x_start = x1 / BIT_ALIGNMENT;
  TCoordinate x_end = ( x2 + BIT_ALIGNMENT - 1 ) / BIT_ALIGNMENT;

  // calculate line offsets inside other bitmap
  TCoordinate bmp_y = y1 - unclipped_y1;
  TCoordinate bmp_x_start = 0;
  if ( negative )
    bmp_x_start = -unclipped_x1 / BIT_ALIGNMENT;

  // calculate right shift
  TCoordinate shift_right;
  if ( unclipped_x1 < 0 )
  {
    shift_right = BIT_ALIGNMENT - ( -unclipped_x1 % BIT_ALIGNMENT );

    if ( shift_right == BIT_ALIGNMENT )
      shift_right = 0;
  }
  else
    shift_right = unclipped_x1 % BIT_ALIGNMENT;

  // accordingly calculate left shift
  TCoordinate shift_left = BIT_ALIGNMENT - shift_right;

  // special case for crappy intel processors: those won't shift by 32 bits.
  bool do_left_shift = shift_left != BIT_ALIGNMENT;

  // whether end-word needs shift-right part
  bool do_right_shift_at_end = x2 % BIT_ALIGNMENT > shift_right || x2 % BIT_ALIGNMENT == 0;

#define DETECT(OFFSET) \
  if (collision_mask) { \
    where->set((dest-OFFSET-line_start)*32+firstBit(collision_mask),y); \
    return true; \
    }

  // boogie
  for ( TCoordinate y = y1;y < y2;y++ )
  {
    TData *line_start = Data + PerLine * y,
                        *dest = line_start + x_start,
                                *end = line_start + x_end;
    TData *src = bmp.Data + bmp.PerLine * bmp_y + bmp_x_start;
    TData collision_mask;

    // +-------+-------+-------+
    // |   ****|*******|***    |
    // +---1---+---2---+---3---+

    if ( do_left_shift )
    {
      // 1
      if ( !negative )
      {
        collision_mask = *dest++ & *src >> shift_right;
        DETECT( 1 )
      }

      // 2
      while ( dest < end - 1 )
      {
        collision_mask = *dest & *src++ << shift_left;
        DETECT( 0 )
        collision_mask = *dest++ & *src >> shift_right;
        DETECT( 1 )
      }

      // 3
      if ( dest < end )
      {
        collision_mask = *dest & *src++ << shift_left;
        DETECT( 0 )
        if ( do_right_shift_at_end )
        {
          collision_mask = *dest++ & *src >> shift_right;
          DETECT( 1 )
        }
      }
    }
    else
    {
      // 1,2,3
      while ( dest < end )
      {
        collision_mask = *dest++ & *src++;
        DETECT( 1 )
      }
    }

    bmp_y++;
  }

#undef DETECT

  return false;
}




void wShapeBitmap::getRimPoint( TCoordVector const &pos, TPreciseVector const &speed, TCoordVector &result ) const
{
  TPreciseVector precise_result = pos;
  TPreciseVector step( speed * ( -1 ) );
  if ( ( step[ 0 ] == 0 ) && ( step[ 1 ] == 0 ) )
    EXGAME_THROWINFO( ECGAME_GENERAL, "Could not determine rim point." )
    if ( NUM_ABS( step[ 0 ] ) > NUM_ABS( step[ 1 ] ) )
      step /= NUM_ABS( step[ 0 ] );
    else
      step /= NUM_ABS( step[ 1 ] );
  TPreciseVector step20( round( step * 20 ) );
  TPreciseVector step5( round( step * 5 ) );
  TPreciseVector step1( round( step ) );
  while ( getPixel( TCoordinate( precise_result[ 0 ] + step20[ 0 ] ), TCoordinate( precise_result[ 1 ] + step20[ 1 ] ) ) )
    precise_result += step20;
  while ( getPixel( TCoordinate( precise_result[ 0 ] + step5[ 0 ] ), TCoordinate( precise_result[ 1 ] + step5[ 1 ] ) ) )
    precise_result += step5;
  while ( getPixel( TCoordinate( precise_result[ 0 ] + step1[ 0 ] ), TCoordinate( precise_result[ 1 ] + step1[ 1 ] ) ) )
    precise_result += step1;
  result = round( precise_result );
}




void wShapeBitmap::getNormal( TCoordVector const &rimpoint, TPreciseVector const &speed,
                              TPreciseVector &result ) const
{
  TPreciseVector start = speed * ( -1 );

  if ( NUM_ABS( start[ 1 ] ) >= NUM_ABS( start[ 0 ] ) )
    start /= NUM_ABS( start[ 1 ] );
  else
    start /= NUM_ABS( start[ 0 ] );

  result.set( 0, 0 );
  TSize foundcount = 0;

  TCoordVector rimp = rimpoint + TCoordVector( ReferencePointX, ReferencePointY );
  TCoordinate rimpx = rimp[ 0 ], rimpy = rimp[ 1 ];

  for ( int radius = 1;radius <= 3; radius++ )
  {
    TCoordVector cwcheckpos = round( start * radius );
    TCoordVector ccwcheckpos = round( start * radius );

    // we do not want to look for white pixels that aren't even there
    if ( getPixel( rimpx + cwcheckpos[ 0 ], rimpy + cwcheckpos[ 1 ] ) )
      continue;

    // the maximum number of steps to take before we reach the point
    // we left from.
    TSize maxsteps = 2 * 4 * radius - 1;
    TSize cwsteps = 0;

    // first go clockwise
    do
    {
      // do clockwise transform
      if ( cwcheckpos[ 0 ] == -radius )
      {
        // we're scanning on left vertical edge
        if ( cwcheckpos[ 1 ] == -radius )
          cwcheckpos[ 0 ] ++;
        else
          cwcheckpos[ 1 ] --;
      }
      else if ( cwcheckpos[ 0 ] == radius )
      {
        // we're scanning on right vertical edge
        if ( cwcheckpos[ 1 ] == radius )
          cwcheckpos[ 0 ] --;
        else
          cwcheckpos[ 1 ] ++;
      }
      else if ( cwcheckpos[ 1 ] == -radius )
      {
        // we're scanning on upper horizontal edge
        if ( cwcheckpos[ 0 ] == radius )
          cwcheckpos[ 1 ] ++;
        else
          cwcheckpos[ 0 ] ++;
      }
      else if ( cwcheckpos[ 1 ] == radius )
      {
        // we're scanning on lower horizontal edge
        if ( cwcheckpos[ 0 ] == -radius )
          cwcheckpos[ 1 ] --;
        else
          cwcheckpos[ 0 ] --;
      }

      cwsteps++;
    }
    while ( cwsteps < maxsteps && !getPixel( rimpx + cwcheckpos[ 0 ], rimpy + cwcheckpos[ 1 ] ) );

    // have we found anything?
    if ( !getPixel( rimpx + cwcheckpos[ 0 ], rimpy + cwcheckpos[ 1 ] ) )
      continue;

    // we don't need maxsteps for the ccw go because cw found something, so
    // ccw will, too.
    do
    {
      // do counterclockwise transform
      if ( ccwcheckpos[ 0 ] == -radius )
      {
        // we're scanning on left vertical edge
        if ( ccwcheckpos[ 1 ] == radius )
          ccwcheckpos[ 0 ] ++;
        else
          ccwcheckpos[ 1 ] ++;
      }
      else if ( ccwcheckpos[ 0 ] == radius )
      {
        // we're scanning on right vertical edge
        if ( ccwcheckpos[ 1 ] == -radius )
          ccwcheckpos[ 0 ] --;
        else
          ccwcheckpos[ 1 ] --;
      }
      else if ( ccwcheckpos[ 1 ] == -radius )
      {
        // we're scanning on upper horizontal edge
        if ( ccwcheckpos[ 0 ] == -radius )
          ccwcheckpos[ 1 ] ++;
        else
          ccwcheckpos[ 0 ] --;
      }
      else if ( ccwcheckpos[ 1 ] == radius )
      {
        // we're scanning on lower horizontal edge
        if ( ccwcheckpos[ 0 ] == radius )
          ccwcheckpos[ 1 ] --;
        else
          ccwcheckpos[ 0 ] ++;
      }
    }
    while ( !getPixel( rimpx + ccwcheckpos[ 0 ], rimpy + ccwcheckpos[ 1 ] ) );

    TPreciseVector tangent;
    if ( cwcheckpos == ccwcheckpos )
    {
      if ( cwsteps < 1 + maxsteps - cwsteps )
        tangent = cwcheckpos;
      else if ( cwsteps == 1 + maxsteps - cwsteps )
      {
        foundcount++;
        result += speed / ( -sqrt( speed * speed ) );
        continue;
      }
      else
        tangent = cwcheckpos * ( -1 );
    }
    else
      tangent = cwcheckpos - ccwcheckpos;

    foundcount++;
    // cheap normal creation: hope this will be correct for the above
    // cw/ccw order, normal has same length as connect.
    result += TPreciseVector( tangent[ 1 ], -tangent[ 0 ] ) / sqrt( tangent * tangent );
  }

  // fallback if nothing found
  if ( foundcount == 0 )
    result = speed / ( -sqrt( speed * speed ) );
  else
    result /= sqrt( result * result );
}




void wShapeBitmap::wipe( bool color )
{
  TSize size = PerLine * Height * sizeof( TData );
  if ( size )
  {
    if ( color )
      memset( Data.get(), FILLED_MASK, size );
    else
      memset( Data.get(), 0, size );
  }
}




void wShapeBitmap::create( int width, int height )
{
  setup( width, height );
  Data.allocate( PerLine * Height );
}




void wShapeBitmap::clear()
{
  Width = 0;
  Height = 0;
  PerLine = 0;
  Data.deallocate();
}




void wShapeBitmap::load( istream &strm )
{
  png_stream_reader reader( strm );
  reader.readInfo();
  if ( reader.getBitDepth() != 1 )
    EXGAME_THROW( ECGAME_INVALIDBITDEPTH )
    reader.updateInfo();
  Width = reader.getWidth();
  Height = reader.getHeight();
  PerLine = reader.getAlignedRowBytes( BYTE_ALIGNMENT ) / 4;

  Data.allocate( PerLine * Height );
  for ( unsigned int i = 0; i < PerLine*Height; i++ )
    Data[ i ] = 0;
  reader.readImage( ( TByte * ) Data.get(), BYTE_ALIGNMENT );
  reader.readEnd();

  reverseByteAlignment( Data.get(), PerLine * Height );

  png_meta_data meta;
  reader.getMetaData( meta );

  ReferencePointX = 0;
  ReferencePointY = 0;
  try
  {
    ReferencePointX = evalSigned( meta[ WAM_PNG_REFPOINTX ] );
    ReferencePointY = evalSigned( meta[ WAM_PNG_REFPOINTY ] );
  }
  catch ( ... )
  { }
}




void wShapeBitmap::save( ostream &strm ) const
{
  png_stream_writer writer( strm );
  writer.setInfo( Width, Height, 1, PNG_COLOR_TYPE_GRAY );

  png_meta_data meta;
  meta[ WAM_PNG_REFPOINTX ] = signed2dec( ReferencePointX );
  meta[ WAM_PNG_REFPOINTY ] = signed2dec( ReferencePointY );
  writer.setMetaData( meta );

  auto_array<TData> temp( Height * PerLine );
  memcpy( temp.get(), Data.get(), BYTE_ALIGNMENT * Height * PerLine );
  reverseByteAlignment( temp.get(), PerLine * Height );

  writer.writeInfo();
  writer.writeImage( ( TByte * ) temp.get(), BYTE_ALIGNMENT );
  writer.writeEnd();
  writer.flush();
}




bool wShapeBitmap::getPixelNoClip( TCoordinate x, TCoordinate y ) const
{
  return ( Data[ PerLine * y + ( x / BIT_ALIGNMENT ) ] & ( LEFT_BIT >> ( x % BIT_ALIGNMENT ) ) );
}




bool wShapeBitmap::getPixel( TCoordinate x, TCoordinate y ) const
{
  if ( ( x < 0 ) || ( y < 0 ) || ( x >= ( signed ) Width ) || ( y >= ( signed ) Height ) )
    return false;
  TData mask = LEFT_BIT >> ( x % BIT_ALIGNMENT );
  return ( Data[ PerLine * y + ( x / BIT_ALIGNMENT ) ] & mask ) != 0;
}




bool wShapeBitmap::getNextPixelOfColor( TCoordinate &x, TCoordinate &y, TCoordinate dx, TCoordinate dy, bool color )
{
  if ( x < 0 || x >= ( signed ) Width )
    return false;
  if ( y < 0 || y >= ( signed ) Height )
    return false;

  if ( dx != 0 )
  {
    TCoordinate copy_x = x;
    TCoordinate copy_y = y;
    // slow fallback
    while ( getPixelNoClip( x, y ) != color )
    {
      copy_x += dx;
      copy_y += dy;
      if ( copy_x < 0 || copy_x >= ( signed ) Width )
        return false;
      if ( copy_y < 0 || copy_y >= ( signed ) Height )
        return false;
    }
    x = copy_x;
    y = copy_y;
    return true;
  }
  else
  {
    TIndex index = PerLine * y + ( x / BIT_ALIGNMENT );
    TDelta offset = dy * PerLine;
    TData mask = LEFT_BIT >> ( x % BIT_ALIGNMENT );
    TSize max_count;
    if ( dy < 0 )
      max_count = y / -dy;
    if ( dy > 0 )
      max_count = ( Height - y ) / dy;
    TData cmp;
    if ( color )
      cmp = mask;
    else
      cmp = 0;

    while ( max_count-- )
    {
      if ( ( Data[ index ] & mask ) == cmp )
      {
        y = index / PerLine;
        return true;
      }
      index += offset;
    }
    return false;
  }
}




void wShapeBitmap::setPixel( TCoordinate x, TCoordinate y )
{
  if ( x < 0 || y < 0 || x >= ( TCoordinate ) Width || y >= ( TCoordinate ) Height )
    return ;
  TData offset = ( x / BIT_ALIGNMENT ) + y * PerLine;
  TData mask = LEFT_BIT >> ( x % BIT_ALIGNMENT );
  switch ( DrawMode )
  {
  case OR:
    Data[ offset ] |= mask;
    break;
  case AND:
    // doesn't make sense here
    break;
  case AND_NOT:
    Data[ offset ] &= ~mask;
    break;
  case XOR:
    Data[ offset ] ^= mask;
    break;
  case DETECT:
    Detector |= Data[ offset ] & mask;
    break;
  }
}




void wShapeBitmap::drawHLine( TCoordinate x1, TCoordinate y, TCoordinate x2 )
{
  if ( y < 0 || y >= TCoordinate( Height ) )
    return ;
  if ( x1 > x2 )
    swap( x1, x2 );
  if ( x2 < 0 )
    return ;
  if ( x1 >= TCoordinate( Width ) )
    return ;
  if ( x1 < 0 )
    x1 = 0;
  if ( x2 > TCoordinate( Width ) )
    x2 = Width;

  TData left_offset = ( x1 / BIT_ALIGNMENT ) + y * PerLine;
  TData right_offset = ( x2 / BIT_ALIGNMENT ) + y * PerLine;
  TData left_mask = leftBitsUnset( x1 % BIT_ALIGNMENT );
  TData right_mask = leftBitsSet( x2 % BIT_ALIGNMENT );

  if ( left_offset == right_offset )
  {
    left_mask &= right_mask;
    right_mask = left_mask;
  }

  switch ( DrawMode )
  {
  case OR:
    Data[ left_offset ] |= left_mask;
    Data[ right_offset ] |= right_mask;
    for ( TIndex offset = left_offset + 1;offset < right_offset;offset++ )
      Data[ offset ] = FILLED_MASK;
    break;
  case AND:
    // doesn't make sense here
    break;
  case AND_NOT:
    Data[ left_offset ] &= ~left_mask;
    Data[ right_offset ] &= ~right_mask;
    for ( TIndex offset = left_offset + 1;offset < right_offset;offset++ )
      Data[ offset ] = 0;
    break;
  case XOR:
    Data[ left_offset ] ^= left_mask;
    if ( left_offset != right_offset )
      Data[ right_offset ] ^= right_mask;
    for ( TIndex offset = left_offset + 1;offset < right_offset;offset++ )
      Data[ offset ] = ~Data[ offset ];
    break;
  case DETECT:
    Detector |= Data[ left_offset ] & left_mask;
    Detector |= Data[ right_offset ] & right_mask;
    for ( TIndex offset = left_offset + 1;offset < right_offset;offset++ )
      Detector |= Data[ offset ];
    break;
  }
}




void wShapeBitmap::drawVLine( TCoordinate x, TCoordinate y1, TCoordinate y2 )
{
  if ( ( x < 0 ) || x >= TCoordinate( Width ) )
    return ;
  if ( y1 > y2 )
    swap( y1, y2 );
  if ( y2 < 0 )
    return ;
  if ( y1 >= TCoordinate( Height ) )
    return ;
  if ( y1 < 0 )
    y1 = 0;
  if ( y2 > TCoordinate( Height ) )
    y2 = Height;

  TIndex offset = ( x / BIT_ALIGNMENT ) + y1 * PerLine;
  TData mask = LEFT_BIT >> ( x % BIT_ALIGNMENT );

  switch ( DrawMode )
  {
  case OR:
    for ( TCoordinate y = y1;y < y2; y++ )
    {
      Data[ offset ] |= mask;
      offset += PerLine;
    }
    break;
  case AND:
    // doesn't make sense here
    break;
  case AND_NOT:
    for ( TCoordinate y = y1;y < y2; y++ )
    {
      Data[ offset ] &= ~mask;
      offset += PerLine;
    }
    break;
  case XOR:
    for ( TCoordinate y = y1;y < y2; y++ )
    {
      Data[ offset ] ^= mask;
      offset += PerLine;
    }
    break;
  case DETECT:
    for ( TCoordinate y = y1;y < y2; y++ )
    {
      Detector |= Data[ offset ] & mask;
      offset += PerLine;
    }
    break;
  }
}




void wShapeBitmap::drawLine( TCoordinate x1, TCoordinate y1, TCoordinate x2, TCoordinate y2 )
{
  drawing_functions::drawLine( *this, x1, y1, x2, y2 );
}




void wShapeBitmap::drawBox( TCoordinate x1, TCoordinate y1, TCoordinate x2, TCoordinate y2 )
{
  drawing_functions::drawBox( *this, x1, y1, x2, y2 );
}




void wShapeBitmap::fillBox( TCoordinate x1, TCoordinate y1, TCoordinate x2, TCoordinate y2 )
{
  drawing_functions::fillBox( *this, x1, y1, x2, y2 );
}




void wShapeBitmap::drawCircle( TCoordinate x, TCoordinate y, TCoordinate r )
{
  drawing_functions::drawCircle( *this, x, y, r );
}




void wShapeBitmap::fillCircle( TCoordinate x, TCoordinate y, TCoordinate r )
{
  drawing_functions::fillCircle( *this, x, y, r );
}




void wShapeBitmap::drawEllipse( TCoordinate x, TCoordinate y, TCoordinate r_x, TCoordinate r_y )
{
  drawing_functions::drawEllipse( *this, x, y, r_x, r_y );
}




void wShapeBitmap::fillEllipse( TCoordinate x, TCoordinate y, TCoordinate r_x, TCoordinate r_y )
{
  drawing_functions::fillEllipse( *this, x, y, r_x, r_y );
}




void wShapeBitmap::drawPolygon( polygon<int> const &poly )
{
  drawing_functions::drawPolygon( *this, poly );
}




void wShapeBitmap::fillPolygon( polygon<int> const &poly )
{
  drawing_functions::fillPolygon( *this, poly );
}




void wShapeBitmap::drawBitmap( TCoordinate at_x, TCoordinate at_y, wShapeBitmap const &bmp )
{
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // when updating this code, please also update doesCollide if applicable
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // calculate rectangle that has to be checked in this bitmap
  TCoordinate x1 = at_x - bmp.ReferencePointX;
  TCoordinate x2 = x1 + bmp.Width;
  TCoordinate y1 = at_y - bmp.ReferencePointY;
  TCoordinate y2 = y1 + bmp.Height;

  TCoordinate unclipped_x1 = x1;
  TCoordinate unclipped_y1 = y1;
  bool negative = unclipped_x1 < 0;

  // clip to this bitmap's dimensions
  if ( x1 < 0 )
    x1 = 0;
  if ( x2 > signed( Width ) )
    x2 = Width;
  if ( y1 < 0 )
    y1 = 0;
  if ( y2 > signed( Height ) )
    y2 = Height;

  // nothing left from clipping?
  if ( y1 > y2 || x1 > x2 )
    return ;

  // calculate line offsets inside this bitmap
  TCoordinate x_start = x1 / BIT_ALIGNMENT;
  TCoordinate x_end = ( x2 + BIT_ALIGNMENT - 1 ) / BIT_ALIGNMENT;

  // calculate line offsets inside other bitmap
  TCoordinate bmp_y = y1 - unclipped_y1;
  TCoordinate bmp_x_start = 0;
  if ( negative )
    bmp_x_start = -unclipped_x1 / BIT_ALIGNMENT;

  // calculate right shift
  TCoordinate shift_right;
  if ( unclipped_x1 < 0 )
  {
    shift_right = BIT_ALIGNMENT - ( -unclipped_x1 % BIT_ALIGNMENT );

    if ( shift_right == BIT_ALIGNMENT )
      shift_right = 0;
  }
  else
    shift_right = unclipped_x1 % BIT_ALIGNMENT;

  // accordingly calculate left shift
  TCoordinate shift_left = BIT_ALIGNMENT - shift_right;

  // special case for crappy intel processors: those won't shift by 32 bits.
  bool do_left_shift = shift_left != BIT_ALIGNMENT;

  // whether end-word needs shift-right part
  bool do_right_shift_at_end = x2 % BIT_ALIGNMENT > shift_right || x2 % BIT_ALIGNMENT == 0;

  // boogie
#define BITMAP_LOOP(OPERATION) \
  for (TCoordinate y = y1;y<y2;y++) { \
    TData *line_start = Data+PerLine*y, \
       *dest = line_start+x_start, \
                *end = line_start+x_end; \
    TData *src = bmp.Data+bmp.PerLine*bmp_y+bmp_x_start; \
    \
    /* +-------+-------+-------+ \
       |   ****|*******|***    | \
       +---1---+---2---+---3---+ \
     */ \
    \
    if (do_left_shift) { \
      /* 1 */ \
      if (!negative) { \
        *dest++ OPERATION (*src >> shift_right); \
        } \
      /* 2 */ \
      while (dest < end-1) { \
        *dest OPERATION *src++ << shift_left; \
        *dest++ OPERATION (*src >> shift_right); \
        } \
      /* 3 */ \
      if (dest < end) { \
        *dest OPERATION *src++ << shift_left; \
 if (do_right_shift_at_end)  \
   *dest++ OPERATION (*src >> shift_right); \
        } \
      } \
    else { \
      /* 1,2,3 */ \
      while (dest < end)  \
        *dest++ OPERATION (*src++); \
      } \
     \
    bmp_y++; \
    }

  switch ( DrawMode )
  {
  case OR:
    BITMAP_LOOP( |= )
    break;
  case AND:
    BITMAP_LOOP( &= )
    break;
  case AND_NOT:
    BITMAP_LOOP( &= ~ )
    break;
  case XOR:
    BITMAP_LOOP( ^= )
    break;
  case DETECT:
    EXGEN_NYI
  }
}




void wShapeBitmap::setup( int width, int height )
{
  Width = width;
  Height = height;
  PerLine = ( width + ( BIT_ALIGNMENT - 1 ) ) / BIT_ALIGNMENT;
}




// Tool functions -------------------------------------------------------------
int wShapeBitmap::firstBit( TData mask )
{
  for ( int i = 0;i < BIT_ALIGNMENT;i++ )
    if ( ( ( mask << i ) & LEFT_BIT ) != 0 )
      return i;
  EXGAME_THROW( ECGAME_NOBITINMASK )
}




wShapeBitmap::TData wShapeBitmap::leftBitsUnset( TIndex bits )
{
  return FILLED_MASK >> bits;
}




wShapeBitmap::TData wShapeBitmap::leftBitsSet( TIndex bits )
{
  return ~leftBitsUnset( bits );
}




void wShapeBitmap::reverseByteAlignment( TData *data, TSize size )
{
  while ( size-- )
  {
    TData result = 0;
    for ( TIndex byte = 0;byte < BYTE_ALIGNMENT / 2;byte++ )
    {
      TUnsigned8 left_byte = ( TUnsigned8 ) ( *data >> ( ( BYTE_ALIGNMENT - 1 - byte ) * 8 ) );
      TUnsigned8 right_byte = ( TUnsigned8 ) ( *data >> ( byte * 8 ) );
      result |= right_byte << ( ( BYTE_ALIGNMENT - 1 - byte ) * 8 );
      result |= left_byte << ( byte * 8 );
    }
    *data++ = result;
  }
}




// serialization --------------------------------------------------------------
// shapebitmaps are serialized in an rle'd manner
namespace
{
wShapeBitmap::TData const RLEEscape = 0xbadfaecal;
}




wStreamWriter &operator<<( wStreamWriter &writer, wShapeBitmap const &bits )
{
  writer
  << ( TUnsigned32 ) bits.width()
  << ( TUnsigned32 ) bits.height()
  << ( TSigned32 ) bits.referencePointX()
  << ( TSigned32 ) bits.referencePointY();

  TSize size = bits.PerLine * bits.height();
  if ( size == 0 )
    return writer;

  wShapeBitmap::TData *first = bits.Data.get(), *last = first + size;
  wShapeBitmap::TData run_value = *first++;
  TUnsigned32 run_size = 1;
  while ( first < last )
  {
    if ( *first == run_value )
    {
      run_size++;
      first++;
    }
    else
    {
      if ( run_size >= 1 && run_value != RLEEscape )
        writer << RLEEscape << run_size << run_value;
      else
        while ( run_size-- )
          writer << run_value;
      run_value = *first++;
      run_size = 1;
    }
  }
  writer << RLEEscape << run_size << run_value;
  return writer;
}




wStreamReader &operator>>( wStreamReader &reader, wShapeBitmap &bits )
{
  TUnsigned32 width, height, ref_x, ref_y;
  reader
  >> width >> height >> ref_x >> ref_y;

  bits.create( width, height );
  bits.setReferencePoint( ref_x, ref_y );

  TSize size = bits.PerLine * bits.height();
  if ( size == 0 )
    return reader;

  wShapeBitmap::TData *first = bits.Data.get(), *last = first + size;
  wShapeBitmap::TData read;

  while ( first < last )
  {
    reader >> read;
    if ( read == RLEEscape )
    {
      TUnsigned32 size;
      reader >> size >> read;
      while ( size-- )
        * first++ = read;
    }
    else
      *first++ = read;
  }
  return reader;
}
