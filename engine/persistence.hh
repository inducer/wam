// ----------------------------------------------------------------------------
//  Description      : Persistence Management
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_PERSISTENCE
#define WAM_PERSISTENCE




#include "utility/manager.hh"




class wGame;




// persistence constants ------------------------------------------------------
#define GJSAVE_PREMANAGER               0
#define GJSAVE_MANAGER                  10
#define GJSAVE_DEFAULT                  100




// wPersistentGameject --------------------------------------------------------
class wPersistentGameject : public virtual wGameject
{
  public:
    enum TPersistenceMode {
      PERSISTENCE_SINGLETON,  // never delete, load into same object
      PERSISTENCE_TRANSIENT // delete when clearing game state, load into new object
  };

    wPersistentGameject( wGame &g );

    virtual void registerMe();
    virtual void unregisterMe();

    virtual TPersistenceMode getPersistenceMode() const = 0;
    virtual TPriority getSaveOrder() const = 0;

    virtual void load( xml_file::tag &tag ) = 0;
    virtual void save( xml_file::tag &tag ) = 0;
};




class wPersistenceManager : public wRegisteringManager<wPersistentGameject>
{
    wGame &Game;

  public:
    wPersistenceManager( wGame &g );

    void loadState( string const &name );
    void saveState( string const &name );
    void clearState();
};




void registerPersistenceCommands( wGame &g );




#endif
