inline void saveBmp( imageInterface< pixelLuminance<int16> > & img, const char * filename )
{
	imageInterface<pixelBGR> wk;
	wk.create( img.width, img.height );
	for ( int y = 0; y < wk.height; ++y ) {
		for ( int x = 0; x < wk.width; ++x ) {
			int v = clamp<int>( 0, img.get( x, y ).y, 255 );
			wk.set( x, y, pixelBGR( v, v, v ) );
		}
	}
	retainer<file> f = file::initialize( filename, true, true );
	if ( f ) {
		bitmap<pixelBGR>::write( f(), wk );
	}
}
inline void saveBmp( array2<int16> & img, const char * filename )
{
	imageInterface<pixelBGR> wk;
	wk.create( img.w, img.h );
	for ( int y = 0; y < wk.height; ++y ) {
		for ( int x = 0; x < wk.width; ++x ) {
			int v = clamp<int>( 0, img( x, y ), 255 );
			wk.set( x, y, pixelBGR( v, v, v ) );
		}
	}
	retainer<file> f = file::initialize( filename, true, true );
	if ( f ) {
		bitmap<pixelBGR>::write( f(), wk );
	}
}
inline void saveBmp( array2<double> & img, const char * filename )
{
	imageInterface<pixelBGR> wk;
	wk.create( img.w, img.h );
	checkMaximum<double> mx;
	checkMinimum<double> mn;
	for ( int y = 0; y < wk.height; ++y ) {
		for ( int x = 0; x < wk.width; ++x ) {
			mx( img( x, y ) );
			mn( img( x, y ) );
		}
	}
	double width = mx() - mn();
	for ( int y = 0; y < wk.height; ++y ) {
		for ( int x = 0; x < wk.width; ++x ) {
			int v = clamp<int>( 0, ( img( x, y ) - mn() ) / width * 255, 255 );
			wk.set( x, y, pixelBGR( v, v, v ) );
		}
	}
	retainer<file> f = file::initialize( filename, true, true );
	if ( f ) {
		bitmap<pixelBGR>::write( f(), wk );
	}
}
