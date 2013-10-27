////////////////////////////////////////////////////////////////////////////////
// Matrix 型と要素数のテンプレート版（数値計算用のため精度と次元が必要で無いときには用いない）
// 未完成 BUG DEBUG

template<typename T>
class matrixC
{
	array< vectorC<T> > mat;
	void allocate( const matrixC & m1, const matrixC & m2 )
	{
		allocate( minimum( m1.row(), m2.row() ), minimum( m1.column(), m2.column() ) );
	}
	void allocate( const int R, const int C )
	{
		mat.resize( R, 0 );
		for ( int i = 0; i < R; ++i ) {
			mat[i].allocate( C );
		}
	}
public:
	matrixC() {}
	matrixC( const int R, const int C ) { allocate( R, C ); zero(); }
	matrixC( const matrixC & m1, const matrixC & m2 ) { allocate( m1, m2 ); zero(); }
	matrixC( const matrixC & m ) { allocate( m.row(), m.column() ); for ( int r = 0; r < m.row(); ++r ) mat[r] = m.mat[r]; }
	matrixC & operator=( const matrixC & m ) { allocate( m.row(), m.column() ); for ( int r = 0; r < row(); ++r ) mat[r] = m.mat[r]; return *this; }
	int row() const { return mat.size; }
	int column() const { if ( ! row() ) return 0; return mat[0].dimension(); }

