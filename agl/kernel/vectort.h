////////////////////////////////////////////////////////////////////////////////
// Vector 型と要素数のテンプレート版（数値計算用のため精度と次元が必要で無いときには用いない）
// 未完成 BUG DEBUG

template<typename T>
class matrixT;

template<typename T>
class vectorT
{
	array<T> vec;
	void allocate( int N ){ vec.resize( N, 0 ); }
	friend class matrixT<T>;
public:
	vectorT() {}
	vectorT( const int dim ) { allocate( dim ); zero(); }
	vectorT( const vectorT & v ) : vec( v.vec ){}
	vectorT & operator=( const vectorT & v ){ vec = v.vec; return *this; }
	void input( const vectorC<T> & v ) { *this = vectorT( v.dimension() ); for ( int i = 0; i < dimension(); ++i ) vec[i] = v[i].real(); }
	void output( vectorC<T> & v ) const { v = vectorC<T>( dimension() ); for ( int i = 0; i < dimension(); ++i ) v[i] = vec[i]; }
	int dimension() const { return vec.size; }
	const vectorT operator-() const { vectorT r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = - vec[i]; return r; }
	const vectorT operator+( const vectorT & v ) const { vectorT r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] + v[i]; return r; }
	const vectorT operator-( const vectorT & v ) const { vectorT r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] - v[i]; return r; }
	const vectorT operator*( const vectorT & v ) const { vectorT r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] * v[i]; return r; }
	const vectorT operator/( const vectorT & v ) const { vectorT r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = zerodivide( vec[i], v[i] ); return r; }
	void operator+=( const vectorT & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] += v[i]; }
	void operator-=( const vectorT & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] -= v[i]; }
	void operator*=( const vectorT & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] *= v[i]; }
	void operator/=( const vectorT & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] = zerodivide( vec[i], v[i] ); }
	const vectorT operator+( const T & v ) const { vectorT r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] + v; return r; }
	const vectorT operator-( const T & v ) const { vectorT r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] - v; return r; }
	const vectorT operator*( const T & v ) const { vectorT r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] * v; return r; }
	const vectorT operator/( const T & v ) const { vectorT r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = zerodivide( vec[i], v ); return r; }
	void operator+=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] += v; }
	void operator-=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] -= v; }
	void operator*=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] *= v; }
	void operator/=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] = zerodivide( vec[i], v ); }
	bool operator==( const vectorT & v ) const { if ( dimension() != v.dimension() ) return false; for ( int i = 0; i < dimension(); ++i ) if ( vec[i] != v[i] ) return false; return true; }
	bool operator!=( const vectorT & v ) const { return ! ( *this == v ); }
	const T & operator[]( int i ) const { return vec[i]; }
	T & operator[]( int i ) { return vec[i]; }
	static const vectorT zero( int N ) { vectorT r( N ); r.zero(); return r; }
	static const vectorT one( int N ) { vectorT r( N ); r.one(); return r; }
	static const vectorT basis( int N, int index ) { vectorT r( N ); r.basis( index ); return r; }
	void zero() { for ( int i = 0; i < dimension(); ++i ) { vec[i] = 0; } }
	void one() { for ( int i = 0; i < dimension(); ++i ) { vec[i] = 1; } }
	void basis( int index ) { for ( int i = 0; i < dimension(); ++i ) { vec[i] = ( i == index ? 1 : 0 ); } }
	void identity() { zero(); }
	bool valid() const
	{
		for ( int i = 0; i < dimension(); ++i ) {
			if ( ! _finite( vec[i] ) || _isnan( vec[i] ) ) return false;
		}
		return true;
	}
	//0に近い要素を０にする
	static const vectorT fix( const vectorT & v )
	{
		vectorT result( v );
		for ( int i = 0; i < v.dimension(); ++i ) {
			if ( absolute( result[i] ) < epsilon ) result[i] = 0;
		}
		return result;
	}
	bool same() const
	{
		for ( int i = 1; i < dimension(); ++i ) if ( vec[0] != vec[i] ) return false;
		return true;
	}
	static const T dot( const vectorT & l, const vectorT & r )
	{
		T sum = 0;
		for ( int i = 0; i < l.dimension() && i < r.dimension(); ++i ) {
			sum += l[i] * r[i];
		}
		return sum;
	}
	static const T norm( const vectorT & v ) { return sqrt( dot( v, v ) ); }
	static const vectorT normalize( const vectorT & v )
	{
		T l = norm( v );
		if ( l == 1 ) return v;
		return v / l;
	}
	static void debug( const vectorT & v, const char * tag = NULL )
	{
		if ( tag ) {
			OutputDebugString( tag );
			OutputDebugString( ":[vector]\n" );
		}
		OutputDebugString( "[ " );
		for ( int i = 0; i < v.dimension(); ++i ) {
			if ( i ) OutputDebugString( ", " );
			OutputDebugString( string( float( v[i] ) ) );
		}
		OutputDebugString( " ]\n" );
	}
};

template<typename T>
const vectorT<T> operator+( const T & s, const vectorT<T> & v ) { return v + s; }
template<typename T>
const vectorT<T> operator-( const T & s, const vectorT<T> & v ) { return - v + s; }
template<typename T>
const vectorT<T> operator*( const T & s, const vectorT<T> & v ) { return v * s; }
template<typename T>
const vectorT<T> operator/( const T & s, const vectorT<T> & v )
{
	vectorT<T> r( v.dimension() );
	for ( int i = 0; i < v.dimension(); ++i ) r[i] = zerodivide( s, v[i] );
	return r;
}
