// ----------------------------------------------------------------------------
//  Description      : WAM animation script language
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_ANIMATOR
#define WAM_ANIMATOR




#include "utility/hash_map.hh"
#include <vector>
#include <ixlib_xml.hh>
#include <ixlib_re.hh>
#include "engine/resource.hh"




class wAnimator
{
  protected:
    class wCommand;

    typedef double wExecutionPriority;

    struct wExecutionContext
    {
      wCommand *InstructionPointer;
      double RemainingDelay;
      wExecutionPriority CurrentPriority;

      wExecutionContext()
          : InstructionPointer( NULL ), RemainingDelay( 0 ),
            CurrentPriority( 0 )
      {}
      wExecutionContext( wCommand *ip, wExecutionPriority priority,double delay )
          : InstructionPointer( ip ), RemainingDelay( delay ),
            CurrentPriority( priority )
      {}
      void step( double seconds );
    };

    wImageManager &ImageManager;

    struct wFrameInfo
    {
      wImageHolder Image;
      bool Dynamic;
      string ScaleXExpression,ScaleYExpression,RotateExpression;
    };

    typedef hash_map<string, wFrameInfo, string_hash> wImageHash;
    wImageHash Images;

    typedef pair<regex_string, string> wRegex;
    typedef vector<wRegex> wRegexList;
    wRegexList Regexes;

    typedef hash_map<string, string, string_hash> wParameterList;
    wParameterList Parameters;

    typedef vector<wExecutionContext> wExecutionStack;
    wExecutionStack Stack;
    wExecutionContext ExecutionContext;

    typedef hash_map<string, wCommand *, string_hash> wLabelList;
    wLabelList Labels;

    typedef vector<wCommand *> wCommandList;
    wCommandList DisposeList;

    struct wEvent
    {
      string Parameter, OldValue, NewValue;
      bool OldRequired, NewRequired;
      wExecutionPriority Priority;
      bool WillReturn;
      wCommand *Handler;
    };
    typedef vector<wEvent> wEventList;
    wEventList EventList;

    typedef queue<string> wSignalQueue;
    wSignalQueue PendingSignals;

    wImage LastDynamicImage;
    wFrameInfo *CurrentFrame;
    bool CurrentFrameChanged;

    // "Next" refers to "textually subsequent", whereas execute returns
    // the next command to be executed
    class wCommand
    {
      protected:
        wAnimator &Animator;

      private:
        wCommand *NextCommand;

      public:
        wCommand( wAnimator &animator )
            : Animator( animator ), NextCommand( NULL )
        {}
        virtual ~wCommand()
        {}
        virtual wExecutionContext execute( double priority ) = 0;
        virtual void setNextCommand( wCommand *cmd );
        wCommand *getNextCommand();
        wCommand *getLastCommand();
    };

    class wIf : public wCommand
    {
      protected:
	wCommand *IfPart, *ElsePart;
	string Parameter, Value;

      public:
	wIf( wAnimator &animator, wCommand *ifpart, wCommand *elsepart,
	    string const &parameter, string const &value )
	  : wCommand( animator ), IfPart( ifpart ), ElsePart( elsepart ),
	Parameter( parameter ), Value( value )
	{}
	wExecutionContext execute( double priority );
	void setNextCommand( wCommand *cmd );
    };

    class wOnReturn : public wCommand
    {
      protected:
	wCommand *Handler;

      public:
	wOnReturn( wAnimator &animator, wCommand *handler )
	  : wCommand( animator ), Handler( handler )
	  {}
	wExecutionContext execute( double priority );
    };

    class wPrioritySet : public wCommand
    {
      protected:
	wExecutionPriority Priority;

      public:
	wPrioritySet( wAnimator &animator, wExecutionPriority priority )
	  : wCommand( animator ), Priority( priority )
	  {}
	wExecutionContext execute( double priority );
    };

    class wGoto : public wCommand
    {
      protected:
	string Label;

      public:
	wGoto( wAnimator &animator, string const &label )
	  : wCommand( animator ), Label( label )
	  {}
	wExecutionContext execute( double priority );
    };

    class wGosub : public wCommand
    {
      protected:
	string Label;

      public:
	wGosub( wAnimator &animator, string const &label )
	  : wCommand( animator ), Label( label )
	  {}
	wExecutionContext execute( double priority );
    };

    class wReturn : public wCommand
    {
      protected:
	bool KillDelay;
      public:
	wReturn( wAnimator &animator, bool killdelay )
	  : wCommand( animator ), KillDelay( killdelay )
	  {}
	wExecutionContext execute( double priority );
    };

    class wDelay : public wCommand
    {
      protected:
	double Seconds;

      public:
	wDelay( wAnimator &animator, double seconds )
	  : wCommand( animator ), Seconds( seconds )
	  {}
	wExecutionContext execute( double priority );
    };

    class wFrame : public wCommand
    {
      protected:
	string Name;

      public:
	wFrame( wAnimator &animator, string const &name )
	  : wCommand( animator ), Name( name )
	  {}
	wExecutionContext execute( double priority );
    };

    class wStop : public wCommand
    {
      public:
	wStop( wAnimator &animator )
	  : wCommand( animator )
	  {}
	wExecutionContext execute( double priority );
    };

    class wSignal : public wCommand
    {
      protected:
	string Name;

      public:
	wSignal( wAnimator &animator, string const &name )
	  : wCommand( animator ), Name( name )
	  {}
	wExecutionContext execute( double priority );
    };

    friend class wIf;
    friend class wOnReturn;
    friend class wGoto;
    friend class wGosub;
    friend class wReturn;
    friend class wDelay;
    friend class wFrame;
    friend class wStop;
    friend class wSignal;

  public:
    wAnimator( wImageManager &imgmgr, istream &script );
    ~wAnimator();
    void change( string const &parameter, string const &value );
    void step( double seconds );
    wImage *getCurrentFrame();
    bool isCurrentFrameChanged();
    TSize countPendingSignals();
    string getSignal();

  protected:
    void setExecutionContext( wExecutionContext const &ctx );
    void parse( xml_file::tag *root );
    void parseImages( xml_file::tag *root );
    void parseRegexes( xml_file::tag *root );
    wCommand *parseScript( xml_file::tag *root );
    void pushContext();
    string replaceParameters( const string &victim );
};




#endif
