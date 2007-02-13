// ----------------------------------------------------------------------------
//  Description      : WAM menu interface
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------




#include <algorithm>
#include "engine/message.hh"
#include "engine/gamebase.hh"
#include "engine/drawable.hh"
#include "engine/resource.hh"
#include "engine/console.hh"
#include "engine/input.hh"
#include "engine/commandmanager.hh"
#include "gamejects/menu.hh"




// constants ------------------------------------------------------------------
#define GJFEATURESET_MENU_MANAGER "Menu" GJFEATURE_MANAGER
#define GJFEATURESET_MENU_BUTTON GJFEATURE_DISPLAYED "Menu" "Butn"
#define GJFEATURESET_MENU_TEXTINPUT GJFEATURE_DISPLAYED "Menu" "TxtI"
#define GJFEATURESET_MENU_TEXT      GJFEATURE_DISPLAYED "Menu" "Text"
#define GJFEATURESET_MENU_IMAGE  GJFEATURE_DISPLAYED "Menu" "Imag"
#define GJFEATURESET_MENU_SPACER GJFEATURE_DISPLAYED "Menu" "Spac"

#define WAM_MSG_MENU_GET_HEIGHT  "menu_get_height"
#define WAM_MSG_MENU_SETUP  "menu_setup"
#define WAM_MSG_MENU_GAIN_FOCUS  "menu_gain_focus"
#define WAM_MSG_MENU_LOSE_FOCUS  "menu_lose_focus"
#define WAM_MSG_MENU_REQUEST_FOCUS "menu_request_focus"
#define WAM_MSG_MENU_ATTACH  "menu_attach"
#define WAM_MSG_MENU_DETACH  "menu_detach"
#define WAM_MSG_MENU_COMPLETE  "menu_complete"
#define WAM_MSG_MENU_END  "menu_end"
#define WAM_MSG_MENU_GET_INPUT  "menu_get_input"
#define WAM_MSG_MENU_GET_POSITION "menu_get_position"

#define WAM_MENU_DARK_FONT  "menu_dark.fnt"
#define WAM_MENU_LIGHT_FONT  "menu_light.fnt"




namespace
{
// manager -> widget
TMessageId MsgGetHeight;   // ret: stringified height
TMessageId MsgSetup;   // par1: stringified pos_y,stringified order
TMessageId MsgGainFocus;  // return: first == "ok": accepted
TMessageId MsgLoseFocus;

// widget -> manager
TMessageId MsgRequestFocus; // par1: stringified id
TMessageId MsgAttach;  // par1: stringified id
TMessageId MsgDetach;  // par1: stringified id

// external -> manager
TMessageId MsgMenuComplete;

// external -> manager, manager->widget
TMessageId MsgMenuEnd;

// external -> widget
TMessageId MsgGetInput;  // return: input
TMessageId MsgGetPosition;  // return: x,y
}




// wMenuManager ---------------------------------------------------------------
class wMenuManager : public wMessageReceiver, wKeyboardInputListener
{
    typedef vector<TId> wWidgetList;
    wWidgetList WidgetList;
    TId FocusedWidget;
    bool HaveFocusedWidget, Completed;

  public:
    wMenuManager( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MENU_MANAGER;
    }
    TPriority getDeleteOrder() const
    {
      return WAM_DELORDER_MANAGER;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getKeyboardInputPriority() const
    {
      return WAM_INPUTPRIO_MENU;
    }
    bool processKeyEvent( wKeyboardEvent &event );
    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );

    bool setFocus( TId id );
    void layout();
};




// wMenuWidget ----------------------------------------------------------------
class wMenuWidget : public wMessageReceiver
{
    TId MenuManager;
    bool Focused;

  public:
    wMenuWidget( wGame &g, TId menu_mgr );

