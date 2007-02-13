// ----------------------------------------------------------------------------
//  Description      : WAM shape/mono bitmap type
// ----------------------------------------------------------------------------
//  Remarks          : none.
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_SHAPEBITMAP
#define WAM_SHAPEBITMAP




#include <ixlib_array.hh>
#include <ixlib_polygon.hh>
#include "utility/basic_types.hh"
#include "utility/stream.hh"




// wShapeBitmap ---------------------------------------------------------------
class wShapeBitmap
{
  public:
    typedef signed int TCoordinate;
    typedef coord_vector<TCoordinate, 2> TCoordVector;
    typedef coord_vector<double, 2> TPreciseVector;
    typedef TUnsigned32 TData;
    enum TDrawMode { OR, AND, AND_NOT, XOR, DETECT };

  private:
    auto_array<TData> Data;
    TSize PerLine;
    TSize Width, Height;
    TCoordinate ReferencePointX, ReferencePointY;
    TDrawMode DrawMode;
    TUnsigned32 Detector;

  public:
    wShapeBitmap();
    wShapeBitmap( wShapeBitmap const &src );
    wShapeBitmap( int width, int height, TCoordinate refx = 0, TCoordinate refy = 0 );

    void copyFrom( wShapeBitmap const & src );

    /** Returns whether the rectangle described by \c rect
     * (with coordinates relative to the origin (not the reference
     * point) intersects with the shape bitmap.
     */
    bool doesCollide( rectangle<TCoordinate> const &rect, TCoordVector *where = NULL ) const;

    /** Returns whether the shape bitmap \c bmp intersects with this
     * shape bitmap. The offset is computed so that the reference
     * point of \c bmp is at \c pos in this bitmap.
     */
    bool doesCollide( TCoordVector const &pos, wShapeBitmap const &bmp,
                      TCoordVector *where = NULL ) const;
    /** For a point \c pos in the shape bitmap returns the 
     * closest rim point in the direction of - \c speed.
     */
    void getRimPoint( TCoordVector const &pos, TPreciseVector const &speed, TCoordVector &result ) const;
    void getNormal( TCoordVector const &rimpoint, TPreciseVector const &speed, TPreciseVector &result ) const;

    void wipe( bool color = false );
    TSize width() const
    {
      return Width;
    }
    TSize height() const
    {
      return Height;
    }
    TCoordinate referencePointX() const
    {
      return ReferencePointX;
    }
    TCoordinate referencePointY() const
    {
      return ReferencePointY;
    }
    void setReferencePoint( TCoordinate x, TCoordinate y )
    {
      ReferencePointX = x;
      ReferencePointY = y;
    }

    void clear();
    void create( int width, int height );
    void load( istream &strm );
    void save( ostream &strm ) const;

    /** Gets pixel at position x,y. If x,y is outside of bitmap,
     * the behavior is undefined, and a crash is likely.
     */
    bool getPixelNoClip( TCoordinate x, TCoordinate y ) const;
    bool getPixel( TCoordinate x, TCoordinate y ) const;
    bool getNextPixelOfColor( TCoordinate &x, TCoordinate &y, TCoordinate dx, TCoordinate dy, bool color );

    // low-level
    TDrawMode getDrawMode() const
    {
      return DrawMode;
    }
    void setDrawMode( TDrawMode drawmode )
    {
      DrawMode = drawmode;
    }
    void resetDetector()
    {
      Detector = 0;
    }
    bool getDetector() const
    {
      return Detector != 0;
    }

    void setPixel( TCoordinate x, TCoordinate y );
    void drawHLine( TCoordinate x1, TCoordinate y, TCoordinate x2 );
    void drawVLine( TCoordinate x, TCoordinate y1, TCoordinate y2 );

    // high-level
    void drawLine( TCoordinate x1, TCoordinate y1, TCoordinate x2, TCoordinate y2 );
    void drawBox( TCoordinate x1, TCoordinate y1, TCoordinate x2, TCoordinate y2 );
    void fillBox( TCoordinate x1, TCoordinate y1, TCoordinate x2, TCoordinate y2 );
    void drawCircle( TCoordinate x, TCoordinate y, TCoordinate r );
    void fillCircle( TCoordinate x, TCoordinate y, TCoordinate r );
    void drawEllipse( TCoordinate x, TCoordinate y, TCoordinate r_x, TCoordinate r_y );
    void fillEllipse( TCoordinate x, TCoordinate y, TCoordinate r_x, TCoordinate r_y );
    void drawPolygon( polygon<int> const &poly );
    void fillPolygon( polygon<int> const &poly );

    void drawBitmap( TCoordinate x, TCoordinate y, wShapeBitmap const &bmp );

  protected:
    void setup( int width, int height );

    static int firstBit( TData mask );
    static TData leftBitsUnset( TSize bits );
    static TData leftBitsSet( TSize bits );
    static void reverseByteAlignment( TData *data, TSize size );

    friend wStreamWriter &operator<<( wStreamWriter &writer, wShapeBitmap const &bits );
    friend wStreamReader &operator>>( wStreamReader &reader, wShapeBitmap &bits );
};




#endif
