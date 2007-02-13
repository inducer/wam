// ----------------------------------------------------------------------------
//  Description      : Pause monitor
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/debug.hh"
#include "engine/names.hh"
#include "engine/gamebase.hh"
#include "engine/instancemanager.hh"
#include "engine/codemanager.hh"
#include "engine/resource.hh"
#include "engine/message.hh"
#include "engine/drawable.hh"
#include "gamejects/pause_monitor.hh"




#define GJFEATURESET_PAUSEMONITOR GJFEATURE_DISPLAYED "PauM"




// wPauseMonitor --------------------------------------------------------------
class wPauseMonitor : public wDrawable, public wMessageReceiver
{
    wImageHolder Image;
    TMessageId PauseMsg, UnpauseMsg;

  public:
    wPauseMonitor( wGame &g );

    wFeatureSet const getFeatureSet() const
    {
      return GJFEATURESET_PAUSEMONITOR;
    }
    void registerMe();
    void startUp();
    void prepareToDie();
    void unregisterMe();

    TPriority getDrawPriority() const
    {
      return WAM_DRAWPRIO_STATUS;
    }
    wDisplayExtent getDisplayExtent();
    void draw( drawable &dpy );

    ref<wMessageReturn> receiveMessage( TMessageId id, string const &par1, void *par2 );
};




// wPauseMonitor --------------------------------------------------------------
wPauseMonitor::wPauseMonitor( wGame &g )
    : wGameject( g ), wDrawable( g ), wMessageReceiver( g ),
    Image( Game.getImageManager(), wImageDescriptor( "pause.png" ) )
{
  PauseMsg = Game.getMessageManager().registerMessage( WAM_MSG_PAUSE );
  UnpauseMsg = Game.getMessageManager().registerMessage( WAM_MSG_UNPAUSE );
}




void wPauseMonitor::registerMe()
{
  wDrawable::registerMe();
  wMessageReceiver::registerMe();
}




void wPauseMonitor::startUp()
{
  wDrawable::startUp();
  wMessageReceiver::startUp();
}




void wPauseMonitor::prepareToDie()
{
  wDrawable::prepareToDie();
  wMessageReceiver::prepareToDie();
}




void wPauseMonitor::unregisterMe()
{
  wDrawable::unregisterMe();
  wMessageReceiver::unregisterMe();
}




wDisplayExtent wPauseMonitor::getDisplayExtent()
{
  wScreenVector center( Game.getDrawableManager().getDisplay().width() / 2, Game.getDrawableManager().getDisplay().height() / 2 );
  return Image->extent() + center;
}



void wPauseMonitor::draw( drawable &dpy )
{
  Image->blit( dpy, dpy.width() / 2, dpy.height() / 2 );
}




ref<wPauseMonitor::wMessageReturn>
wPauseMonitor::receiveMessage( TMessageId id, string const &par1, void *par2 )
{
  if ( id == PauseMsg )
    show();
  if ( id == UnpauseMsg )
    hide();
  return NULL;
}




// registration ---------------------------------------------------------------
namespace
{
wGameject *createPauseMonitor( wGame &g )
{
  return new wPauseMonitor( g );
}
}




void registerPauseMonitor( wGame &g )
{
  g.getGameCodeManager().registerClass( GJFEATURESET_PAUSEMONITOR, "", createPauseMonitor );
}