    void registerMe();
    void unregisterMe();

    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );

    virtual TSize height() const = 0;
    virtual void setup( TCoordinate y, TIndex order ) = 0;
    virtual bool gainFocus();
    virtual void loseFocus();
    virtual void end();

    void requestFocus();

    bool isFocused() const
    {
      return Focused;
    }
};




// wMenuButton ----------------------------------------------------------------
class wMenuButton : public wMenuWidget, public wMouseInputListener,
      public wKeyboardInputListener, public wDrawable, wTickReceiver
{
    wFontHolder DarkFont, LightFont;
    string Caption;
    ref<value> Function;
    TCoordinate YPosition;

    bool SetupReceived;
    TIndex Order;
    float XSpeed, XOffset;

  public:
    wMenuButton( wGame &g, TId menu_mgr, string const &caption, ref<value> function );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MENU_BUTTON;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getKeyboardInputPriority() const
    {
      return WAM_INPUTPRIO_MENU;
    }
    bool processKeyEvent( wKeyboardEvent &event );

    TPriority getMouseInputPriority() const
    {
      return WAM_INPUTPRIO_MENU;
    }
    bool processMouseEvent( wMouseEvent &event );

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_MENU;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    TSize height() const;
    void setup( TCoordinate y, TIndex order );
    bool gainFocus();
    void loseFocus();

    void tick( float seconds );
};




// wMenuTextInput -------------------------------------------------------------
class wMenuTextInput : public wMenuWidget, public wMouseInputListener,
      public wKeyboardInputListener, public wDrawable, wTickReceiver
{
    wFontHolder DarkFont, LightFont;
    string Caption;
    string Contents;
    TCoordinate YPosition;

    bool SetupReceived;
    TIndex Order;
    float XSpeed, XOffset;

  public:
    wMenuTextInput( wGame &g, TId menu_mgr, string const &caption, string const &contents );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MENU_TEXTINPUT;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getKeyboardInputPriority() const
    {
      return WAM_INPUTPRIO_MENU;
    }
    bool processKeyEvent( wKeyboardEvent &event );

    TPriority getMouseInputPriority() const
    {
      return WAM_INPUTPRIO_MENU;
    }
    bool processMouseEvent( wMouseEvent &event );

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_MENU;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    TSize height() const;
    void setup( TCoordinate y, TIndex order );
    bool gainFocus();
    void loseFocus();

    void tick( float seconds );

    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );
};




// wMenuText ------------------------------------------------------------------
class wMenuText : public wMenuWidget, public wDrawable
{
    wFontHolder Font;
    string Caption;
    TCoordinate YPosition;

  public:
    wMenuText( wGame &g, TId menu_mgr, string const &caption );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MENU_TEXT;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_MENU;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    TSize height() const;
    void setup( TCoordinate y, TIndex order );
    bool gainFocus()
    {
      return false;
    }
};




// wMenuImage -----------------------------------------------------------------
class wMenuImage : public wMenuWidget, public wDrawable
{
    wImageHolder Image;
    TCoordinate YPosition;

  public:
    wMenuImage( wGame &g, TId menu_mgr, wImageDescriptor const &desc );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MENU_IMAGE;
    }

    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_MENU;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    TSize height() const;
    void setup( TCoordinate y, TIndex order );
    bool gainFocus()
    {
      return false;
    }

    ref<wMessageReturn>
    receiveMessage( TMessageId id, string const &par1, void *par2 );
};




// wMenuSpacer ----------------------------------------------------------------
class wMenuSpacer : public wMenuWidget
{
    TSize Height;

  public:
    wMenuSpacer( wGame &g, TId menu_mgr, TSize height )
        : wGameject( g ), wMenuWidget( g, menu_mgr ), Height( height )
    {}

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_MENU_SPACER;
    }

    TSize height() const
    {
      return Height;
    }
    void setup( TCoordinate y, TIndex order )
    {}
    bool gainFocus()
    {
      return false;
    }
};




