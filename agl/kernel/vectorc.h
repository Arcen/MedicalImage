////////////////////////////////////////////////////////////////////////////////
// Vector 型と要素数のテンプレート版（数値計算用のため精度と次元が必要で無いときには用いない）
// 未完成 BUG DEBUG

template<typename T>
class matrixC;

template<typename T>
class vectorC
{
	array< complex<T> > vec;
	void allocate( int N ){ vec.resize( N, 0 ); }
	friend class matrixC<T>;
public:
	vectorC() {}
	vectorC( const int dim ) { allocate( dim ); zero(); }
	vectorC( const vectorC & v ) : vec( v.vec ){}
	vectorC & operator=( const vectorC & v ){ vec = v.vec; return *this; }
	int dimension() const { return vec.size; }
	const vectorC operator-() const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = - vec[i]; return r; }
	const vectorC operator+( const vectorC & v ) const { vectorC r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] + v[i]; return r; }
	const vectorC operator-( const vectorC & v ) const { vectorC r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] - v[i]; return r; }
	const vectorC operator*( const vectorC & v ) const { vectorC r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] * v[i]; return r; }
	const vectorC operator/( const vectorC & v ) const { vectorC r( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] / v[i]; return r; }
	void operator+=( const vectorC & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] += v[i]; }
	void operator-=( const vectorC & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] -= v[i]; }
	void operator*=( const vectorC & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] *= v[i]; }
	void operator/=( const vectorC & v ) { allocate( minimum( dimension(), v.dimension() ) ); for ( int i = 0; i < dimension(); ++i ) vec[i] /= v[i]; }
	const vectorC operator+( const complex<T> & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] + v; return r; }
	const vectorC operator-( const complex<T> & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] - v; return r; }
	const vectorC operator*( const complex<T> & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] * v; return r; }
	const vectorC operator/( const complex<T> & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] / v; return r; }
	void operator+=( const complex<T> & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] += v; }
	void operator-=( const complex<T> & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] -= v; }
	void operator*=( const complex<T> & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] *= v; }
	void operator/=( const complex<T> & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] /= v; }
	const vectorC operator+( const T & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] + v; return r; }
	const vectorC operator-( const T & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] - v; return r; }
	const vectorC operator*( const T & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] * v; return r; }
	const vectorC operator/( const T & v ) const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i] / v; return r; }
	void operator+=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] += v; }
	void operator-=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] -= v; }
	void operator*=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] *= v; }
	void operator/=( const T & v ) { for ( int i = 0; i < dimension(); ++i ) vec[i] /= v; }
	bool operator==( const vectorC & v ) const { if ( dimension() != v.dimension() ) return false; for ( int i = 0; i < dimension(); ++i ) if ( vec[i] != v[i] ) return false; return true; }
	bool operator!=( const vectorC & v ) const { return ! ( *this == v ); }
	const complex<T> & operator[]( int i ) const { return vec[i]; }
	complex<T> & operator[]( int i ) { return vec[i]; }
	const vectorC conjugate() const { vectorC r( dimension() ); for ( int i = 0; i < r.dimension(); ++i ) r[i] = vec[i].conjugate(); return r; }
	static const vectorC zero( int N ) { vectorC r( N ); r.zero(); return r; }
	static const vectorC one( int N ) { vectorC r( N ); r.one(); return r; }
	static const vectorC basis( int N, int index ) { vectorC r( N ); r.basis( index ); return r; }
	void zero() { for ( int i = 0; i < dimension(); ++i ) { vec[i] = 0; } }
	void one() { for ( int i = 0; i < dimension(); ++i ) { vec[i] = 1; } }
	void basis( int index ) { for ( int i = 0; i < dimension(); ++i ) { vec[i] = ( i == index ? 1 : 0 ); } }
	void identity() { zero(); }
	bool valid() const
	{
		for ( int i = 0; i < dimension(); ++i ) {
			if ( ! vec[i].valid() ) return false;
		}
		return true;
	}
	//0に近い要素を０にする
	static const vectorC fix( const vectorC & v )
	{
		vectorC result( v );
		for ( int i = 0; i < v.dimension(); ++i ) {
			if ( complex<T>::norm( result[i] ) < epsilon ) result[i] = 0;
		}
		return result;
	}
	bool same() const
	{
		for ( int i = 1; i < dimension(); ++i ) if ( vec[0] != vec[i] ) return false;
		return true;
	}
	static const complex<T> dot( const vectorC & l, const vectorC & r )
	{
		complex<T> sum = 0;
		for ( int i = 0; i < l.dimension() && i < r.dimension(); ++i ) {
			sum += l[i] * r[i].conjugate();
		}
		return sum;
	}
	static const T norm( const vectorC & v ) { return sqrt( dot( v, v ).real() ); }
	static const vectorC normalize( const vectorC & v )
	{
		T l = norm( v );
		if ( l == 1 ) return v;
		return v / l;
	}
	static void debug( const vectorC & v, const char * tag = NULL )
	{
		if ( tag ) {
			OutputDebugString( tag );
			OutputDebugString( ":[vector]\n" );
		}
		OutputDebugString( "[ " );
		for ( int i = 0; i < v.dimension(); ++i ) {
			if ( i ) OutputDebugString( ", " );
			complex<T>::debug( v[i] );
		}
		OutputDebugString( " ]\n" );
	}
};

template<typename T>
const vectorC<T> operator+( const complex<T> & s, const vectorC<T> & v ) { return v + s; }
template<typename T>
const vectorC<T> operator-( const complex<T> & s, const vectorC<T> & v ) { return - v + s; }
template<typename T>
const vectorC<T> operator*( const complex<T> & s, const vectorC<T> & v ) { return v * s; }
template<typename T>
const vectorC<T> operator/( const complex<T> & s, const vectorC<T> & v )
{
	vectorC<T> r( v.dimension() );
	for ( int i = 0; i < v.dimension(); ++i ) r[i] = s / v[i];
	return r;
}
