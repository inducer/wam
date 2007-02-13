// ----------------------------------------------------------------------------
//  Description      : WAM base-class-of-everything
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include "utility/object.hh"




// wObject --------------------------------------------------------------------
bool wObject::hasFeatures( wFeatureSet const &features ) const
{
  wFeatureSetProcessor myset( getFeatureSet() );
  return myset.hasFeatures( features );
}




