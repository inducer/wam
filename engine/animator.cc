// ----------------------------------------------------------------------------
//  Description      : WAM animation script language
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_polygon.hh>
#include <ixlib_numconv.hh>
#include "utility/exgame.hh"
#include "utility/debug.hh"
#include "engine/animator.hh"
#include "engine/resource.hh"




/*
An animation file consists of the following sections:

<animation>
  <images>
    <image name="identifier" src="file.png" 
      [scale_x="xxx"] [scale_y="yyy"] [rotate="rrr"]/>
    <dynimage name="identifier" src="file.png" 
      [scale_x="xxx"] [scale_y="yyy"] [rotate="rrr"]/>
    The difference between image and dynimage is that for dynimage,
    the scale/rotate parameters may contain $(parameter) constructs.
  </images>

  <regexes>
    <regex original="aaa" replacement="bbb">
    All occurrences of aaa within the current image identifier
    are replaced by bbb. Backreferences are allowed.
    Regexes are executed in order.
  </regexes>

  <script>
    see below...
  </script>
</animation>

The script language knows the following commands:
 
-----------------------------------------------------------------------------
<if parameter="..." value="...">
  commands
  </if>
<else>
  commands
</else>
-----------------------------------------------------------------------------
<on parameter="..." [old="..."] [new="..."] [priority="p"] [will_return]>
  commands
</on>
<on return>
  commands
</on>
 
if these conditions are met and currently executing code has 
priority <= p (default 0), execute commands.
will_return puts return information on the stack for use with the
<return/> statement.
-----------------------------------------------------------------------------
<priority value="..."/>
 
set priority of currently executed code
-----------------------------------------------------------------------------
<label name="..."/>
<goto target="..."/>
<gosub target="..."/>
<return [kill_delay]/>

kill_delay means the current delay is not finished, if any.
-----------------------------------------------------------------------------
<delay time="..."/>
 
delay for ... milliseconds
-----------------------------------------------------------------------------
<frame name="..."/>
 
set the current frame to this image name. name may contain $(parameter) 
constructs.
-------------------------------------------------------------------------------
<stop/>

halts program execution
-----------------------------------------------------------------------------
<signal name="..."/>
 
signal the controlling program with name. name may contain $(parameter) 
constructs.
*/



// wAnimator::wExecutionContext -----------------------------------------------
void wAnimator::wExecutionContext::step( double seconds )
{
  while ( seconds >= RemainingDelay && InstructionPointer )
  {
    seconds -= RemainingDelay;
    *this = InstructionPointer->execute( CurrentPriority );
  }
  RemainingDelay -= seconds;
  if ( RemainingDelay < 0 )
    RemainingDelay = 0;
}




// wAnimator::wCommand --------------------------------------------------------
void wAnimator::wCommand::setNextCommand( wCommand *cmd )
{
  NextCommand = cmd;
}




wAnimator::wCommand *wAnimator::wCommand::getNextCommand()
{
  return NextCommand;
}




wAnimator::wCommand *wAnimator::wCommand::getLastCommand()
{
  wCommand * current = this, *tug;
  while ( current )
  {
    tug = current;
    current = current->getNextCommand();
  }
  return tug;
}




// wAnimator::wIf -------------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wIf::execute( double priority )
{
  wCommand *result;
  if ( Animator.Parameters[ Parameter ] == Value )
    result = IfPart;
  else
    result = ElsePart;

  if ( !result )
    result = getNextCommand();
  return wExecutionContext( result, priority, 0 );
}




void wAnimator::wIf::setNextCommand( wCommand *cmd )
{
  wCommand::setNextCommand( cmd );
  if ( IfPart )
    IfPart->getLastCommand()->setNextCommand( cmd );
  if ( ElsePart )
    ElsePart->getLastCommand()->setNextCommand( cmd );
}




// wAnimator::wOnReturn -------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wOnReturn::execute( double priority )
{
  Animator.Stack.push_back( wExecutionContext( Handler, priority, 0 ) );
  return wExecutionContext( getNextCommand(), priority, 0 );
}




// wAnimator::wPrioritySet ----------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wPrioritySet::execute( double priority )
{
  return wExecutionContext( getNextCommand(), Priority, 0 );
}




// wAnimator::wGoto -----------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wGoto::execute( double priority )
{
  if ( Animator.Labels.count( Label ) == 0 )
    EXGAME_THROWINFO( ECGAME_ANISCRIPT, ( "unknown label:" + Label ).c_str() );
  return wExecutionContext( Animator.Labels[ Label ], priority, 0 );
}




