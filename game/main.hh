// ----------------------------------------------------------------------------
//  Description      : WAM main game lib registry
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2001 by Team WAM
// ----------------------------------------------------------------------------





#ifndef WAM_MAIN_LIB_REGISTRY
#define WAM_MAIN_LIB_REGISTRY




#include "engine/gamebase.hh"




extern "C"
{
  void wamInitializeModule( wGame & s, const char * libname );
}




#endif
