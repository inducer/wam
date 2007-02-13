// ----------------------------------------------------------------------------
//  Description      : Persistence Management
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#include <iterator>
#include <algorithm>
#include <time.h>
#include <ixlib_xml.hh>
#include "utility/debug.hh"
#include "utility/manager_impl.hh"
#include "utility/filemanager.hh"
#include "utility/string_pack.hh"
#include "engine/gamebase.hh"
#include "engine/commandmanager.hh"
#include "engine/persistence.hh"




#define XML_TAG_SAVEGAME "savegame"
#define XML_TAG_GAMEJECTS "gamejects"
#define XML_TAG_GAMEJECT "gameject"
#define XML_TAG_VERSION  "version"
#define XML_TAG_LIBRARIES "libraries"
#define XML_TAG_LIBRARY  "library"
#define XML_ATTR_MAJOR  "major"
#define XML_ATTR_MINOR  "minor"
#define XML_ATTR_DATE  "date"
#define XML_ATTR_RELATIVE  "relative"
#define XML_ATTR_PATH  "path"
#define XML_ATTR_FEATURESET     "features"
#define XML_ATTR_PERSIST_MODE "persistmode"
#define XML_VALUE_SINGLETON "singleton"
#define XML_VALUE_TRANSIENT "transient"




// tools ----------------------------------------------------------------------
namespace
{
struct less_SaveOrder_wGameject
{
  bool operator() ( wPersistentGameject const *const op1, wPersistentGameject const *const op2 ) const
  {
    return op1->getSaveOrder() < op2->getSaveOrder();
  }
};
}




// console interface ----------------------------------------------------------
WAM_DECLARE_COMMAND( game_save, "Save", "save game state", "filename" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_save", 1, 1 )

  Game.getPersistenceManager().saveState( parameters[ 0 ] ->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( game_load, "Load", "load game state", "filename" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_load", 1, 1 )

  Game.getPersistenceManager().loadState( parameters[ 0 ] ->toString() );
  return makeNull();
}




WAM_DECLARE_COMMAND( game_clear, "Clea", "reset game state", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "game_clear", 0, 0 )

  Game.getPersistenceManager().clearState();
  return makeNull();
}




// wPersistentGameject --------------------------------------------------------
wPersistentGameject::wPersistentGameject( wGame &g )
    : wGameject( g )
{}




void wPersistentGameject::registerMe()
{
  Game.getPersistenceManager().registerObject( this );
}




void wPersistentGameject::unregisterMe()
{
  Game.getPersistenceManager().unregisterObject( this );
}




// wPersistenceManager --------------------------------------------------------
template wRegisteringManager<wPersistentGameject>
;




wPersistenceManager::wPersistenceManager( wGame &g )
    : Game( g )
{}





void wPersistenceManager::loadState( string const &name )
{
  wamPrintDebugLevel( "loading saved game " + name, WAM_DEBUG_NORMAL );
  ref<wFileManager::wStream> file =
    Game.getFileManager().openFile( WAM_FT_SAVEGAME, name );

  clearState();

  xml_file infile;
  infile.read( *file );

  // begin parsing xml

  // parse header
  xml_file::tag *roottag = infile.getRootTag();
  xml_file::tag *version = roottag->findTag( XML_TAG_VERSION );
  if ( !version )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "missing version tag" );
  unsigned int major, minor;
  version->Attributes[ XML_ATTR_MAJOR ] >>= major;
  version->Attributes[ XML_ATTR_MINOR ] >>= minor;
  if ( major > WAM_MAJOR_VERSION || ( major == WAM_MAJOR_VERSION && minor > WAM_MINOR_VERSION ) )
    wamPrintDebugLevel( "saved game version higher than program version", WAM_DEBUG_WARN );

  // load shared libraries
  xml_file::tag *shlibs = roottag->findTag( XML_TAG_LIBRARIES );
  if ( !shlibs )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "missing libraries section" );

  FOREACH( first, shlibs->Children, vector<xml_file::tag *> )
  {
    bool relative;
    ( *first ) ->Attributes[ XML_ATTR_RELATIVE ] >>= relative;
    string name = ( *first ) ->Attributes[ XML_ATTR_PATH ];
    if ( relative )
      name = Game.getFileManager().getBaseDirectory() + name;
    Game.getGameCodeManager().loadLibrary( name );
  }

  // load gamejects
  xml_file::tag *gjs = roottag->findTag( XML_TAG_GAMEJECTS );
  if ( !gjs )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "missing gamejects section" );

  FOREACH( first, gjs->Children, vector<xml_file::tag *> )
  {
    if ( ( *first ) ->getName() != XML_TAG_GAMEJECT )
      EXGAME_THROWINFO( ECGAME_FILEFORMAT, "invalid tag" );

    wFeatureSetProcessor features;
    ( *first ) ->Attributes[ XML_ATTR_FEATURESET ] >>= features;

    bool singleton = ( *first ) ->Attributes[ XML_ATTR_PERSIST_MODE ] == XML_VALUE_SINGLETON;

    wGameject *temp = NULL;
    wPersistentGameject *pgj;
    if ( singleton )
    {
      try
      {
        temp = Game.getInstanceManager().getObjectByFeatureSet( features.c_str() );
        wamPrintDebug( ( "loading singleton " + unsigned2dec( temp->id() ) + " with features " + features.getDisplayRep() ).c_str() );
      }
      catch ( ... )
      {
        temp = Game.getInstanceManager().createObject( features.c_str() );
        wamPrintDebug( ( "creating singleton " + unsigned2dec( temp->id() ) + " with features " + features.getDisplayRep() ).c_str() );
      }
    }
    else
    {
      temp = Game.getInstanceManager().createObject( features.c_str() );
      wamPrintDebug( ( "creating object " + unsigned2dec( temp->id() ) + " with features " + features.getDisplayRep() ).c_str() );
    }

    pgj = dynamic_cast<wPersistentGameject *>( temp );
    if ( pgj == NULL )
      EXGAME_THROWINFO( ECGAME_GENERAL, "cast to persistent gj failed" )
      pgj->load( **first );
  }
}