// wMenuManager ---------------------------------------------------------------
wMenuManager::wMenuManager( wGame &g )
    : wGameject( g ), wMessageReceiver( g ), wKeyboardInputListener( g ), HaveFocusedWidget( false ),
    Completed( false )
{}




void wMenuManager::registerMe()
{
  wMessageReceiver::registerMe();
  wKeyboardInputListener::registerMe();
}




void wMenuManager::startUp()
{
  wMessageReceiver::startUp();
  wKeyboardInputListener::startUp();
}




void wMenuManager::prepareToDie()
{
  wMessageReceiver::prepareToDie();
  wKeyboardInputListener::prepareToDie();
}




void wMenuManager::unregisterMe()
{
  wMessageReceiver::unregisterMe();
  wKeyboardInputListener::unregisterMe();
}




bool wMenuManager::processKeyEvent( wKeyboardEvent &event )
{
  if ( event.getType() == wKeyboardEvent::KEY_PRESS )
  {
    if ( event.getKey() == SDLK_UP )
    {
      wWidgetList::const_iterator first = WidgetList.begin(), last = WidgetList.end();
      if ( HaveFocusedWidget )
      {
        while ( first != last )
        {
          if ( *first == FocusedWidget )
            break;
          first++;
        }
        while ( first != WidgetList.begin() )
        {
          first--;
          if ( setFocus( *first ) )
            break;
        }
      }
      else
      {
        FOREACH_CONST( first, WidgetList, wWidgetList )
        if ( setFocus( *first ) )
          break;
      }
      return true;
    }
    if ( event.getKey() == SDLK_DOWN )
    {
      bool found = !HaveFocusedWidget;
      FOREACH_CONST( first, WidgetList, wWidgetList )
      {
        if ( found )
        {
          if ( setFocus( *first ) )
            break;
        }
        if ( HaveFocusedWidget && *first == FocusedWidget )
          found = true;
      }
      return true;
    }
  }
  return false;
}




ref<wMenuManager::wMessageReturn>
wMenuManager::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == MsgRequestFocus )
  {
    setFocus( evalUnsigned( par1 ) );
  }
  else if ( id == MsgAttach )
  {
    WidgetList.push_back( evalUnsigned( par1 ) );
    if ( Completed )
      layout();
  }
  else if ( id == MsgDetach )
  {
    TId id = evalUnsigned( par1 );
    if ( id == FocusedWidget )
      HaveFocusedWidget = false;
    wWidgetList::iterator it = find( WidgetList.begin(), WidgetList.end(), id );

    if ( it == WidgetList.end() )
      EXGAME_THROWINFO( ECGAME_NOTFOUND, ( "menu widget having id " + par1 ).c_str() )

      WidgetList.erase( it );
    if ( Completed )
      layout();
  }
  else if ( id == MsgMenuComplete )
  {
    layout();

    FOREACH_CONST( first, WidgetList, wWidgetList )
    if ( setFocus( *first ) )
      break;

    Completed = true;
  }
  else if ( id == MsgMenuEnd )
  {
    FOREACH_CONST( first, WidgetList, wWidgetList )
    Game.getMessageManager().sendTo( *first, MsgMenuEnd );
    Game.getInstanceManager().requestDestruction( this );
  }
  return NULL;
}




bool wMenuManager::setFocus( TId id )
{
  if ( Game.getMessageManager().sendTo( id, MsgGainFocus ) ->first != "ok" )
    return false;

  if ( HaveFocusedWidget )
    Game.getMessageManager().sendTo( FocusedWidget, MsgLoseFocus );
  FocusedWidget = id;
  HaveFocusedWidget = true;
  return true;
}




