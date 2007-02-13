// ----------------------------------------------------------------------------
//  Description      : WAM message manager
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_MESSAGE
#define WAM_MESSAGE




#include <queue>
#include <ixlib_garbage.hh>
#include "utility/manager.hh"
#include "engine/gameject.hh"



void registerMessageManagerCommands( wGame &g );




// types ----------------------------------------------------------------------
typedef unsigned int TMessageId;
typedef string wMessageName;




// wMessageReceiver ----------------------------------------------------------
class wMessageReceiver : virtual public wGameject
{
  public:
    wMessageReceiver( wGame &g );

    void registerMe();
    void unregisterMe();

    typedef pair<string, void *> wMessageReturn;
    virtual ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 ) = 0;
};




// wMessageManager -----------------------------------------------------------
class wMessageManager : public wRegisteringManager<wMessageReceiver>
{
    typedef vector<wMessageName> wMessageRegister;
    wMessageRegister MessageRegister;

    struct wMessage
    {
      wMessageReceiver *Destination;
      TMessageId Id;
      string Parameter1;
      void *Parameter2;
    };
    typedef queue<wMessage> wMessageQueue;
    wMessageQueue MessageQueue;

  public:

    TSize getFirstMessageId() const
    {
      return 0;
    }
    TSize getFirstUnusedMessageId() const
    {
      return MessageRegister.size();
    }
    TMessageId registerMessage( wMessageName const &name );
    wMessageName getMessageName( TMessageId id ) const;

    void run();

    typedef vector<ref<wMessageReceiver::wMessageReturn> > wResultSet;
    void post( TMessageId id, string const &par1 = "", void *par2 = NULL );
    void postTo( wGameject *rec, TMessageId id, string const &par1 = "", void *par2 = NULL );
    void send( TMessageId id, wResultSet *res = NULL, string const &par1 = "", void *par2 = NULL );
    ref<wMessageReceiver::wMessageReturn> sendTo( wGameject *rec, TMessageId id, string const &par1 = "", void *par2 = NULL );
    ref<wMessageReceiver::wMessageReturn> sendTo( wObject::TId id, TMessageId msg_id, string const &par1 = "", void *par2 = NULL );

  private:
    wMessageReceiver *castFromGameject( wGameject *gj );
};




#endif