void wPersistenceManager::saveState( string const &name )
{
  wamPrintDebugLevel( "saving game to " + name, WAM_DEBUG_NORMAL );

  // start building xml
  xml_file::tag *roottag = new xml_file::tag( XML_TAG_SAVEGAME );
  char datebuf[ 40 ];
  time_t timebuf = time( NULL );
  tm *mytime = localtime( &timebuf );
  strftime( datebuf, 40, "%Y-%m-%d %k:%M:%S", mytime );
  roottag->Attributes[ XML_ATTR_DATE ] = datebuf;

  // include version number
  xml_file::tag *version = new xml_file::tag( XML_TAG_VERSION );
  version->Attributes[ XML_ATTR_MAJOR ] <<= WAM_MAJOR_VERSION;
  version->Attributes[ XML_ATTR_MINOR ] <<= WAM_MINOR_VERSION;
  roottag->appendTag( version );

  // include list of used shared libs
  xml_file::tag *libs = new xml_file::tag( XML_TAG_LIBRARIES );
  set
    <string> usedlibs;
  copy( Game.getGameCodeManager().lib_begin(),
        Game.getGameCodeManager().lib_end(),
        inserter( usedlibs, usedlibs.begin() ) );

  FOREACH( first, usedlibs, set
             <string> )
  {
    xml_file::tag * shlib = new xml_file::tag( XML_TAG_LIBRARY );
    string name = *first;
    string basedir = Game.getFileManager().getBaseDirectory();
    bool relative = name.substr( 0, basedir.size() ) == basedir;
    shlib->Attributes[ XML_ATTR_RELATIVE ] <<= relative;
    if ( relative )
      name = name.substr( basedir.size() );
    shlib->Attributes[ XML_ATTR_PATH ] <<= name;
    libs->appendTag( shlib );
  }
  roottag->appendTag( libs );

  // include all used gamejects
  xml_file::tag *gjs = new xml_file::tag( XML_TAG_GAMEJECTS );

  typedef vector<wPersistentGameject *> wSaveList;

  wSaveList gjlist;
  copy( begin(), end(), back_inserter( gjlist ) );
  sort( gjlist.begin(), gjlist.end(), less_SaveOrder_wGameject() );

  FOREACH( first, gjlist, wSaveList )
  {
    xml_file::tag * gj = new xml_file::tag( XML_TAG_GAMEJECT );
    gj->Attributes[ XML_ATTR_FEATURESET ] <<= ( *first ) ->getFeatureSet();

    if ( ( *first ) ->getPersistenceMode() == wPersistentGameject::PERSISTENCE_SINGLETON )
      gj->Attributes[ XML_ATTR_PERSIST_MODE ] = XML_VALUE_SINGLETON;
    else
      gj->Attributes[ XML_ATTR_PERSIST_MODE ] = XML_VALUE_TRANSIENT;

    ( *first ) ->save( *gj );
    gjs->appendTag( gj );
  }
  roottag->appendTag( gjs );

  // write everything to disk
  xml_file outfile;
  outfile.setRootTag( roottag );

  ref<wFileManager::wStream> file =
    Game.getFileManager().openFile( WAM_FT_SAVEGAME, name, "", ios::out );
  outfile.write( *file );
}




void wPersistenceManager::clearState()
{
  FOREACH( first, *this, wPersistenceManager )
  if ( ( *first ) ->getPersistenceMode() == wPersistentGameject::PERSISTENCE_TRANSIENT )
    Game.getInstanceManager().requestDestruction( *first );
}




void registerPersistenceCommands( wGame &g )
{
  register_game_save( g );
  register_game_load( g );
  register_game_clear( g );
}