void wMenuManager::layout()
{
  TSize totalheight = 0;
  FOREACH_CONST( first, WidgetList, wWidgetList )
  totalheight += evalUnsigned( Game.getMessageManager().sendTo( *first, MsgGetHeight ) ->first );

  TCoordinate y = ( Game.getDrawableManager().getDisplay().height() - totalheight ) / 2;

  TIndex order = 0;
  FOREACH_CONST( first, WidgetList, wWidgetList )
  {
    Game.getMessageManager().sendTo( *first, MsgSetup, unsigned2dec( y ) + "," + unsigned2dec( order ) );
    y += evalUnsigned( Game.getMessageManager().sendTo( *first, MsgGetHeight ) ->first );
    order++;
  }
}




// wMenuWidget ----------------------------------------------------------------
wMenuWidget::wMenuWidget( wGame &g, TId menu_mgr )
    : wGameject( g ), wMessageReceiver( g ), MenuManager( menu_mgr ), Focused( false )
{}




void wMenuWidget::registerMe()
{
  wMessageReceiver::registerMe();

  Game.getMessageManager().sendTo( MenuManager, MsgAttach, unsigned2dec( id() ) );
}




void wMenuWidget::unregisterMe()
{
  Game.getMessageManager().sendTo( MenuManager, MsgDetach, unsigned2dec( id() ) );

  wMessageReceiver::unregisterMe();
}




ref<wMenuWidget::wMessageReturn>
wMenuWidget::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == MsgGetHeight )
    return new wMessageReturn( unsigned2dec( height() ), NULL );
  else if ( id == MsgSetup )
  {
    regex_string re( "([0-9]*)\\,([0-9]*)" );
    re.match( par1 );
    setup( evalUnsigned( re.getBackref( 0 ) ), evalUnsigned( re.getBackref( 1 ) ) );
  }
  else if ( id == MsgGainFocus )
  {
    if ( gainFocus() )
      return new wMessageReturn( "ok", NULL );
    else
      return new wMessageReturn( "", NULL );
  }
  else if ( id == MsgLoseFocus )
    loseFocus();
  else if ( id == MsgMenuEnd )
    end();
  return NULL;
}




bool wMenuWidget::gainFocus()
{
  Focused = true;
  return true;
}




void wMenuWidget::loseFocus()
{
  Focused = false;
}




void wMenuWidget::end()
{
  Game.getInstanceManager().requestDestruction( this );
}




void wMenuWidget::requestFocus()
{
  Game.getMessageManager().sendTo( MenuManager, MsgRequestFocus, unsigned2dec( id() ) );
}




// wMenuButton ----------------------------------------------------------------
wMenuButton::wMenuButton( wGame &g, TId menu_mgr, string const &caption, ref<value> function )
    : wGameject( g ), wMenuWidget( g, menu_mgr ), wMouseInputListener( g ), wKeyboardInputListener( g ),
    wDrawable( g ), wTickReceiver( g ),
    DarkFont( Game.getFontManager(), WAM_MENU_DARK_FONT ), LightFont(
      Game.getFontManager(), WAM_MENU_LIGHT_FONT ), Caption( caption ), Function( function ),
    YPosition( 0 ), SetupReceived( false )
{}



void wMenuButton::registerMe()
{
  wMenuWidget::registerMe();
  wMouseInputListener::registerMe();
  wKeyboardInputListener::registerMe();
  wDrawable::registerMe();
  wTickReceiver::registerMe();
}




void wMenuButton::startUp()
{
  wMenuWidget::startUp();
  wMouseInputListener::startUp();
  wKeyboardInputListener::startUp();
  wDrawable::startUp();
  wTickReceiver::startUp();
}




void wMenuButton::prepareToDie()
{
  wMenuWidget::prepareToDie();
  wMouseInputListener::prepareToDie();
  wKeyboardInputListener::prepareToDie();
  wDrawable::prepareToDie();
  wTickReceiver::prepareToDie();
}




void wMenuButton::unregisterMe()
{
  wMenuWidget::unregisterMe();
  wMouseInputListener::unregisterMe();
  wKeyboardInputListener::unregisterMe();
  wDrawable::unregisterMe();
  wTickReceiver::unregisterMe();
}




