// ----------------------------------------------------------------------------
//  Description      : Transmission stream classes
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif
#include <zlib.h>
#include "utility/stream.hh"
#include "utility/exgame.hh"




// wStreamStore ---------------------------------------------------------------
void wStreamStore::setCapacity( TSize capacity )
{
  auto_array<TByte> newstore( capacity );
  memcpy( newstore.get(), Store.get(), Store.capacity() );
  Store = newstore;
}




void wStreamStore::enlargeCapacity( TSize min_new_capacity )
{
  if ( min_new_capacity < Store.capacity() )
    return ;

  TSize new_capacity = Store.capacity();
  if ( new_capacity < 16 )
    new_capacity = 16;
  while ( new_capacity < min_new_capacity )
    new_capacity *= 2;

  setCapacity( new_capacity );
}




// wStreamWriter --------------------------------------------------------------
void wStreamWriter::ensureSpace( TSize space )
{
  TSize neededsize = CurrentPosition + space;
  while ( neededsize > Store.capacity() )
    Store.enlargeCapacity( neededsize );
}




// wStreamWriter insertion ----------------------------------------------------
wStreamWriter &operator<<( wStreamWriter &writer, TUnsigned8 data )
{
  writer.ensureSpace( sizeof( data ) );
  *( ( TUnsigned8 * ) writer.getPosition() ) = data;
  writer.incrementPosition( sizeof( data ) );
  return writer;
}




wStreamWriter &operator<<( wStreamWriter &writer, TUnsigned16 data )
{
  writer.ensureSpace( sizeof( data ) );
  *( ( TUnsigned16 * ) writer.getPosition() ) = htons( data );
  writer.incrementPosition( sizeof( data ) );
  return writer;
}




wStreamWriter &operator<<( wStreamWriter &writer, TUnsigned32 data )
{
  writer.ensureSpace( sizeof( data ) );
  *( ( TUnsigned32 * ) writer.getPosition() ) = htonl( data );
  writer.incrementPosition( sizeof( data ) );
  return writer;
}




wStreamWriter &operator<<( wStreamWriter &writer, TSigned32 data )
{
  writer.ensureSpace( sizeof( data ) );
  *( ( TSigned32 * ) writer.getPosition() ) = htonl( ( TUnsigned32 ) data );
  writer.incrementPosition( sizeof( data ) );
  return writer;
}




wStreamWriter &operator<<( wStreamWriter &writer, float data )
{
  writer.ensureSpace( sizeof( data ) );
  *( ( float * ) writer.getPosition() ) = data;
  writer.incrementPosition( sizeof( data ) );
  return writer;
}




wStreamWriter &operator<<( wStreamWriter &writer, double data )
{
  writer.ensureSpace( sizeof( data ) );
  *( ( double * ) writer.getPosition() ) = data;
  writer.incrementPosition( sizeof( data ) );
  return writer;
}




wStreamWriter &operator<<( wStreamWriter &writer, string const &data )
{
  writer << ( TUnsigned32 ) data.size();
  writer.ensureSpace( data.size() );
  memcpy( writer.getPosition(), data.data(), data.size() );
  writer.incrementPosition( data.size() );
  return writer;
}




wStreamWriter &operator<<( wStreamWriter &writer, wStreamStore const &data )
{
  writer << ( TUnsigned32 ) data.usedSize();
  writer.ensureSpace( data.usedSize() );
  memcpy( writer.getPosition(), data.get(), data.usedSize() );
  writer.incrementPosition( data.usedSize() );
  return writer;
}




void insert( wStreamWriter &writer, wStreamStore const &data )
{
  writer.ensureSpace( data.usedSize() );
  memcpy( writer.getPosition(), data.get(), data.usedSize() );
  writer.incrementPosition( data.usedSize() );
}




// wStreamReader extraction ---------------------------------------------------
wStreamReader &operator>>( wStreamReader &reader, TUnsigned8 &data )
{
  data = *( ( TUnsigned8 * ) reader.getPosition() );
  reader.incrementPosition( sizeof( TUnsigned8 ) );
  return reader;
}




