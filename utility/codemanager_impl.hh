// ----------------------------------------------------------------------------
//  Description      : WAM class manager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <algorithm>
#include <string.h>
#include <ixlib_re.hh>
#include <ixlib_numconv.hpp>
#include "utility/exgame.hh"
#include "utility/debug.hh"
#include "utility/filemanager.hh"
#include "utility/codemanager.hh"

#ifdef HAVE_DYNAMIC_LINKING
 #ifdef WIN32
 #include <windows.h>
 #else
 #include <dlfcn.h>
 #endif
#endif




// wCodeManager ---------------------------------------------------------------
template <class Object, class MainClass>
void wCodeManager<Object, MainClass>::registerLibrary( string const &pathname, wLibraryInitFunction initf )
{
  initf( MainObject, pathname.c_str() );
  LibList.push_back( pathname );
}




template <class Object, class MainClass>
void wCodeManager<Object, MainClass>::loadLibrary( string const &pathname )
{
  string basename = pathname;
  regex_string re( "([^/\\]+)$" );
  if ( re.match( pathname ) )
    basename = re.getBackref( 0 );

  wLibraryInitFunction initf = NULL;

  FOREACH_CONST( first, LibList, wLibList )
  if ( ( *first ) == basename )
  {
    wamPrintDebug( "shared lib " + basename + " already loaded" );
    return ;
  }

  wamPrintDebugLevel( "loading shared lib " + pathname, WAM_DEBUG_NORMAL );
#ifdef HAVE_DYNAMIC_LINKING
 #ifdef WIN32
 #else

  void *handle = dlopen( pathname.c_str(), RTLD_NOW );
  if ( !handle )
    EXGAME_THROWINFO( ECGAME_SHAREDLIB, dlerror() )

    initf = reinterpret_cast<wLibraryInitFunction>(
        dlsym( handle, "wamInitializeModule" ));

#endif
 #endif

  if ( !initf )
    EXGAME_THROWINFO( ECGAME_SHAREDLIB, pathname.c_str() )

    registerLibrary( basename, initf );
}




template <class Object, class MainClass>
void wCodeManager<Object, MainClass>::loadCode( string const &base, string const &mpack )
{
  loadLibrary( MainObject.getFileManager().getFilename( WAM_FT_CODE, base, mpack ).c_str() );
}




template <class Object, class MainClass>
void wCodeManager<Object, MainClass>::loadAllCode( string const &mpack )
{
  wFileManager::wFilenameList fl;
  wFileRegMask fm( ".*\\.so$" );
  MainObject.getFileManager().listFilenames( WAM_FT_CODE, fl, &fm, mpack );

  FOREACH_CONST( first, fl, wFileManager::wFilenameList )
  loadCode( *first );
}




template <class Object, class MainClass>
Object *wCodeManager<Object, MainClass>::createObject( wFeatureSet const &features ) const
{
  return getClass( features ) ->instantiate( MainObject );
}




template <class Object, class MainClass>
typename wCodeManager<Object, MainClass>::wClassIdentifier
wCodeManager<Object, MainClass>::registerClass( wFeatureSet const &features, string const &libname, wInstantiationFunc creator )
{
  wClassIdentifier result = NextId;
  wClass cls = { NextId++, features, libname, creator };
  ClassList.push_back( cls );
  return result;
}



template <class Object, class MainClass>
void wCodeManager<Object, MainClass>::unregisterClass( wClassIdentifier id )
{
  ClassList.erase( getClass( id ) );
}




template <class Object, class MainClass>
typename wCodeManager<Object, MainClass>::class_iterator
wCodeManager<Object, MainClass>::getClass( wClassIdentifier id )
{
  FOREACH( first, ClassList, typename wClassList )
  if ( first->Id == id )
    return first;

  EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "class id " + unsigned2dec( id ) ).c_str() )
}




template <class Object, class MainClass>
typename wCodeManager<Object, MainClass>::class_const_iterator
wCodeManager<Object, MainClass>::getClass( wFeatureSet const &features ) const
{
  wFeatureSetProcessor feat = features;

  FOREACH_CONST( first, ClassList, typename wClassList )
  if ( first->Features.hasFeatures( features ) )
    return first;

  EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "class having " + feat.getDisplayRep() ).c_str() )
}
