// ----------------------------------------------------------------------------
//  Description      : WAM debugging
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM, all rights reserved.
// ----------------------------------------------------------------------------




#ifndef WAM_DEBUG_HEADER
#define WAM_DEBUG_HEADER




#include <ixlib_string.hh>
#include "wam_config.hh"
#include "utility/base.hh"




// Constants ------------------------------------------------------------------
enum wDebugLevel {
  WAM_DEBUG_OFF,
  WAM_DEBUG_WARN,
  WAM_DEBUG_NORMAL,
  WAM_DEBUG_INFO,
  WAM_DEBUG_VERBOSE,
  WAM_DEBUG_PERIODIC,
  WAM_DEBUG_ALL
};





#ifdef WAM_DEBUG
void wamPrintDebug( string const &what );
void wamPrintDebugLevel( string const &what, wDebugLevel level );
#else
#define wamPrintDebug(STR) /* nothing */
#define wamPrintDebugLevel(STR,LEVEL) /* nothing */
#endif

wDebugLevel wamGetDebugLevel();
void wamSetDebugLevel( wDebugLevel level );




#endif
