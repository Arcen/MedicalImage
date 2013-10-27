////////////////////////////////////////////////////////////////////////////////
// ï°ëfêîå^
// ñ¢äÆê¨ BUG DEBUG

template<typename T>
class complex
{
	T r, c;
public:
	complex() {}
	complex( const T & rl, const T & cmplx = 0 ) : r( rl ), c( cmplx ) {}
	complex( const complex & v ) : r( v.r ), c( v.c ){}
	complex & operator=( const complex & v ){ r = v.r; c = v.c; return *this; }
	const complex operator-() const { return complex( -r, -c ); }
	const complex operator+( const complex & v ) const { return complex( r + v.r, c + v.c ); }
	const complex operator-( const complex & v ) const { return complex( r - v.r, c - v.c ); }
	const complex operator*( const complex & v ) const { return complex( r * v.r - c * v.c, r * v.c + c * v.r ); }
	const complex operator/( const complex & v ) const { return complex( zerodivide( r * v.r + c * v.c, v.r * v.r + v.c * v.c ), zerodivide( c * v.r - r * v.c, v.r * v.r + v.c * v.c ) ); }
	void operator+=( const complex & v ) { r += v.r; c += v.c; }
	void operator-=( const complex & v ) { r -= v.r; c -= v.c; }
	void operator*=( const complex & v ) { *this = *this * v; }
	void operator/=( const complex & v ) { *this = *this / v; }
	const complex operator+( const T & v ) const { return complex( r + v, c ); }
	const complex operator-( const T & v ) const { return complex( r - v, c ); }
	const complex operator*( const T & v ) const { return complex( r * v, c * v ); }
	const complex operator/( const T & v ) const { return complex( zerodivide( r, v ), zerodivide( c, v ) ); }
	void operator+=( const T & v ) { r += v.r; }
	void operator-=( const T & v ) { r -= v.r; }
	void operator*=( const T & v ) { *this = *this * v; }
	void operator/=( const T & v ) { *this = *this / v; }
	bool operator==( const complex & v ) const { return r == v.r && c == v.c; }
	bool operator!=( const complex & v ) const { return ! ( *this == v ); }
	//èáèòÇíËã`
	bool operator<( const complex & v ) const 
	{
		//é¿êîÇ»ÇÁÇŒÅCé¿êîÇ≈î‰är
		if ( absolute( c ) < epsilon && absolute( v.c ) < epsilon ) {
			//óºï˚Ç™é¿êî
			return r < v.r;
		}
		//ï°ëfêîÇ»ÇÁÇŒÅCÉmÉãÉÄÇ≈î‰är
		if ( absolute( c ) > epsilon && absolute( v.c ) > epsilon ) {
			//óºï˚Ç™ï°ëfêî
			return norm( *this ) < norm( v );
		}
		//ï–ï˚ÇæÇØÇ™ï°ëfêîÇ»ÇÁÇŒÅCï°ëfêîÇ™è¨
		return ( absolute( c ) > epsilon );
	}
	bool operator>( const complex & v ) const 
	{
		if ( *this == v ) return false;
		return ! ( *this < v );
	}
	bool operator<=( const complex & v ) const 
	{
		if ( *this == v ) return true;
		return ( *this < v );
	}
	bool operator>=( const complex & v ) const 
	{
		if ( *this == v ) return true;
		return ! ( *this < v );
	}
	const T real() const
	{
		return r;
	}
	const T imaginary() const
	{
		return c;
	}
	bool valid() const
	{
		if ( ! _finite( r ) || _isnan( r ) ) return false;
		if ( ! _finite( c ) || _isnan( c ) ) return false;
		return true;
	}
	//0Ç…ãﬂÇ¢óvëfÇÇOÇ…Ç∑ÇÈ
	static const complex fix( const complex & v )
	{
		complex result( v );
		if ( absolute( v.r ) < epsilon ) v.r = 0;
		if ( absolute( v.c ) < epsilon ) v.c = 0;
		return result;
	}
	const complex conjugate() const { return complex( r, -c ); }
	static const complex dot( const complex & l, const complex & r )
	{
		return l * r.conjugate();
	}
	static const complex square( const complex & v ) { return v * v; }
	static const complex squareroot( const complex & v )
	{
		if ( v.c == 0 ) {
			if ( v.r < 0 ) {
				return complex( 0, sqrt( - v.r ) );//+-Ç™Ç†ÇÈÇ™ÅCê≥ÇÃÇ›Ç∆Ç∑ÇÈ
			} else {
				return complex( sqrt( v.r ), 0 );//+-Ç™Ç†ÇÈÇ™ÅCê≥ÇÃÇ›Ç∆Ç∑ÇÈ
			}
		}
		complex result;
		T r2 = 0.5 * ( v.r + sqrt( v.r * v.r + v.c * v.c ) );//Ç±Ç±Ç≈Ç‡ê≥ïâÇ™Ç†ÇÈÇ™ê≥ÇÃÇ›Ç™âÇéùÇ¬
		result.r = sqrt( r2 );//+-Ç™Ç†ÇÈÇ™ÅCê≥ÇÃÇ›Ç∆Ç∑ÇÈ
		result.c = v.c / 2.0 / result.r;
		return result;
	}
	static T norm( const complex & v ) { return sqrt( dot( v, v ).r ); }
	static const complex normalize( const complex & v )
	{
		T n = norm( v );
		if ( n == 1 ) return v;
		return v / n;
	}
	static void debug( const complex & v, const char * tag = NULL )
	{
		if ( tag ) {
			OutputDebugString( tag );
			OutputDebugString( ":[complex]\n" );
		}
		OutputDebugString( "[ " );
		OutputDebugString( string( float( v.r ) ) );
		if ( v.c ) {
			OutputDebugString( ", " );
			OutputDebugString( string( float( v.c ) ) );
		}
		OutputDebugString( " ]" );
	}
};

template<typename T>
const complex<T> operator+( const T & s, const complex<T> & v ) { return v + s; }
template<typename T>
const complex<T> operator-( const T & s, const complex<T> & v ) { return - v + s; }
template<typename T>
const complex<T> operator*( const T & s, const complex<T> & v ) { return v * s; }
template<typename T>
const complex<T> operator/( const T & s, const complex<T> & v )
{
	return complex<T>( s ) / v;
}
