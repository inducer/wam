// ----------------------------------------------------------------------------
//  Description      : WAM basic types
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_BASIC_TYPES
#define WAM_BASIC_TYPES




#include <ixlib_geometry.hh>
#include <ixlib_string.hh>
#include "wam_config.hh"
#include "utility/base.hh"




// constants ------------------------------------------------------------------
#define WAM_FEATURE_SEGMENT 4
#define WAM_FEATURE_SEPARATOR '.'

#define WAMID_INVALID       0
#define WAMID_FIRSTVALID      1000




// basic types ----------------------------------------------------------------
typedef signed int TPriority;
typedef char const *wFeatureSet;

// use exclusively to express game coordinates
typedef int TGameCoordinate;
typedef coord_vector<TGameCoordinate> wGameVector;
typedef rectangle<TGameCoordinate> wGameExtent;




coord_vector<int, 2> round( coord_vector<double, 2> const &vec );
coord_vector<int, 2> round( coord_vector<float, 2> const &vec );




class wFeatureSetProcessor : public string
{
    typedef string Super;

  public:
    // This just duplicates constructors from Super
    wFeatureSetProcessor()
    { }
    wFeatureSetProcessor( const Super &str, TSize pos = 0, TSize n = npos )
        : string( str, pos, n )
    { }
    wFeatureSetProcessor( const char *s, TSize n )
        : string( s, n )
    { }
    wFeatureSetProcessor( const char *s )
        : string( s )
    { }
    wFeatureSetProcessor( TSize n, char c )
        : string( n, c )
    { }
    wFeatureSetProcessor( char c )
        : string( 1, c )
    { }
    template <class InputIterator>
    wFeatureSetProcessor( InputIterator begin, InputIterator end )
        : string( begin, end )
    { }

    TSize countFeatures() const;
    string getFeature( TIndex idx ) const;
    bool hasFeature( string const &feat ) const;
    bool hasFeatures( wFeatureSet const &feat ) const;
    string getDisplayRep() const;
    void parseDisplayRep( string const &rep );
};




#endif
