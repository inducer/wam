// ----------------------------------------------------------------------------
//  Description      : File management
// ----------------------------------------------------------------------------
//  Remarks          :
//    getFilename does not fail in any case. It may return nonexistent
//    filenames.
//    openFile will fail with an exception if nothing was found.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_FILEMANAGER
#define WAM_FILEMANAGER




#include <fstream>
#include <vector>
#include <dirent.h>
#include <ixlib_string.hh>
#include <ixlib_re.hh>
#include <ixlib_garbage.hh>
#include "utility/base.hh"




// file name components -------------------------------------------------------
enum wFileType {
  WAM_FT_CONFIG,
  WAM_FT_SAVEGAME,
  WAM_FT_IMAGE,
  WAM_FT_SHAPE,
  WAM_FT_FONT,
  WAM_FT_SCRIPT,
  WAM_FT_CODE,
  WAM_FT_SOUND,
  WAM_FT_MUSIC,
  WAM_FT_DATA,
};




// wFileMask -----------------------------------------------------------------
class wFileMask
{
  public:
    virtual ~wFileMask()
    {}
    virtual bool select( const dirent *d ) = 0;
};




// wFileRegMask --------------------------------------------------------------
class wFileRegMask : public wFileMask
{
  private:
    regex_string Regex;
  public:
    wFileRegMask( regex_string const &re )
        : wFileMask(), Regex( re )
    {}
    bool select( const dirent *d );
};




// wFileManager --------------------------------------------------------------
class wFileManager
{
  public:
    typedef vector<string> wMissionPackList;

  private:
    wMissionPackList MissionPacks;

    typedef vector<string> wSearchPath;

    string BaseDirectory;

  public:
    typedef vector<string> wFilenameList;
    typedef fstream wStream;

    wFileManager( string const &basedir );

    void listMissionPacks( wMissionPackList &packs );
    void addMissionPack( string const &name );
    void removeMissionPack( string const &name );

    string getFilename( wFileType ft, string const &base, string const &mpack = "" );
    ref<wStream> openFile( wFileType ft, string const &base, string const &mpack = "", ios::openmode mode = ios::in );
    void listFilenames( wFileType ft, wFilenameList &fnl, wFileMask *fm = NULL, string const &mpack = "" );
    string getBaseDirectory() const
    {
      return BaseDirectory;
    }

  private:
    char const *getTypeDirectory( wFileType ft );
    void getSearchPath( wFileType ft, wSearchPath &path );
    bool tryOpenFile( string const &filename );
    void listDirectory( char const *dir, wFilenameList &fnl, wFileMask *fm );
};




#endif
