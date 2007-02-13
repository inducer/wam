// ----------------------------------------------------------------------------
//  Description      : Resource management
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_RESOURCEMANAGEMENT
#define WAM_RESOURCEMANAGEMENT




#include <sdlucid_audio.hh>
#include "utility/resourcemanager.hh"
#include "utility/shapebitmap.hh"
#include "output/image.hh"
#include "engine/gamebase.hh"




void registerResourceManagerCommands( wGame &g );




// wImageManager --------------------------------------------------------------
struct wImageDescriptor
{
  string FileName;
  double StretchX, StretchY;
  double Rotate;

  wImageDescriptor( string const &filename, double stretchx = 1, double stretchy = 1, double rotate = 0 );
};




string describeResource( wImageDescriptor const &desc );
string getResourceFileName( wImageDescriptor const &desc );
bool operator<( wImageDescriptor const &desc1, wImageDescriptor const &desc2 );



class wImageManager : public wStreamResourceManager<wImage, wImageDescriptor>
{
    typedef wStreamResourceManager<wImage, wImageDescriptor> Super;
  public:
    wImageManager( wFileManager &fm )
        : Super( fm )
    {}
    wImage *load( wFileManager::wStream &file, wImageDescriptor const &desc );
    wFileType getFileType()
    {
      return WAM_FT_IMAGE;
    }
};




// wShapeBitmapManager -------------------------------------------------------
class wShapeBitmapManager : public wStreamResourceManager<wShapeBitmap>
{
    typedef wStreamResourceManager<wShapeBitmap> Super;
  public:
    wShapeBitmapManager( wFileManager &fm )
        : Super( fm )
    {}
    wShapeBitmap *load( wFileManager::wStream &file, string const &name );
    wFileType getFileType()
    {
      return WAM_FT_SHAPE;
    }
};




// wFontManager --------------------------------------------------------------
class wFontManager : public wStreamResourceManager<font>
{
    typedef wStreamResourceManager<font> Super;
  public:
    wFontManager( wFileManager &fm )
        : Super( fm )
    {}
    font *load( wFileManager::wStream &file, string const &name );
    wFileType getFileType()
    {
      return WAM_FT_FONT;
    }
};




// wSoundManager -------------------------------------------------------------
class wSoundManager : public wStreamResourceManager<audio_data>
{
    typedef wStreamResourceManager<audio_data> Super;
  public:
    wSoundManager( wFileManager &fm )
        : Super( fm )
    {}
    audio_data *load( wFileManager::wStream &file, string const &name );
    wFileType getFileType()
    {
      return WAM_FT_SOUND;
    }
};




// wResourceHolder -----------------------------------------------------------
template <class T, class Mgr, class Descriptor = string>
class wResourceHolder
{
  protected:
    Mgr *Manager;
    T *Resource;

  public:
    wResourceHolder();
    wResourceHolder( wResourceHolder const &src );
    wResourceHolder( Mgr &mgr );
    wResourceHolder( Mgr &mgr, Descriptor const &desc );
    ~wResourceHolder();
    wResourceHolder &operator=( wResourceHolder const &src );
    void set
      ( Descriptor const &desc );
    void set
      ( T *newres );
    void setManager( Mgr &mgr )
    {
      Manager = &mgr;
    }
    T *get
    () const
    {
      return Resource;
    }

    T *operator->() const
    {
      return Resource;
    }
    T &operator*() const
    {
      return * Resource;
    }
};




typedef wResourceHolder<wImage, wImageManager, wImageDescriptor> wImageHolder;
typedef wResourceHolder<wShapeBitmap, wShapeBitmapManager> wShapeBitmapHolder;
typedef wResourceHolder<font, wFontManager> wFontHolder;




// wSoundHolder --------------------------------------------------------------
class wSoundHolder : public wResourceHolder<audio_data, wSoundManager>
{
    typedef wResourceHolder<audio_data, wSoundManager> Super;
    typedef vector<audio_manager::TStreamId> wStreamList;
    wStreamList Streams;
    audio_manager *AudioManager;

  public:
    wSoundHolder( wGame &game, string const &name ) ;
    ~wSoundHolder();
    audio_manager::TStreamId play( TSize repeatcount = 1, float delay = 0, float vol = 1, float panning = 0 );
    bool hasActiveStreams();

  private:
    void updateStreamList();
};




#endif
