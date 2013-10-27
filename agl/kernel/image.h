#include "pixel.h"

template<class P>
class imageBase : public array<P>
{
public:
	enum {
		dot = P::dot,
	};
protected:
	// one channel type
	const P & getInternal( int index ) const { return data[index]; }
	void setInternal( int index, const P & p ) { data[index] = p; }
	const P & get( int index ) const { return (*static_cast<const array<P>*>(this))[index]; }
	void set( int index, const P & p ) { (*static_cast<array<P>*>(this))[index] = p; }

	bool create( const int isize )
	{
		if ( isize < 0 ) return false;
		allocate( isize );
		return true;
	}
public:
	imageBase( const int isize = 0 )
	{
		create( isize );
	}
	virtual ~imageBase()
	{
	}
	void fill( const P & p )
	{
		if ( size == 0 ) return;
		setInternal( 0, p );
		memorySet( data, size * dot, data, dot );
	}
	bool enable() const { return size > 0; }
	virtual void finalize() = 0;
};

template<class P>
class imageInterface : public imageBase<P>
{
	array<P*> table_y;
	enum {
		defaultTopDown = 
#if defined( WIN32 )
		false,
#else
		true,
#endif
	};
public:
	bool topdown;
	int width, height;
public:
	imageInterface( int iwidth = 0, int iheight = 0 ) : imageBase<P>( 0 ), width( 0 ), height( 0 ), topdown( defaultTopDown )
	{ create( iwidth, iheight ); }
	imageInterface( const imageInterface & value ) : imageBase<P>( 0 ), width( 0 ), height( 0 ), topdown( defaultTopDown )
	{ *this = value; }
	virtual ~imageInterface(){}
	virtual void finalize()
	{
		width = height = 0;
		table_y.release();
		release();
	}
	const bool inrect( int x, int y ) const
	{ return between( 0, x, width - 1 ) && between( 0, y, height - 1 ); }
	const P * getAddress( int x, int y ) const { return table_y.data[y] + x; }
	P * getAddress( int x, int y ) { return table_y.data[y] + x; }
	const P & getInternal( int x, int y ) const { return *getAddress( x, y ); }
	void setInternal( int x, int y, const P & p ) { *getAddress( x, y ) = p; }
	const P & get( int x, int y ) const { if ( ! inrect( x, y ) ) { static P wk; return wk; }return *getAddress( x, y ); }
	void set( int x, int y, const P & p ) { if ( ! inrect( x, y ) ) return; *getAddress( x, y ) = p; }

	bool create( const int iwidth, const int iheight, const P initial = P() )
	{
		if ( iwidth < 0 || iheight < 0 ) return false;
		if ( width == iwidth && height == iheight ) return true;
		width = iwidth; height = iheight;
		table_y.allocate( height );
		if ( ! imageBase<P>::create( width * height ) ) return false;
		if ( ! enable() ) return true;
		int i;
		if ( topdown ) {
			table_y[ 0 ] = data;
			for ( i = 1; i < height; i++ ) {
				table_y[ i ] = table_y[ i - 1 ] + width;
			}
		} else {
			table_y[ height - 1 ] = data;
			for ( i = height - 2; i >= 0; i-- ) {
				table_y[ i ] = table_y[ i + 1 ] + width;
			}
		}
		fill( initial );
		return true;
	}
	template<class Q>
	void reformat( const imageInterface<Q> & src )
	{
		create( src.width, src.height );
		pixelT<P::element> p;
		for ( int x = 0; x < width; ++x ) {
			for ( int y = 0; y < height; ++y ) {
				const pixelT<Q::element> q = src.get( x, y );
				p.convert( q );
				set( x, y, p );
			}
		}
	}
	bool copy( const imageInterface & src, const rectangle2<int> & dr, const rectangle2<int> & sr )
	{
		if ( ! enable() || ! src.enable() ) return true;
		if ( dr.width() == sr.width() && dr.height() == sr.height() ) {
			int w = dr.width(), h = dr.height();
			if ( dr.left == 0 && dr.top == 0 && sr.left == 0 && sr.top == 0 &&
				dr.right == width && sr.right == src.width &&
				dr.bottom == height && sr.bottom == src.height ) {
				if ( topdown == src.topdown ) {
					if ( topdown == true ) {
						memcpy( getAddress( 0, 0 ), src.getAddress( 0, 0 ), size * dot );
					} else {
						memcpy( getAddress( 0, h - 1 ), src.getAddress( 0, h - 1 ), size * dot );
					}
					return true;
				} else {
					for ( int y = 0; y < h; ++y ) {
						memcpy( getAddress( 0, y ), src.getAddress( 0, y ), width * dot );
					}
					return true;
				}
			}
			for ( int x = 0; x < w; ++x ) {
				for ( int y = 0; y < h; ++y ) {
					set( x + dr.left, y + dr.top, src.get( x + sr.left, y + sr.top ) );
				}
			}
		} else {
			int sw = sr.width(), sh = sr.height();
			int dw = dr.width(), dh = dr.height();
			decimal rw = decimal( sw ) / decimal( dw ), rh = decimal( sh ) / decimal( dh );
			for ( int x = 0; x < dw; ++x ) {
				for ( int y = 0; y < dh; ++y ) {
					set( x + dr.left, y + dr.top, src.get( int( x * rw ) + sr.left, int( y * rh ) + sr.top ) );
				}
			}
		}
		return true;
	}
	bool copy( const imageInterface & src, const point2<int> & dstsize, const point2<int> & srcsize )
	{
		return copy( src, rectangle2<int>( 0, 0, dstsize.x, dstsize.y ), rectangle2<int>( 0, 0, srcsize.x, srcsize.y ) );
	}
	bool copy( const imageInterface & src )
	{
		return copy( src, point2<int>( width, height ), point2<int>( src.width, src.height ) );
	}
	imageInterface & operator=( const imageInterface & src )
	{
		create( src.width, src.height );
		if ( enable() ) copy( src );
		return *this;
	}
	bool resize( const int iwidth, const int iheight )
	{
		if ( ! enable() ) return false;
		if ( iwidth == 0 || iheight == 0 ) return true;
		image wk( *this );
		finalize();
		create( iwidth, iheight );
		copy( wk );
		return true;
	}
	bool load( const char * filenmae );
	bool save( const char * filenmae ) const;
};

typedef imageInterface<pixel>	image;

#include "tga.h"
#include "bitmap.h"

template<class P>
bool imageInterface<P>::load( const char * _filename )
{
	if ( P::band != 1 ) return false;
	string filename = _filename;
	file * f = file::initialize( filename, false, true );
	if ( ! f ) {
		if ( ! filename.reverseCompare( ".tga" ) ) filename += ".tga";
		f = file::initialize( filename, false, true );
		if ( ! f ) return false;
	}
	bool result = true;
	if ( filename.reverseCompare( ".tga" ) && targa<P>::read( *f, *this ) ) {
	} else if ( filename.reverseCompare( ".bmp" ) && bitmap<P>::read( *f, *this ) ) {
	} else {
		result = false;
	}
	file::finalize( f );
	return result;
}

template<class P>
bool imageInterface<P>::save( const char * _filename ) const
{
	if ( P::band != 1 ) return false;
	string filename = _filename;
	if ( ! filename.reverseCompare( ".tga" ) ) filename += ".tga";
	file * f = file::initialize( filename, true, true );
	if ( ! f ) return false;
	if ( targa<P>::write( *f, *this ) ) return true;
	file::finalize( f );
	return false;
}

