// ----------------------------------------------------------------------------
//  Description      : Game exception
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/exgame.hh"




#define TGAME_GENERAL      "General failure"
#define TGAME_NOTFOUND     "Item not found"
#define TGAME_NOTYET       "Function not yet implemented"
#define TGAME_SHAREDLIB   "Shared library error"
#define TGAME_NOSUCHID   "No such ID"
#define TGAME_DOUBLEID   "Duplicate ID"
#define TGAME_RESOURCENOTAVAIL  "Resource not available"
#define TGAME_OVERFREEDRES  "Freed resource with refcount 0"
#define TGAME_INVALIDBITDEPTH  "Invalid bit depth"
#define TGAME_IOERROR                   "File I/O error"
#define TGAME_FILEFORMAT                "File in wrong format"
#define TGAME_NOBITINMASK               "No '1' in bit mask"
#define TGAME_CREATIONFAILED            "Creation of object failed (returned null)"
#define TGAME_CONSOLE          "Console error"
#define TGAME_INVALIDFILETYPE  "Invalid file type"
#define TGAME_NETERROR   "Network error"
#define TGAME_REFUNAVAILABLE  "Reference unavailable"
#define TGAME_ANISCRIPT   "Animation script error"




static char *PlainText[] = {
                             TGAME_GENERAL, TGAME_NOTFOUND, TGAME_NOTYET, TGAME_SHAREDLIB, TGAME_NOSUCHID,
                             TGAME_DOUBLEID, TGAME_RESOURCENOTAVAIL,
                             TGAME_OVERFREEDRES, TGAME_INVALIDBITDEPTH, TGAME_IOERROR,
                             TGAME_FILEFORMAT, TGAME_NOBITINMASK, TGAME_CREATIONFAILED,
                             TGAME_CONSOLE, TGAME_INVALIDFILETYPE, TGAME_NETERROR, TGAME_REFUNAVAILABLE,
                             TGAME_ANISCRIPT
                           };




// wGameExceptiong ------------------------------------------------------------
char *wGameException::getText() const
{
  return PlainText[ Error ];
}