// wAnimator::wGosub ----------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wGosub::execute( double priority )
{
  if ( Animator.Labels.count( Label ) == 0 )
    EXGAME_THROWINFO( ECGAME_ANISCRIPT, ( "unknown label:" + Label ).c_str() )
    Animator.pushContext();
  return wExecutionContext( Animator.Labels[ Label ], priority, 0 );
}




// wAnimator::wReturn ---------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wReturn::execute( double priority )
{
  if ( Animator.Stack.size() )
  {
    wExecutionContext result = Animator.Stack.back();
    if ( KillDelay )
      result.RemainingDelay = 0;
    Animator.Stack.pop_back();
    return result;
  }
  else
    EXGAME_THROWINFO( ECGAME_ANISCRIPT, "return on empty stack" );
}




// wAnimator::wDelay ----------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wDelay::execute( double priority )
{
  return wExecutionContext( getNextCommand(), priority, Seconds );
}




// wAnimator::wFrame ----------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wFrame::execute( double priority )
{
  string result = Animator.replaceParameters( Name );

  FOREACH_CONST( first, Animator.Regexes, wAnimator::wRegexList )
  {
    regex_string rx( first->first );
    result = rx.replaceAll( result, first->second );
  }

  if ( Animator.Images.count( result ) == 0 )
    EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "image not in animator:" + result ).c_str() );

  Animator.CurrentFrame = &Animator.Images[ result ];
  Animator.CurrentFrameChanged = true;

  wamPrintDebugLevel( "animator: setting image '" + result + "' for '" + Name + "'", WAM_DEBUG_PERIODIC );
  return wExecutionContext( getNextCommand(), priority, 0 );
}




// wAnimator::wStop -----------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wStop::execute( double priority )
{
  return wExecutionContext( NULL, priority, 0 );
}




// wAnimator::wSignal ---------------------------------------------------------
wAnimator::wExecutionContext
wAnimator::wSignal::execute( double priority )
{
  string result = Animator.replaceParameters( Name );

  Animator.PendingSignals.push( result );

  wamPrintDebugLevel( "animator: signalling '" + result + "' for '" + Name + "'", WAM_DEBUG_PERIODIC );
  return wExecutionContext( getNextCommand(), priority, 0 );
}




// wAnimator ------------------------------------------------------------------
wAnimator::wAnimator( wImageManager &imgmgr, istream &script )
    : ImageManager( imgmgr ), 
      CurrentFrame( NULL ), CurrentFrameChanged( false )
{
  xml_file xml;
  xml.read( script );
  parse( xml.getRootTag() );
}




wAnimator::~wAnimator()
{
  FOREACH( first, DisposeList, wCommandList )
  delete * first;
}




void wAnimator::change( string const &parameter, string const &value )
{
  bool executed_handler = false;
  FOREACH( first, EventList, wEventList )
    if ( first->Parameter == parameter
	&& ( !first->OldRequired || first->OldValue == Parameters[ parameter ] )
	&& ( !first->NewRequired || first->NewValue == value )
	&& ( ExecutionContext.CurrentPriority <= first->Priority ) )
    {
      wamPrintDebugLevel( "animator: executing handler for '" + parameter + "' change to '" + value + 
	    "', current prio " + float2dec( ExecutionContext.CurrentPriority )+
	    " new prio " + float2dec( first->Priority ), 
	    WAM_DEBUG_PERIODIC );
      executed_handler = true;

      if ( first->WillReturn )
      {
	Stack.push_back( ExecutionContext );
      }
      else
	Stack.clear();

      setExecutionContext( wExecutionContext( first->Handler, first->Priority, 0 ) );
      break;
    }
  if ( !executed_handler )
    wamPrintDebugLevel( "animator: not executing handler for '" + parameter + "' change to '" + value + "'", WAM_DEBUG_ALL );
  Parameters[ parameter ] = value;
}




void wAnimator::step( double seconds )
{
  ExecutionContext.step( seconds );
}




wImage *wAnimator::getCurrentFrame()
{
  CurrentFrameChanged = false;

  if ( !CurrentFrame ) 
    return NULL;

  wFrameInfo &fi = *CurrentFrame;
  if ( fi.Dynamic )
  {
    float scale_x = evalFloat( replaceParameters( fi.ScaleXExpression ) );
    float scale_y = evalFloat( replaceParameters( fi.ScaleYExpression ) );
    float rotate = evalFloat( replaceParameters( fi.RotateExpression ) );

    affine_transformation tx;
    tx.identity();
    tx.scale( scale_x, scale_y );
    tx.rotate( rotate / 180 * M_PI );
    LastDynamicImage.transformFrom( *fi.Image, tx );
    return &LastDynamicImage;
  }
  else
    return fi.Image.get();
}




