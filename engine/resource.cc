// ----------------------------------------------------------------------------
//  Description      : Resource management
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include <memory>
#include <fstream>
#include <ixlib_exgen.hh>
#include <ixlib_numconv.hh>
#include "utility/debug.hh"
#include "utility/exgame.hh"
#include "utility/resourcemanager_impl.hh"
#include "engine/commandmanager.hh"
#include "engine/console.hh"
#include "engine/resource.hh"




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( res_load_image, "LoIm", "load image into resource cache", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "res_load_image", 1, 1 );

  wImageHolder img( Game.getImageManager(), parameters[0]->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( list_res_images, "LiIm", "list objects registered to image manager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_res_images", 0, 0 )

  list<wImageDescriptor> l;
  Game.getImageManager().listObjects( l );

  auto_ptr<js_array> arr( new js_array( l.size() ) );

  TIndex idx = 0;

  FOREACH_CONST( first, l, list<wImageDescriptor> )
  ( *arr ) [ idx++ ] = makeConstant( describeResource( *first ) );
  return arr.release();
}




WAM_DECLARE_COMMAND( list_res_shapebitmaps, "LiSh", "list objects registered to shape manager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_res_shapebitmaps", 0, 0 )

  list<string> l;
  Game.getShapeBitmapManager().listObjects( l );

  auto_ptr<js_array> arr( new js_array( l.size() ) );

  TIndex idx = 0;

  FOREACH_CONST( first, l, list<string> )
  ( *arr ) [ idx++ ] = makeConstant( describeResource( *first ) );
  return arr.release();
}




WAM_DECLARE_COMMAND( list_res_fonts, "LiFo", "list objects registered to font manager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_res_fonts", 0, 0 )

  list<string> l;
  Game.getFontManager().listObjects( l );

  auto_ptr<js_array> arr( new js_array( l.size() ) );

  TIndex idx = 0;

  FOREACH_CONST( first, l, list<string> )
  ( *arr ) [ idx++ ] = makeConstant( describeResource( *first ) );
  return arr.release();
}



WAM_DECLARE_COMMAND( list_res_sounds, "LiSn", "list objects registered with sound resource manager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "list_res_sounds", 0, 0 )

  list<string> l;
  Game.getSoundManager().listObjects( l );

  auto_ptr<js_array> arr( new js_array( l.size() ) );

  TIndex idx = 0;

  FOREACH_CONST( first, l, list<string> )
  ( *arr ) [ idx++ ] = makeConstant( describeResource( *first ) );
  return arr.release();
}




void registerResourceManagerCommands( wGame &g )
{
  register_res_load_image( g );
  register_list_res_images( g );
  register_list_res_shapebitmaps( g );
  register_list_res_fonts( g );
  register_list_res_sounds( g );
}




// Template instantiation -----------------------------------------------------
template class wResourceManager<wImage, wImageDescriptor>;
template class wResourceManager<wShapeBitmap>;
template class wResourceManager<font>;
template class wResourceManager<audio_data>;
template class wStreamResourceManager<wImage>;
template class wStreamResourceManager<wShapeBitmap>;
template class wStreamResourceManager<font>;
template class wStreamResourceManager<audio_data>;




// wImageDescriptor ----------------------------------------------------------
wImageDescriptor::wImageDescriptor( string const &filename, double stretchx, double stretchy, double rotate )
    : FileName( filename ), StretchX( stretchx ), StretchY( stretchy ), Rotate( rotate )
{}




// descriptor support ---------------------------------------------------------
string describeResource( wImageDescriptor const &desc )
{
  return desc.FileName + " at "
         + float2dec( desc.StretchX, 4 ) + " x " + float2dec( desc.StretchY, 4 )
         + " rot " + float2dec( desc.Rotate, 4 ) + " deg";
}




string getResourceFileName( wImageDescriptor const &desc )
{
  return desc.FileName;
}




bool operator<( wImageDescriptor const &desc1, wImageDescriptor const &desc2 )
{
  if ( desc1.FileName == desc2.FileName )
  {
    if ( desc1.StretchX == desc2.StretchX )
      return desc1.StretchY < desc2.StretchY;
    else
      return desc1.StretchX < desc2.StretchX;
  }
  else
    return desc1.FileName < desc2.FileName;
}




// wImageManager ------------------------------------------------------------
wImage *wImageManager::load( wFileManager::wStream &file, wImageDescriptor const &desc )
{
  wamPrintDebugLevel( "res: loading image " + describeResource( desc ), WAM_DEBUG_VERBOSE );

  wImage data;
  data.loadPNG( file );

  wImage transformed;
  if ( desc.StretchX == 1 && desc.StretchY == 1 && desc.Rotate == 0 )
    transformed.copyFrom( data );
  else
  {
    if ( desc.Rotate == 0 )
      transformed.stretchFrom( data, desc.StretchX, desc.StretchY );
    else
    {
      affine_transformation tx;
      tx.identity();
      tx.scale( desc.StretchX, desc.StretchY );
      tx.rotate( desc.Rotate );
      transformed.transformFrom( data, tx );
    }
  }

  auto_ptr<wImage> converted( new wImage );
  converted->convertForAcceleratedBlitFrom( transformed );
  return converted.release();
}