bool wMenuButton::processKeyEvent( wKeyboardEvent &event )
{
  if ( isFocused() && event.getType() == wKeyboardEvent::KEY_PRESS && event.getKey() == SDLK_RETURN )
  {
    Function->call( value::parameter_list() );
    return true;
  }
  return false;
}




bool wMenuButton::processMouseEvent( wMouseEvent &event )
{
  wMouseEvent::wMouseVector pos = event.getPosition();
  if ( pos[ 1 ] >= YPosition && pos[ 1 ] <= int( YPosition + height() ) )
  {
    if ( !isFocused() )
      requestFocus();

    if ( event.getType() == wMouseEvent::BUTTON_PRESS )
      Function->call( value::parameter_list() );
    return true;
  }
  return false;
}




wDisplayExtent wMenuButton::getDisplayExtent()
{
  return wDisplayExtent( 0, YPosition, Game.getDrawableManager().getDisplay().width(), YPosition + height() );
}




void wMenuButton::draw( drawable &dpy )
{
  font * fnt;
  if ( isFocused() )
    fnt = LightFont.get();
  else
    fnt = DarkFont.get();

  wDisplayExtent ext = fnt->extent( Caption );
  TCoordinate xpos = ( dpy.width() - ext.width() ) / 2 +
                     ( int ) ( dpy.width() * ( XOffset * XOffset ) );
  fnt->print( dpy, xpos, YPosition, Caption );
}




TSize wMenuButton::height() const
{
  return DarkFont->BottomLine - DarkFont->TopLine;
}




void wMenuButton::setup( TCoordinate y, TIndex order )
{
  YPosition = y;
  Order = order;
  SetupReceived = true;
  XSpeed = -1;
  XOffset = 1 + ( float ) order / 16;
  if ( !isVisible() )
    show();
}




bool wMenuButton::gainFocus()
{
  wMenuWidget::gainFocus();
  requestRedraw();
  return true;
}




void wMenuButton::loseFocus()
{
  wMenuWidget::loseFocus();
  requestRedraw();
}




void wMenuButton::tick( float seconds )
{
  if ( SetupReceived && XSpeed != 0 )
  {
    XOffset += XSpeed * seconds;
    if ( XOffset <= 0 )
    {
      XSpeed = 0;
      XOffset = 0;
    }
    requestRedraw();
  }
}




// wMenuTextInput ----------------------------------------------------------------
wMenuTextInput::wMenuTextInput( wGame &g, TId menu_mgr, string const &caption, string const &contents )
    : wGameject( g ), wMenuWidget( g, menu_mgr ), wMouseInputListener( g ), wKeyboardInputListener( g ),
    wDrawable( g ), wTickReceiver( g ),
    DarkFont( Game.getFontManager(), WAM_MENU_DARK_FONT ), LightFont(
      Game.getFontManager(), WAM_MENU_LIGHT_FONT ), Caption( caption ), Contents( contents ),
    YPosition( 0 ), SetupReceived( false )
{}



void wMenuTextInput::registerMe()
{
  wMenuWidget::registerMe();
  wMouseInputListener::registerMe();
  wKeyboardInputListener::registerMe();
  wDrawable::registerMe();
  wTickReceiver::registerMe();
}




void wMenuTextInput::startUp()
{
  wMenuWidget::startUp();
  wMouseInputListener::startUp();
  wKeyboardInputListener::startUp();
  wDrawable::startUp();
  wTickReceiver::startUp();
}




void wMenuTextInput::prepareToDie()
{
  wMenuWidget::prepareToDie();
  wMouseInputListener::prepareToDie();
  wKeyboardInputListener::prepareToDie();
  wDrawable::prepareToDie();
  wTickReceiver::prepareToDie();
}




