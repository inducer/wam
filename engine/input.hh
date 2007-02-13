// ----------------------------------------------------------------------------
//  Description      : WAM server-side input manager
// ----------------------------------------------------------------------------
//  Remarks          :
//    Zero is highest priority.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_GAME_INPUT
#define WAM_GAME_INPUT




#include <SDL.h>
#include <list>
#include "utility/manager.hh"
#include "engine/gameject.hh"




void registerInputManagerCommands( wGame &g );




// wKeyboardEvent -------------------------------------------------------------
class wKeyboardEvent
{
  public:
    enum TEventType { KEY_PRESS, KEY_RELEASE };

  private:
    TEventType Type;

    SDLKey Key;
    SDLMod Modifier;
    TUnsigned16 Unicode;

  public:
    wKeyboardEvent()
    {}
    wKeyboardEvent( SDL_KeyboardEvent const &event );

    TEventType getType() const
    {
      return Type;
    }
    SDLKey getKey() const
    {
      return Key;
    }
    SDLMod getModifier() const
    {
      return Modifier;
    }

    bool isPrintable();
    TUnsigned16 getUnicode() const;
    char getASCII() const;
};




class wMouseEvent
{
  public:
    enum TEventType { MOTION, BUTTON_PRESS, BUTTON_RELEASE };
    typedef coord_vector<int, 2> wMouseVector;

  private:
    TEventType Type;

    wMouseVector Position;
    wMouseVector RelativeMovement;
    TIndex Button;

  public:
    wMouseEvent()
    {}
    wMouseEvent( SDL_MouseMotionEvent const &event );
    wMouseEvent( SDL_MouseButtonEvent const &event );

    TEventType getType() const
    {
      return Type;
    }
    TIndex getButton() const
    {
      return Button;
    }
    wMouseVector const &getPosition() const
    {
      return Position;
    }
    wMouseVector const &getRelativeMovement() const
    {
      return RelativeMovement;
    }
};




// Constants ------------------------------------------------------------------
#define WAM_INPUTPRIO_SCROLLER          0
#define WAM_INPUTPRIO_POINTER           10
#define WAM_INPUTPRIO_CONSOLE           20
#define WAM_INPUTPRIO_MENU         30
#define WAM_INPUTPRIO_GAMEJECT          40
#define WAM_INPUTPRIO_METAMANAGER       50




// wKeyboardInputListener ----------------------------------------------------
class wKeyboardInputListener : virtual public wGameject
{
  public:
    wKeyboardInputListener( wGame &g )
        : wGameject( g )
    {}
    virtual TPriority getKeyboardInputPriority() const = 0;
    // const intentionally left out: listeners can act as filters
    virtual bool processKeyEvent( wKeyboardEvent &event ) = 0;
    void registerMe();
    void unregisterMe();
};




// wMouseInputListener ----------------------------------------------------
class wMouseInputListener : virtual public wGameject
{
  public:
    wMouseInputListener( wGame &g )
        : wGameject( g )
    {}
    virtual TPriority getMouseInputPriority() const = 0;
    // const intentionally left out: listeners can act as filters
    virtual bool processMouseEvent( wMouseEvent &event ) = 0;
    void registerMe();
    void unregisterMe();
};




// wInputManager --------------------------------------------------------------
class wInputManager : public wPriorityManager<wKeyboardInputListener>,
      public wPriorityManager<wMouseInputListener>
{
    typedef wPriorityManager<wKeyboardInputListener> KeyboardSuper;
    typedef wPriorityManager<wMouseInputListener> MouseSuper;

  public:
    typedef KeyboardSuper::iterator keyboard_iterator;
    typedef KeyboardSuper::const_iterator keyboard_const_iterator;
    typedef MouseSuper::iterator mouse_iterator;
    typedef MouseSuper::const_iterator mouse_const_iterator;

    wInputManager();

    keyboard_iterator keyboard_begin()
    {
      return KeyboardSuper::begin();
    }
    keyboard_const_iterator keyboard_begin() const
    {
      return KeyboardSuper::begin();
    }
    keyboard_iterator keyboard_end()
    {
      return KeyboardSuper::end();
    }
    keyboard_const_iterator keyboard_end() const
    {
      return KeyboardSuper::end();
    }
    mouse_iterator mouse_begin()
    {
      return MouseSuper::begin();
    }
    mouse_const_iterator mouse_begin() const
    {
      return MouseSuper::begin();
    }
    mouse_iterator mouse_end()
    {
      return MouseSuper::end();
    }
    mouse_const_iterator mouse_end() const
    {
      return MouseSuper::end();
    }

    void registerObject( wKeyboardInputListener *obj );
    void unregisterObject( wKeyboardInputListener *obj );
    void registerObject( wMouseInputListener *obj );
    void unregisterObject( wMouseInputListener *obj );

    bool run();
    void processEvent( wKeyboardEvent const &event );
    void processEvent( wMouseEvent const &event );

  private:
    TPriority getPriority( wKeyboardInputListener const *const o ) const
    {
      return o->getKeyboardInputPriority();
    }
    TPriority getPriority( wMouseInputListener const *const o ) const
    {
      return o->getMouseInputPriority();
    }
};




#endif
