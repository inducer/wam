// ----------------------------------------------------------------------------
//  Description      : Resource management
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/basic_types.hh"
#include "utility/resourcemanager.hh"




// descriptor support ---------------------------------------------------------
inline string describeResource( string const &desc )
{
  return desc;
}




inline string getResourceFileName( string const &desc )
{
  return desc;
}




// wResourceManager -----------------------------------------------------------
template <class T, class Descriptor>
wResourceManager<T, Descriptor>::~wResourceManager()
{
  typename wResourceMap::iterator first;

  while ( ( first = Resources.begin() ) != Resources.end() )
  {
    delete first->second.Data;
    wamPrintDebugLevel( "res: cleanup, freeing " + describeResource( first->first ), WAM_DEBUG_VERBOSE );
    Resources.erase( first );
  }
}




template <class T, class Descriptor>
void wResourceManager<T, Descriptor>::listObjects( wResourceList &l )
{
  FOREACH( first, Resources, typename wResourceMap )
  l.push_back( first->first );
}




template <class T, class Descriptor>
T *wResourceManager<T, Descriptor>::get
  ( Descriptor const &desc )
{
  typename wResourceMap::iterator first = Resources.find( desc );

  if ( first != Resources.end() )
  {
    wResource & data = first->second;
    wamPrintDebugLevel( "res: " + describeResource( desc ) + " already loaded", WAM_DEBUG_VERBOSE );
    data.RefCount++;
    return data.Data;
  }
  else
  {
    wResource res;
    res.RefCount = 0;
    res.Data = load( desc );
    res.RefCount++;
    Resources[ desc ] = res;
    return res.Data;
  }
}




template <class T, class Descriptor>
void wResourceManager<T, Descriptor>::addReference( T *resource )
{
  FOREACH( first, Resources, typename wResourceMap )
  if ( first->second.Data == resource )
  {
    wamPrintDebugLevel( "res: added ref to " + describeResource( first->first ), WAM_DEBUG_VERBOSE );
    first->second.RefCount++;
    return ;
  }
  EXGAME_THROWINFO( ECGAME_RESOURCENOTAVAIL, "wResourceManager::addReference" );
}




template <class T, class Descriptor>
void wResourceManager<T, Descriptor>::release( T *resource )
{
  FOREACH( first, Resources, typename wResourceMap )
  if ( first->second.Data == resource )
  {
    if ( first->second.RefCount == 0 )
      EXGAME_THROWINFO( ECGAME_OVERFREEDRES, describeResource( first->first ).c_str() )
      wamPrintDebugLevel( "res: released " + describeResource( first->first ), WAM_DEBUG_VERBOSE );
    first->second.RefCount--;
    first->second.CleanupGraceCount = 3;
    return ;
  }
  EXGAME_THROWINFO( ECGAME_RESOURCENOTAVAIL, "wResourceManager::release" );
}




template <class T, class Descriptor>
void wResourceManager<T, Descriptor>::cleanUp()
{
  typename wResourceMap::iterator 
    first = Resources.begin(), last = Resources.end();

  while ( first != last )
  {
    bool advance = true;
    if ( first->second.RefCount == 0 )
    {
      if ( first->second.CleanupGraceCount == 0 )
      {
        delete first->second.Data;
        wamPrintDebug( "res: freeing " + describeResource( first->first ) );
        Resources.erase( first );
        first = Resources.begin();
        last = Resources.end();
        advance = false;
      }
      else
        first->second.CleanupGraceCount--;
    }
    if ( advance )
      first++;
  }
}




// wStreamResourceManager -------------------------------------------------------
template <class T, class Descriptor>
T *wStreamResourceManager<T, Descriptor>::load( Descriptor const &desc )
{
  ref<wFileManager::wStream>
  file( FileManager.openFile( getFileType(), getResourceFileName( desc ) ) );
  return load( *file, desc );
}
