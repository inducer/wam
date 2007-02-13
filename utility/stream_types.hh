// ----------------------------------------------------------------------------
//  Description      : WAM Stream i/o for basic types
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_STREAM_TYPES
#define WAM_STREAM_TYPES




#include "utility/basic_types.hh"
#include "utility/stream.hh"




// Stream insertion and extraction for basic types ----------------------------
inline wStreamWriter &operator<<( wStreamWriter &sw, wVector const &pos )
{
  sw << pos[ 0 ] << pos[ 1 ];
  return sw;
}




inline wStreamReader &operator>>( wStreamReader &sr, wVector &pos )
{
  sr >> pos[ 0 ] >> pos[ 1 ];
  return sr;
}




#endif
