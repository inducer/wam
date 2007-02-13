// ----------------------------------------------------------------------------
//  Description      : File management
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WIN32
 #include <pwd.h>
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <iterator>
#include <fstream>
#include <ixlib_exgen.hh>
#include <dirent.h>
#include "utility/exgame.hh"
#include "utility/filemanager.hh"
#include "utility/debug.hh"




IXLIB_GARBAGE_DECLARE_MANAGER( wFileManager::wStream )




namespace
{
string getHomeDirectory()
{
#ifdef WIN32
  return ".";
#else

  passwd *pwd = getpwuid( getuid() );
  if ( pwd->pw_dir )
    return pwd->pw_dir;
  else
    return ".";
#endif

}

string getApplicationDirectory()
{
#ifdef WIN32
  return getHomeDirectory() + "/config";
#else

  return getHomeDirectory() + "/.wam";
#endif

}

void makeDirectory( const char *name )
{
#ifdef WIN32
  if ( mkdir( name ) )
    EXGAME_THROWINFO( ECGAME_GENERAL, ( "could not create directory: " + string( name ) ).c_str() )
#else
  if ( mkdir( name, 0700 ) )
    EXGAME_THROWINFO( ECGAME_GENERAL, ( "could not create directory: " + string( name ) ).c_str() )
#endif

  }
}




// wFileRegMask ---------------------------------------------------------------
bool wFileRegMask::select( const dirent *d )
{
  return Regex.match( d->d_name );
}




// wFileManager ---------------------------------------------------------------
wFileManager::wFileManager( string const &basedir )
    : BaseDirectory( basedir )
{
  string appdir = getApplicationDirectory();

  struct stat statbuf;
  if ( stat( appdir.c_str(), &statbuf ) )
    makeDirectory( appdir.c_str() );
}




void wFileManager::listMissionPacks( wMissionPackList &packs )
{
  copy( MissionPacks.begin(), MissionPacks.end(), back_inserter( packs ) );
}




void wFileManager::addMissionPack( string const &name )
{
  wMissionPackList::iterator
  first = MissionPacks.begin(), last = MissionPacks.end();

  while ( first != last )
    if ( *first++ == name )
      return ;

  MissionPacks.push_back( name );
}




void wFileManager::removeMissionPack( string const &name )
{
  wMissionPackList::iterator
  first = MissionPacks.begin(), last = MissionPacks.end();

  while ( first != last )
  {
    if ( *first == name )
    {
      MissionPacks.erase( first );
      return ;
    }
    first++;
  }
  EXGAME_THROWINFO( ECGAME_NOTFOUND, name.c_str() )
}




string wFileManager::getFilename( wFileType ft, string const &base, string const &mpack )
{
  if ( mpack.size() )
    return BaseDirectory + '/' + mpack + '/' + getTypeDirectory( ft ) + '/' + base;

  wSearchPath sp;
  getSearchPath( ft, sp );

  wSearchPath::const_iterator
  first = sp.begin(), last = sp.end();

  while ( first != last )
  {
    string fn = *first + '/' + base;
    if ( tryOpenFile( fn ) )
      return fn;
    first++;
  }
  if ( sp.size() )
    return sp.front() + '/' + base;
  return base;
}




ref<wFileManager::wStream>
wFileManager::openFile( wFileType ft, string const &base, string const &mpack, ios::openmode mode )
{

  string fn = getFilename( ft, base, mpack );
  ref<wStream> file = new wStream( fn.c_str(), mode | ios::binary );

  if ( !file->bad() )
    return file;
  else
    EXGAME_THROWINFO( ECGAME_RESOURCENOTAVAIL, fn.c_str() )
  }




void wFileManager::listFilenames( wFileType ft, wFilenameList &fnl, wFileMask *fm, string const &mpack )
{
  string typedir = getTypeDirectory( ft );
  if ( mpack.size() )
    listDirectory( ( BaseDirectory + '/' + mpack + '/' + typedir ).c_str(), fnl, fm );
  else
  {
    wSearchPath sp;
    getSearchPath( ft, sp );

    wSearchPath::const_iterator
    first = sp.begin(), last = sp.end();

    while ( first != last )
    {
      listDirectory( first->c_str(), fnl, fm );
      first++;
    }
  }
}





char const *wFileManager::getTypeDirectory( wFileType ft )
{
  switch ( ft )
  {
  case WAM_FT_IMAGE:
    return "images";
  case WAM_FT_SHAPE:
    return "shapes";
  case WAM_FT_FONT:
    return "fonts";
  case WAM_FT_SCRIPT:
    return "scripts";
  case WAM_FT_CODE:
    return "code";
  case WAM_FT_SOUND:
    return "sounds";
  case WAM_FT_MUSIC:
    return "music";
  case WAM_FT_DATA:
    return "data";
  default:
    EXGAME_THROW( ECGAME_INVALIDFILETYPE )
  }
}




void wFileManager::getSearchPath( wFileType ft, wSearchPath &path )
{
  if ( ft == WAM_FT_SAVEGAME )
  {
    string sg_dir = getApplicationDirectory() + "/savegames";

    struct stat statbuf;
    if ( stat( sg_dir.c_str(), &statbuf ) )
      makeDirectory( sg_dir.c_str() );
    path.push_back( sg_dir );
    return ;
  }

  if ( ft == WAM_FT_CONFIG )
  {
    path.push_back( getApplicationDirectory() );
    return ;
  }

  path.clear();
  string typedir = getTypeDirectory( ft );

  wMissionPackList::const_iterator
  first = MissionPacks.begin(), last = MissionPacks.end();

  if ( first == last )
    path.push_back( BaseDirectory + '/' + typedir );
  else
  {
    do
    {
      last--;
      path.push_back( BaseDirectory + '/' + *last + '/' + typedir );
    }
    while ( first != last );
  }
}




bool wFileManager::tryOpenFile( string const &filename )
{
  ifstream file( filename.c_str() );
  return !file.bad();
}




void wFileManager::listDirectory( char const *dir, wFilenameList &fnl, wFileMask *fm )
{
  DIR * directory;
  dirent *entry;

  directory = opendir( dir );
  if ( directory )
  {
    while ( ( entry = readdir( directory ) ) )
      fnl.push_back( entry->d_name );
    closedir( directory );
  }
}
