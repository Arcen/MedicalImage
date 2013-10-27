////////////////////////////////////////////////////////////////////////////////
// Matrix 22

class matrix33;
class matrix44;

class matrix22
{
	struct instance { decimal m00, m01, m10, m11; };
public:
	const static matrix22 identity;
	const static matrix22 zero, one;
	union {
		decimal array[4];
		decimal mat[2][2];
		struct { vector2 x, y; };
		struct { decimal m00, m01, m10, m11; };
	};
	matrix22() {}
	matrix22( const matrix22 & m ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & m ); }
//	matrix22( const matrix33 & m );
//	matrix22( const matrix44 & m );
	matrix22( const decimal & _m00, const decimal & _m01, const decimal & _m10, const decimal & _m11 )
	{
		m00 = _m00; m01 = _m01;
		m10 = _m10; m11 = _m11;
	}
	matrix22( const vector2 & v0, const vector2 & v1 ) { x = v0; y = v1; }
	matrix22( const decimal * v ) { *this = *reinterpret_cast<const matrix22*>( v ); }

	matrix22 & operator=( const matrix22 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); return *this; }

	const matrix22 operator-() const { return matrix22( -x, -y ); }
	const matrix22 operator+( const matrix22 & m ) const { return matrix22( x+m.x, y+m.y ); }
	const matrix22 operator-( const matrix22 & m ) const { return matrix22( x-m.x, y-m.y ); }
	void operator+=( const matrix22 & m ) { x+=m.x; y+=m.y; }
	void operator-=( const matrix22 & m ) { x-=m.x; y-=m.y; }
	const matrix22 operator+( const decimal & m ) const { return matrix22( x+m, y+m ); }
	const matrix22 operator-( const decimal & m ) const { return matrix22( x-m, y-m ); }
	const matrix22 operator*( const decimal & m ) const { return matrix22( x*m, y*m ); }
	const matrix22 operator/( const decimal & m ) const { return matrix22( x/m, y/m ); }
	void operator+=( const decimal & m ) { x+=m; y+=m; }
	void operator-=( const decimal & m ) { x-=m; y-=m; }
	void operator*=( const decimal & m ) { x*=m; y*=m; }
	void operator/=( const decimal & m ) { x/=m; y/=m; }
	bool operator==( const matrix22 & m ) const { return ( x == m.x && y == m.y ); }
	bool operator!=( const matrix22 & m ) const { return ( x != m.x || y != m.y ); }
	operator decimal * () { return array; }
	operator const decimal * () const { return array; }
	vector2 * vec() { return & x; }
	const vector2 * vec() const { return & x; }

	const matrix22 operator*( const matrix22 & m ) const
	{
		const matrix22 & tm = m.transpose();
		return matrix22( vector2::dot2( x, tm.x ), vector2::dot2( x, tm.y ), 
						  vector2::dot2( y, tm.x ), vector2::dot2( y, tm.y ) );
	}
	void operator*=( const matrix22 & v ) { *this = *this * v; }
private:
	friend class vector2;
	vector2 operator*( const vector2 & v ) const
	{
		return vector2( vector2::dot2( x, v ), vector2::dot2( y, v ) );
	}
