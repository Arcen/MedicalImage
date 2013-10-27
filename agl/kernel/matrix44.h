////////////////////////////////////////////////////////////////////////////////
// Matrix 44

class matrix44
{
	struct instance { decimal m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33; };
public:
	const static matrix44 identity;
	const static matrix44 one;
	const static matrix44 zero;
	union {
		decimal array[16];
		decimal mat[4][4];
		struct { vector4 x, y, z, w; };
		struct { decimal m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33; };
	};
	matrix44() {}
	matrix44( const matrix22 & m )
	{
		*this = matrix44( m.m00, m.m01, 
						   m.m10, m.m11 );
	}
	matrix44( const matrix33 & m )
	{
		*this = matrix44( m.m00, m.m01, m.m02, 
						   m.m10, m.m11, m.m12, 
						   m.m20, m.m21, m.m22 );
	}
	matrix44( const matrix44 & m ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & m ); }
	matrix44( const decimal & _m00, const decimal & _m01, const decimal & _m02, const decimal & _m03,
			   const decimal & _m10, const decimal & _m11, const decimal & _m12, const decimal & _m13,
			   const decimal & _m20, const decimal & _m21, const decimal & _m22, const decimal & _m23,
			   const decimal & _m30, const decimal & _m31, const decimal & _m32, const decimal & _m33 )
	{
		m00 = _m00; m01 = _m01; m02 = _m02; m03 = _m03;
		m10 = _m10; m11 = _m11; m12 = _m12; m13 = _m13;
		m20 = _m20; m21 = _m21; m22 = _m22; m23 = _m23;
		m30 = _m30; m31 = _m31; m32 = _m32; m33 = _m33;
	}
	matrix44( const decimal & _m00, const decimal & _m01, const decimal & _m02,
			   const decimal & _m10, const decimal & _m11, const decimal & _m12,
			   const decimal & _m20, const decimal & _m21, const decimal & _m22 )
	{
		m00 = _m00; m01 = _m01; m02 = _m02; m03 = 0;
		m10 = _m10; m11 = _m11; m12 = _m12; m13 = 0;
		m20 = _m20; m21 = _m21; m22 = _m22; m23 = 0;
		m30 = 0; m31 = 0; m32 = 0; m33 = 1;
	}
	matrix44( const decimal & _m00, const decimal & _m01,
			   const decimal & _m10, const decimal & _m11 )
	{
		m00 = _m00; m01 = _m01; m02 = 0; m03 = 0;
		m10 = _m10; m11 = _m11; m12 = 0; m13 = 0;
		m20 = 0; m21 = 0; m22 = 1; m23 = 0;
		m30 = 0; m31 = 0; m32 = 0; m33 = 1;
	}
	matrix44( const vector4 & v0, const vector4 & v1, const vector4 & v2, const vector4 & v3 )
	{
		x = v0; y = v1; z = v2; w = v3;
	}
	matrix44( const decimal * m ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( m ); }

	matrix44 & operator=( const matrix44 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); return *this; }

	const matrix44 operator-() const { return matrix44( -x, -y, -z, -w ); }
	const matrix44 operator+( const matrix44 & m ) const { return matrix44( x+m.x, y+m.y, z+m.z, w+m.w ); }
	const matrix44 operator-( const matrix44 & m ) const { return matrix44( x-m.x, y-m.y, z-m.z, w-m.w ); }
	void operator+=( const matrix44 & m ) { x+=m.x; y+=m.y; z+=m.z; w+=m.w; }
	void operator-=( const matrix44 & m ) { x-=m.x; y-=m.y; z-=m.z; w-=m.w; }
	const matrix44 operator+( const decimal & m ) const { return matrix44( x+m, y+m, z+m, w+m ); }
	const matrix44 operator-( const decimal & m ) const { return matrix44( x-m, y-m, z-m, w-m ); }
	const matrix44 operator*( const decimal & m ) const { return matrix44( x*m, y*m, z*m, w*m ); }
	const matrix44 operator/( const decimal & m ) const { return matrix44( x/m, y/m, z/m, w/m ); }
	void operator+=( const decimal & m ) { x+=m; y+=m; z+=m; w+=m; }
	void operator-=( const decimal & m ) { x-=m; y-=m; z-=m; w-=m; }
	void operator*=( const decimal & m ) { x*=m; y*=m; z*=m; w*=m; }
	void operator/=( const decimal & m ) { x/=m; y/=m; z/=m; w/=m; }
	bool operator==( const matrix44 & m ) const { return ( x == m.x && y == m.y && z == m.z && w == m.w ); }
	bool operator!=( const matrix44 & m ) const { return ( x != m.x || y != m.y || z != m.z || w != m.w ); }
	operator decimal * () { return array; }
	operator const decimal * () const { return array; }
	vector4 * vec() { return & x; }
	const vector4 * vec() const { return & x; }

	const matrix44 operator*( const matrix44 & m ) const
	{
		const matrix44 & tm = m.transpose();
		return matrix44( vector4::dot4( x, tm.x ), vector4::dot4( x, tm.y ), vector4::dot4( x, tm.z ), vector4::dot4( x, tm.w ), 
						  vector4::dot4( y, tm.x ), vector4::dot4( y, tm.y ), vector4::dot4( y, tm.z ), vector4::dot4( y, tm.w ), 
						  vector4::dot4( z, tm.x ), vector4::dot4( z, tm.y ), vector4::dot4( z, tm.z ), vector4::dot4( z, tm.w ), 
						  vector4::dot4( w, tm.x ), vector4::dot4( w, tm.y ), vector4::dot4( w, tm.z ), vector4::dot4( w, tm.w ) );
	}
	void operator*=( const matrix44 & v ) { *this = *this * v; }
