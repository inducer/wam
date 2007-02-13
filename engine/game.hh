// ----------------------------------------------------------------------------
//  Description      : WAM game object
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_GAME
#define WAM_GAME




#include <vector>
#include "utility/debug.hh"
#include "utility/stream.hh"
#include "utility/filemanager.hh"
#include "engine/gamebase.hh"
#include "engine/resource.hh"
#include "engine/gameject.hh"
#include "engine/instancemanager.hh"
#include "engine/codemanager.hh"
#include "engine/tick.hh"
#include "engine/collision.hh"
#include "engine/input.hh"
#include "engine/message.hh"
#include "engine/persistence.hh"
#include "engine/network_internal.hh"
#include "engine/drawable.hh"




// wGameImpl -----------------------------------------------------------------
class wGameFinal : public wGame
{
  protected:
    bool IsMaster;
    bool Quitflag;
    bool PauseMode;
    bool DebugMode;
    bool RestartMainLoop;
    float GameTime;
    unsigned long LastTickStart;

    wGameInstanceManager InstanceManager;
    wGameCodeManager GameCodeManager;
    wNetworkManager NetworkManager;
    wPersistenceManager PersistenceManager;

    // instance managers
    wTickManager TickManager;
    wCollisionManager CollisionManager;
    wInputManager InputManager;
    wDrawableManager DrawableManager;
    wMessageManager MessageManager;

    // manager gamejects
    wScrollManager *ScrollManager;
    wConsole *Console;

    // resource managers
    wFileManager FileManager;
    wImageManager ImageManager;
    wFontManager FontManager;
    wShapeBitmapManager ShapeBitmapManager;
    wSoundManager SoundManager;

    // hardware
    audio_manager *AudioManager;

  public:
    wGameFinal( string const &basedir, display &dpy, audio_manager *amgr );
    void startUp();
    virtual ~wGameFinal();

    void cleanUp();
    void resetManagers();

    void setQuitflag()
    {
      Quitflag = true;
    }
    bool getQuitflag()
    {
      return Quitflag;
    }
    float getGameTime()
    {
      return GameTime;
    }

    bool loopIteration();

    void addMissionPack( string const &name );
    void removeMissionPack( string const &name );

    bool getDebugMode()
    {
      return DebugMode;
    }
    void setDebugMode( bool value )
    {
      DebugMode = value;
    }
    bool getPauseMode()
    {
      return PauseMode;
    }
    void setPauseMode( bool value );

    // item accessors
    wGameInstanceManager &getInstanceManager()
    {
      return InstanceManager;
    }
    wGameCodeManager &getGameCodeManager()
    {
      return GameCodeManager;
    }
    wNetworkManager &getNetworkManager()
    {
      return NetworkManager;
    }
    wPersistenceManager &getPersistenceManager()
    {
      return PersistenceManager;
    }

    // instance managers
    wTickManager &getTickManager()
    {
      return TickManager;
    }
    wCollisionManager &getCollisionManager()
    {
      return CollisionManager;
    }
    wInputManager &getInputManager()
    {
      return InputManager;
    }
    wDrawableManager &getDrawableManager()
    {
      return DrawableManager;
    }
    wMessageManager &getMessageManager()
    {
      return MessageManager;
    }

    // manager gamejects
    wScrollManager &getScrollManager()
    {
      if ( ScrollManager )
        return * ScrollManager;
      else
        EXGAME_THROWINFO( ECGAME_REFUNAVAILABLE, "scroll manager" )
      }
    wConsole &getConsole()
    {
      if ( Console )
        return * Console;
      else
        EXGAME_THROWINFO( ECGAME_REFUNAVAILABLE, "console" )
      }

    void setScrollManager( wScrollManager *scrl )
    {
      ScrollManager = scrl;
    }
    void setConsole( wConsole *cons )
    {
      Console = cons;
    }

    // resource managers
    wFileManager &getFileManager()
    {
      return FileManager;
    }
    wImageManager &getImageManager()
    {
      return ImageManager;
    }
    wFontManager &getFontManager()
    {
      return FontManager;
    }
    wShapeBitmapManager &getShapeBitmapManager()
    {
      return ShapeBitmapManager;
    }
    wSoundManager &getSoundManager()
    {
      return SoundManager;
    }

    // hardware
    audio_manager *getAudioManager()
    {
      return AudioManager;
    }
};




#endif
