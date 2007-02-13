// ----------------------------------------------------------------------------
//  Description      : WAM string packing
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 2000 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_STRING_PACK
#define WAM_STRING_PACK




#include "utility/basic_types.hh"
#include "utility/stream.hh"




// insertion ------------------------------------------------------------------
void operator<<=( string &str, bool data );
void operator<<=( string &str, unsigned char data );
void operator<<=( string &str, unsigned short data );
void operator<<=( string &str, unsigned int data );
void operator<<=( string &str, signed int data );
void operator<<=( string &str, signed short data );
void operator<<=( string &str, float data );
void operator<<=( string &str, double data );
void operator<<=( string &str, string const &data );
void operator<<=( string &str, char const *data );
void operator<<=( string &str, coord_vector<int, 2> const &data );
void operator<<=( string &str, coord_vector<float, 2> const &data );
void operator<<=( string &str, coord_vector<double, 2> const &data );
void operator<<=( string &str, wStreamStore const &data );




// extraction -----------------------------------------------------------------
void operator>>=( string const &str, bool &data );
void operator>>=( string const &str, unsigned char &data );
void operator>>=( string const &str, unsigned short &data );
void operator>>=( string const &str, unsigned int &data );
void operator>>=( string const &str, signed int &data );
void operator>>=( string const &str, signed short &data );
void operator>>=( string const &str, double &data );
void operator>>=( string const &str, float &data );
void operator>>=( string const &str, string &data );
void operator>>=( string const &str, coord_vector<int, 2> &data );
void operator>>=( string const &str, coord_vector<float, 2> &data );
void operator>>=( string const &str, coord_vector<double, 2> &data );
void operator>>=( string const &str, wStreamStore &data );




#endif