wStreamReader &operator>>( wStreamReader &reader, TUnsigned16 &data )
{
  data = ntohs( *( ( TUnsigned16 * ) reader.getPosition() ) );
  reader.incrementPosition( sizeof( TUnsigned16 ) );
  return reader;
}




wStreamReader &operator>>( wStreamReader &reader, TUnsigned32 &data )
{
  data = ntohl( *( ( TUnsigned32 * ) reader.getPosition() ) );
  reader.incrementPosition( sizeof( TUnsigned32 ) );
  return reader;
}




wStreamReader &operator>>( wStreamReader &reader, TSigned32 &data )
{
  data = ( TSigned32 ) ntohl( *( ( TUnsigned32 * ) reader.getPosition() ) );
  reader.incrementPosition( sizeof( TUnsigned32 ) );
  return reader;
}




wStreamReader &operator>>( wStreamReader &reader, double &data )
{
  data = *( ( double * ) reader.getPosition() );
  reader.incrementPosition( sizeof( double ) );
  return reader;
}




wStreamReader &operator>>( wStreamReader &reader, float &data )
{
  data = *( ( float * ) reader.getPosition() );
  reader.incrementPosition( sizeof( float ) );
  return reader;
}




wStreamReader &operator>>( wStreamReader &reader, string &data )
{
  TUnsigned32 length;
  reader >> length;
  char *loc = ( char * ) reader.getPosition();
  data.assign( loc, loc + length );
  reader.incrementPosition( length );
  return reader;
}




wStreamReader &operator>>( wStreamReader &reader, wStreamStore &data )
{
  TUnsigned32 size;
  reader >> size;
  data.enlargeCapacity( size );
  memcpy( data.get(), reader.getPosition(), size );
  data.setUsedSize( size );
  reader.incrementPosition( size );
  return reader;
}




// compression ----------------------------------------------------------------
namespace
{
string stringifyZLibError( int error )
{
  switch ( error )
  {
  case Z_OK:
    return "no error";
  case Z_MEM_ERROR:
    return "memory error";
  case Z_BUF_ERROR:
    return "buffer error";
  case Z_DATA_ERROR:
    return "data error";
  default:
    return "unknown error";
  }
}
}




void compress( wStreamWriter &dest, wStreamReader &src )
{
  unsigned long size = src.getRemainingSize() + src.getRemainingSize() / 1000 + 20;
  dest.ensureSpace( size );

  dest << ( TUnsigned32 ) src.getRemainingSize();
  TUnsigned32 *compressed_size_ptr = ( TUnsigned32 * ) dest.getPosition();
  dest << ( TUnsigned32 ) 0;

  int error = compress( ( TByte * ) dest.getPosition(), &size, ( TByte * ) src.getPosition(), src.getRemainingSize() );
  if ( error != Z_OK )
    EXGAME_THROWINFO( ECGAME_GENERAL, ( "zlib compression error: " + stringifyZLibError( error ) ).c_str() )
    src.incrementPosition( src.getRemainingSize() );
  dest.incrementPosition( size );
  *compressed_size_ptr = htonl( size );
}




void decompress( wStreamWriter &dest, wStreamReader &src )
{
  TUnsigned32 uncompressed_size, compressed_size;
  src >> uncompressed_size >> compressed_size;

  dest.ensureSpace( uncompressed_size );
  unsigned long tmp_size = uncompressed_size;

  int error = uncompress( ( TByte * ) dest.getPosition(), &tmp_size, ( TByte * ) src.getPosition(), compressed_size );
  if ( error != Z_OK )
    EXGAME_THROWINFO( ECGAME_GENERAL, ( "zlib decompression error: " + stringifyZLibError( error ) ).c_str() )
    src.incrementPosition( compressed_size );
  dest.incrementPosition( uncompressed_size );
}




void compress( wStreamStore &dest, wStreamStore &src )
{
  wStreamReader reader( src );
  dest.clear();
  wStreamWriter writer( dest );
  compress( writer, reader );
}




void decompress( wStreamStore &dest, wStreamStore &src )
{
  wStreamReader reader( src );
  dest.clear();
  wStreamWriter writer( dest );
  decompress( writer, reader );
}
