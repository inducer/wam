// ----------------------------------------------------------------------------
//  Description      : Console
// ----------------------------------------------------------------------------
//  Remarks          :
//    Before doing output to the console, call clearPrompt.
//    After ..., call printPrompt.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_CONSOLE
#define WAM_CONSOLE




#include "engine/commandmanager.hh"
#include "engine/gameject.hh"
#include "engine/input.hh"
#include "engine/tick.hh"




// registration ---------------------------------------------------------------
#define GJFEATURESET_CONSOLE GJFEATURE_MANAGER "Cons"

void registerConsole( wGame &g );




// wConsoleDisplay -----------------------------------------------------------
class wConsoleDisplay
{
  public:
    virtual ~wConsoleDisplay()
    {}
    virtual void clearLine() = 0;
    virtual void output( string const &txt ) = 0;
    virtual void newLine() = 0;
    virtual void flush() = 0;

    virtual void enable()
    {}
    virtual void disable()
    {}
}
;




// fake output ----------------------------------------------------------------
extern struct wNewLine
  {}
newl;
extern struct wClearLine
  {}
clearl;




// wConsole ------------------------------------------------------------------
class wConsole : public wKeyboardInputListener, public wTickReceiver
{
    list<string> History;
    list<string>::iterator HistoryPosition;

    float KeyRepeat;
    float KeyWait;
    wKeyboardEvent LastKey;
    bool KeyRepeating;
    double WaitTillNextRepetition;

    string EditBuffer, CommandBuffer;
    TIndex CursorPosition;
    bool CursorInOnPhase;
    float CursorBlinkCounter;
    static const float CursorBlinkSpeed = 0.3;
    int PromptInhibit;
    bool Enabled;

    wCommandManager CommandManager;
    wConsoleDisplay *Display;

  public:

    wConsole const &operator<<( string const &what ) const;
    wConsole const &operator<<( char const *what ) const;
    wConsole const &operator<<( double what ) const;
    wConsole const &operator<<( unsigned int what ) const;
    wConsole const &operator<<( int what ) const;
    wConsole const &operator<<( char what ) const;
    wConsole const &operator<<( wNewLine const &newl ) const;
    wConsole const &operator<<( wClearLine const &clearl ) const;

    wConsole( wGame &g );
    ~wConsole();

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    float getKeyWait() const
    {
      return KeyWait;
    }
    void setKeyWait( float seconds )
    {
      KeyWait = seconds;
    }
    float getKeyRepeat() const
    {
      return KeyRepeat;
    }
    void setKeyRepeat( float seconds )
    {
      KeyRepeat = seconds;
    }
    void printDebug( string const &what );

    wConsoleDisplay *getDisplay() const
    {
      return Display;
    }
    void setDisplay( wConsoleDisplay *disp );

    void clearPrompt();
    void printPrompt();

    bool isConsoleEnabled() const
    {
      return Enabled;
    }
    void enableConsole( bool enabled );

    void tick( float seconds );
    bool processKeyEvent( wKeyboardEvent &event );

    signed int getKeyboardInputPriority() const
    {
      return WAM_INPUTPRIO_CONSOLE;
    }
    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_CONSOLE;
    }
    TPriority getDeleteOrder() const
    {
      return WAM_DELORDER_CONSOLE;
    }

    wCommandManager &getCommandManager()
    {
      return CommandManager;
    }

  private:
    bool getPreviousHistoryCommand( string &cmd );
    bool getNextHistoryCommand( string &cmd );
    void addHistoryCommand( string const &cmd );
};




// wConsoleLocker ------------------------------------------------------------
class wConsoleLocker
{
    wConsole &Console;

  public:
    wConsoleLocker( wConsole &cns )
        : Console( cns )
    {
      Console.clearPrompt();
    }
    ~wConsoleLocker()
    {
      Console.printPrompt();
    }
};




#endif