void wMenuTextInput::unregisterMe()
{
  wMenuWidget::unregisterMe();
  wMouseInputListener::unregisterMe();
  wKeyboardInputListener::unregisterMe();
  wDrawable::unregisterMe();
  wTickReceiver::unregisterMe();
}




bool wMenuTextInput::processKeyEvent( wKeyboardEvent &event )
{
  if ( isFocused() && event.getType() == wKeyboardEvent::KEY_PRESS )
  {
    if ( event.getModifier() == 0 && event.getKey() == SDLK_BACKSPACE )
    {
      if ( Contents.size() )
      {
        Contents.erase( Contents.size() - 1, 1 );
        requestRedraw();
      }
      return true;
    }
    else if ( event.isPrintable() )
    {
      Contents += event.getASCII();
      requestRedraw();
      return true;
    }
  }
  return false;
}




bool wMenuTextInput::processMouseEvent( wMouseEvent &event )
{
  wMouseEvent::wMouseVector pos = event.getPosition();
  if ( pos[ 1 ] >= YPosition && pos[ 1 ] <= int( YPosition + height() ) )
  {
    if ( !isFocused() )
      requestFocus();
    return true;
  }
  return false;
}




wDisplayExtent wMenuTextInput::getDisplayExtent()
{
  return wDisplayExtent( 0, YPosition, Game.getDrawableManager().getDisplay().width(), YPosition + height() );
}




void wMenuTextInput::draw( drawable &dpy )
{
  string contents = Contents;
  font *fnt;
  if ( isFocused() )
  {
    fnt = LightFont.get();
    contents += "_";
  }
  else
    fnt = DarkFont.get();

  TSize caption_width = fnt->extent( Caption ).width();
  TSize contents_width = LightFont->extent( contents ).width();

  TCoordinate xpos = ( dpy.width() - caption_width - contents_width ) / 2 +
                     ( int ) ( dpy.width() * ( XOffset * XOffset ) );
  fnt->print( dpy, xpos, YPosition, Caption );
  fnt->print( dpy, xpos + caption_width, YPosition, contents );
}




TSize wMenuTextInput::height() const
{
  return DarkFont->BottomLine - DarkFont->TopLine;
}




void wMenuTextInput::setup( TCoordinate y, TIndex order )
{
  YPosition = y;
  Order = order;
  SetupReceived = true;
  XSpeed = -1;
  XOffset = 1 + ( float ) order / 16;
  if ( !isVisible() )
    show();
}




bool wMenuTextInput::gainFocus()
{
  wMenuWidget::gainFocus();
  requestRedraw();
  return true;
}




void wMenuTextInput::loseFocus()
{
  wMenuWidget::loseFocus();
  requestRedraw();
}




void wMenuTextInput::tick( float seconds )
{
  if ( SetupReceived && XSpeed != 0 )
  {
    XOffset += XSpeed * seconds;
    if ( XOffset <= 0 )
    {
      XSpeed = 0;
      XOffset = 0;
    }
    requestRedraw();
  }
}




ref<wMenuTextInput::wMessageReturn>
wMenuTextInput::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == MsgGetInput )
    return new wMessageReturn( Contents, NULL );
  else
    return wMenuWidget::receiveMessage( id, par1, par2 );
}




// wMenuText ----------------------------------------------------------------
wMenuText::wMenuText( wGame &g, TId menu_mgr, string const &caption )
    : wGameject( g ), wMenuWidget( g, menu_mgr ), wDrawable( g ),
    Font( Game.getFontManager(), WAM_MENU_DARK_FONT ), Caption( caption ),
    YPosition( 0 )
{}



void wMenuText::registerMe()
{
  wMenuWidget::registerMe();
  wDrawable::registerMe();
}




void wMenuText::startUp()
{
  wMenuWidget::startUp();
  wDrawable::startUp();
}




void wMenuText::prepareToDie()
{
  wMenuWidget::prepareToDie();
  wDrawable::prepareToDie();
}




