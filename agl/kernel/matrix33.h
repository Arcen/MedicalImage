////////////////////////////////////////////////////////////////////////////////
// Matrix 33

class matrix44;
class matrix33
{
	struct instance { decimal m00, m01, m02, m10, m11, m12, m20, m21, m22; };
public:
	const static matrix33 identity;
	const static matrix33 zero;
	const static matrix33 one;
	union {
		decimal array[9];
		decimal mat[3][3];
		struct { vector3 x, y, z; };
		struct { decimal m00, m01, m02, m10, m11, m12, m20, m21, m22; };
	};
	matrix33() {}
	matrix33( const matrix22 & m ) { *this = matrix33( m.m00, m.m01, m.m10, m.m11 ); }
	matrix33( const matrix33 & m ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & m ); }
//	matrix33( const matrix44 & m );
	matrix33( const decimal & _m00, const decimal & _m01, const decimal & _m02,
				const decimal & _m10, const decimal & _m11, const decimal & _m12,
				const decimal & _m20, const decimal & _m21, const decimal & _m22 )
	{
		m00 = _m00; m01 = _m01; m02 = _m02;
		m10 = _m10; m11 = _m11; m12 = _m12;
		m20 = _m20; m21 = _m21; m22 = _m22;
	}
	matrix33( const decimal & _m00, const decimal & _m01,
				const decimal & _m10, const decimal & _m11 )
	{
		m00 = _m00; m01 = _m01; m02 = 0;
		m10 = _m10; m11 = _m11; m12 = 0;
		m20 = 0; m21 = 0; m22 = 1;
	}
	matrix33( const vector3 & v0, const vector3 & v1, const vector3 & v2 )
	{
		x = v0; y = v1; z = v2;
	}
	matrix33( const decimal * v ) { *this = *reinterpret_cast<const matrix33*>( v ); }

	matrix33 & operator=( const matrix33 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); return *this; }

	const matrix33 operator-() const { return matrix33( -x, -y, -z ); }
	const matrix33 operator+( const matrix33 & m ) const { return matrix33( x+m.x, y+m.y, z+m.z ); }
	const matrix33 operator-( const matrix33 & m ) const { return matrix33( x-m.x, y-m.y, z-m.z ); }
	void operator+=( const matrix33 & m ) { x+=m.x; y+=m.y; z+=m.z; }
	void operator-=( const matrix33 & m ) { x-=m.x; y-=m.y; z-=m.z; }
	const matrix33 operator+( const decimal & m ) const { return matrix33( x+m, y+m, z+m ); }
	const matrix33 operator-( const decimal & m ) const { return matrix33( x-m, y-m, z-m ); }
	const matrix33 operator*( const decimal & m ) const { return matrix33( x*m, y*m, z*m ); }
	const matrix33 operator/( const decimal & m ) const { return matrix33( x/m, y/m, z/m ); }
	void operator+=( const decimal & m ) { x+=m; y+=m; z+=m; }
	void operator-=( const decimal & m ) { x-=m; y-=m; z-=m; }
	void operator*=( const decimal & m ) { x*=m; y*=m; z*=m; }
	void operator/=( const decimal & m ) { x/=m; y/=m; z/=m; }
	bool operator==( const matrix33 & m ) const { return ( x == m.x && y == m.y && z == m.z ); }
	bool operator!=( const matrix33 & m ) const { return ( x != m.x || y != m.y || z != m.z ); }
	operator decimal * () { return array; }
	operator const decimal * () const { return array; }
	vector3 * vec() { return & x; }
	const vector3 * vec() const { return & x; }
	const matrix33 operator*( const matrix33 & m ) const
	{
		const matrix33 & tm = m.transpose();
		return matrix33( vector3::dot3( x, tm.x ), vector3::dot3( x, tm.y ), vector3::dot3( x, tm.z ), 
						  vector3::dot3( y, tm.x ), vector3::dot3( y, tm.y ), vector3::dot3( y, tm.z ), 
						  vector3::dot3( z, tm.x ), vector3::dot3( z, tm.y ), vector3::dot3( z, tm.z ) );
	}
	void operator*=( const matrix33 & v ) { *this = *this * v; }
private:
	friend class vector3;
	vector3 operator*( const vector3 & v ) const
	{
		return vector3( vector3::dot3( x, v ), vector3::dot3( y, v ), vector3::dot3( z, v ) );
	}