// wShapeBitmapManager -------------------------------------------------------
wShapeBitmap *wShapeBitmapManager::load( wFileManager::wStream &file, string const &name )
{
  wamPrintDebugLevel( "res: loading shape bitmap " + name, WAM_DEBUG_VERBOSE );

  wShapeBitmap *data = new wShapeBitmap;
  data->load( file );

  return data;
}




// wFontManager --------------------------------------------------------------
font *wFontManager::load( wFileManager::wStream &file, string const &name )
{
  wamPrintDebugLevel( "res: loading font " + name, WAM_DEBUG_VERBOSE );

  string data_name = name.substr( 0, name.size() - 4 ) + ".png";
  ref<wFileManager::wStream> datastrm( FileManager.openFile( getFileType(), data_name ) );

  font data;
  data.load( file, *datastrm );

  auto_ptr<font> converted( new font );
  converted->convertForAcceleratedBlitFrom( data );
  return converted.release();
}




// wResourceHolder ------------------------------------------------------------
template <class T, class Mgr, class Descriptor>
wResourceHolder<T, Mgr, Descriptor>::wResourceHolder()
    : Manager( NULL ), Resource( NULL )
{}




template <class T, class Mgr, class Descriptor>
wResourceHolder<T, Mgr, Descriptor>::wResourceHolder( wResourceHolder const &src )
    : Manager( src.Manager ), Resource( src.Resource )
{
  if ( Manager && Resource )
    Manager->addReference( Resource );
}




template <class T, class Mgr, class Descriptor>
wResourceHolder<T, Mgr, Descriptor>::wResourceHolder( Mgr &mgr )
    : Manager( &mgr ), Resource( NULL )
{}




template <class T, class Mgr, class Descriptor>
wResourceHolder<T, Mgr, Descriptor>::wResourceHolder( Mgr &mgr, Descriptor const &desc )
    : Manager( &mgr ), Resource( Manager->get
                                 ( desc ) )
{}




template <class T, class Mgr, class Descriptor>
wResourceHolder<T, Mgr, Descriptor>::~wResourceHolder()
{
  if ( Manager && Resource )
    Manager->release( Resource );
}




template <class T, class Mgr, class Descriptor>
wResourceHolder<T, Mgr, Descriptor> &
wResourceHolder<T, Mgr, Descriptor>::operator=( wResourceHolder const &src )
{
  if ( Manager && Resource )
    Manager->release( Resource );
  Manager = src.Manager;
  Resource = src.Resource;
  if ( Manager && Resource )
    Manager->addReference( Resource );
  return *this;
}




template <class T, class Mgr, class Descriptor>
void wResourceHolder<T, Mgr, Descriptor>::set
  ( Descriptor const &desc )
{
  if ( Manager )
    set
      ( Manager->get
          ( desc ) );
  else
    EXGAME_THROWINFO( ECGAME_RESOURCENOTAVAIL, "holder missing manager" );
}




template <class T, class Mgr, class Descriptor>
void wResourceHolder<T, Mgr, Descriptor>::set
  ( T *newres )
{
  if ( Manager && Resource )
    Manager->release( Resource );
  Resource = newres;
}




// wSoundManager --------------------------------------------------------------
audio_data *wSoundManager::load( wFileManager::wStream &file, string const &name )
{
  wamPrintDebugLevel( "res: loading sound " + name, WAM_DEBUG_VERBOSE );

  auto_ptr<audio_data> data( new audio_data );
  data->loadWAV( file );
  return data.release();
}




template class wResourceHolder<wImage, wImageManager, wImageDescriptor>;
template class wResourceHolder<wShapeBitmap, wShapeBitmapManager>;
template class wResourceHolder<font, wFontManager>;
template class wResourceHolder<audio_data, wSoundManager>;




// wSoundHolder --------------------------------------------------------------
wSoundHolder::wSoundHolder( wGame &game, string const &name )
    : Super( game.getSoundManager(), name ), AudioManager( game.getAudioManager() )
{}




wSoundHolder::~wSoundHolder()
{
  updateStreamList();
  FOREACH( first, Streams, wStreamList )
  // NULL check not necessary: no AudioManager => we won't insert streams!
  AudioManager->removeStream( *first );
}




audio_manager::TStreamId wSoundHolder::play( TSize repeatcount, float delay, float vol, float panning )
{
  if ( AudioManager )
  {
    ref<audio_stream> strm = new audio_data_stream( *Resource, repeatcount, delay, vol, panning );
    audio_manager::TStreamId id = AudioManager->addStream( strm );
    Streams.push_back( id );
    return id;
  }
  return 0;
}




bool wSoundHolder::hasActiveStreams()
{
  updateStreamList();
  return Streams.size();
}




void wSoundHolder::updateStreamList()
{
  wStreamList::iterator first = Streams.begin(), last = Streams.end();
  while ( first != last )
  {
    if ( !AudioManager->isStreamActive( *first ) )
    {
      first = Streams.erase( first );
      last = Streams.end();
    }
    else
      first++;
  }
}
