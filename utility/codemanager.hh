// ----------------------------------------------------------------------------
//  Description      : WAM class manager
// ----------------------------------------------------------------------------
//  Remarks          :
//    It is silently assumed that a complete class id is enough to
//    unambiguously specify its class.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_CLASSMANAGER
#define WAM_CLASSMANAGER




#include <dirent.h>
#include <set>
#include <list>
#include "utility/basic_types.hh"




// wCodeManager----------------------------------------------------------------
template <class Object, class MainClass>
class wCodeManager
{
  public:
    typedef unsigned int wClassIdentifier;
    typedef Object *( *wInstantiationFunc ) ( MainClass &s );

    struct wClass
    {
      wClassIdentifier Id;
      wFeatureSetProcessor Features;
      string SharedLibName;
      wInstantiationFunc instantiate;
    };
    typedef void ( *wLibraryInitFunction ) ( MainClass &, const char * );

  private:
    wClassIdentifier NextId;
    typedef vector<string> wLibList;
    wLibList LibList;
    typedef vector<wClass> wClassList;
    wClassList ClassList;

  protected:
    MainClass &MainObject;

  public:
    typedef typename wClassList::iterator class_iterator;
    typedef typename wClassList::const_iterator class_const_iterator;
    typedef typename wLibList::iterator lib_iterator;
    typedef typename wLibList::const_iterator lib_const_iterator;

    wCodeManager( MainClass &s )
        : NextId( 0 ), MainObject( s )
    {}

    class_const_iterator class_begin() const
    {
      return ClassList.begin();
    }
    class_const_iterator class_end() const
    {
      return ClassList.end();
    }
    lib_const_iterator lib_begin() const
    {
      return LibList.begin();
    }
    lib_const_iterator lib_end() const
    {
      return LibList.end();
    }

    void registerLibrary( string const &pathname, wLibraryInitFunction initf );
    void loadLibrary( string const &pathname );
    void loadCode( string const &base, string const &mpack = "" );
    void loadAllCode( string const &mpack = "" );

    Object *createObject( wFeatureSet const &features ) const;

    wClassIdentifier registerClass( wFeatureSet const &features, string const &libname, Object *( *creator ) ( MainClass & ) );
    void unregisterClass( wClassIdentifier id );

  private:
    class_iterator getClass( wClassIdentifier id );
    class_const_iterator getClass( wFeatureSet const &feat ) const;
};




#endif
