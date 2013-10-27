
template<class P>
class volume : public imageBase<P>
{
	array<int> table_y, table_z;
	void setTable()
	{
		table_y.allocate( height );
		table_z.allocate( depth );
		for ( int y = 0; y < height; ++y ) table_y[y] = y * width;
		for ( int z = 0; z < depth; ++z ) table_z[z] = z * plane;
	}
public:
	int width, height, depth, plane;
	volume( const int iwidth = 0, const int iheight = 0, const int idepth = 0 ) :
		imageBase( iwidth * iheight * idepth ), 
		width( iwidth ), height( iheight ), depth( idepth ), plane( iwidth * iheight )
	{
		setTable();
	}
	volume( const volume & value ) :
		imageBase( value.width * value.height * value.depth ), 
		width( value.width ), height( value.height ), depth( value.depth ), plane( value.plane )
	{
		setTable();
		*this = value;
	}
	virtual ~volume(){}
	inline const bool inRect( int x, int y, int z ) const { return between( 0, x, width - 1 ) && between( 0, y, height - 1 ) && between( 0, z, depth - 1 ); }
	inline const P * getAddress( int x, int y, int z ) const { return data + x + table_y.data[ y ] + table_z.data[ z ]; }
	inline P * getAddress( int x, int y, int z ) { return data + x + table_y.data[ y ] + table_z.data[ z ]; }
	inline const P & getInternal( int x, int y, int z ) const { return *getAddress( x, y, z ); }
	inline void setInternal( int x, int y, int z, const P & p ) { *getAddress( x, y, z ) = p; }
	inline const P & get( int x, int y, int z ) const { if ( ! inRect( x, y ) ) return P(); return *getAddress( x, y, z ); }
	inline void set( int x, int y, int z, const P & p ) { if ( ! inRect( x, y ) ) return; *getAddress( x, y, z ) = p; }
	inline bool resize( const int iwidth, const int iheight, const int idepth )
	{
		if ( width == iwidth && height == iheight && depth == idepth ) return true;
		width = iwidth; height = iheight; depth = idepth; plane = iwidth * iheight;
		bool result = imageBase::resize( width * height * depth );
		setTable();
		return result;
	}
	bool copy( const volume & src, const rect3D & dr, const rect3D & sr )
	{
		if ( dr.width() == sr.width() && dr.height() == sr.height() && dr.depth() == sr.depth() ) {
			int w = dr.width(), h = dr.height(), d = dr.depth();
			if ( dr.left == 0 && dr.top == 0 && sr.left == 0 && sr.top == 0 && dr.near == 0 && sr.near == 0 ) {
				memcpy( data, src.data, length * dot );
				return true;
			}
			for ( int x = 0; x < w; ++x ) {
				for ( int y = 0; y < h; ++y ) {
					for ( int z = 0; z < d; ++z ) {
						set( x + dr.left, y + dr.top, z + dr.near, src.get( x + sr.left, y + sr.top, z + sr.near ) );
					}
				}
			}
		} else {
			int sw = sr.width(), sh = sr.height(), sd = sr.depth();
			int dw = dr.width(), dh = dr.height(), dd = dr.depth();
			decimal rw = decimal( sw ) / decimal( dw ), rh = decimal( sh ) / decimal( dh ), rd = decimal( sd ) / decimal( dd );
			for ( int x = 0; x < dw; ++x ) {
				for ( int y = 0; y < dh; ++y ) {
					for ( int z = 0; z < dd; ++z ) {
						set( x + dr.left, y + dr.top, z + dr.nea, src.get( int( x * rw ) + sr.left, int( y * rh ) + sr.top, int( z * rd ) + sr.near ) );
					}
				}
			}
		}
	}
	template<class Q>
	bool copySameBand( const volume<Q> & src, const rect3D & dr, const rect3D & sr )
	{
		if ( dr.width() == sr.width() && dr.height() == sr.height() && dr.depth() == sr.depth() ) {
			int w = dr.width(), h = dr.height(), d = dr.depth();
			for ( int x = 0; x < w; ++x ) {
				for ( int y = 0; y < h; ++y ) {
					for ( int z = 0; z < d; ++z ) {
						set( x + dr.left, y + dr.top, z + dr.near, src.get( x + sr.left, y + sr.top, z + sr.near ).pixel() );
					}
				}
			}
		} else {
			int sw = sr.width(), sh = sr.height(), sd = sr.depth();
			int dw = dr.width(), dh = dr.height(), dd = dr.depth();
			decimal rw = decimal( sw ) / decimal( dw ), rh = decimal( sh ) / decimal( dh ), rd = decimal( sd ) / decimal( dd );
			for ( int x = 0; x < dw; ++x ) {
				for ( int y = 0; y < dh; ++y ) {
					for ( int z = 0; z < dd; ++z ) {
						set( x + dr.left, y + dr.top, z + dr.nea, src.get( int( x * rw ) + sr.left, int( y * rh ) + sr.top, int( z * rd ) + sr.near ).pixel() );
					}
				}
			}
		}
	}
	template<class Q>
	bool copyOther( const volume<Q> & src, const rect3D & dr, const rect3D & sr )
	{
		if ( band == Q::band ) return copyBand( src, dr, sr );
		if ( dr.width() == sr.width() && dr.height() == sr.height() && dr.depth() == sr.depth() ) {
			int w = dr.width(), h = dr.height(), d = dr.depth();
			for ( int x = 0; x < w; ++x ) {
				for ( int y = 0; y < h; ++y ) {
					for ( int z = 0; z < d; ++z ) {
						set( x + dr.left, y + dr.top, z + dr.near, src.get( x + sr.left, y + sr.top, z + sr.near )() );
					}
				}
			}
		} else {
			int sw = sr.width(), sh = sr.height(), sd = sr.depth();
			int dw = dr.width(), dh = dr.height(), dd = dr.depth();
			decimal rw = decimal( sw ) / decimal( dw ), rh = decimal( sh ) / decimal( dh ), rd = decimal( sd ) / decimal( dd );
			for ( int x = 0; x < dw; ++x ) {
				for ( int y = 0; y < dh; ++y ) {
					for ( int z = 0; z < dd; ++z ) {
						set( x + dr.left, y + dr.top, z + dr.near, src.get( int( x * rw ) + sr.left, int( y * rh ) + sr.top, int( z * rd ) + sr.near )() );
					}
				}
			}
		}
	}
	inline bool copy( const volume & src, const point3D & dstsize, const point3D & srcsize ) { return copy( src, rect3D( 0, 0, 0, dstsize.x, dstsize.y, dstsize.z ), point3D( 0, 0, 0, srcsize.x, srcsize.y, srcsize.z ) ); }
	inline bool copy( const volume & src ) { return copy( src, point3D( width, height, depth ), point3D( src.width, src.height, src.depth ) ); }
	inline bool duplicate( const volume & src ) { resize( src.width, src.height, src.depth ); return copy( src ); }
	inline const volume & operator=( const volume & src ) { duplicate( src ); return *this; }
	virtual bool load( const char * filename );
	virtual bool save( const char * filename ) const;
};
