// ----------------------------------------------------------------------------
//  Description      : WAM basic types
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#include <cmath>
#include <ixlib_geometry_impl.hh>
#include "utility/basic_types.hh"
#include "utility/exgame.hh"




#define DISPLAY_SEP  ','




// tool functions -------------------------------------------------------------
coord_vector<int, 2> round( coord_vector<double, 2> const &vec )
{
  return coord_vector<int, 2>( ( int ) vec[ 0 ], ( int ) vec[ 1 ] );
}




coord_vector<int, 2> round( coord_vector<float, 2> const &vec )
{
  return coord_vector<int, 2>( ( int ) vec[ 0 ], ( int ) vec[ 1 ] );
}




// wFeatureSetProcessor -------------------------------------------------------
TSize wFeatureSetProcessor::countFeatures() const
{
  return size() / WAM_FEATURE_SEGMENT;
}




string wFeatureSetProcessor::getFeature( TIndex idx ) const
{
  return substr( idx * WAM_FEATURE_SEGMENT, WAM_FEATURE_SEGMENT );
}




bool wFeatureSetProcessor::hasFeature( string const &feat ) const
{
  for ( TIndex i = 0;i < countFeatures();i++ )
    if ( getFeature( i ) == feat )
      return true;
  return false;
}




bool wFeatureSetProcessor::hasFeatures( wFeatureSet const &feat ) const
{
  wFeatureSetProcessor demand( feat );

  for ( TIndex i = 0;i < demand.countFeatures();i++ )
    if ( !hasFeature( demand.getFeature( i ) ) )
      return false;
  return true;
}




string wFeatureSetProcessor::getDisplayRep() const
{
  string result;
  for ( TIndex i = 0;i < countFeatures();i++ )
  {
    if ( result.size() )
      result += DISPLAY_SEP;
    result += getFeature( i );
  }
  return result;
}




void wFeatureSetProcessor::parseDisplayRep( string const &rep )
{
  string result;
  TIndex idx = 0;
  while ( idx < rep.size() )
  {
    string temp = rep.substr( idx, WAM_FEATURE_SEGMENT );
    idx += WAM_FEATURE_SEGMENT;
    if ( temp.size() < WAM_FEATURE_SEGMENT )
      EXGAME_THROWINFO( ECGAME_GENERAL, ( "invalid feature set: " + rep ).c_str() )
      result += temp;
    if ( idx < rep.size() )
    {
      if ( rep[ idx ] != DISPLAY_SEP )
        EXGAME_THROWINFO( ECGAME_GENERAL, ( "invalid feature set: " + rep ).c_str() )
        idx++;
    }
  }
  operator=( result );
}