public:
	const matrix22 operator/( const matrix22 & m ) const
	{
		return *this * inverse( m );
	}
	void operator/=( const matrix22 & v ) { *this = *this / v; }

	static const matrix22 rotation( const decimal & angle )
	{
		const decimal c = cos( angle ), s = sin( angle );
		return matrix22( c, s, -s, c );
	}
	//�]�u
	const matrix22 transpose() const { return matrix22( m00, m10, m01, m11 ); }
	static const matrix22 transpose( const matrix22 & m ) { return m.transpose(); }
	//�s��l
	decimal determinant() const
	{
		return m00 * m11 - m10 * m01;
	}
	static decimal determinant( const matrix22 & m ) { return m.determinant(); }
	//���s��
	decimal minor( const int i, const int j ) const
	{
		switch ( j ) {
		case 0:
			switch ( i ) {
			case 0: return m11;
			case 1: return m01;
			}
			break;
		case 1:
			switch ( i ) {
			case 0: return m10;
			case 1: return m00;
			}
			break;
		}
		assert( 0 );
		return 0;
	}
	static decimal minor( const matrix22 & m, const int i, const int j ) { return m.minor( i, j ); }
	//�]���q
	decimal cofactor( const int i, const int j ) const { return ( ( ( i + j ) & 1 ) ? -1 : 1 ) * minor( i, j ); }
	static decimal cofactor( const matrix22 & m, const int i, const int j ) { return m.cofactor( i, j ); }
	//�]���q�s��
	const matrix22 adjoint() const { return matrix22( cofactor( 0, 0 ), cofactor( 0, 1 ), cofactor( 1, 0 ), cofactor( 1, 1 ) ); }
	static matrix22 adjoint( const matrix22 & m ) { return m.adjoint(); }
	//�t�s��
	static const matrix22 inverse( const matrix22 &m )
	{
		decimal d = m.determinant();
		if ( fabs( d ) < epsilon ) return identity;
		return transpose( adjoint( m ) ) / d;
	}
	static const matrix22 normalize( const matrix22 & m )
	{
		decimal l = m.determinant();
		if ( l == 1 ) return m;
		if ( l < epsilon ) return identity;
		return m / l;
	}
	//����
	static decimal dot( const matrix22 & l, const matrix22 & r )
	{
		decimal result = 0;
		for ( int i = 0; i < 4; ++i ) result += l.array[i] * r.array[i];
		return result;
	}
	//�m����
	static decimal norm( const matrix22 & m )
	{
		return ::sqrt( dot( m, m ) );
	}
	//���R�ΐ��̏搔
	static matrix22 exp( const matrix22 & m )
	{
		matrix22 A = m;
		decimal j = maximum<decimal>( 0, 1 + ::log( norm( A ) ) / log2 );
		A = A * ::pow( 2, j );
		matrix22 D = identity, N = identity, X = identity;
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
	static bool symmetric( const matrix22 & m )
	{
		return m.m01 == m.m10;
	}
	//�s��̏搔�i�Ίp�����āA������΂��̂܂܁A�����łȂ��Ȃ�΁A���l��I�Ɂj
	static matrix22 pow( const matrix22 & m, decimal n )
	{
		if ( symmetric( m ) ) {
			//�Ίp�����ĉ���
			::array<decimal> eval;
			::array<vector2> evec;
			matrix22 P;
			if ( eigen( m, eval, evec, P ) ) {
				const matrix22 Pt = transpose( P );
				const matrix22 lamda = P * m * transpose( P );
				matrix22 X = identity;
				for ( int i = 0; i < 2; ++i ) {
					X.mat[i][i] = ::pow( lamda.mat[i][i], n );
				}
				return Pt * X * P;
			}
		}
		//�Ίp�����ł��Ȃ��ꍇ�ɂ́A�Q��ƕ�������p���Đ��������Čv�Z����

		const int maxNum = 0x400;
		matrix22 A = m;
		if ( n < 0 ) {
			n *= -1;
			A = inverse( A );
		}
		if ( ! between( 1 / decimal( maxNum ), n, decimal( maxNum ) ) ) return identity;//�傫�����͏ȗ�
		//�P�ȏ�̏搔
		int over = int( n );
		matrix22 X = identity, N = A;
		while ( over ) {
			if ( over & 1 ) {
				X *= N;
			}
			over /= 2;
			N *= N;
		}
		//�P�����̏搔
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
	//�s��̑ΐ�
	static matrix22 log( const matrix22 & m )
	{
		matrix22 A = m;
		int k = 0;
		while ( norm( A - identity ) > 0.5 ) {
			A = root2( A );
			++k;
		}
		A = identity - A;
		matrix22 Z = A, X = A;
		int i = 1;
		while ( norm( Z ) > roughEpsilon ) {
			Z = Z * A;
			++i;
			X += Z / decimal( i );
		}
		return X * ::pow( static_cast<decimal>( 2.0 ), static_cast<decimal>( k ) );
	}
	//�s��̕�����
	static matrix22 root2( const matrix22 & A )
	{
		matrix22 X = A, Y = identity;
		while ( norm( X * X - A ) > roughEpsilon ) {
			matrix22 iX = inverse( X ), iY = inverse( Y );
			X = ( X + iY ) / 2.0;
			Y = ( Y + iX ) / 2.0;
		}
		return X;
	}
	//�s�̌���
	void swapbra( int i, int j )
	{
		for ( int k = 0; k < 2; ++k ) swap( mat[i][k], mat[j][k] );
	}
	//��̌���
	void swapket( int i, int j )
	{
		for ( int k = 0; k < 2; ++k ) swap( mat[k][i], mat[k][j] );
	}
	//�s��ʂ̍s�ɌW�����|���đ���
	void addbra( int from, int to, decimal c )
	{
		for ( int i = 0; i < 2; ++i ) mat[to][i] += c * mat[from][i];
	}
	//�v�f�̐�Βl���ő�̈ʒu
	bool maximumelement( int & r, int & c, int start_r = 0, int start_c = 0, int end_r = 2, int end_c = 2 )
	{
		checkMaximum<decimal> mx;
		for ( int i = maximum( 0, start_c ); i < minimum( 2, end_r ); ++i ) {
			for ( int j = maximum( 0, start_r ); j < minimum( 2, end_c ); ++j ) {
				mx( fabs( mat[i][j] ), i + j * 2 );
			}
		}
		if ( ! mx ) return false;
		r = mx.sub % 2;
		c = mx.sub / 2;
		return true;
	}
	//�s��̊K��
	static int rank( const matrix22 & m )
	{
		matrix22 r = m;
		int max_r = 0, max_c = 0;
		r.maximumelement( max_r, max_c );
		if ( r.mat[max_r][max_c] ) {
			if ( max_r != 0 ) r.swapbra( 0, max_r );
			if ( max_c != 0 ) r.swapket( 0, max_c );
			//���̉��̗v�f���O�ɂ���悤�ɁA������
			r.addbra( 0, 1, - r.m10 / r.m00 ); r.m10 = 0;
		}
		if ( roughEpsilon < fabs( r.m11 ) ) return 2;
		if ( roughEpsilon < fabs( r.m00 ) ) return 1;
		return 0;
	}
	//�ŗL�l����ŗL�x�N�g�����擾�B
	static bool eigenvector( const matrix22 & m, decimal eval, ::array<vector2> & evec )
	{
		matrix22 X = m - identity * eval;
		switch ( rank( X ) ) {
		case 0:
			evec.push_back( vector2( 1, 0 ) );
			evec.push_back( vector2( 0, 1 ) );
			break;
		case 1:
			{
				vector2 l = ( vector2::length2( X.y ) < vector2::length2( X.x ) ? X.x : X.y );
				evec.push_back( vector2::normalize2( vector2( l.y, -l.x ) ) );
			}
			break;
		case 2: return false;
		}
		return true;
	}
	//�ŗL�l�i�傫�����Ƀ\�[�g�j�E�ŗL�x�N�g���i���K���ς݁j�E�Ίp���p�s���Ԃ��i�����ȏꍇ�j�B
	static bool eigen( const matrix22 & m, ::array<decimal> & eval, ::array<vector2> & evec, matrix22 & P )
	{
		P = identity;
		// m * x = �� x => ( m - ��I ) x = 0 => | m - ��I | = 0�i�ŗL�������j
		// ( m00 - �� ) * ( m11 - �� ) - m10 * m01 = 0
		// ��^2 + ( - m00 - m11 ) �� + m00 * m11 - m10 * m01 = 0
		decimal b = - m.m00 - m.m11, c = determinant( m );
		equation()( 1, b, c, eval );
		for ( int i = 0; i < eval.size; ++i ) {
			if ( ! eigenvector( m, eval[i], evec ) ) {
				return false;
			}
		}
		if ( evec.size == 2 ) P = matrix22( evec[0], evec[1] );
		return true;
	}
};
inline const matrix22 operator+( const decimal & s, const matrix22 & v ) { return v + s; }
inline const matrix22 operator-( const decimal & s, const matrix22 & v ) { return - v + s; }
inline const matrix22 operator*( const decimal & s, const matrix22 & v ) { return v * s; }
inline const matrix22 operator/( const decimal & s, const matrix22 & v ) { return matrix22( s/v.x, s/v.y ); }
inline const vector2 vector2::operator*( const matrix22 & m ) const { return m.transpose() * *this; }

#ifdef __GLOBAL__
const matrix22 matrix22::identity( 1, 0, 0, 1 );
const matrix22 matrix22::zero( 0, 0, 0, 0 );
const matrix22 matrix22::one( 1, 1, 1, 1 );
#endif

