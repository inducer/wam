// ----------------------------------------------------------------------------
//  Description      : Command manager
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_COMMANDMANAGER
#define WAM_COMMANDMANAGER




#include <dirent.h>
#include <vector>
#include <stack>
#include <ixlib_javascript.hh>
#include "engine/instancemanager.hh"
#include "engine/gameject.hh"




using namespace javascript;




// constants ------------------------------------------------------------------
#define GJFEATURESET_CONSOLECOMMAND "CCmd"




// wCommand -------------------------------------------------------------------
class wCommand : public wGameject, public value
{
  public:
    typedef vector<string> wParameterList;

    wCommand( wGame &g )
        : wGameject( g )
    {}

    void registerMe();
    void unregisterMe();

    virtual string getCommandString() = 0;

    virtual string getCommandLine() = 0;
    virtual string getHelpText() = 0;

    // javascript interface
    value_type getType() const
    {
      return VT_FUNCTION;
    }
    ref<value> duplicate();
};




// wCommandManager ------------------------------------------------------------
class wCommandManager : public value
{
  protected:
    struct wCommandInfo
    {
      wCommand *Command;
      no_free_ref<value> CommandRef;
    };

    wGame &Game;
    no_free_ref<value> CommandManagerRef;
    typedef vector<wCommandInfo> wCommandList;
    wCommandList CommandList;
    javascript::interpreter Interpreter;

  public:
    wCommandManager( wGame &game );
    virtual ~wCommandManager();

    void getRegisteredObjects( list<wCommand *> &l );
    void registerObject( wCommand *cmd );
    void unregisterObject( wCommand *cmd );

    wCommand *getCommand( string const &cmdstr );

    ref<value> execute( string const &cmd );
    ref<value> execute( istream &istr );
    ref<value> executeScript( string const &script, string const &mpack = "" );

    void completeCommandLine( string &cmdln );

  protected:
    // javascript interface
    value_type getType() const
    {
      return VT_HOST;
    }
    ref<value> lookup( string const &identifier );

    wCommandList::iterator getLowerBound( string const &cmdstr );
    wCommandList::iterator getIterator( string const &cmdstr );
};




#define WAM_DECLARE_COMMAND(COMMANDSTRING,ADD_FEATURES,HELPTEXT,PAR) \
  namespace wamCommands { \
    class COMMANDSTRING : public wCommand { \
      public: \
        COMMANDSTRING(wGame &g) \
          : wCommand(g) {\
          } \
        wFeatureSet const getFeatureSet() const { \
          return GJFEATURESET_CONSOLECOMMAND ADD_FEATURES; \
          } \
        string getCommandString() {\
          return #COMMANDSTRING; \
          } \
        string getCommandLine() { \
          return #COMMANDSTRING " " PAR; \
          } \
        string getHelpText() { \
          return HELPTEXT; \
          } \
        ref<value> call(parameter_list const &parameters); \
      }; \
    } \
  namespace { \
    inline void register_##COMMANDSTRING(wGame &g) { \
      wGameject *cmd = new wamCommands::COMMANDSTRING(g); \
      g.getInstanceManager().insertObject(cmd); \
      } \
    } \
  ref<value> wamCommands::COMMANDSTRING::call(value::parameter_list const &parameters)





#endif