void wMenuText::unregisterMe()
{
  wMenuWidget::unregisterMe();
  wDrawable::unregisterMe();
}




wDisplayExtent wMenuText::getDisplayExtent()
{
  return wDisplayExtent( 0, YPosition, Game.getDrawableManager().getDisplay().width(), YPosition + height() );
}




void wMenuText::draw( drawable &dpy )
{
  TSize width = Font->extent( Caption ).width();

  TCoordinate xpos = ( dpy.width() - width ) / 2;
  Font->print( dpy, xpos, YPosition, Caption );
}




TSize wMenuText::height() const
{
  return Font->BottomLine - Font->TopLine;
}




void wMenuText::setup( TCoordinate y, TIndex order )
{
  YPosition = y;
  if ( !isVisible() )
    show();
}




// wMenuImage ----------------------------------------------------------------
wMenuImage::wMenuImage( wGame &g, TId menu_mgr, wImageDescriptor const &desc )
    : wGameject( g ), wMenuWidget( g, menu_mgr ), wDrawable( g ),
    Image( Game.getImageManager(), desc ), YPosition( 0 )
{}



void wMenuImage::registerMe()
{
  wMenuWidget::registerMe();
  wDrawable::registerMe();
}




void wMenuImage::startUp()
{
  wMenuWidget::startUp();
  wDrawable::startUp();
}




void wMenuImage::prepareToDie()
{
  wMenuWidget::prepareToDie();
  wDrawable::prepareToDie();
}




void wMenuImage::unregisterMe()
{
  wMenuWidget::unregisterMe();
  wDrawable::unregisterMe();
}




wDisplayExtent wMenuImage::getDisplayExtent()
{
  return wDisplayExtent( 0, YPosition, Game.getDrawableManager().getDisplay().width(), YPosition + height() );
}




void wMenuImage::draw( drawable &dpy )
{
  TCoordinate xpos = ( dpy.width() - Image->width() ) / 2;
  Image->blit( dpy, xpos, YPosition );
}




TSize wMenuImage::height() const
{
  return Image->height();
}




void wMenuImage::setup( TCoordinate y, TIndex order )
{
  YPosition = y;
  if ( !isVisible() )
    show();
}




ref<wMenuImage::wMessageReturn>
wMenuImage::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == MsgGetPosition )
  {
    TCoordinate xpos = ( Game.getDrawableManager().getDisplay().width() - Image->width() ) / 2;
    return new wMessageReturn(
             signed2dec( xpos ) + "," + signed2dec( YPosition ),
             NULL );
  }
  else
    return wMenuWidget::receiveMessage( id, par1, par2 );
}