bool wAnimator::isCurrentFrameChanged()
{
  return !CurrentFrame || CurrentFrame->Dynamic || CurrentFrameChanged;
}




TSize wAnimator::countPendingSignals()
{
  return PendingSignals.size();
}




string wAnimator::getSignal()
{
  string result = PendingSignals.front();
  PendingSignals.pop();
  return result;
}




// internal -------------------------------------------------------------------
void wAnimator::setExecutionContext( wExecutionContext const &ctx )
{
  ExecutionContext = ctx;
}




void wAnimator::parse( xml_file::tag *root )
{
  if ( root->getName() != "animation" )
    EXGAME_THROWINFO( ECGAME_ANISCRIPT, "'animation' expected" )

    FOREACH_CONST( first, *root, xml_file::tag )
  {
    if ( ( *first ) ->getName() == "images" )
      parseImages( *first );
    else if ( ( *first ) ->getName() == "regexes" )
      parseRegexes( *first );
    else if ( ( *first ) ->getName() == "script" )
      ExecutionContext.InstructionPointer = parseScript( *first );
    else
      EXGAME_THROWINFO( ECGAME_ANISCRIPT, "'images','regexes' or 'script' expected" )
    }
}




void wAnimator::parseImages( xml_file::tag *root )
{
  FOREACH_CONST( first, *root, xml_file::tag )
  {
    if ( ( *first )->getName() == "image" )
    {
      xml_file::tag &img = **first;

      double stretchx = 1, stretchy = 1, rotate = 0;
      if ( img.Attributes.count( "scale_x" ) )
	stretchx = evalFloat( img.Attributes[ "scale_x" ] );
      if ( img.Attributes.count( "scale_y" ) )
	stretchy = evalFloat( img.Attributes[ "scale_y" ] );
      if ( img.Attributes.count( "rotate" ) )
	rotate = evalFloat( img.Attributes[ "rotate" ] ) / 180 * Pi;

      wFrameInfo fi;
      fi.Image = wImageHolder( ImageManager, wImageDescriptor( img.Attributes[ "src" ], stretchx, stretchy, rotate ) );
      fi.Dynamic = false;
      Images[ img.Attributes[ "name" ] ] = fi;
    }
    else if ( ( *first )->getName() == "dynimage" )
    {
      xml_file::tag &img = **first;

      wFrameInfo fi;
      fi.ScaleXExpression = "1";
      fi.ScaleYExpression = "1";
      fi.RotateExpression = "0";

      if ( img.Attributes.count( "scale_x" ) )
	fi.ScaleXExpression = img.Attributes[ "scale_x" ];
      if ( img.Attributes.count( "scale_y" ) )
	fi.ScaleYExpression = img.Attributes[ "scale_y" ];
      if ( img.Attributes.count( "rotate" ) )
	fi.RotateExpression = img.Attributes[ "rotate" ];

      fi.Image = wImageHolder( ImageManager, wImageDescriptor( img.Attributes[ "src" ] ) ); 
      fi.Dynamic = true;

      Images[ img.Attributes[ "name" ] ] = fi;
    }
    else
      EXGAME_THROWINFO( ECGAME_ANISCRIPT, "'image' or 'dynimage' expected" );
  }
}




void wAnimator::parseRegexes( xml_file::tag *root )
{
  FOREACH_CONST( first, *root, xml_file::tag )
  {
    if ( ( *first ) ->getName() != "regex" )
      EXGAME_THROWINFO( ECGAME_ANISCRIPT, "'regex' expected" )

      wRegex rx( ( *first ) ->Attributes[ "original" ], ( *first ) ->Attributes[ "replacement" ] );
    Regexes.push_back( rx );
  }
}




