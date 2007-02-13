// ----------------------------------------------------------------------------
//  Description      : Resource management
// ----------------------------------------------------------------------------
//  Remarks          :
//    Names must be specified in the form 'base/fonts/32-15' (e.g.)
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_RESOURCEMANAGER
#define WAM_RESOURCEMANAGER




#include <list>
#include <map>
#include <memory>
#include <ixlib_string.hh>
#include "utility/filemanager.hh"




// wResourceManager -----------------------------------------------------------
template <class T, class Descriptor = string>
class wResourceManager
{
  protected:
    struct wResource
    {
      TSize RefCount;
      TSize CleanupGraceCount;
      T *Data;
    };

    typedef map<Descriptor, wResource> wResourceMap;
    map<Descriptor, wResource> Resources;
  public:
    virtual ~wResourceManager();

    typedef list<Descriptor> wResourceList;
    void listObjects( wResourceList &l );

    T *get
    ( Descriptor const &desc );
    void addReference( T *resource );
    void release( T *resource );

    void cleanUp();


  protected:
    virtual T *load( Descriptor const &desc ) = 0;
};




// wStreamResourceManager -------------------------------------------------------
template <class T, class Descriptor = string>
class wStreamResourceManager : public wResourceManager<T, Descriptor>
{
  protected:
    wFileManager &FileManager;

  public:
    wStreamResourceManager( wFileManager &fm )
        : FileManager( fm )
    {}

  protected:
    virtual T *load( Descriptor const &desc );
    virtual T *load( wFileManager::wStream &file, Descriptor const &desc ) = 0;
    virtual wFileType getFileType() = 0;
};




#endif
