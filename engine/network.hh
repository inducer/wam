// ----------------------------------------------------------------------------
//  Description      : WAM network exchange
// ----------------------------------------------------------------------------
//  Remarks          :
//    None of the core interfaces care about deletion. Gamejects should handle
//    that all by themselves, for example like wAutoReplicatingNetworkedGj
//    does.
//    Terminology: "parents" are gamejects that create "children" on other
//    hosts. To establish communications, a "child" must register as a
//    child of its parent. (any gameject can register as a child, it need
//    not have been created by the parent. is this adoption? :-)
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_NETWORK
#define WAM_NETWORK




#include "engine/gameject.hh"
#include "engine/gamebase.hh"
#include "utility/stream.hh"




class wStreamReader;
class wStreamWriter;




// wNetworkedGameject ---------------------------------------------------------
class wNetworkConnection;




class wNetworkedGameject : public virtual wGameject
{
    // the net connection via which this gameject was created, if any.
  public:
    wNetworkedGameject( wGame &s );

    void registerMe();
    void unregisterMe();

    virtual void notifyNewConnection( wNetworkConnection *cnx ) = 0;
    virtual void notifyEndedConnection( wNetworkConnection *cnx ) = 0;
    // to actually receive messages, call registerAsChildOf here
    virtual void notifyRemoteInitiation( wNetworkConnection *cnx, wObject::TId remote_id ) = 0;

    virtual void readMessage( wNetworkConnection *from, wStreamReader &reader ) = 0;

  protected:
    bool atMaster() const;
    void requestChildCreationAt( wNetworkConnection *at, bool singleton );
    void requestChildCreation( wNetworkConnection *except = NULL, bool singleton = false );
    void registerAsChildOf( wNetworkConnection *cnx, wObject::TId remote_id );
    void sendNetMessage( wNetworkConnection *to, wNetworkConnection *except, wStreamStore const &msg );
};




class wAutoReplicatingNetworkedGameject : public wNetworkedGameject
{
    typedef wNetworkedGameject super;

  protected:
    enum TReplicationRole {
      UNDETERMINED,
      ORIGINAL,
      COPY,
  };

  private:
    TReplicationRole Role;
    wNetworkConnection *ParentConnection;
    bool Dying;

    typedef TUnsigned8 TMessageKind;
    static const TMessageKind MSG_STATE_DUMP = 0;
    static const TMessageKind MSG_UPDATE = 1;
    static const TMessageKind MSG_DELETE = 2;

  public:
    wAutoReplicatingNetworkedGameject( wGame &s );

    void notifyNewConnection( wNetworkConnection *cnx );
    void notifyEndedConnection( wNetworkConnection *cnx );
    void notifyRemoteInitiation( wNetworkConnection *cnx, wObject::TId remote_id );

    void registerMe();
    void prepareToDie();

    void readMessage( wNetworkConnection *from, wStreamReader &reader );

    virtual void writeStateDump( wStreamWriter &writer ) = 0;
    virtual void writeUpdate( wStreamWriter &writer ) = 0;
    virtual void readStateDump( wStreamReader &reader ) = 0;
    virtual void readUpdate( wStreamReader &reader ) = 0;

  protected:
    void requestSendUpdate();
    TReplicationRole networkRole()
    {
      return Role;
    }

  private:
    void sendMessage( wNetworkConnection *to, wNetworkConnection *except, TMessageKind kind );
};




#endif
