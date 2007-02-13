// ----------------------------------------------------------------------------
//  Description      : png marking tool
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Andreas Kloeckner
// ----------------------------------------------------------------------------




#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <ixlib_cmdline.hh>
#include <ixlib_exbase.hh>
#include <ixlib_array.hh>
#include <ixlib_re.hh>
#include <ixlib_numconv.hh>
#include <sdlucid_png.hh>
#include <sdlucid_video.hh>
#include "utility/png_info.hh"




using namespace std;
using namespace ixion;
using namespace png;




// needed for win32 compatibility
extern "C"
{
  int main( int argc, char **argv );
}




int main( int argc, char **argv )
{
  try
  {
    command_line cmdline( argc, argv );
    cout << "markpng 0.10 by Team WAM" << endl;

    if ( cmdline.count( "--help" ) || cmdline.count( "-h" ) || !cmdline.count( "in=" ) )
    {
      cout << "usage: markpng in=<file> [out=<file>] [<options>]" << endl
      << "where <options> is some of:" << endl
      << "  --show                   " << endl
      << "  --refpoint=<x>,<y>       set reference point" << endl
      << "  --trans=<r>,<g>,<b>      set alpha key color" << endl
      << "  --shape                  transform grayscale image to 1bpp" << endl;
      return 0;
    }

    ifstream infile( cmdline.get( "in=" ).c_str() );
    png_stream_reader reader( infile );
    reader.readInfo();
    TSize bytesize = reader.getAlignedRowBytes( 1 ) * reader.getHeight();
    auto_array<TByte> image( bytesize );
    reader.readImage( image.get() );
    reader.readEnd();

    if ( cmdline.count( "--show" ) )
    {
      cout << "size: " << reader.getWidth() << "x" << reader.getHeight() << endl;
      if ( reader.getColorType() == PNG_COLOR_TYPE_RGB ||
           reader.getColorType() == PNG_COLOR_TYPE_RGB_ALPHA )
      {
        try
        {
          png_color_16p color;
          reader.getTransparencyTrueColor( color );

	  if ( (color->red & 0xff) == 0 && 
	      (color->green & 0xff) == 0 && 
	      (color->blue & 0xff) == 0 &&
	      ( color->red || color->green || color->blue ) )
	  {
	    color->red >>= 8;
	    color->green >>= 8;
	    color->blue >>= 8;
	  }

          cout << "transparent color: (r,g,b)=("
          << ( color->red >> 8 ) << ','
          << ( color->green >> 8 ) << ','
          << ( color->blue >> 8 ) << ')' << endl;
        }
        catch ( ... )
        {
          cout << "no transparency in image" << endl;
        }
      }
      else
        cout << "not an RGB image, cannot show transparency" << endl;
      png::png_meta_data meta;
      reader.getMetaData( meta );
      cout << "reference point: (x,y)=("
      << meta[ WAM_PNG_REFPOINTX ] << ','
      << meta[ WAM_PNG_REFPOINTY ] << ')' << endl;
      return 0;
    }

    if ( !cmdline.count( "out=" ) )
    {
      cerr << "markpng: must specify output file" << endl;
      return 1;
    }

    // create output file -----------------------------------------------------
    ofstream outfile( cmdline.get( "out=" ).c_str() );
    png_stream_writer writer( outfile );

    if ( cmdline.count( "--shape" ) )
    {
      if ( reader.getColorType() != PNG_COLOR_TYPE_GRAY )
      {
        cerr << "markpng: only grayscale images can be transformed into shape bitmaps" << endl;
        return 1;
      }
      if ( reader.getBitDepth() > 8 )
      {
        cerr << "markpng: only images with bpp <= 8 can be transformed into shape bitmaps" << endl;
        return 1;
      }

      writer.setInfo( reader.getWidth(), reader.getHeight(), 1, PNG_COLOR_TYPE_GRAY );

      png_color_8 sigbits;
      sigbits.red = 0;
      sigbits.blue = 0;
      sigbits.green = 0;
      sigbits.gray = 1;
      sigbits.alpha = 0;
      writer.setSignificantBits( sigbits );

      TSize rowsize = ( reader.getWidth() + 7 ) / 8;
      auto_array<TByte> newimg( reader.getHeight() * rowsize );
      memset( newimg.get(), 0, reader.getHeight() * rowsize );

      for ( TIndex y = 0;y < reader.getHeight();y++ )
      {
        TByte bit = 128;
        TByte *dest = newimg + y * rowsize;
        TByte *src = image + y * reader.getAlignedRowBytes(1);
        for ( TIndex x = 0;x < reader.getWidth();x++, bit >>= 1 )
        {
          if ( bit == 0 )
          {
            dest++;
            bit = 128;
          }
          if ( *src++ )
            * dest |= bit;
        }
      }
      image = newimg;
    }
    else
    {
      writer.setInfo( reader.getWidth(), reader.getHeight(), reader.getBitDepth(),
                      reader.getColorType() );
    }

    // texts & refpoint -------------------------------------------------------
    png_meta_data meta;
    reader.getMetaData( meta );
    if ( cmdline.count( "--refpoint=" ) )
    {
      regex_string re( "^([0-9]+)\\,([0-9]+)$" );
      if ( !re.match( cmdline.get( "--refpoint=" ) ) )
      {
        cerr << "markpng: invalid parameter format" << endl;
        return 1;
      }
      meta[ WAM_PNG_REFPOINTX ] = re.getBackref( 0 );
      meta[ WAM_PNG_REFPOINTY ] = re.getBackref( 1 );
    }
    writer.setMetaData( meta );


    // gamma transcription ----------------------------------------------------
    try
    {
      writer.setGamma( reader.getGamma() );
    }
    catch ( ... )
    {}

    // transparency transcription ---------------------------------------------
    try
    {
      png_color * palette;
      int colcount = reader.getPalette( palette );
      writer.setPalette( palette, colcount );

      png_byte *transentries;
      TSize transcount = reader.getTransparencyIndexed( transentries );
      writer.setTransparencyIndexed( transentries, transcount );
    }
    catch ( ... )
    {}

    // transparency handling --------------------------------------------------
    if ( reader.getColorType() == PNG_COLOR_TYPE_RGB ||
         reader.getColorType() == PNG_COLOR_TYPE_RGB_ALPHA )
    {
      png_color_16p transcolor;
      png_color_16 mytcolor;
      bool trans_set = false;

      try
      {
        reader.getTransparencyTrueColor( transcolor );
        trans_set = true;
      }
      catch ( ... )
      {}

      if ( cmdline.count( "--trans=" ) )
      {
        regex_string re( "^([0-9]+)\\,([0-9]+)\\,([0-9]+)$" );
        if ( !re.match( cmdline.get( "--trans=" ) ) )
        {
          cerr << "markpng: invalid parameter format" << endl;
          return 1;
        }

        mytcolor.index = 0;
        mytcolor.red = evalUnsigned( re.getBackref( 0 ) );
        mytcolor.green = evalUnsigned( re.getBackref( 1 ) );
        mytcolor.blue = evalUnsigned( re.getBackref( 2 ) );
        mytcolor.gray = 0;
        transcolor = &mytcolor;
        trans_set = true;
      }

      if ( trans_set )
        writer.setTransparencyTrueColor( *transcolor );
    }

    // write image ------------------------------------------------------------
    writer.writeInfo();
    writer.writeImage( image.get() );
    writer.writeEnd();
  }
  catch ( exception & ex )
  {
    cerr << ex.what() << endl;
  }
  catch ( ... )
  {
    cerr << "exit via unknown exception" << endl;
  }
}
