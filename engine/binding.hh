// ----------------------------------------------------------------------------
//  Description      : Binding manager
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_BINDING
#define WAM_BINDING




#include <set>
#include "engine/input.hh"
#include "engine/console.hh"




// registration ---------------------------------------------------------------
#define GJFEATURESET_BINDINGMGR  GJFEATURE_MANAGER "Bind"

void registerBindingManager( wGame &g );




// types ----------------------------------------------------------------------
struct wKey
{
  bool Mouse;
  union {
    TIndex Button;
    SDLKey Key;
  };
};

typedef set
  <wKey> wKeyList;

bool operator==( wKey const &key1, wKey const &key2 );
bool operator!=( wKey const &key1, wKey const &key2 );
bool operator<=( wKeyList const &keys1, wKeyList const &keys2 );
bool operator==( wKeyList const &keys1, wKeyList const &keys2 );




// wBindingManager -----------------------------------------------------------
class wBindingManager : public wKeyboardInputListener, public wMouseInputListener
{
  public:
    struct wKeyBinding
    {
      wKeyList Keys;
      ref<value> OnPress, OnRelease;
      bool Active;
    };

    typedef vector<wKeyBinding> wBindingList;

  private:
    wBindingList Bindings;
    wKeyList ActiveKeys;
    bool BindingsChanged;

  public:
    typedef wBindingList::iterator iterator;
    typedef wBindingList::const_iterator const_iterator;

    wBindingManager( wGame &g );

    TSize size() const
    {
      return Bindings.size();
    }

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_BINDINGMGR;
    }
    TPriority getKeyboardInputPriority() const
    {
      return WAM_INPUTPRIO_METAMANAGER;
    }
    TPriority getMouseInputPriority() const
    {
      return WAM_INPUTPRIO_METAMANAGER;
    }
    bool processKeyEvent( wKeyboardEvent &event );
    bool processMouseEvent( wMouseEvent &event );

    void registerMe();
    void unregisterMe();

    void bind( wKeyList const &keys, ref<value> onpress, ref<value> onpress = NULL );
    void unbind( wKeyList const &keys );
    void clear();
    wKeyBinding const &getBinding( wKeyList const &keys ) const;

    iterator begin()
    {
      return Bindings.begin();
    }
    const_iterator begin() const
    {
      return Bindings.begin();
    }
    iterator end()
    {
      return Bindings.end();
    }
    const_iterator end() const
    {
      return Bindings.end();
    }

    static wKey text2key( string const &str );
    static string key2text( wKey const &key );

    static wKeyList text2keys( string const &str );
    static string keys2text( wKeyList const &keys );

  private:
    bool internalUnbind( wKeyList const &keys );
    void update();
};




#endif
