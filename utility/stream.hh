// ----------------------------------------------------------------------------
//  Description      : Transmission stream classes
// ----------------------------------------------------------------------------
//  Remarks          : none
//
// ----------------------------------------------------------------------------
//  (c) Copyright 1999 by Team WAM
// ----------------------------------------------------------------------------




#ifndef WAM_STREAM
#define WAM_STREAM




#include <cstring>
#include <ixlib_base.hh>
#include <ixlib_array.hh>
#include <ixlib_string.hh>
#include "utility/base.hh"
#include "utility/basic_types.hh"




#define WAMSTR_ALIGNMENT  1




class wStreamStore
{
  protected:
    auto_array<TByte> Store;
    TSize UsedSize;

  public:
    wStreamStore( TSize capacity = 1024 )
        : Store( capacity ), UsedSize( 0 )
    {}
    wStreamStore( void *data, TSize capacity )
        : Store( capacity ), UsedSize( capacity )
    {
      memcpy( Store.get(), data, capacity );
    }

    void setCapacity( TSize capacity );
    void enlargeCapacity( TSize min_new_capacity );

    void clear()
    {
      Store.deallocate();
      UsedSize = 0;
    }
    TByte *get
    ()
    {
      return Store.get();
    }
    TByte const *get
    () const
    {
      return Store.get();
    }
    TByte *end()
    {
      return Store.get() + UsedSize;
    }
    TByte const *end() const
    {
      return Store.get() + UsedSize;
    }
    TSize capacity() const
    {
      return Store.capacity();
    }

    void setUsedSize( TSize usedsz )
    {
      UsedSize = usedsz;
    }
    TSize usedSize() const
    {
      return UsedSize;
    }
    void *alignPointer( void *ptr )
    {
#if WAMSTR_ALIGNMENT != 1
      TSize pos_align = ( ptr - Store.get() ) % WAMSTR_ALIGNMENT;
      return arg + WAMSTR_ALIGNMENT - pos_align;
#endif

      return ptr;
    }
};




class wStream
{
  protected:
    wStreamStore &Store;
    TIndex CurrentPosition;

  public:
    wStream( wStreamStore &store, TIndex from_pos )
        : Store( store ), CurrentPosition( from_pos )
    {}

    void *getPosition() const
    {
      return Store.get() + CurrentPosition;
    }
    void incrementPosition( TSize bytes )
    {
      CurrentPosition += bytes;
      align();
    }
    wStreamStore &getStore() const
    {
      return Store;
    }

  protected:
    void align()
    {
      // *** Fill in implementation here
    }
};




class wStreamWriter : public wStream
{
  public:
    wStreamWriter( wStreamStore &store )
        : wStream( store, store.usedSize() )
    {}
    ~wStreamWriter()
    {
      commit();
    }
    void commit()
    {
      Store.setUsedSize( CurrentPosition );
    }
    void ensureSpace( TSize space );

};




// wStreamReader --------------------------------------------------------------
class wStreamReader : public wStream
{
  public:
    wStreamReader( wStreamStore &store, TIndex from_pos = 0 )
        : wStream( store, from_pos )
    {}

    TSize getRemainingSize()
    {
      return Store.usedSize() - CurrentPosition;
    }
};




// insertion ------------------------------------------------------------------
wStreamWriter &operator<<( wStreamWriter &writer, TUnsigned8 data );
wStreamWriter &operator<<( wStreamWriter &writer, TUnsigned16 data );
wStreamWriter &operator<<( wStreamWriter &writer, TUnsigned32 data );
wStreamWriter &operator<<( wStreamWriter &writer, TSigned32 data );
wStreamWriter &operator<<( wStreamWriter &writer, double data );
wStreamWriter &operator<<( wStreamWriter &writer, float data );
wStreamWriter &operator<<( wStreamWriter &writer, string const &data );
wStreamWriter &operator<<( wStreamWriter &writer, wStreamStore const &data );
void insert( wStreamWriter &writer, wStreamStore const &data );




template <class T>
inline wStreamWriter &operator<<( wStreamWriter &writer, coord_vector<T, 2> const &data )
{
  writer << data[ 0 ] << data[ 1 ];
  return writer;
}




// extraction -----------------------------------------------------------------
wStreamReader &operator>>( wStreamReader &reader, TUnsigned8 &data );
wStreamReader &operator>>( wStreamReader &reader, TUnsigned16 &data );
wStreamReader &operator>>( wStreamReader &reader, TUnsigned32 &data );
wStreamReader &operator>>( wStreamReader &reader, TSigned32 &data );
wStreamReader &operator>>( wStreamReader &reader, double &data );
wStreamReader &operator>>( wStreamReader &reader, float &data );
wStreamReader &operator>>( wStreamReader &reader, string &data );
wStreamReader &operator>>( wStreamReader &reader, wStreamStore &data );




template <class T>
inline wStreamReader &operator>>( wStreamReader &reader, coord_vector<T, 2> &data )
{
  reader >> data[ 0 ] >> data[ 1 ];
  return reader;
}




// compression ----------------------------------------------------------------
void compress( wStreamWriter &dest, wStreamReader &src );
void decompress( wStreamWriter &dest, wStreamReader &src );
void compress( wStreamStore &dest, wStreamStore &src );
void decompress( wStreamStore &dest, wStreamStore &src );




#endif
