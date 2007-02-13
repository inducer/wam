// ----------------------------------------------------------------------------
//  Description      : WAM string packing
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#include <ixlib_numconv.hh>
#include <ixlib_re.hh>
#include "utility/exgame.hh"
#include "utility/string_pack.hh"




// insertion ------------------------------------------------------------------
void operator<<=( string &str, bool data )
{
  if ( data )
    str = "true";
  else
    str = "false";
}




void operator<<=( string &str, unsigned char data )
{
  str = unsigned2dec( data );
}




void operator<<=( string &str, unsigned short data )
{
  str = unsigned2dec( data );
}




void operator<<=( string &str, unsigned int data )
{
  str = unsigned2dec( data );
}




void operator<<=( string &str, signed int data )
{
  str = signed2dec( data );
}




void operator<<=( string &str, signed short data )
{
  str = signed2dec( data );
}




void operator<<=( string &str, float data )
{
  str = float2dec( data );
}




void operator<<=( string &str, double data )
{
  str = float2dec( data );
}




void operator<<=( string &str, string const &data )
{
  str = data;
}




void operator<<=( string &str, char const *data )
{
  str = data;
}




void operator<<=( string &str, coord_vector<int, 2> const &data )
{
  str = signed2dec( data[ 0 ] ) + ':' + signed2dec( data[ 1 ] );
}




void operator<<=( string &str, coord_vector<float, 2> const &data )
{
  str = float2dec( data[ 0 ] ) + ':' + float2dec( data[ 1 ] );
}




void operator<<=( string &str, coord_vector<double, 2> const &data )
{
  str = float2dec( data[ 0 ] ) + ':' + float2dec( data[ 1 ] );
}




void operator<<=( string &str, wStreamStore const &data )
{
  base64encode( str, data.get(), data.usedSize() );
}





// extraction -----------------------------------------------------------------
void operator>>=( string const &str, bool &data )
{
  data = str == "true";
}




void operator>>=( string const &str, unsigned char &data )
{
  data = evalUnsigned( str );
}




void operator>>=( string const &str, unsigned short &data )
{
  data = evalUnsigned( str );
}




void operator>>=( string const &str, unsigned int &data )
{
  data = evalUnsigned( str );
}




void operator>>=( string const &str, signed int &data )
{
  data = evalSigned( str );
}




void operator>>=( string const &str, signed short &data )
{
  data = evalSigned( str );
}




void operator>>=( string const &str, double &data )
{
  data = evalFloat( str );
}




void operator>>=( string const &str, float &data )
{
  data = evalFloat( str );
}




void operator>>=( string const &str, string &data )
{
  data = str;
}




void operator>>=( string const &str, coord_vector<int, 2> &data )
{
  regex_string re = "^([^:]+)\\:(.+)$";
  if ( !re.match( str ) )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "cannot parse vector" )
    data[ 0 ] = evalSigned( re.getBackref( 0 ) );
  data[ 1 ] = evalSigned( re.getBackref( 1 ) );
}




void operator>>=( string const &str, coord_vector<float, 2> &data )
{
  regex_string re = "^([^:]+)\\:(.+)$";
  if ( !re.match( str ) )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "cannot parse vector" )
    data[ 0 ] = evalFloat( re.getBackref( 0 ) );
  data[ 1 ] = evalFloat( re.getBackref( 1 ) );
}




void operator>>=( string const &str, coord_vector<double, 2> &data )
{
  regex_string re = "^([^:]+)\\:(.+)$";
  if ( !re.match( str ) )
    EXGAME_THROWINFO( ECGAME_FILEFORMAT, "cannot parse vector" )
    data[ 0 ] = evalFloat( re.getBackref( 0 ) );
  data[ 1 ] = evalFloat( re.getBackref( 1 ) );
}




void operator>>=( string const &str, wStreamStore &data )
{
  data.enlargeCapacity( getMaxBase64DecodedSize( str.size() ) );
  data.setUsedSize( base64decode( data.get(), str ) );
}
