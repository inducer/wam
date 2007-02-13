// ----------------------------------------------------------------------------
//  Description      : WAM game object predeclaration
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_GAMEBASE
#define WAM_GAMEBASE




#include <ixlib_string.hh>
#include "utility/base.hh"




// predeclaration -------------------------------------------------------------
class wGameInstanceManager;
class wGameCodeManager;
class wNetworkManager;
class wPersistenceManager;

// instance managers
class wTickManager;
class wCollisionManager;
class wInputManager;
class wDrawableManager;
class wMessageManager;

// manager gamejects
class wScrollManager;
class wConsole;

// resource managers
class wFileManager;
class wImageManager;
class wFontManager;
class wShapeBitmapManager;
class wSoundManager;

// hardware
namespace sdl
{
class audio_manager;
}




// wGame ---------------------------------------------------------------------
class wGame
{
  public:
    virtual void cleanUp() = 0;

    virtual void setQuitflag() = 0;
    virtual bool getQuitflag() = 0;
    virtual float getGameTime() = 0;

    virtual void addMissionPack( string const &name ) = 0;
    virtual void removeMissionPack( string const &name ) = 0;

    virtual bool getDebugMode() = 0;
    virtual void setDebugMode( bool value ) = 0;
    virtual bool getPauseMode() = 0;
    virtual void setPauseMode( bool value ) = 0;

    virtual wGameInstanceManager &getInstanceManager() = 0;
    virtual wGameCodeManager &getGameCodeManager() = 0;
    virtual wNetworkManager &getNetworkManager() = 0;
    virtual wPersistenceManager &getPersistenceManager() = 0;

    // instance managers
    virtual wTickManager &getTickManager() = 0;
    virtual wCollisionManager &getCollisionManager() = 0;
    virtual wInputManager &getInputManager() = 0;
    virtual wDrawableManager &getDrawableManager() = 0;
    virtual wMessageManager &getMessageManager() = 0;

    // manager gamejects
    virtual wScrollManager &getScrollManager() = 0;
    virtual wConsole &getConsole() = 0;

    virtual void setScrollManager( wScrollManager *scrl = NULL ) = 0;
    virtual void setConsole( wConsole *cons = NULL ) = 0;

    // resource managers
    virtual wFileManager &getFileManager() = 0;
    virtual wImageManager &getImageManager() = 0;
    virtual wFontManager &getFontManager() = 0;
    virtual wShapeBitmapManager &getShapeBitmapManager() = 0;
    virtual wSoundManager &getSoundManager() = 0;

    // hardware
    virtual sdl::audio_manager *getAudioManager() = 0;
};




#endif