wAnimator::wCommand *
wAnimator::parseScript( xml_file::tag *root )
{
  wCommand * first_cmd = NULL, *last_cmd, *current_cmd = NULL;

  vector<string> pending_labels;

  FOREACH_CONST( first, *root, xml_file::tag )
  {
    current_cmd = NULL;
    // if ---------------------------------------------------------------------
    if ( ( *first ) ->getName() == "if" )
    {
      string parameter = ( *first ) ->Attributes[ "paramter" ];
      string value = ( *first ) ->Attributes[ "value" ];
      wCommand *ifpart = parseScript( *first );
      wCommand *elsepart = NULL;
      if ( first != last && first[ 1 ] ->getName() == "else" )
      {
        first++;
        elsepart = parseScript( *first );
      }

      current_cmd = new wIf( *this, ifpart, elsepart, parameter, value );
    }
    // on ---------------------------------------------------------------------
    else if ( ( *first ) ->getName() == "on" )
    {
      if ( ( *first ) ->Attributes.count( "return" ) )
      {
        current_cmd = new wOnReturn( *this, parseScript( *first ) );
      }
      else
      {
        wEvent ev;
        ev.Parameter = ( *first ) ->Attributes[ "parameter" ];
        ev.OldRequired = ( *first ) ->Attributes.count( "old" ) != 0;
        ev.NewRequired = ( *first ) ->Attributes.count( "new" ) != 0;
        ev.OldValue = ( *first ) ->Attributes[ "old" ];
        ev.NewValue = ( *first ) ->Attributes[ "new" ];
	if ( ( *first )->Attributes.count( "priority" ) != 0 )
	  ev.Priority = evalFloat( ( *first )->Attributes[ "priority" ] );
	else
	  ev.Priority = 0;
        ev.WillReturn = ( *first ) ->Attributes.count( "will_return" ) != 0;
        ev.Handler = parseScript( *first );
        EventList.push_back( ev );
      }
    }
    // priority ---------------------------------------------------------------
    else if ( ( *first ) ->getName() == "priority" )
    {
      current_cmd = new wPrioritySet( *this, evalFloat( ( *first ) ->Attributes[ "value" ] ) );
    }
    // label ------------------------------------------------------------------
    else if ( ( *first ) ->getName() == "label" )
    {
      pending_labels.push_back( ( *first ) ->Attributes[ "name" ] );
    }
    // goto -------------------------------------------------------------------
    else if ( ( *first ) ->getName() == "goto" )
    {
      current_cmd = new wGoto( *this, ( *first ) ->Attributes[ "target" ] );
    }
    // gosub ------------------------------------------------------------------
    else if ( ( *first ) ->getName() == "gosub" )
    {
      current_cmd = new wGosub( *this, ( *first ) ->Attributes[ "target" ] );
    }
    // return -----------------------------------------------------------------
    else if ( ( *first ) ->getName() == "return" )
    {
      current_cmd = new wReturn( *this, ( *first ) ->Attributes.count( "kill_delay" ) != 0 );
    }
    // delay ------------------------------------------------------------------
    else if ( ( *first ) ->getName() == "delay" )
    {
      current_cmd = new wDelay( *this, evalFloat( ( *first ) ->Attributes[ "time" ] ) );
    }
    // frame ------------------------------------------------------------------
    else if ( ( *first ) ->getName() == "frame" )
    {
      current_cmd = new wFrame( *this, ( *first ) ->Attributes[ "name" ] );
    }
    // stop -------------------------------------------------------------------
    else if ( ( *first ) ->getName() == "stop" )
    {
      current_cmd = new wStop( *this );
    }
    // signal -----------------------------------------------------------------
    else if ( ( *first ) ->getName() == "signal" )
    {
      current_cmd = new wSignal( *this, ( *first ) ->Attributes[ "name" ] );
    }
    else
      EXGAME_THROWINFO( ECGAME_ANISCRIPT, ( "script command expected, " + ( *first ) ->getName() + " found" ).c_str() )

      if ( current_cmd )
      {
        DisposeList.push_back( current_cmd );
        if ( first_cmd == NULL )
          first_cmd = current_cmd;
        else
          last_cmd->setNextCommand( current_cmd );
        last_cmd = current_cmd;

        FOREACH_CONST( first, pending_labels, vector<string> )
        Labels[ *first ] = current_cmd;
        pending_labels.clear();
      }
  }

  return first_cmd;
}




void wAnimator::pushContext()
{
  if ( ExecutionContext.InstructionPointer )
    Stack.push_back( ExecutionContext );
}




string wAnimator::replaceParameters( const string &victim )
{
  string result = victim;

  regex_string rx( "\\$\\((.*?)\\)" );
  TIndex matchindex = 0;
  while ( rx.match( result, matchindex ) )
  {
    matchindex = rx.getMatchIndex();
    string match = rx.getMatch();

    match.erase( 0, 2 );
    match.erase( match.size() - 1 );
    result.erase( matchindex, rx.getMatchLength() );
    result.insert( matchindex, Parameters[ match ] );
  }

  return result;
}

