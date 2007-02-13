// ----------------------------------------------------------------------------
//  Description      : Command manager
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <memory>
#include <ixlib_re.hh>
#include "utility/exgame.hh"
#include "utility/filemanager.hh"
#include "engine/codemanager.hh"
#include "engine/instancemanager.hh"
#include "engine/console.hh"
#include "engine/gamebase.hh"
#include "engine/commandmanager.hh"




#define JS_GAME_NAMESPACE  "wam"




// wCommand -----------------------------------------------------------
void wCommand::registerMe()
{
  Game.getConsole().getCommandManager().registerObject( this );
}




void wCommand::unregisterMe()
{
  Game.getConsole().getCommandManager().unregisterObject( this );
}




ref<value> wCommand::duplicate()
{
  // functions are not mutable
  return this;
}




// wCommandManager ----------------------------------------------------
wCommandManager::wCommandManager( wGame &game )
    : Game( game ), CommandManagerRef( this )
{
  javascript::addStandardLibrary( Interpreter );
  Interpreter.RootScope->addMember( JS_GAME_NAMESPACE, this );
}




wCommandManager::~wCommandManager()
{
  Interpreter.RootScope->clear();
}




void wCommandManager::getRegisteredObjects( list<wCommand *> &l )
{
  FOREACH_CONST( first, CommandList, wCommandList )
  l.push_back( first->Command );
}




void wCommandManager::registerObject( wCommand *cmd )
{
  wCommandList::iterator inspos = getLowerBound( cmd->getCommandString() );
  if ( inspos != CommandList.end() && inspos->Command->getCommandString() == cmd->getCommandString() )
    EXGAME_THROWINFO( ECGAME_CONSOLE, ( "duplicate command: " + cmd->getCommandString() ).c_str() );
  wCommandInfo info = { cmd, cmd };
  CommandList.insert( inspos, info );
}




void wCommandManager::unregisterObject( wCommand *cmd )
{
  CommandList.erase( getLowerBound( cmd->getCommandString() ) );
}




wCommand *wCommandManager::getCommand( string const &cmdstr )
{
  return getIterator( cmdstr ) ->Command;
}




ref<value> wCommandManager::execute( string const &cmd )
{
  wConsoleLocker locker( Game.getConsole() );
  return Interpreter.execute( cmd );
}




ref<value> wCommandManager::execute( istream &istr )
{
  wConsoleLocker locker( Game.getConsole() );
  return Interpreter.execute( istr );
}




ref<value> wCommandManager::executeScript( string const &script, string const &mpack )
{
  return execute( *Game.getFileManager().openFile( WAM_FT_SCRIPT, script, mpack ) );
}




void wCommandManager::completeCommandLine( string &cmdln )
{
  string cmd = cmdln;

  string const my_namespace = JS_GAME_NAMESPACE ".";

  if ( cmd == my_namespace.substr( 0, cmd.size() ) )
  {
    cmdln = my_namespace;
    return ;
  }
  if ( cmd.substr( 0, my_namespace.size() ) != my_namespace )
    return ;
  cmd = cmd.substr( my_namespace.size() );
  if ( cmd.size() == 0 )
    return ;

  string completion = "";
  bool complete;

  FOREACH_CONST( first, CommandList, wCommandList )
  {
    string cstr = first->Command->getCommandString();

    if ( cstr.substr( 0, cmd.size() ) == cmd )
    {
      if ( completion.size() == 0 )
      {
        completion = cstr;
        complete = true;
      }
      else
      {
        // just the unambiguous part
        TSize size = cmd.size();
        while ( size < cstr.size() && size < completion.size() && completion[ size ] == cstr[ size ] )
          size++;
        completion.resize( size );
        complete = false;
      }
    }
  }
  if ( completion.size() >= cmd.size() )
  {
    cmd = "wam." + completion;
    if ( complete )
      cmd += "(";
    cmdln = cmd;
  }
}




ref<value> wCommandManager::lookup( string const &identifier )
{
  return getCommand( identifier );
}




wCommandManager::wCommandList::iterator wCommandManager::getLowerBound( string const &cmdstr )
{
  if ( CommandList.size() == 0 )
    return CommandList.begin();

  TIndex first = 0, last = CommandList.size();

  // binary search between first and last
  while ( last - first > 1 )
  {
    int mid = ( last + first ) / 2;
    if ( CommandList[ mid ].Command->getCommandString() < cmdstr )
      first = mid;
    else
      last = mid;
  }

  if ( CommandList[ first ].Command->getCommandString() >= cmdstr )
    return CommandList.begin() + first;
  else
    return CommandList.begin() + last;
}




wCommandManager::wCommandList::iterator wCommandManager::getIterator( string const &cmdstr )
{
  wCommandList::iterator pos = getLowerBound( cmdstr );

  if ( pos == CommandList.end() || pos->Command->getCommandString() != cmdstr )
    EXGAME_THROWINFO( ECGAME_CONSOLE, ( "no such command: " + cmdstr ).c_str() )
    return pos;
}