// javascript interface -------------------------------------------------------
WAM_DECLARE_COMMAND( menu_createmanager, "Mcmg", "create menu manager", "" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_createmanager", 0, 0 );

  wGameject *gj = new wMenuManager( Game );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




WAM_DECLARE_COMMAND( menu_addbutton, "Mamb", "add menu button", "menu_mgr caption function" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_addmenubutton", 3, 3 );

  wGameject *gj = new wMenuButton( Game, parameters[ 0 ] ->toInt(),
                                   parameters[ 1 ] ->toString(), parameters[ 2 ] );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




WAM_DECLARE_COMMAND( menu_addinput, "Mami", "add menu input", "menu_mgr caption contents" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_addmenuinput", 3, 3 );

  wGameject *gj = new wMenuTextInput( Game, parameters[ 0 ] ->toInt(),
                                      parameters[ 1 ] ->toString(), parameters[ 2 ] ->toString() );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




WAM_DECLARE_COMMAND( menu_getinput, "Mgin", "get menu input", "input_widget" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_getinput", 1, 1 );

  return makeConstant(
           Game.getMessageManager().sendTo( parameters[ 0 ] ->toInt(), MsgGetInput ) ->first );
}




WAM_DECLARE_COMMAND( menu_addtext, "Mamt", "add menu text", "menu_mgr caption" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_addmenutext", 2, 2 );

  wGameject *gj = new wMenuText( Game, parameters[ 0 ] ->toInt(), parameters[ 1 ] ->toString() );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




WAM_DECLARE_COMMAND( menu_addimage, "Maig", "add menu image", "menu_mgr name" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_addimage", 2, 2 );

  wGameject *gj = new wMenuImage( Game, parameters[ 0 ] ->toInt(), parameters[ 1 ] ->toString() );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




WAM_DECLARE_COMMAND( menu_getposition, "Mgpo", "get menu position as 2-element array", "image_widget" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_getposition", 1, 1 );

  string pos_str = Game.getMessageManager().sendTo( parameters[ 0 ] ->toInt(), MsgGetPosition ) ->first;
  regex_string rx( "^(\\-?[0-9]+)\\,(\\-?[0-9]+)$" );
  rx.match( pos_str );

  ref<js_array, value> array( new js_array() );
  ( *array ) [ 0 ] = makeConstant( evalSigned( rx.getBackref( 0 ) ) );
  ( *array ) [ 1 ] = makeConstant( evalSigned( rx.getBackref( 1 ) ) );
  return array;
}




WAM_DECLARE_COMMAND( menu_addspacer, "Masp", "add menu spacer", "menu_mgr name" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_addspacer", 2, 2 );

  wGameject *gj = new wMenuSpacer( Game, parameters[ 0 ] ->toInt(), parameters[ 1 ] ->toInt() );
  Game.getInstanceManager().insertObject( gj );
  return makeConstant( gj->id() );
}




WAM_DECLARE_COMMAND( menu_complete, "Mcmp", "notify manager that menu is complete", "menu_mgr" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_complete", 1, 1 );

  Game.getMessageManager().sendTo( parameters[ 0 ] ->toInt(), MsgMenuComplete );
  return makeNull();
}




WAM_DECLARE_COMMAND( menu_end, "Mend", "delete menu", "menu_mgr" )
{
  IXLIB_JS_ASSERT_PARAMETERS( "menu_end", 1, 1 );

  Game.getMessageManager().sendTo( parameters[ 0 ] ->toInt(), MsgMenuEnd );
  return makeNull();
}




// registration ---------------------------------------------------------------
void registerMenuCommands( wGame &g )
{
  register_menu_createmanager( g );
  register_menu_addbutton( g );
  register_menu_addinput( g );
  register_menu_getinput( g );
  register_menu_addtext( g );
  register_menu_addimage( g );
  register_menu_getposition( g );
  register_menu_addspacer( g );
  register_menu_complete( g );
  register_menu_end( g );

  MsgGetHeight = g.getMessageManager().registerMessage( WAM_MSG_MENU_GET_HEIGHT );
  MsgSetup = g.getMessageManager().registerMessage( WAM_MSG_MENU_SETUP );
  MsgGainFocus = g.getMessageManager().registerMessage( WAM_MSG_MENU_GAIN_FOCUS );
  MsgLoseFocus = g.getMessageManager().registerMessage( WAM_MSG_MENU_LOSE_FOCUS );
  MsgRequestFocus = g.getMessageManager().registerMessage( WAM_MSG_MENU_REQUEST_FOCUS );
  MsgAttach = g.getMessageManager().registerMessage( WAM_MSG_MENU_ATTACH );
  MsgDetach = g.getMessageManager().registerMessage( WAM_MSG_MENU_DETACH );
  MsgMenuComplete = g.getMessageManager().registerMessage( WAM_MSG_MENU_COMPLETE );
  MsgMenuEnd = g.getMessageManager().registerMessage( WAM_MSG_MENU_END );
  MsgGetInput = g.getMessageManager().registerMessage( WAM_MSG_MENU_GET_INPUT );
  MsgGetPosition = g.getMessageManager().registerMessage( WAM_MSG_MENU_GET_POSITION );
}