public:
	const matrix33 operator/( const matrix33 & m ) const
	{
		return *this * inverse( m );
	}
	void operator/=( const matrix33 & v ) { *this = *this / v; }
	//転置
	const matrix33 transpose() const { return matrix33( m00, m10, m20, m01, m11, m21, m02, m12, m22 ); }
	static const matrix33 transpose( const matrix33 & m ) { return m.transpose(); }
	//行列値
	decimal determinant() const
	{
		return 
			m00 * ( m11 * m22 - m12 * m21 ) + 
			m01 * ( m12 * m20 - m10 * m22 ) + 
			m02 * ( m10 * m21 - m11 * m20 );
	}
	static decimal determinant( const matrix33 & m ) { return m.determinant(); }
	//小行列
	const matrix22 minor( const int i, const int j ) const
	{
		switch ( j ) {
		case 0:
			switch ( i ) {
			case 0: return matrix22( m11, m12, m21, m22 );
			case 1: return matrix22( m01, m02, m21, m22 );
			case 2: return matrix22( m01, m02, m11, m12 );
			}
			break;
		case 1:
			switch ( i ) {
			case 0: return matrix22( m10, m12, m20, m22 );
			case 1: return matrix22( m00, m02, m20, m22 );
			case 2: return matrix22( m00, m02, m10, m12 );
			}
			break;
		case 2:
			switch ( i ) {
			case 0: return matrix22( m10, m11, m20, m21 );
			case 1: return matrix22( m00, m01, m20, m21 );
			case 2: return matrix22( m00, m01, m10, m11 );
			}
			break;
		}
		assert( 0 );
		return matrix22::identity;
	}
	static matrix22 minor( const matrix33 & m, const int i, const int j ) { return m.minor( i, j ); }
	//余因子
	decimal cofactor( const int i, const int j ) const { return ( ( ( i + j ) & 1 ) ? -1 : 1 ) * ( minor( i, j ) ).determinant(); }
	static decimal cofactor( const matrix33 & m, const int i, const int j ) { return m.cofactor( i, j ); }
	//余因子行列
	const matrix33 adjoint() const { return matrix33( cofactor( 0, 0 ), cofactor( 0, 1 ), cofactor( 0, 2 ), cofactor( 1, 0 ), cofactor( 1, 1 ), cofactor( 1, 2 ), cofactor( 2, 0 ), cofactor( 2, 1 ), cofactor( 2, 2 ) ); }
	static matrix33 adjoint( const matrix33 & m ) { return m.adjoint(); }
	//逆行列
	static const matrix33 inverse( const matrix33 &m )
	{
		decimal d = m.determinant();
		if ( fabs( d ) < epsilon ) return identity;
		return transpose( adjoint( m ) ) / d;
	}
	static const matrix33 normalize( const matrix33 & m )
	{
		decimal l = m.determinant();
		if ( l == 1 ) return m;
		if ( l < epsilon ) return identity;
		return m / l;
	}
	static const matrix33 scale( const vector3 & v )
	{
		return matrix33( v.x, 0, 0,
						0, v.y, 0,
						0, 0, v.z );
	}
	static const matrix33 rotation( const quaternion & q )
	{
		const decimal	x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
		const decimal	xx = q.x * x2, xy = q.x * y2, xz = q.x * z2,
						yy = q.y * y2, yz = q.y * z2, zz = q.z * z2,
						wx = q.w * x2, wy = q.w * y2, wz = q.w * z2;
		return matrix33( 1 - (yy + zz), xy + wz, xz - wy,
						xy - wz, 1 - (xx + zz), yz + wx,
						xz + wy, yz - wx, 1 - (xx + yy) );
	}
	static const quaternion extract( const matrix33 & v )
	{
		quaternion q;
		//行列が直行行列か調べる
		if ( 0 ) {
			const matrix33 vt = transpose( v );
			const matrix33 t1 = v * vt - identity, t2 = vt * v - identity;
			if ( roughEpsilon < dot( t1, t1 ) + dot( t2, t2 ) ) 
				return quaternion::identity;
		}
		// Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
		// article "Quaternion Calculus and Fast Animation".
		const decimal ( & mat )[3][3] = v.mat;
		decimal trace = mat[0][0]+mat[1][1]+mat[2][2];
		if ( trace > 0 ) {
			// |w| > 1/2, may as well choose w > 1/2
			decimal root = sqrt(trace+1);  // 2w
			q.w = 0.5 * root;
			root = 0.5/root;  // 1/(4w)
			q.x = (mat[2][1]-mat[1][2])*root;
			q.y = (mat[0][2]-mat[2][0])*root;
			q.z = (mat[1][0]-mat[0][1])*root;
		} else {
			// |w| <= 1/2
			static int next[3] = { 1, 2, 0 };
			int i = 0;
			if ( mat[1][1] > mat[0][0] ) i = 1;
			if ( mat[2][2] > mat[i][i] ) i = 2;
			int j = next[i];
			int k = next[j];
			decimal root = sqrt(mat[i][i]-mat[j][j]-mat[k][k]+1);//2w
			q.array[i] = 0.5 * root;
			root = 0.5/root;  // 1/(4w)
			q.w		   = (mat[k][j]-mat[j][k])*root;
			q.array[j] = (mat[j][i]+mat[i][j])*root;
			q.array[k] = (mat[k][i]+mat[i][k])*root;
		}
		return q;
	}
	static const matrix33 shear( const vector3 & v )
	{
		return matrix33( 1, 0, 0,
						v.x, 1, 0,
						v.y, v.z, 1 );
	}
	const matrix22 getMatrix22() const { return matrix22( m00, m01, m10, m11 ); }
	static decimal dot( const matrix33 & l, const matrix33 & r )
	{
		decimal result = 0;
		for ( int i = 0; i < 9; ++i ) result += l.array[i] * r.array[i];
		return result;
	}
	//ノルム
	static decimal norm( const matrix33 & m )
	{
		return ::sqrt( dot( m, m ) );
	}
	//自然対数の乗数
	static matrix33 exp( const matrix33 & m )
	{
		matrix33 A = m;
		decimal j = maximum<decimal>( 0, 1 + ::log( norm( A ) ) / log2 );
		A = A * ::pow( 2, j );
		matrix33 D = identity, N = identity, X = identity;
		decimal c = 1;
		const int q = 6;
		for ( int k = 1; k <= q; ++k ) {
			c *= ( q - k + 1 ) / ( k * ( 2 * q + k - 1 ) );
			X = A * X;
			N += X * c;
			D += X * ( k & 1 ? -c : c );
		}
		X = inverse( D ) * N;
		return pow( X, 2 * j );
	}
	static bool symmetric( const matrix33 & m )
	{
		return m.m01 == m.m10 && m.m02 == m.m20 && m.m12 == m.m21;
	}
	//行列の乗数（対角化して、解ければそのまま、そうでないならば、数値解的に）
	static matrix33 pow( const matrix33 & m, decimal n )
	{
		if ( symmetric( m ) ) {
			//対角化して解く
			::array<decimal> eval;
			::array<vector3> evec;
			matrix33 P;
			if ( eigen( m, eval, evec, P ) ) {
				const matrix33 Pt = transpose( P );
				const matrix33 lamda = P * m * transpose( P );
				matrix33 X = identity;
				for ( int i = 0; i < 3; ++i ) {
					X.mat[i][i] = ::pow( lamda.mat[i][i], n );
				}
				return Pt * X * P;
			}
		}
		//対角化ができない場合には、２乗と平方根を用いて制限をつけて計算する

		const int maxNum = 0x400;
		matrix33 A = m;
		if ( n < 0 ) {
			n *= -1;
			A = inverse( A );
		}
		if ( ! between( 1 / decimal( maxNum ), n, decimal( maxNum ) ) ) return identity;//大きい数は省略
		//１以上の乗数
		int over = int( n );
		matrix33 X = identity, N = A;
		while ( over ) {
			if ( over & 1 ) {
				X *= N;
			}
			over /= 2;
			N *= N;
		}
		//１未満の乗数
		N = m;
		double wk;
		int under = int( modf( n, & wk ) * maxNum );
		int flag = maxNum;
		while ( under ) {
			if ( under & flag ) {
				X *= N;
				under ^= flag;
			}
			flag >>= 1;
			N = root2( N );
		}
		return X;
	}
	//行列の対数
	static matrix33 log( const matrix33 & m )
	{
		matrix33 A = m;
		int k = 0;
		while ( norm( A - identity ) > 0.5 ) {
			A = root2( A );
			++k;
		}
		A = identity - A;
		matrix33 Z = A, X = A;
		int i = 1;
		while ( norm( Z ) > roughEpsilon ) {
			Z = Z * A;
			++i;
			X += Z / decimal( i );
		}
		return X * ::pow( static_cast<decimal>( 2.0 ), static_cast<decimal>( k ) );
	}
	//行列の平方根
	static matrix33 root2( const matrix33 & A )
	{
		matrix33 X = A, Y = identity;
		while ( norm( X * X - A ) > roughEpsilon ) {
			matrix33 iX = inverse( X ), iY = inverse( Y );
			X = ( X + iY ) / 2.0;
			Y = ( Y + iX ) / 2.0;
		}
		return X;
	}
	//行の交換
	void swapbra( int i, int j )
	{
		for ( int k = 0; k < 3; ++k ) swap( mat[i][k], mat[j][k] );
	}
	//列の交換
	void swapket( int i, int j )
	{
		for ( int k = 0; k < 3; ++k ) swap( mat[k][i], mat[k][j] );
	}
	//行を別の行に係数を掛けて足す
	void addbra( int from, int to, decimal c )
	{
		for ( int i = 0; i < 3; ++i ) mat[to][i] += c * mat[from][i];
	}
	//要素の絶対値が最大の位置
	bool maximumelement( int & r, int & c, int start_r = 0, int start_c = 0, int end_r = 3, int end_c = 3 )
	{
		checkMaximum<decimal> mx;
		for ( int i = maximum( 0, start_c ); i < minimum( 3, end_r ); ++i ) {
			for ( int j = maximum( 0, start_r ); j < minimum( 3, end_c ); ++j ) {
				mx( fabs( mat[i][j] ), i + j * 3 );
			}
		}
		if ( ! mx ) return false;
		r = mx.sub % 3;
		c = mx.sub / 3;
		return true;
	}
	//行列の階数
	static int rank( const matrix33 & m )
	{
		matrix33 r = m;
		//第１行の最大の要素を対角にもってくる
		int max_r = 0, max_c = 0;
		r.maximumelement( max_r, max_c );
		if ( r.mat[max_r][max_c] ) {
			if ( max_r != 0 ) r.swapbra( 0, max_r );
			if ( max_c != 0 ) r.swapket( 0, max_c );
			//その下の要素を０にするように、加える
			r.addbra( 0, 1, - r.m10 / r.m00 ); r.m10 = 0;
			r.addbra( 0, 2, - r.m20 / r.m00 ); r.m20 = 0;
			//中央の要素を右下からの最大の要素に置き換えて
			int max_r = 0, max_c = 0;
			r.maximumelement( max_r, max_c, 1, 1 );
			if ( r.mat[max_r][max_c] ) {
				if ( max_r != 1 ) r.swapbra( 1, max_r );
				if ( max_c != 1 ) r.swapket( 1, max_c );
				r.addbra( 1, 2, - r.m21 / r.m11 ); r.m21 = 0;
			}
		}
		if ( roughEpsilon < fabs( r.m22 ) ) return 3;
		if ( roughEpsilon < fabs( r.m11 ) ) return 2;
		if ( roughEpsilon < fabs( r.m00 ) ) return 1;
		return 0;
	}
	//固有値から固有ベクトルを取得。
	static bool eigenvector( const matrix33 & m, decimal eval, ::array<vector3> & evec )
	{
		matrix33 X = m - identity * eval;
		int rnk = rank( X );
		switch ( rnk ) {
		case 0://Xはzeroなので、適当な直交基底を設定
			evec.push_back( vector3( 1, 0, 0 ) );
			evec.push_back( vector3( 0, 1, 0 ) );
			evec.push_back( vector3( 0, 0, 1 ) );
			break;
		case 1:
		case 2:
			{//階数の数のベクトルと直交する４－階数個のベクトル
				::array<vector3> nv;
				for ( int i = 0; i < 3; ++i ) {
					if ( X.vec()[i] != vector3::zero ) {
						nv.push_back( vector3::normalize3( X.vec()[i] ) );
					}
				}
				if ( nv.size < rnk ) return false;

				::array<vector3> uv;
				while ( uv.size < rnk ) {
					checkMaximum<decimal> mx;
					for ( int i = 0; i < nv.size; ++i ) {
						checkMinimum<decimal> mn;
						for ( int j = 0; j < uv.size; ++j ) {
							mn( vector3::length3( nv[i] - uv[j] ) );
						}
						if ( mn ) mx( mn(), i );
					}
					if ( mx ) {
						uv.push_back( nv[mx.sub] );
					} else {
						uv.push_back( nv[0] );
					}
				}
				vector3::gramschmidt( uv );
				for ( int i = rnk; i < 3; ++i ) evec.push_back( uv[i] );
			}
			break;
			//逆行列が存在してしまう
		case 3: return false;
		}
		return true;
	}
	//固有値（大きい順にソート）・固有ベクトル（正規化済み）・対角化用行列を返す（正則な場合）。
	static bool eigen( const matrix33 & m, ::array<decimal> & eval, ::array<vector3> & evec, matrix33 & P )
	{
		P = identity;
		// m * x = λ x => ( m - λI ) x = 0 => | m - λI | = 0（固有多項式）
		// ( m00 - λ ) * ( ( m11 - λ ) * ( m22 - λ ) - m12 * m21 ) +
		// m01 * ( m12 * m20 - m10 * ( m22 - λ ) ) + 
		// m02 * ( m10 * m21 - ( m11 - λ ) * m20 ) = 0

		// ( m00 - λ ) * ( m11 - λ ) * ( m22 - λ ) +
		// - m12 * m21 * ( m00 - λ ) + 
		// - m01 * m10 * ( m22 - λ ) + 
		// - m02 * m20 * ( m11 - λ ) +
		// m01 * m12 * m20 + m02 * m10 * m21 = 0

		// -λ^3 + ( m00 + m11 + m22 ) λ^2 - ( m00 m11 + m11 m22 + m22 m00 ) λ + m00 m11 m22
		// m12 m21 λ - m12 m21 m00
		// m01 m10 λ - m01 m10 m22
		// m02 m20 λ - m02 m20 m11
		// m01 m12 m20 + m02 m10 m21 = 0

		// λ^3 + 
		// - ( m00 + m11 + m22 ) λ^2 +
		// ( m00 m11 + m11 m22 + m22 m00 - m12 m21 - m01 m10 - m02 m20 ) λ +
		// m12 m21 m00 + m01 m10 m22 + m02 m20 m11 - m00 m11 m22 - m01 m12 m20 - m02 m10 m21 = 0

		decimal b = - m.m00 - m.m11 - m.m22;
		decimal c = m.m00 * m.m11 + m.m11 * m.m22 + m.m22 * m.m00 
			- m.m12 * m.m21 - m.m01 * m.m10 - m.m02 * m.m20;
		decimal d = m.m12 * m.m21 * m.m00 + m.m01 * m.m10 * m.m22 
			+ m.m02 * m.m20 * m.m11 - m.m00 * m.m11 * m.m22 
			- m.m01 * m.m12 * m.m20 - m.m02 * m.m10 * m.m21;
		equation()( 1, b, c, d, eval );
		for ( int i = 0; i < eval.size; ++i ) {
			if ( ! eigenvector( m, eval[i], evec ) ) {
				return false;
			}
		}
		if ( evec.size == 3 ) P = matrix33( evec[0], evec[1], evec[2] );
		return true;
	}
};
inline const matrix33 operator+( const decimal & s, const matrix33 & v ) { return v + s; }
inline const matrix33 operator-( const decimal & s, const matrix33 & v ) { return - v + s; }
inline const matrix33 operator*( const decimal & s, const matrix33 & v ) { return v * s; }
inline const matrix33 operator/( const decimal & s, const matrix33 & v ) { return matrix33( s/v.x, s/v.y, s/v.z ); }
inline const vector3 vector3::operator*( const matrix33 & m ) const { return m.transpose() * *this; }
#ifdef __GLOBAL__
const matrix33 matrix33::identity( 1, 0, 0, 0, 1, 0, 0, 0, 1 );
const matrix33 matrix33::zero( 0, 0, 0, 0, 0, 0, 0, 0, 0 );
const matrix33 matrix33::one( 1, 1, 1, 1, 1, 1, 1, 1, 1 );
#endif