	const matrixC operator-() const { matrixC result( row(), column() ); for ( int r = 0; r < row(); ++r ) result[r] = - mat[r]; return result; }
	const matrixC operator+( const matrixC & m ) const { matrixC result( *this, m ); for ( int r = 0; r < result.row(); ++r ) result[r] = mat[r] + m.mat[r]; return result; }
	const matrixC operator-( const matrixC & m ) const { matrixC result( *this, m ); for ( int r = 0; r < result.row(); ++r ) result[r] = mat[r] - m.mat[r]; return result; }
	void operator+=( const matrixC & m ) { allocate( *this, m ); for ( int r = 0; r < row(); ++r ) mat[r] += m.mat[r]; }
	void operator-=( const matrixC & m ) { allocate( *this, m ); for ( int r = 0; r < row(); ++r ) mat[r] -= m.mat[r]; }
	const matrixC operator+( const complex<T> & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] += m; return result; }
	const matrixC operator-( const complex<T> & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] -= m; return result; }
	const matrixC operator*( const complex<T> & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] *= m; return result; }
	const matrixC operator/( const complex<T> & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] /= m; return result; }
	void operator+=( const complex<T> & m ) { for ( int r = 0; r < row(); ++r ) mat[r] += m; }
	void operator-=( const complex<T> & m ) { for ( int r = 0; r < row(); ++r ) mat[r] -= m; }
	void operator*=( const complex<T> & m ) { for ( int r = 0; r < row(); ++r ) mat[r] *= m; }
	void operator/=( const complex<T> & m ) { for ( int r = 0; r < row(); ++r ) mat[r] /= m; }
	const matrixC operator+( const T & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] += m; return result; }
	const matrixC operator-( const T & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] -= m; return result; }
	const matrixC operator*( const T & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] *= m; return result; }
	const matrixC operator/( const T & m ) const { matrixC result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] /= m; return result; }
	void operator+=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] += m; }
	void operator-=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] -= m; }
	void operator*=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] *= m; }
	void operator/=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] /= m; }
	bool operator==( const matrixC & m ) const { if ( row() != v.row() || column() != v.column() ) return false; for ( int i = 0; i < row(); ++i ) if ( mat[i] != m[i] ) return false; return true; }
	bool operator!=( const matrixC & m ) const { return ! ( *this == m ); }
	const vectorC<T> & operator[]( int r ) const { return mat[r]; }
	vectorC<T> & operator[]( int r ) { return mat[r]; }
	const vectorC<T> bra( int r ) const
	{
		return mat[r];
	}
	const vectorC<T> ket( int c ) const
	{
		vectorC<T> result( row() );
		for ( int r = 0; r < row(); ++r ) {
			result[r] = mat[r][c];
		}
		return result;
	}
	static matrixC bra( const vectorC<T> & v )
	{
		matrixC result( 1, v.dimension() );
		result[0] = v;
		return result;
	}
	static matrixC ket( const vectorC<T> & v )
	{
		matrixC result( v.dimension(), 1 );
		for ( int r = 0; r < v.dimension(); ++r ) {
			result[r][0] = v[r];
		}
		return result;
	}
	const matrixC operator*( const matrixC & m ) const
	{
		const int LR = row(), LC = column(), RR = m.row(), RC = m.column();
		matrixC result = matrixC::zero( LR, RC );
		for ( int r = 0; r < LR; ++r ) {
			for ( int c = 0; c < RC; ++c ) {
				complex<T> & sum = result[r][c];
				for ( int s = 0; s < LC && s < RR; ++s ) {
					sum += mat[r][s] * m[s][c];
				}
			}
		}
		return result;
	}
	const vectorC<T> operator*( const vectorC<T> v )
	{
		vectorC<T> result = vectorC<T>::zero( row() );
		for ( int r = 0; r < row(); ++r ) {
			complex<T> & sum = result[r];
			for ( int c = 0; c < column() && c < v.dimension(); ++c ) {
				sum += mat[r][c] * v[c];
			}
		}
		return result;
	}
	void zero() { for ( int r = 0; r < row(); ++r ) mat[r].zero(); }
	void one() { for ( int r = 0; r < row(); ++r ) mat[r].one(); }
	void identity() { for ( int r = 0; r < row(); ++r ) mat[r].basis( r ); }
	static const matrixC identity( int N ) { matrixC result( N, N ); result.identity(); return result; }
	static const matrixC zero( int N ) { matrixC result( N, N ); result.zero(); return result; }
	static const matrixC one( int N ) { matrixC result( N, N ); result.one(); return result; }
	static const matrixC identity( int R, int C ) { matrixC result( R, C ); result.identity(); return result; }
	static const matrixC zero( int R, int C ) { matrixC result( R, C ); result.zero(); return result; }
	static const matrixC one( int R, int C ) { matrixC result( R, C ); result.one(); return result; }
	//行の交換
	void swapbra( int i, int j )
	{
		for ( int c = 0; c < column(); ++c ) swap( mat[i][c], mat[j][c] );
	}
	//列の交換
	void swapket( int i, int j )
	{
		for ( int r = 0; r < row(); ++r ) swap( mat[r][i], mat[r][j] );
	}
	//行を別の行に係数を掛けて足す
	void addbra( int from, int to, T v )
	{
		for ( int c = 0; c < column(); ++c ) mat[to][c] += v * mat[from][c];
	}
	//有効か調べる
	bool valid() const
	{
		for ( int r = 0; r < row(); ++r ) if ( ! mat[r].valid() ) return false;
		return true;
	}
	//0に近い要素を０にする
	static const matrixC fix( const matrixC & m )
	{
		matrixC result( m );
		for ( int i = 0; i < m.row(); ++i ) {
			result[i] = vectorC<T>::fix( result[i] );
		}
		return result;
	}
	//転置
	static const matrixC transpose( const matrixC & m )
	{
		matrixC result( m.column(), m.row() );
		for ( int r = 0; r < m.row(); ++r ) {
			for ( int c = 0; c < m.column(); ++c ) {
				result[c][r] = m[r][c];
			}
		}
		return result;
	}
	//対角の積
	const complex<T> trace() const
	{
		complex<T> result = 1;
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result *= mat[r][r];
		}
		return result;
	}
	//下三角行列
	bool lowerMatrix( T limit = epsilon ) const
	{
		T sum = 0;
		for ( int r = 0; r < row(); ++r ) {
			for ( int c = 0; c < r && c < column(); ++c ) {
				sum += complex<T>::norm( mat[r][c] );
			}
		}
		return ( sum < limit );
	}
	//上三角行列
	bool upperMatrix( T limit = epsilon ) const
	{
		T sum = 0;
		for ( int r = 0; r < row(); ++r ) {
			for ( int c = r + 1; c < column(); ++c ) {
				sum += complex<T>::norm( mat[r][c] );
			}
		}
		return ( sum < limit );
	}
	//内積
	static const complex<T> dot( const matrixC & left, const matrixC & right )
	{
		complex<T> result = 0;
		for ( int r = 0; r < left.row() && r < right.row(); ++r ) {
			result += vectorC<T>::dot( left[r], right[r] );
		}
		return result;
	}
	//ノルム
	static const T norm( const matrixC & m )
	{
		checkMaximum<T> mx;
		for ( int r = 0; r < m.row(); ++r ) {
			checkMaximum<T> mx;
			T sum = 0;
			for ( int c = 0; c < m.column(); ++c ) {
				sum += complex<T>::norm( m[r][c] );
			}
			mx( sum );
		}
		return mx ? mx() : 0;
	}
	//正方行列
	bool squareMatrix() const
	{
		if ( row() != column() ) return false;
		return true;
	}
	const matrixC conjugate() const 
	{
		matrixC r( *this );
		for ( int i = 0; i < row(); ++i ) r[i] = mat[i].conjugate();
		return r;
	}
	const matrixC minor( int leftup, int rightbottom ) const
	{
		matrixC result = identity( rightbottom - leftup );
		for ( int r = leftup; r < rightbottom && r < row(); ++r ) {
			for ( int c = leftup; c < rightbottom && c < column(); ++c ) {
				result[r-leftup][c-leftup] = mat[r][c];
			}
		}
		return result;
	}
	const matrixC diag() const
	{
		matrixC result = identity( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r][r] = mat[r][r];
		}
		return result;
	}
	const matrixC inverseDiag() const
	{
		matrixC result = identity( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r][r] = 1 / mat[r][r];
		}
		return result;
	}
	// 上三角行列として、ガウスの吐き出し法を行う
	// R x = b
	bool rSolve( const vectorC<T> & b, vectorC<T> & x, const vectorC<int> * exchange = NULL ) const
	{
		if ( ! squareMatrix() ) return false;
		const int N = row();
		if ( exchange && exchange->dimension() != N ) return false;
		if ( b.dimension() != N ) return false;
		x = vectorC<T>::zero( N );
		// r * x = bを解く
		for ( int i = N - 1; i >= 0; i-- ) {
			const int ii = exchange ? (*exchange)[i] : i;
			complex<T> t = b[ii];
			for ( int j = i + 1; j < N; j++ ) {
				t -= mat[ii][j] * x[j];
			}
			x[i] = t / mat[ii][i];//0ならば不定
		}
		return true;
	}
	//QR分解
	//行列Aを直交行列Qと右上三角行列Rに分解する．
	//条件：各列ベクトルが1次独立、正方か縦長の行列であること
	void qrFactorization( matrixC & q, matrixC & r ) const
	{
		//if ( row() < column() ) return false;//縦長の行列の時のみ計算可能
		const int R = row();
		const int C = column();
		//修正グラムシュミットの方法
		matrixC qT = transpose( *this );
		r = identity( C );
		for ( int k = 0; k < C; ++k ) {
			//対角要素について
			complex<T> & rkk = r[k][k];
			rkk = vectorC<T>::norm( qT[k] );
			//if ( rkk == 0 ) return false;
			qT[k] /= rkk;//rkk=0->qTk=0
			for ( int j = k + 1; j < C; ++j ) {//その右
				complex<T> & rkj = r[k][j];
				rkj = vectorC<T>::dot( qT[k], qT[j] );//rkk=0->rkj=0
				//qT[j] = vectorC<T>::fix( qT[j] - qT[k] * rkj );
				qT[j] -= qT[k] * rkj;
			}
		}
		q = transpose( qT );
		matrixC<T>::debug( *this, "this" );
		matrixC<T>::debug( q, "Q" );
		matrixC<T>::debug( r, "R" );
		matrixC<T>::debug( q*r, "QR" );
		matrixC<T>::debug( r*q, "RQ" );
		matrixC<T>::debug( q*qT.conjugate(), "QQ'" );
		matrixC<T>::debug( qT.conjugate()*q, "Q'Q" );
		matrixC wk = q * r - *this;
		complex<T> len = dot( wk, wk );
		if ( complex<T>::norm( len ) > epsilon ) {
			len = len;
			DebugBreak();
		}
	}
	//行列をQR変換し、吐き出し法で方程式を解く
	bool qrSolve( const vectorC<T> & b, vectorC<T> & x ) const
	{
		matrixC q, matrixC r;
		qrFactorization( q, r );
		// q r x = b
		// r x = qT b
		q = transpose( q );
		return r.rSolve( q * b, x );
	}
	//QR変換
	//A=QRと分解し、A'=RQと変換する。これは、A'=Q'AQで相似変換
	//条件：正方行列、QR分解可能（各列ベクトルが1次独立）
	bool qrTransform( matrixC & P )
	{
		if ( ! squareMatrix() ) return false;
		matrixC q, r;
		qrFactorization( q, r );
		*this = r * q;
		P = transpose( q ).conjugate();
		return true;
	}
	static void debug( const matrixC & m, const char * tag = NULL )
	{
		if ( tag ) {
			OutputDebugString( tag );
			OutputDebugString( ":[matrix]\n" );
		}
		for ( int i = 0; i < m.row(); ++i ) {
			vectorC<T>::debug( m[i] );
		}
	}
};

//四則演算
template<typename T>
const matrixC<T> operator+( const complex<T> & s, const matrixC<T> & m ) { return m + s; }
template<typename T>
const matrixC<T> operator-( const complex<T> & s, const matrixC<T> & m ) { return - m + s; }
template<typename T>
const matrixC<T> operator*( const complex<T> & s, const matrixC<T> & m ) { return m * s; }
template<typename T>
const matrixC<T> operator/( const complex<T> & s, const matrixC<T> & m )
{
	matrixC<T> result( m );
	for ( int r = 0; r < result.row(); ++r ) {
		for ( int c = 0; c < result.column(); ++c ) {
			result[r][c] = s / m[r][c];
		}
	}
	return r;
}

template<typename T>
const vectorC<T> operator*( const vectorC<T> v, const matrixC<T> & m )
{
	vectorC<T> result = vectorC<T>::zero( column() );
	for ( int c = 0; c < column(); ++c ) {
		complex<T> & sum = result[c];
		for ( int r = 0; r < row() && r < v.dimension(); ++r ) {
			sum += v[r] * m[r][c];
		}
	}
	return result;
}
