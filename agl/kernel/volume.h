
template<class P>
class volume : public imageBase<P>
{
	array<int> table_y;
	array<P*> table_z;
public:
	int width, height, depth, plane;
	volume( const int iwidth = 0, const int iheight = 0, const int idepth = 0 ) : imageBase<P>( 0 ), width( 0 ), height( 0 ), depth( 0 ), plane( 0 )
	{ create( iwidth, iheight, idepth ); }
	volume( const volume & value ) : imageBase<P>( 0 ), width( 0 ), height( 0 ), depth( 0 ), plane( 0 )
	{ *this = value; }
	virtual ~volume(){ finalize(); }
	const bool inRect( int x, int y, int z ) const { return between( 0, x, width - 1 ) && between( 0, y, height - 1 ) && between( 0, z, depth - 1 ); }
	const P * getAddress( int x, int y, int z ) const { return table_z.data[ z ] + x + table_y.data[ y ]; }
	P * getAddress( int x, int y, int z ) { return table_z.data[ z ] + x + table_y.data[ y ]; }
	const P & getInternal( int x, int y, int z ) const { return *getAddress( x, y, z ); }
	void setInternal( int x, int y, int z, const P & p ) { *getAddress( x, y, z ) = p; }
	const P & get( int x, int y, int z ) const { if ( ! inRect( x, y ) ) { static P wk; return wk; } return *getAddress( x, y, z ); }
	void set( int x, int y, int z, const P & p ) { if ( ! inRect( x, y ) ) return; *getAddress( x, y, z ) = p; }
	virtual void finalize()
	{
		width = height = depth = plane = 0;
		table_y.release();
		table_z.release();
		array<P>::release();
	}
	bool create( const int iwidth, const int iheight, const int idepth )
	{
		if ( width == iwidth && height == iheight && depth == idepth ) return true;
		width = iwidth; height = iheight; depth = idepth; plane = iwidth * iheight;
		if ( ! ( width == 0 && height == 0 && depth == 0 ) && ( width <= 0 || height <= 0 || depth <= 0 ) ) return false;
		if ( ! imageBase<P>::create( width * height * depth ) ) return false;
		if ( width == 0 ) return true;
		table_y.allocate( height );
		table_z.allocate( depth );
		for ( int y = 0; y < height; ++y ) table_y[y] = y * width;
		for ( int z = 0; z < depth; ++z ) table_z[z] = data + z * plane;
		return true;
	}
	template<class Q>
	void reformat( const volume<Q> & src )
	{
		create( src.width, src.height );
		for ( int x = 0; x < width; ++x ) {
			for ( int y = 0; y < height; ++y ) {
				for ( int z = 0; z < depth; ++z ) {
					set( x, y, z, src.get( x, y, z ) );
				}
			}
		}
	}
	bool copy( const volume & src, const rectangle3<int> & dr, const rectangle3<int> & sr )
	{
		if ( dr.width() == sr.width() && dr.height() == sr.height() && dr.depth() == sr.depth() ) {
			int w = dr.width(), h = dr.height(), d = dr.depth();
			if ( dr.left == 0 && dr.top == 0 && sr.left == 0 && sr.top == 0 && 
				dr.front == 0 && sr.front == 0 &&
				dr.right == width && sr.right == src.width &&
				dr.bottom == height && sr.bottom == src.height &&
				dr.back == depth && sr.back == src.depth ) {
				memcpy( getAddress( 0, 0, 0 ), src.getAddress( 0, 0, 0 ), length * dot );
				return true;
			}
			for ( int x = 0; x < w; ++x ) {
				for ( int y = 0; y < h; ++y ) {
					for ( int z = 0; z < d; ++z ) {
						set( x + dr.left, y + dr.top, z + dr.front, src.get( x + sr.left, y + sr.top, z + sr.front ) );
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
						set( x + dr.left, y + dr.top, z + dr.nea, src.get( int( x * rw ) + sr.left, int( y * rh ) + sr.top, int( z * rd ) + sr.front ) );
					}
				}
			}
		}
	}
	bool copy( const volume & src, const point3<int> & dstsize, const point3<int> & srcsize ) { return copy( src, rectangle3<int>( 0, 0, 0, dstsize.x, dstsize.y, dstsize.z ), point3<int>( 0, 0, 0, srcsize.x, srcsize.y, srcsize.z ) ); }
	bool copy( const volume & src ) { return copy( src, point3<int>( width, height, depth ), point3<int>( src.width, src.height, src.depth ) ); }
	const volume & operator=( const volume & src ) { reformat( src ); return *this; }
	virtual bool load( const char * filename );
	virtual bool save( const char * filename ) const;
};
