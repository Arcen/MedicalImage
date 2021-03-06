////////////////////////////////////////////////////////////////////////////////
// 複素数型
// 未完成 BUG DEBUG

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
	//順序を定義
	bool operator<( const complex & v ) const 
	{
		//実数ならば，実数で比較
		if ( absolute( c ) < epsilon && absolute( v.c ) < epsilon ) {
			//両方が実数
			return r < v.r;
		}
		//複素数ならば，ノルムで比較
		if ( absolute( c ) > epsilon && absolute( v.c ) > epsilon ) {
			//両方が複素数
			return norm( *this ) < norm( v );
		}
		//片方だけが複素数ならば，複素数が小
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
	//0に近い要素を０にする
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
				return complex( 0, sqrt( - v.r ) );//+-があるが，正のみとする
			} else {
				return complex( sqrt( v.r ), 0 );//+-があるが，正のみとする
			}
		}
		complex result;
		T r2 = 0.5 * ( v.r + sqrt( v.r * v.r + v.c * v.c ) );//ここでも正負があるが正のみが解を持つ
		result.r = sqrt( r2 );//+-があるが，正のみとする
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
