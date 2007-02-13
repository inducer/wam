// ----------------------------------------------------------------------------
//  Description      : Game exception
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_EXGAME
#define WAM_EXGAME




#include <ixlib_exbase.hh>
#include "utility/base.hh"




// Error codes ----------------------------------------------------------------
#define ECGAME_GENERAL                  0
#define ECGAME_NOTFOUND                 1
#define ECGAME_NOTYET                   2
#define ECGAME_SHAREDLIB                3
#define ECGAME_NOSUCHID                 4
#define ECGAME_DOUBLEID                 5
#define ECGAME_RESOURCENOTAVAIL         6
#define ECGAME_OVERFREEDRES             7
#define ECGAME_INVALIDBITDEPTH          8
#define ECGAME_IOERROR                  9
#define ECGAME_FILEFORMAT               10
#define ECGAME_NOBITINMASK              11
#define ECGAME_CREATIONFAILED           12
#define ECGAME_CONSOLE                  13
#define ECGAME_INVALIDFILETYPE          14
#define ECGAME_NET                      15
#define ECGAME_REFUNAVAILABLE  16
#define ECGAME_ANISCRIPT  17




// Throw macro ----------------------------------------------------------------
#define EXGAME_THROW(CODE)\
  throw wGameException(CODE,NULL,__FILE__,__LINE__);
#define EXGAME_THROWINFO(CODE,INFO)\
  throw wGameException(CODE,(char const *) INFO,__FILE__,__LINE__);




// Game exception -------------------------------------------------------------
struct wGameException : public base_exception
{
  wGameException( TErrorCode error, string const &info, char *module = NULL,
                  TIndex line = 0 )
      : base_exception( error, info.c_str(), module, line, "GAME" )
  { }
  virtual char *getText() const;
};




#endif