private:
	friend class vector4;
	vector4 operator*( const vector4 & v ) const
	{
		return vector4( vector4::dot4( x, v ), vector4::dot4( y, v ), vector4::dot4( z, v ), vector4::dot4( w, v ) );
	}
public:
	const matrix44 operator/( const matrix44 & m ) const
	{
		return *this * inverse( m );
	}
	void operator/=( const matrix44 & v ) { *this = *this / v; }

	//�]�u
	const matrix44 transpose() const { return matrix44( m00, m10, m20, m30, m01, m11, m21, m31, m02, m12, m22, m32, m03, m13, m23, m33 ); }
	static const matrix44 transpose( const matrix44 & m ) { return m.transpose(); }
	//�s��l
	decimal determinant() const
	{
		decimal result = 0;
		if ( m03 ) result -= m03 * minor( 0, 3 ).determinant();
		if ( m13 ) result += m13 * minor( 1, 3 ).determinant();
		if ( m23 ) result -= m23 * minor( 2, 3 ).determinant();
		if ( m33 ) result += m33 * minor( 3, 3 ).determinant();
		return result;
	}
	static decimal determinant( const matrix44 & m ) { return m.determinant(); }
	//���s��
	const matrix33 minor( const int i, const int j ) const
	{
		switch ( j ) {
		case 0:
			switch ( i ) {
			case 0: return matrix33( m11, m12, m13, m21, m22, m23, m31, m32, m33 );
			case 1: return matrix33( m01, m02, m03, m21, m22, m23, m31, m32, m33 );
			case 2: return matrix33( m01, m02, m03, m11, m12, m13, m31, m32, m33 );
			case 3: return matrix33( m01, m02, m03, m11, m12, m13, m21, m22, m23 );
			}
			break;
		case 1:
			switch ( i ) {
			case 0: return matrix33( m10, m12, m13, m20, m22, m23, m30, m32, m33 );
			case 1: return matrix33( m00, m02, m03, m20, m22, m23, m30, m32, m33 );
			case 2: return matrix33( m00, m02, m03, m10, m12, m13, m30, m32, m33 );
			case 3: return matrix33( m00, m02, m03, m10, m12, m13, m20, m22, m23 );
			}
			break;
		case 2:
			switch ( i ) {
			case 0: return matrix33( m10, m11, m13, m20, m21, m23, m30, m31, m33 );
			case 1: return matrix33( m00, m01, m03, m20, m21, m23, m30, m31, m33 );
			case 2: return matrix33( m00, m01, m03, m10, m11, m13, m30, m31, m33 );
			case 3: return matrix33( m00, m01, m03, m10, m11, m13, m20, m21, m23 );
			}
			break;
		case 3:
			switch ( i ) {
			case 0: return matrix33( m10, m11, m12, m20, m21, m22, m30, m31, m32 );
			case 1: return matrix33( m00, m01, m02, m20, m21, m22, m30, m31, m32 );
			case 2: return matrix33( m00, m01, m02, m10, m11, m12, m30, m31, m32 );
			case 3: return matrix33( m00, m01, m02, m10, m11, m12, m20, m21, m22 );
			}
			break;
		}
		assert( 0 );
		return matrix33::identity;
	}
	static matrix33 minor( const matrix44 & m, const int i, const int j ) { return m.minor( i, j ); }
	//�]���q
	decimal cofactor( const int i, const int j ) const { return ( ( ( i + j ) & 1 ) ? -1 : 1 ) * ( minor( i, j ) ).determinant(); }
	static decimal cofactor( const matrix44 & m, const int i, const int j ) { return m.cofactor( i, j ); }
	//�]���q�s��
	const matrix44 adjoint() const { return matrix44( cofactor( 0, 0 ), cofactor( 0, 1 ), cofactor( 0, 2 ), cofactor( 0, 3 ), cofactor( 1, 0 ), cofactor( 1, 1 ), cofactor( 1, 2 ), cofactor( 1, 3 ), cofactor( 2, 0 ), cofactor( 2, 1 ), cofactor( 2, 2 ), cofactor( 2, 3 ), cofactor( 3, 0 ), cofactor( 3, 1 ), cofactor( 3, 2 ), cofactor( 3, 3 ) ); }
	static matrix44 adjoint( const matrix44 & m ) { return m.adjoint(); }
	//�t�s��
	static const matrix44 inverse( const matrix44 &m )
	{
		decimal d = determinant( m );
		if ( fabs( d ) < epsilon ) return identity;
		return transpose( adjoint( m ) ) / d;
	}
	static const matrix44 normalize4( const matrix44 & m ) 
	{
		decimal l = m.determinant();
		if ( l == 1 ) return m;
		if ( l < epsilon ) return identity;
		return m / l;
	}
	const vector3 translation() const
	{
		return vector3( m30, m31, m32 );
	}
	static const matrix44 translate( const vector3 & v )
	{
		return matrix44( 1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						v.x, v.y, v.z, 1 );
	}
	static const matrix44 scale( const vector3 & v ) { return matrix33::scale( v ); }
	static const matrix44 rotation( const quaternion & q ) { return matrix33::rotation( q ); }
	static const matrix44 rotation( const decimal & angle )
	{
		return matrix33::rotation( quaternion( vector3( 0, 0, 1 ), angle ) );
	}
	static const matrix44 shear( const vector3 & v ) { return matrix33::shear( v ); }
	static const matrix44 removeOuter( const matrix44 & m )
	{
		return matrix44( m.m00, m.m01, m.m02, 0, m.m10, m.m11, m.m12, 0, m.m20, m.m21, m.m22, 0, 0, 0, 0, 1 );
	}
	const matrix22 getMatrix22() const { return matrix22( m00, m01, m10, m11 ); }
	const matrix33 getMatrix33() const { return matrix33( m00, m01, m02, m10, m11, m12, m20, m21, m22 ); }
	static decimal dot( const matrix44 & l, const matrix44 & r )
	{
		decimal result = 0;
		for ( int i = 0; i < 16; ++i ) result += l.array[i] * r.array[i];
		return result;
	}
	//�m����
	static decimal norm( const matrix44 & m )
	{
		return ::sqrt( dot( m, m ) );
	}
	//���R�ΐ��̏搔
	static matrix44 exp( const matrix44 & m )
	{
		matrix44 A = m;
		int j = maximum<int>( 0, 1 + ::floor( ::log( norm( A ) ) / log2 ) );
		A = A * ::pow( static_cast<decimal>( 2.0 ), static_cast<decimal>( j ) );
		matrix44 D = identity, N = identity, X = identity;
		decimal c = 1;
		const int q = 6;
		for ( int k = 1; k <= q; ++k ) {
			c *= ( q - k + 1 ) / ( k * ( 2 * q - k + 1 ) );
			X = A * X;
			N += X * c;
			D += X * ( k & 1 ? -c : c );
		}
		X = inverse( D ) * N;
		return pow( X, 2 * j );
	}
	static bool symmetric( const matrix44 & m )
	{
		return transpose( m ) == m;
	}
	//�s��̏搔�i�Ίp�����āA������΂��̂܂܁A�����łȂ��Ȃ�΁A���l��I�Ɂj
	static matrix44 pow( const matrix44 & m, decimal n )
	{
		if ( symmetric( m ) ) {
			//�Ίp�����ĉ���
			::array<decimal> eval;
			::array<vector4> evec;
			matrix44 P;
			if ( eigen( m, eval, evec, P ) ) {
				const matrix44 Pt = transpose( P );
				const matrix44 lamda = P * m * transpose( P );
				matrix44 X = identity;
				for ( int i = 0; i < 4; ++i ) {
					X.mat[i][i] = ::pow( lamda.mat[i][i], n );
				}
				return Pt * X * P;
			}
		}
		//�Ίp�����ł��Ȃ��ꍇ�ɂ́A�Q��ƕ�������p���Đ��������Čv�Z����

		const int maxNum = 0x400;
		matrix44 A = m;
		if ( n < 0 ) {
			n *= -1;
			A = inverse( A );
		}
		if ( ! between( 1 / decimal( maxNum ), n, decimal( maxNum ) ) ) return identity;//�傫�����͏ȗ�
		//�P�ȏ�̏搔
		int over = int( n );
		matrix44 X = identity, N = A;
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
	//�s��̏搔
	static matrix44 pow( const matrix44 & m, int n )
	{
		matrix44 A = m;
		if ( n < 0 ) {
			n *= -1;
			A = inverse( A );
		}
		matrix44 X = identity, N = A;
		while ( n ) {
			if ( n & 1 ) {
				X *= N;
			}
			n /= 2;
			N *= N;
		}
		return X;
	}
	//�s��̑ΐ�
	static matrix44 log( const matrix44 & m )
	{
		matrix44 A = m;
		int k = 0;
		while ( norm( A - identity ) > 0.5 ) {
			A = root2( A );
			++k;
		}
		A = identity - A;
		matrix44 Z = A, X = A;
		int i = 1;
		while ( norm( Z ) > roughEpsilon ) {
			Z = Z * A;
			++i;
			X += Z / decimal( i );
		}
		return X * ::pow( static_cast<decimal>( 2.0 ), static_cast<decimal>( k ) );
	}
	//�s��̕�����
	static matrix44 root2( const matrix44 & A )
	{
		matrix44 X = A, Y = identity;
		while ( norm( X * X - A ) > roughEpsilon ) {
			matrix44 iX = inverse( X ), iY = inverse( Y );
			X = ( X + iY ) / 2.0;
			Y = ( Y + iX ) / 2.0;
		}
		return X;
	}
	//�s�̌���
	void swapbra( int i, int j )
	{
		for ( int k = 0; k < 4; ++k ) swap( mat[i][k], mat[j][k] );
	}
	//��̌���
	void swapket( int i, int j )
	{
		for ( int k = 0; k < 4; ++k ) swap( mat[k][i], mat[k][j] );
	}
	//�s��ʂ̍s�ɌW�����|���đ���
	void addbra( int from, int to, decimal c )
	{
		for ( int i = 0; i < 4; ++i ) mat[to][i] += c * mat[from][i];
	}
	//�v�f�̐�Βl���ő�̈ʒu
	bool maximumelement( int & r, int & c, int start_r = 0, int start_c = 0, int end_r = 3, int end_c = 3 )
	{
		checkMaximum<decimal> mx;
		for ( int i = maximum( 0, start_c ); i < minimum( 4, end_r ); ++i ) {
			for ( int j = maximum( 0, start_r ); j < minimum( 4, end_c ); ++j ) {
				mx( fabs( mat[i][j] ), i + j * 4 );
			}
		}
		if ( ! mx ) return false;
		r = mx.sub % 4;
		c = mx.sub / 4;
		return true;
	}
	//�s��̊K��
	static int rank( const matrix44 & m )
	{
		matrix44 r = m;
		//��P�s�̍ő�̗v�f��Ίp�ɂ����Ă���
		int max_r = 0, max_c = 0;
		r.maximumelement( max_r, max_c );
		if ( r.mat[max_r][max_c] ) {
			if ( max_r != 0 ) r.swapbra( 0, max_r );
			if ( max_c != 0 ) r.swapket( 0, max_c );
			//���̉��̗v�f���O�ɂ���悤�ɁA������
			r.addbra( 0, 1, - r.m10 / r.m00 ); r.m10 = 0;
			r.addbra( 0, 2, - r.m20 / r.m00 ); r.m20 = 0;
			r.addbra( 0, 3, - r.m30 / r.m00 ); r.m30 = 0;
			int max_r = 0, max_c = 0;
			r.maximumelement( max_r, max_c, 1, 1 );
			if ( r.mat[max_r][max_c] ) {
				if ( max_r != 1 ) r.swapbra( 1, max_r );
				if ( max_c != 1 ) r.swapket( 1, max_c );
				r.addbra( 1, 2, - r.m21 / r.m11 ); r.m21 = 0;
				r.addbra( 1, 3, - r.m31 / r.m11 ); r.m31 = 0;
				int max_r = 0, max_c = 0;
				r.maximumelement( max_r, max_c, 1, 1 );
				if ( r.mat[max_r][max_c] ) {
					if ( max_r != 2 ) r.swapbra( 2, max_r );
					if ( max_c != 2 ) r.swapket( 2, max_c );
					r.addbra( 2, 3, - r.m32 / r.m22 ); r.m32 = 0;
				}
			}
		}
		if ( roughEpsilon < fabs( r.m33 ) ) return 4;
		if ( roughEpsilon < fabs( r.m22 ) ) return 3;
		if ( roughEpsilon < fabs( r.m11 ) ) return 2;
		if ( roughEpsilon < fabs( r.m00 ) ) return 1;
		return 0;
	}
	//�ŗL�l����ŗL�x�N�g�����擾�B
	static bool eigenvector( const matrix44 & m, decimal eval, ::array<vector4> & evec )
	{
		matrix44 X = m - identity * eval;
		int rnk = rank( X );
		switch ( rnk ) {
		case 0://X��zero�Ȃ̂ŁA�K���Ȓ�������ݒ�
			evec.push_back( vector4( 1, 0, 0, 0 ) );
			evec.push_back( vector4( 0, 1, 0, 0 ) );
			evec.push_back( vector4( 0, 0, 1, 0 ) );
			evec.push_back( vector4( 0, 0, 0, 1 ) );
			break;
		case 1:
		case 2:
		case 3:
			{//�K���̐��̃x�N�g���ƒ�������S�|�K���̃x�N�g��
				::array<vector4> nv;
				for ( int i = 0; i < 4; ++i ) {
					if ( X.vec()[i] != vector4::zero ) {
						nv.push_back( vector4::normalize4( X.vec()[i] ) );
					}
				}
				if ( nv.size < rnk ) return false;

				::array<vector4> uv;
				while ( uv.size < rnk ) {
					checkMaximum<decimal> mx;
					for ( int i = 0; i < nv.size; ++i ) {
						checkMinimum<decimal> mn;
						for ( int j = 0; j < uv.size; ++j ) {
							mn( vector4::length4( nv[i] - uv[j] ) );
						}
						if ( mn ) mx( mn(), i );
					}
					if ( mx ) {
						uv.push_back( nv[mx.sub] );
					} else {
						uv.push_back( nv[0] );
					}
				}
				vector4::gramschmidt( uv );
				for ( int i = rnk; i < 4; ++i ) evec.push_back( uv[i] );
			}
			break;
			//�t�s�񂪑��݂��Ă��܂�
		case 4: return false;
		}
		return true;
	}
	//�ŗL�l�i�傫�����Ƀ\�[�g�j�E�ŗL�x�N�g���i���K���ς݁j�E�Ίp���p�s���Ԃ��i�����ȏꍇ�j�B
	static bool eigen( const matrix44 & m, ::array<decimal> & eval, ::array<vector4> & evec, matrix44 & P )
	{
		P = identity;
		// m * x = �� x => ( m - ��I ) x = 0 => | m - ��I | = 0�i�ŗL�������j
		//�s�񎮂̓W�J��
		// m00 | m11 m12 m13 | - m10 | m01 m02 m03 | + m20 | m01 m02 m03 | - m30 | m01 m02 m03 |
		//     | m21 m22 m23 |       | m21 m22 m23 |       | m11 m12 m13 |       | m11 m12 m13 |
		//     | m31 m32 m33 |       | m31 m32 m33 |       | m31 m32 m33 |       | m21 m22 m23 |

		// m00 ( m11 ( m22 m33 - m32 m23 ) - m21 ( m12 m33 - m32 m13 ) + m31 ( m12 m23 - m22 m13 ) )
		//-m10 ( m01 ( m22 m33 - m32 m23 ) - m21 ( m02 m33 - m32 m03 ) + m31 ( m02 m23 - m22 m03 ) )
		// m20 ( m01 ( m12 m33 - m32 m13 ) - m11 ( m02 m33 - m32 m03 ) + m31 ( m02 m13 - m12 m03 ) )
		//-m30 ( m01 ( m12 m23 - m22 m13 ) - m11 ( m02 m23 - m22 m03 ) + m21 ( m02 m13 - m12 m03 ) )

		// m00 m11 m22 m33 - m00 m11 m32 m23 - m00 m21 m12 m33 + m00 m21 m32 m13 + m00 m31 m12 m23 - m00 m31 m22 m13
		//-m10 m01 m22 m33 + m10 m01 m32 m23 + m10 m21 m02 m33 - m10 m21 m32 m03 - m10 m31 m02 m23 + m10 m31 m22 m03
		// m20 m01 m12 m33 - m20 m01 m32 m13 - m20 m11 m02 m33 + m20 m11 m32 m03 + m20 m31 m02 m13 - m20 m31 m12 m03
		//-m30 m01 m12 m23 + m30 m01 m22 m13 + m30 m11 m02 m23 - m30 m11 m22 m03 - m30 m21 m02 m13 + m30 m21 m12 m03

		//�ɂ�����Ƃ���𔲂��o��
		//   m00 m11 m22 m33 
		// - m00 m11 m32 m23 - m00 m22 m31 m13 - m00 m33 m21 m12 
		// - m11 m22 m30 m03 - m11 m33 m20 m02 - m22 m33 m10 m01 
		// + m00 m21 m32 m13 + m00 m31 m12 m23
		// + m11 m20 m32 m03 + m11 m30 m02 m23 
		// + m22 m30 m01 m13 + m22 m10 m31 m03
		// + m33 m10 m21 m02 + m33 m20 m01 m12 
		// + m10 m01 m32 m23 - m10 m21 m32 m03 - m10 m31 m02 m23 
		// + m20 m31 m02 m13 - m20 m01 m32 m13 - m20 m31 m12 m03 
		// + m30 m21 m12 m03 - m30 m01 m12 m23 - m30 m21 m02 m13 

		//�ɂ�}��
		//   ( m00 - ��) ( m11 - ��) ( m22 - ��) ( m33 - ��) 
		// - ( m00 - ��) ( m11 - ��) m32 m23 - ( m00 - ��) ( m22 - ��) m31 m13 - ( m00 - ��) ( m33 - ��) m21 m12 
		// - ( m11 - ��) ( m22 - ��) m30 m03 - ( m11 - ��) ( m33 - ��) m20 m02 - ( m22 - ��) ( m33 - ��) m10 m01 
		// + ( m00 - ��) m21 m32 m13 + ( m00 - ��) m31 m12 m23
		// + ( m11 - ��) m20 m32 m03 + ( m11 - ��) m30 m02 m23 
		// + ( m22 - ��) m30 m01 m13 + ( m22 - ��) m10 m31 m03
		// + ( m33 - ��) m10 m21 m02 + ( m33 - ��) m20 m01 m12 
		// + m10 m01 m32 m23 - m10 m21 m32 m03 - m10 m31 m02 m23 
		// + m20 m31 m02 m13 - m20 m01 m32 m13 - m20 m31 m12 m03 
		// + m30 m21 m12 m03 - m30 m01 m12 m23 - m30 m21 m02 m13 

		//�ɂɂ��ēW�J
		// ��^4 
		// - ( m00 + m11 + m22 + m33 ) ��^3 
		// + ( m00 m11 + m00 m22 + m00 m33 + m11 m22 + m11 m33 + m22 m33 ) ��^2
		// - ( m11 m22 m33 + m00 m22 m33 + m00 m11 m33 + m00 m11 m22 ) ��
		// + m00 m11 m22 m33
		decimal b = - ( m.m00 + m.m11 + m.m22 + m.m33 );
		decimal c = ( m.m00 * m.m11 + m.m00 * m.m22 + m.m00 * m.m33 + m.m11 * m.m22 + m.m11 * m.m33 + m.m22 * m.m33 );
		decimal d = - ( m.m11 * m.m22 * m.m33 + m.m00 * m.m22 * m.m33 + m.m00 * m.m11 * m.m33 + m.m00 * m.m11 * m.m22 );
		decimal e = m.m00 * m.m11 * m.m22 * m.m33;

		// - ( m32 m23 + m31 m13 + m21 m12 + m30 m03 + m20 m02 + m10 m01 ) ��^2
		// ( + m00 m32 m23 + m00 m31 m13 + m00 m21 m12 + m11 m30 m03 + m11 m20 m02 + m22 m10 m01 ) ��
		// ( + m11 m32 m23 + m22 m31 m13 + m33 m21 m12 + m22 m30 m03 + m33 m20 m02 + m33 m10 m01 ) ��
		// ( - m00 m11 m32 m23 - m00 m22 m31 m13 - m00 m33 m21 m12 - m11 m22 m30 m03 - m11 m33 m20 m02 - m22 m33 m10 m01 )
		c -= ( m.m32 * m.m23 + m.m31 * m.m13 + m.m21 * m.m12 + m.m30 * m.m03 + m.m20 * m.m02 + m.m10 * m.m01 );
		d += ( m.m00 * m.m32 * m.m23 + m.m00 * m.m31 * m.m13 + m.m00 * m.m21 * m.m12 + m.m11 * m.m30 * m.m03 + m.m11 * m.m20 * m.m02 + m.m22 * m.m10 * m.m01 );
		d += ( m.m11 * m.m32 * m.m23 + m.m22 * m.m31 * m.m13 + m.m33 * m.m21 * m.m12 + m.m22 * m.m30 * m.m03 + m.m33 * m.m20 * m.m02 + m.m33 * m.m10 * m.m01 );
		e -= ( m.m00 * m.m11 * m.m32 * m.m23 + m.m00 * m.m22 * m.m31 * m.m13 + m.m00 * m.m33 * m.m21 * m.m12 + m.m11 * m.m22 * m.m30 * m.m03 + m.m11 * m.m33 * m.m20 * m.m02 + m.m22 * m.m33 * m.m10 * m.m01 );

		// - (  m21 m32 m13 + m31 m12 m23 + m20 m32 m03 + m30 m02 m23 + m30 m01 m13 + m10 m31 m03 + m10 m21 m02 + m20 m01 m12 ) ��
		// + m00 m21 m32 m13 + m00 m31 m12 m23
		// + m11 m20 m32 m03 + m11 m30 m02 m23 
		// + m22 m30 m01 m13 + m22 m10 m31 m03
		// + m33 m10 m21 m02 + m33 m20 m01 m12 
		d -= ( m.m21 * m.m32 * m.m13 + m.m31 * m.m12 * m.m23 + m.m20 * m.m32 * m.m03 + m.m30 * m.m02 * m.m23 + m.m30 * m.m01 * m.m13 + m.m10 * m.m31 * m.m03 + m.m10 * m.m21 * m.m02 + m.m20 * m.m01 * m.m12 );
		e += m.m00 * m.m21 * m.m32 * m.m13 + m.m00 * m.m31 * m.m12 * m.m23;
		e += m.m11 * m.m20 * m.m32 * m.m03 + m.m11 * m.m30 * m.m02 * m.m23;
		e += m.m22 * m.m30 * m.m01 * m.m13 + m.m22 * m.m10 * m.m31 * m.m03;
		e += m.m33 * m.m10 * m.m21 * m.m02 + m.m33 * m.m20 * m.m01 * m.m12;

		// + m10 m01 m32 m23 - m10 m21 m32 m03 - m10 m31 m02 m23 
		// + m20 m31 m02 m13 - m20 m01 m32 m13 - m20 m31 m12 m03 
		// + m30 m21 m12 m03 - m30 m01 m12 m23 - m30 m21 m02 m13 
		e += m.m10 * m.m01 * m.m32 * m.m23 - m.m10 * m.m21 * m.m32 * m.m03 - m.m10 * m.m31 * m.m02 * m.m23;
		e += m.m20 * m.m31 * m.m02 * m.m13 - m.m20 * m.m01 * m.m32 * m.m13 - m.m20 * m.m31 * m.m12 * m.m03;
		e += m.m30 * m.m21 * m.m12 * m.m03 - m.m30 * m.m01 * m.m12 * m.m23 - m.m30 * m.m21 * m.m02 * m.m13;

		equation()( 1, b, c, d, e, eval );

		::array<decimal> x;
		for ( int i = 0; i < eval.size; ++i ) {
			if ( ! eigenvector( m, eval[i], evec ) ) {
				return false;
			}
		}
		if ( evec.size == 4 ) P = matrix44( evec[0], evec[1], evec[2], evec[3] );
		return true;
	}
};

inline const matrix44 operator+( const decimal & s, const matrix44 & v ) { return v + s; }
inline const matrix44 operator-( const decimal & s, const matrix44 & v ) { return - v + s; }
inline const matrix44 operator*( const decimal & s, const matrix44 & v ) { return v * s; }
inline const matrix44 operator/( const decimal & s, const matrix44 & v ) { return matrix44( s/v.x, s/v.y, s/v.z, s/v.w ); }
inline const vector4 vector4::operator*( const matrix44 & m ) const { return m.transpose() * *this; }
inline const vector3 vector3::operator*( const matrix44 & m ) const { vector4 wk = vector4( *this ) * m; return wk.getVector3() / wk.w; }

#ifdef __GLOBAL__
const matrix44 matrix44::identity( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 );
const matrix44 matrix44::zero( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
const matrix44 matrix44::one( 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 );
#endif
