////////////////////////////////////////////////////////////////////////////////
// Matrix 型と要素数のテンプレート版（数値計算用のため精度と次元が必要で無いときには用いない）
// 未完成 BUG DEBUG

template<typename T>
class matrixT
{
	array< vectorT<T> > mat;
	// 横の行(row)をベクトルとしてもち、縦の列(column)方向の数
	void allocate( const matrixT & m1, const matrixT & m2 )
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
	matrixT() {}
	matrixT( const int R, const int C ) { allocate( R, C ); zero(); }
	matrixT( const matrixT & m1, const matrixT & m2 ) { allocate( m1, m2 ); zero(); }
	matrixT( const matrixT & m ) { *this = m; }
	matrixT & operator=( const matrixT & m ) { allocate( m.row(), m.column() ); for ( int r = 0; r < row(); ++r ) mat[r] = m.mat[r]; return *this; }
	void input( const matrixC<T> & m ) { *this = matrixT( m.row(), m.column() ); for ( int r = 0; r < row(); ++r ) for ( int c = 0; c < column(); ++c ) mat[r][c] = m[r][c].real(); }
	void output( matrixC<T> & m ) const { m = matrixC<T>( row(), column() ); for ( int r = 0; r < row(); ++r ) for ( int c = 0; c < column(); ++c ) m[r][c] = mat[r][c]; }
	//行(row)の数、若しくは列(column)の要素数
	int row() const { return mat.size; }
	//列(column)の数、若しくは行(row)の要素数
	int column() const { if ( ! row() ) return 0; return mat.first().dimension(); }
	//演算
	const matrixT operator-() const { matrixT result( row(), column() ); for ( int r = 0; r < row(); ++r ) result[r] = - mat[r]; return result; }
	const matrixT operator+( const matrixT & m ) const { matrixT result( *this, m ); for ( int r = 0; r < result.row(); ++r ) result[r] = mat[r] + m.mat[r]; return result; }
	const matrixT operator-( const matrixT & m ) const { matrixT result( *this, m ); for ( int r = 0; r < result.row(); ++r ) result[r] = mat[r] - m.mat[r]; return result; }
	void operator+=( const matrixT & m ) { allocate( *this, m ); for ( int r = 0; r < row(); ++r ) mat[r] += m.mat[r]; }
	void operator-=( const matrixT & m ) { allocate( *this, m ); for ( int r = 0; r < row(); ++r ) mat[r] -= m.mat[r]; }
	const matrixT operator+( const T & m ) const { matrixT result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] += m; return result; }
	const matrixT operator-( const T & m ) const { matrixT result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] -= m; return result; }
	const matrixT operator*( const T & m ) const { matrixT result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] *= m; return result; }
	const matrixT operator/( const T & m ) const { matrixT result( *this ); for ( int r = 0; r < result.row(); ++r ) result[r] /= m; return result; }
	void operator+=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] += m; }
	void operator-=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] -= m; }
	void operator*=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] *= m; }
	void operator/=( const T & m ) { for ( int r = 0; r < row(); ++r ) mat[r] /= m; }
	bool operator==( const matrixT & m ) const { if ( row() != v.row() || column() != v.column() ) return false; for ( int i = 0; i < row(); ++i ) if ( mat[i] != m[i] ) return false; return true; }
	bool operator!=( const matrixT & m ) const { return ! ( *this == m ); }
	const vectorT<T> & operator[]( int r ) const { return mat[r]; }
	vectorT<T> & operator[]( int r ) { return mat[r]; }
	const matrixT operator*( const matrixT & m ) const
	{
		const int LR = row(), LC = column(), RR = m.row(), RC = m.column();
		matrixT result = matrixT::zero( LR, RC );
		for ( int r = 0; r < LR; ++r ) {
			for ( int c = 0; c < RC; ++c ) {
				T & sum = result[r][c];
				for ( int s = 0; s < LC && s < RR; ++s ) {
					sum += mat[r][s] * m[s][c];
				}
			}
		}
		return result;
	}
	const vectorT<T> operator*( const vectorT<T> v )
	{
		vectorT<T> result = vectorT<T>::zero( row() );
		for ( int r = 0; r < row(); ++r ) {
			T & sum = result[r];
			for ( int c = 0; c < column() && c < v.dimension(); ++c ) {
				sum += mat[r][c] * v[c];
			}
		}
		return result;
	}
	//横ベクトルとして取り出す。
	const vectorT<T> bra( int r ) const
	{
		return mat[r];
	}
	//縦ベクトルとして取り出す。
	const vectorT<T> ket( int c ) const
	{
		vectorT<T> result( row() );
		for ( int r = 0; r < row(); ++r ) {
			result[r] = mat[r][c];
		}
		return result;
	}
	//横ベクトルを行列形式に変換
	static matrixT bra( const vectorT<T> & v )
	{
		matrixT result( 1, v.dimension() );
		result[0] = v;
		return result;
	}
	//縦ベクトルを行列形式に変換
	static matrixT ket( const vectorT<T> & v )
	{
		matrixT result( v.dimension(), 1 );
		for ( int r = 0; r < v.dimension(); ++r ) {
			result[r][0] = v[r];
		}
		return result;
	}
	void zero() { for ( int r = 0; r < row(); ++r ) mat[r].zero(); }
	void one() { for ( int r = 0; r < row(); ++r ) mat[r].one(); }
	void identity() { for ( int r = 0; r < row(); ++r ) mat[r].basis( r ); }
	static const matrixT identity( int N ) { matrixT result( N, N ); result.identity(); return result; }
	static const matrixT zero( int N ) { matrixT result( N, N ); result.zero(); return result; }
	static const matrixT one( int N ) { matrixT result( N, N ); result.one(); return result; }
	static const matrixT identity( int R, int C ) { matrixT result( R, C ); result.identity(); return result; }
	static const matrixT zero( int R, int C ) { matrixT result( R, C ); result.zero(); return result; }
	static const matrixT one( int R, int C ) { matrixT result( R, C ); result.one(); return result; }
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
	static const matrixT fix( const matrixT & m )
	{
		matrixT result( m );
		for ( int i = 0; i < m.row(); ++i ) {
			result[i] = vectorT<T>::fix( result[i] );
		}
		return result;
	}
	//転置
	static const matrixT transpose( const matrixT & m )
	{
		matrixT result( m.column(), m.row() );
		for ( int r = 0; r < m.row(); ++r ) {
			for ( int c = 0; c < m.column(); ++c ) {
				result[c][r] = m[r][c];
			}
		}
		return result;
	}
	//内積
	static T dot( const matrixT & left, const matrixT & right )
	{
		T result = 0;
		for ( int r = 0; r < left.row() && r < right.row(); ++r ) {
			result += vectorT<T>::dot( left[r], right[r] );
		}
		return result;
	}
	//ノルム
	static T norm( const matrixT & m )
	{
		checkMaximum<T> mx;
		for ( int r = 0; r < m.row(); ++r ) {
			checkMaximum<T> mx;
			T sum = 0;
			for ( int c = 0; c < m.column(); ++c ) {
				sum += absolute( m[r][c] );
			}
			mx( sum );
		}
		return mx ? mx() : 0;
	}
	//対角の積
	const T trace() const
	{
		T result = 1;
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result *= mat[r][r];
		}
		return result;
	}
	//対角行列
	bool diagMatrix( T limit = epsilon ) const
	{
		T sum = 0;
		for ( int r = 0; r < row(); ++r ) {
			for ( int c = 0; c < column(); ++c ) {
				if ( r != c ) sum += absolute( mat[r][c] );
			}
		}
		return ( sum < limit );
	}
	//下三角行列
	bool lowerMatrix( T limit = epsilon ) const
	{
		T sum = 0;
		for ( int r = 0; r < row(); ++r ) {
			for ( int c = 0; c < r && c < column(); ++c ) {
				sum += absolute( mat[r][c] );
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
				sum += absolute( mat[r][c] );
			}
		}
		return ( sum < limit );
	}
	//正方行列
	bool squareMatrix() const
	{
		if ( row() != column() ) return false;
		return true;
	}
	//対称行列
	bool symmetricMatrix() const
	{
		if ( row() != column() ) return false;
		for ( int r = 0; r < row(); ++r ) {
			for ( int c = 0; c < r; ++c ) {
				if ( absolute( mat[c][r] - mat[c][r] ) > epsilon ) return false;
			}
		}
		return true;
	}
	//小行列を取得
	//縦方向と横方向の最初と最後の要素を指定
	const matrixT minor( int rs, int cs, int re, int ce ) const
	{
		matrixT result( re - rs, ce - cs );
		for ( int r = 0; r+rs < row() && r < result.row(); ++r ) {
			for ( int c = 0; c+cs < column() && c < result.column(); ++c ) {
				result[r][c] = mat[r+rs][c+cs];
			}
		}
		return result;
	}
	//小行列を取得
	const matrixT minor( int start, int end ) const
	{
		return minor( start, start, end, end );
	}
	//小行列をコピーして合成
	void copy( int rs, int cs, const matrixT & src )
	{
		const int DR = row();
		const int DC = column();
		const int SR = src.row();
		const int SC = src.column();
		for ( int r = 0; r+rs < DR && r < SR; ++r ) {
			for ( int c = 0; c+cs < DC && c < SC; ++c ) {
				mat[r+rs][c+cs] = src[r][c];
			}
		}
	}
	//対角要素のみをもつ正方行列を作成
	const matrixT diag() const
	{
		matrixT result = identity( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r][r] = mat[r][r];
		}
		return result;
	}
	//対角要素のベクトルを作成
	const vectorT<T> diagVector() const
	{
		vectorT<T> result = vectorT<T>::zero( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r] = mat[r][r];
		}
		return result;
	}
	//対角行列の一般逆行列を戻す
	const matrixT inverseDiag() const
	{
		matrixT result = identity( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r][r] = zerodivide<T>( 1, mat[r][r] );
		}
		return result;
	}
	//LU分解
	//入力した行列をlu分解する
	//L左下三角行列の対角成分が１，U右上三角行列でL * Uの形に行列を分解する
	//ガウスの消去法で用いた対角要素に掛けた値をL行列の０にした個所に代入して置く事でLが計算できる．
	//Uはガウスの消去法の結果である．
	//ソースの元はアルゴリズム辞典

	//注　ガウスの消去法：左下の要素を０にするために
	//同じ列の対角要素の値にある値を掛けてその行全体を引く．
	//これを左上から左から右へ，その後上から下へ順に行っていく手法
	// LU分解の結果を用いてm*x=bからxを解く
	bool luFactorization( matrixT & lu, vectorT<int> & exchange ) const
	{
		if ( ! squareMatrix() ) return false;
		int N = row();
		vectorT<T> weight( N );
		exchange = vectorT<int>( N );
		for ( int k = 0; k < N; k++ ) {  // 各列について
			exchange[k] = k;// 行交換情報の初期値
			checkMaximum<T> mx;// その行の絶対値最大の要素を求める
			for ( int j = 0; j < N; j++ ) {
				mx( absolute( mat[k][j] ) );
			}
			if ( mx() == 0 ) return false; // 0 なら行列はLU分解できない
			weight[k] = T( 1.0 / mx() ); // 最大絶対値の逆数
		}
		lu = *this;
		for ( int k = 0; k < N; k++ ) {  // 各行について
			// より下の各行について重み×絶対値 が最大の行を見つける
			{
				checkMaximum<T> mx;
				for ( int i = k; i < N; i++ ) {
					const int ii = exchange[i];
					mx( absolute( lu[ii][k] ) * weight[ii], i );
				}
				// 行番号を交換
				swap( exchange[mx.sub], exchange[k] );
			}
			const int ik = exchange[k];//交換した後のk
			const T vkk = lu[ik][k];//対角成分
			if ( vkk == 0 ) return false;// 0 なら行列はLU分解できない
			for ( int i = k + 1; i < N; i++) { // Gauss消去法
				const int ii = exchange[i];//交換した後のi
				lu[ii][k] /= vkk;//Lの要素
				const T vik = lu[ii][k];
				for ( int j = k + 1; j < N; j++) {
					lu[ii][j] -= vik * lu[ik][j];
				}
			}
		}
		return true;
	}
	// 下三角行列として、ガウスの吐き出し法を行う
	// L x = b
	bool lSolve( const vectorT<T> & b, vectorT<T> & x, const vectorT<int> * exchange = NULL ) const
	{
		if ( ! squareMatrix() ) return false;
		const int N = row();
		if ( exchange && exchange->dimension() != N ) return false;
		if ( b.dimension() != N ) return false;
		x = vectorT<T>::zero( N );
		// l * x = bを解く
		for ( int i = 0; i < N; i++ ) {
			const int ii = exchange ? (*exchange)[i] : i;
			T t = b[ii];
			for ( int j = 0; j < i; j++ ) {
				t -= mat[ii][j] * x[j];
			}
			x[i] = zerodivide( t, mat[ii][i] );//0ならば不定
		}
		return true;
	}
	// 上三角行列として、ガウスの吐き出し法を行う
	// R x = b
	bool rSolve( const vectorT<T> & b, vectorT<T> & x, const vectorT<int> * exchange = NULL ) const
	{
		if ( ! squareMatrix() ) return false;
		const int N = row();
		if ( exchange && exchange->dimension() != N ) return false;
		if ( b.dimension() != N ) return false;
		x = vectorT<T>::zero( N );
		// r * x = bを解く
		for ( int i = N - 1; i >= 0; i-- ) {
			const int ii = exchange ? (*exchange)[i] : i;
			T t = b[ii];
			for ( int j = i + 1; j < N; j++ ) {
				t -= mat[ii][j] * x[j];
			}
			x[i] = zerodivide( t, mat[ii][i] );//0ならば不定
		}
		return true;
	}
	//LU分解にて方程式を解く（this=lu）LU x = b
	bool luSolve( const vectorT<T> & b, vectorT<T> & x, const vectorT<int> * exchange = NULL ) const
	{
		matrixT L = *this;
		matrixT R = *this;
		for ( int r = 0; r < row(); ++r ) {
			const int rr = exchange ? (*exchange)[r] : r;
			L[rr][r] = 1;
			for ( int c = r + 1; c < column(); ++c ) {
				L[rr][c] = 0;
			}
			for ( int c = 0; c < r; ++c ) {
				R[rr][c] = 0;
			}
		}
		// L U x = b
		// L y = bを解き、U x = yを解く
		vectorT<T> y;
		if ( ! L.lSolve( b, y, exchange ) ) return false;
		if ( ! R.rSolve( y, x, exchange ) ) return false;
		return true;
	}
	//GramSchmidt
	//直交ベクトルを計算する。
	//条件：各列ベクトルが1次独立でない場合には、数は少なくなる。
	void gramSchmidt( matrixT & q ) const
	{
		if ( row() < column() ) {//横長ならば、転置して計算する
			matrixT AT = transpose( *this );
			AT.gramSchmidt( q );
			q = transpose( q );
			return;
		}
		//縦長なので、列ベクトルを直交するように求める
		const int R = row();
		const int C = column();
		//グラムシュミットの方法
		matrixT AT = transpose( *this );
		array< vectorT<T> > orthogonal;
		orthogonal.reserve( C );
		for ( int c = 0; c < C; ++c ) {
			const vectorT<T> & vc = AT[c];//c列のベクトル
			vectorT<T> vo = vc;//直交するベクトル
			//これまでに求めたベクトル群と平行な成分は全て削除する
			for ( int i = 0; i < orthogonal.size; ++i ) {
				vo -= orthogonal[i] * vectorT<T>::dot( vc, orthogonal[i] );
			}
			//垂直な成分があるか調べる
			T n = vectorT<T>::norm( vo );
			if ( n < epsilon ) continue;//垂直な成分が無いので、独立ではないため削除
			orthogonal.push_back( vo / n );//正規化して保存
		}
		q.allocate( orthogonal.size, R );
		for ( int c = 0; c < orthogonal.size; ++c ) {
			q[c] = orthogonal[c];
		}
		q = transpose( q );
	}
	//行列の階数　SVDによる解法が普通みたいだが、1次独立なベクトルがいくつあるかを計算して求める
	int rank() const
	{
		matrixT q;
		gramSchmidt( q );
		return minimum( q.row(), q.column() );
	}
	//QR分解
	//行列Aを直交行列Qと右上三角行列Rに分解する．
	//条件：各列ベクトルが1次独立、正方か縦長の行列であること
	void qrFactorization( matrixT & q, matrixT & r ) const
	{
		//if ( row() < column() ) return false;//縦長の行列の時のみ計算可能
		const int R = row();
		const int C = column();
		//修正グラムシュミットの方法
		matrixT qT = transpose( *this );
		r = identity( C );
		for ( int k = 0; k < C; ++k ) {
			//対角要素について
			T & rkk = r[k][k];
			rkk = vectorT<T>::norm( qT[k] );
			//零ベクトルでも計算を続ける
			qT[k] /= rkk;//rkk=0->qTk=0
			for ( int j = k + 1; j < C; ++j ) {//その右
				T & rkj = r[k][j];
				rkj = vectorT<T>::dot( qT[k], qT[j] );//rkk=0->rkj=0
				qT[j] -= qT[k] * rkj;
			}
		}
		q = transpose( qT );
	}
	//行列をQR変換し、吐き出し法で方程式を解く
	bool qrSolve( const vectorT<T> & b, vectorT<T> & x ) const
	{
		matrixT q, matrixT r;
		qrFactorization( q, r );
		// q r x = b
		// r x = qT b
		return r.rSolve( transpose( q ) * b, x );
	}
	//QR変換
	//A=QRと分解し、A'=RQと変換する。これは、A'=Q'AQで相似変換
	//P^-1APと相似変換を行う時のPも戻す
	//条件：正方行列、QR分解可能（各列ベクトルが1次独立）
	bool qrTransform( matrixT & P )
	{
		if ( ! squareMatrix() ) return false;
		matrixT q, r;
		qrFactorization( q, r );
		*this = r * q;
		P = transpose( q );
		return true;
	}
	//コレスキー分解 mat=L (L*)の形に分解
	//条件，正定値対称正則（全ての固有値が正），全ての主座小行列式が非零
	bool choleskyFactorization( matrixT & l ) const
	{
		if ( ! symmetricMatrix() ) return false;
		const int N = row();
		l = identity( N );
		for ( int i = 0; i < N; ++i ) {
			for ( int j = 0; j < i; ++j ) {
				T sum = 0;
				for ( int k = 0; k < j; ++k ) {
					sum += l[i][k] * l[j][k];
				}
				l[i][j] = zerodivide( sqrt( mat[i][j] - sum ), l[j][j] );
			}
			T sum = 0;
			for ( int k = 0; k < i; ++k ) {
				sum += l[i][k] * l[i][k];
			}
			T & lii = l[i][i];
			lii = mat[i][i] - sum;
			if ( lii <= 0 ) return false;
			lii = sqrt( lii );
		}
		return true;
	}
	//修正コレスキー分解 mat=L D (L*)の形に分解
	//条件，対称正則，全ての主座小行列式が非零
	bool choleskyFactorization( matrixT<T> & l, matrixT<T> & d ) const
	{
		if ( ! symmetricMatrix( *this ) ) return false;
		const int N = row();
		l = identity( N );
		d = identity( N );
		for ( int i = 0; i < N; ++i ) {
			for ( int j = 0; j < i; ++j ) {
				T & lij = l[i][j];
				lij = mat[i][j];
				for ( int k = 0; k < j; ++k ) {
					lij -= d[k][k] * l[i][k] * l[j][k];
				}
				if ( lij < 0 ) return false;
				lij = zerodivide( lij, d[j][j] );
			}
			T & dii = d[i][i];
			dii = mat[i][i];
			for ( int k = 0; k < i; ++k ) {
				dii -= d[k][k] * square( l[i][k] );
			}
			if ( dii <= 0 ) return false;
		}
		return true;
	}
	//逆行列
	bool inverse( matrixT<T> & result ) const
	{
		if ( ! squareMatrix() ) {
			result = generalInverse();//特異値分解が必要
			return false;
		}
		const int N = row();
		result = identity( N );
		//LU分解
		matrixT lu;//計算用
		vectorT<int> exchange;//ピボット選択のためのテーブル　行を入れ替える
		if ( ! luFactorization( lu, exchange ) ) {
			result = generalInverse();//特異値分解が必要
			return false;
		}
		//ｘに逆行列の１列が入るようにｂを単位行列から抜き出して計算する
		vectorT<T> b;
		vectorT<T> x;
		for ( int i = 0; i < N; ++i ) {
			b = vectorT<T>::zero( N );
			b[i] = 1;
			if ( ! lu.luSolve( b, x, & exchange ) ) {
				result = generalInverse();//特異値分解が必要
				return false;
			}
			for ( int j = 0; j < N; ++j ) result[j][i] = x[j];
		}
		return true;
	}

	//行列値
	static T determinant( const matrixT<T> & m )
	{
		if ( ! m.squareMatrix() ) return 0;
		const int N = m.row();
		if ( N == 1 ) return m[0][0];
		//次元が大きいときにはLU分解し，対角要素の積から行列式を計算する
		matrixT lu;//計算用
		vectorT<int> exchange;//ピボット選択のためのテーブル　行を入れ替える
		if ( ! m.luFactorization( lu, exchange ) ) return 0;
		T result = 1;
		for ( int i = 0; i < N; ++i ) result *= lu[exchange[i]][i];
		return result;
	}

	//正規化
	static const matrixT normalize( const matrixT & m ) 
	{
		T l = determinant( m );
		if ( absolute( l - 1 ) < epsilon ) return m;
		return m / l;
	}
	//Householder変換
	//列ベクトルvに垂直な平面鏡に映った列ベクトルxの像yは
	//u=normalize(v)
	//y=x-2(u･x)u
	//y=(E-2v(vT)/((vT)v)   行列形式

	//x(x1,...,xn)Tをy(x1,...,xi,y,0,...,0)T に移すHouseholder変換 0<=i<=N-2
	//s=length(xi+1,...,xn) 長さを計算
	//y=-sign( x[i] )*|s| 符号に気をつけて、yの要素を計算
	//v=c(x-y)のため v(0,...,0,c( (xi+1) - y ), c(xi+2),...,c xn)
	//cはvが√２の長さを持つための係数
	//v・v = s^2 + y^2 - 2 xi y = 2 ( s^2 + | xi | * |s| ) = 2 c
	//c=s^2 + | xi | * |s|で割ればいい。
	//i: number of constant element
	void householder( const vectorT<T> & x, vectorT<T> & v, T & y, int constant )
	{
		const int N = x.dimension();
		*this = identity( N );
		v = vectorT<T>::zero( N );
		y = 0;
		if ( N <= 0 || ! between( 0, constant, N - 2 ) ) {//無変換
			return;
		}
		T s = 0;//変更可能な要素のユークリッドノルムを計算
		for ( int j = constant; j < N; ++j ) {
			s += square( x[j] );
		}
		s = sqrt( s );
		if ( s == 0 ) {//長さが０なので，全て０．変換は単位行列
			return;
		}
		y = - plus( x[constant] ) * s;//y=-sign( x[i] )*|s|
		v = x;
		for ( int j = 0; j < constant; ++j ) {
			v[j] = 0;
		}
		v[constant] = x[constant] - y;
		const T c = ::sqrt( square( s ) + absolute( x[constant] * s ) );
		v /= c;
		*this -= ket( v ) * bra( v );//縦ベクトルｘ横ベクトルで行列にして，単位行列から引く
	}

	//x(x1,...,xn)Tをy(y1,0,...,0)T |にうつす変換
	//二重対角化で用いる
	void householder1( const vectorT<T> & x, vectorT<T> & v, T & y1 )
	{
		householder( x, v, y1, 0 );
	}

	//x(x1,...,xn)Tをy(x1,y2,0,...,0)T |にうつす変換
	//三重対角化、Hessenberg化で用いる
	void householder2( const vectorT<T> & x, vectorT<T> & v, T & y2 )
	{
		householder( x, v, y2, 1 );
	}

	//Givens変換
	// ( c, -s )
	// ( s,  c )
	//単位行列において、を変換したい２つの行、列の対応する位置にcosθと±sinθの要素をもつ回転変換
	//通常２つの行または列のうちある要素を零にするように回転する。
	
	//G (a,b)T，aを零にするようなGivens変換の角度を返す
	static const T givensLeftA( const T a, const T b )
	{ return atan2( a, b ); }
	//G (a,b)T，bを零にするようなGivens変換の角度を返す
	static const T givensLeftB( const T a, const T b )
	{ return atan2( -b, a ); }
	//(a,b) G，aを零にするようなGivens変換の角度を返す
	static const T givensRightA( const T a, const T b )
	{ return atan2( -a, b ); }
	//(a,b) G，bを零にするようなGivens変換の角度を返す
	static const T givensRightB( const T a, const T b )
	{ return atan2( b, a ); }

	//左側から掛けて，iとkの行を変更しi,jの要素を０にするようなGivens変換行列を返す
	const matrixT givensLeft( const int i, const int j, const int k )
	{
		matrixT g = identity( row(), column() );
		const T & sita = ( j < i ) ? //消す要素が対角線からどちらにあるかが指標となる
			givensLeftB( mat[k][j], mat[i][j] ) ://左下にある場合
			givensLeftA( mat[i][j], mat[k][j] );//右上にある場合
		const T & c = cos( sita );
		const T & s = sin( sita );
		//左下の要素に正のsが来るようにする
		if ( k < i ) {
			g[k][k] = c; g[k][i] = -s;
			g[i][k] = s; g[i][i] = c;
		} else {
			g[i][i] = c; g[i][k] = -s;
			g[k][i] = s; g[k][k] = c;
		}
		return g;
	}
	//右側から掛けて，kとjの列を変更しi,jの要素を０にするようなGivens変換行列を返す
	const matrixT givensRight( const int i, const int j, const int k )
	{
		matrixT g = identity( row(), column() );
		const T & sita = ( j < i ) ? 
			givensRightA( mat[i][j], mat[i][k] ) : 
			givensRightB( mat[i][k], mat[i][j] );
		const T & c = cos( sita );
		const T & s = sin( sita );
		if ( j < k ) {
			g[j][j] = c; g[j][k] = -s;
			g[k][j] = s; g[k][k] = c;
		} else {
			g[k][k] = c; g[k][j] = -s;
			g[j][k] = s; g[j][j] = c;
		}
		return g;
	}
	//左側から掛けて，iとjの行を変更しi,jの要素を０にするようなGivens変換行列を返す
	const matrixT givensLeft( const int i, const int j )
	{
		return givensLeft( i, j, j );
	}
	//右側から掛けて，iとjの列を変更しi,jの要素を０にするようなGivens変換行列を返す
	const matrixT givensRight( const int i, const int j )
	{
		return givensRight( i, j, i );
	}
	//三重対角化
	//A->P† D3 Pと相似変換を行う時のPも戻す
	void diag3( matrixT & P, bool U = true )
	{
		*this = fix( *this );
		const int N = row();
		P = identity( N );
		if ( ! squareMatrix() ) return;
		vectorT<T> x, v, vi;
		x = vectorT<T>::zero( N );
		v = vectorT<T>::zero( N );
		T y2;
		matrixT Pi;
		for ( int i = 0; i < N - 2; ++i ) {
			//右下の小行列だけを気にする
			x = vectorT<T>::zero( N - i );//０にする列の要素だけを取り出す
			for ( int j = i; j < N; ++j ) {
				x[j-i] = mat[j][i];
			}
			Pi.householder2( x, vi, y2 );
			//次元をあわせる
			for ( int j = 0; j < i; ++j ) v[j] = 0;
			for ( int j = i; j < N; ++j ) v[j] = vi[j-i];
			Pi = identity( N ) - ket( v ) * bra( v );
			P = P * Pi;//hessenberg化のときの射影は正規直交行列のため、転置はいらない
			*this = fix( Pi * *this * Pi );
			//零になっている個所を０に書き換えておく
			for ( int j = i + 2; j < N; ++j ) {
				if ( U ) mat[i][j] = 0;//Uがfalseの時には，右上に要素があるhessenberg化を行っている
				mat[j][i] = 0;
			}
		}
	}
	//Hessenberg化
	void hessenberg( matrixT & P )
	{
		diag3( P, false );
	}
	//二重対角化
	//householder変換により，左右から異なる直交変換で二重対角化を行う．
	//条件：正方行列
	//A->L D2 R=diag2と直交変換を行う時のL,Rも戻す
	void diag2( matrixT & L, matrixT & R )
	{
		*this = fix( *this );
		L = identity( row() );
		R = identity( column() );
		vectorT<T> x, v;
		T y;
		matrixT P;
		for ( int i = 0; i < row() - 1 && i < column() - 1; ++i ) {
			//[i][i]より下の値を０とする
			x = ket( i );
			P.householder( x, v, y, i );
			L = P * L;
			*this = fix( P * *this );

			if ( i == column() - 2 ) break;//ここで終了判定
			
			//[i][i+1]より右の値を０とする
			x = bra( i );
			P.householder( x, v, y, i+1 );
			R = R * P;
			*this = fix( *this * P );
		}
	}
private:
	////////////////////////////////////////////////////////////////////////////////
	//三重対角化対称行列の[3][0],[0][3]の非対角要素が０の時に３重対角行列に戻すための追い込み
	//追い込みの順序はドキュメント参照
	void chasingDiag3( matrixT & u )
	{
		if ( column() <= 2 ) return;
		const int c = column();
		for ( int i = 2; i < column(); ++i ) {
			if ( mat[i][i-2] == 0 ) return;
			const matrixT & g = givensLeft( i, i - 2, i - 1 );
			// ( U† G† ) G A G† ( G U )
			u = g * u;
			*this = fix( g * *this * transpose( g ) );
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//三重対角化対称行列用の固有値分解
	//A=U† D Uに分解 Dは自身に入る
	//U A U†=D
	void evd4diag3( matrixT & u )
	{
		// U† D3 U ->U'† D U'

		const int C = column();

		//１列の三重対角行列の場合には，固有値分解が終了している
		if ( C == 1 ) return;

		while ( ! diagMatrix() ) {
			//最初のGivens変換を求める。
			matrixT G = identity( C, C );
			{
				const T & a = mat[0][0];
				const T & b = mat[1][0];
				const T & d = mat[1][1];
				const T D = square( a - d ) + 4 * square( b );
				const T lamda = ( a + d - sqrt( D ) ) / 2;
				const T size = sqrt( square( a - lamda ) + square( b ) );
				G[0][0] = ( a - lamda ) / size; G[0][1] = -b / size; 
				G[1][0] = b / size; G[1][1] = ( a - lamda ) / size;
			}
			// G† A Gとして変形する
			matrixT GT = transpose( G );
			u = GT * u;// ( U† G ) G† A G ( G† U )
			*this = fix( GT * *this * G );
			chasingDiag3( u );

			//左上の非対角要素をチェック
			if ( absolute( mat[1][0] ) < epsilon ) {
				//この状態でi,iで４つに分けた時には右上も零行列になるため、２つに分けて処理を行う
				//C==2なら、すでに対角化済みである。
				if ( C == 2 ) return;

				// 固有値を一つ固定し、右下の行列のみで計算する
				matrixT d1 = minor( 1, 1, row(), column() );
				matrixT u1 = identity( d1.column(), d1.column() );
				d1.evd4diag3( u1 );
				copy( 1, 1, d1 );

				matrixT nu = identity( row(), column() );
				nu.copy( 1, 1, u1 );
				u = nu * u;
				return;
			}
		}
	}
public:
	////////////////////////////////////////////////////////////////////////////////
	//固有値の解析用に固有値の絶対値の大きい順にソート
	static void sort4evd( vectorT<T> & values, matrixT & vectors )
	{
		const int N = values.dimension();
		//選択ソート
		for ( int i = 0; i < N; ++i ) {
			//最大を探す
			checkMaximum<T> mx;
			for ( int j = i; j < N; ++j ) {
				mx( absolute( values[j] ), j );
			}
			if ( i != mn.sub ) {
				swap( values[i], values[mx.sub] );
				swap( vectors[i], vectors[mx.sub] );
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//実対称行列の固有値と固有ベクトル
	//Householder-QR法による
	//非対象実行列の固有値・固有ベクトルは複素数になり、一般逆行列を求めるだけならば特異値分解で十分なため、実対称行列のみとする。
	bool eigen( vectorT<T> & values, matrixT & vectors ) const
	{
		if ( ! symmetricMatrix() ) return false;
		//三重対角行列に変更
		matrixT A = *this;
		// A->P† D3 P
		A.diag3( vectors );
		//固有値分解
		// P† D3 P ->U† D U
		A.evd4diag3( vectors );
		values = A.diagVector();
		vectors = transpose( vectors );//evec eval evec†にする
		//sort4evd( values, vectors );
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////
	//冪乗法
	//最大の固有値と対応する固有ベクトルを求める
	void powerMethod( T & eigenvalue, vectorT<T> & eigenvector ) const
	{
		eigenvector = vectorT<T>::normalize( vectorT<T>::one( column() ) );
		do {
			eigenvalue = vectorT<T>::dot( eigenvector, *this * eigenvector );
			eigenvector = vectorT<T>::normalize( *this * eigenvector );
		} while ( absolute( eigenvalue - vectorT<T>::dot( eigenvector, *this * eigenvector ) ) < tiny );
	}
	////////////////////////////////////////////////////////////////////////////////
	//レイリー商逆反復法
	//固有ベクトルの近似値から固有値を得、固有ベクトルも近似していく
	void rayleighQuotientInverseIterationMethod( T & eigenvalue, vectorT<T> & eigenvector ) const
	{
		matrixT A = *this;
		matrixT I = identity( row() );
		eigenvalue = rayleighQuotient( eigenvector );
		while ( vectorT<T>::norm( *this * eigenvector - eigenvector * eigenvalue ) > epsilon ) {
			vectorT<T> nv = eigenvector;
			( A - I * eigenvalue ).qrSolve( eigenvector, nv );
			eigenvector = normalize( nv );
			eigenvalue = rayleighQuotient( eigenvector );
		};
	}
	////////////////////////////////////////////////////////////////////////////////
	//レイリー商
	//近似的な固有ベクトルから，固有値を計算する
	const T rayleighQuotient( const vectorT<T> & eigenvector ) const
	{
		return zerodivide( vectorT<T>::dot( *this * eigenvector, eigenvector ), 
			vectorT<T>::dot( eigenvector, eigenvector ) );
	}
private:
	////////////////////////////////////////////////////////////////////////////////
	//特異値分解
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	//縦型長方二重対角行列の右下の対角要素が０の時にその上の要素を０にするためのGivens変換
	//追い込みの順序はドキュメント参照
	void chasingRightBottom( matrixT & u )
	{
		if ( column() <= 1 ) return;
		const int c = column();
		for ( int i = c - 2; 0 <= i; --i ) {
			if ( mat[i][c-1] == 0 ) return;
			const matrixT & g = givensRight( i, c - 1 );
			u = u * g;// X * G * G† * U†=X * G * (U*G)†
			*this = fix( *this * g );
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//縦型長方二重対角行列の右下でない対角要素が０の時にその右の要素を０にするためのGivens変換
	//追い込みの順序はドキュメント参照
	void chasingDiag( matrixT & v, const int i )
	{
		if ( column() <= 1 ) return;
		const int c = column();
		for ( int j = i + 1; j < c; ++j ) {
			if ( absolute( mat[i][j] ) < epsilon ) return;
			const matrixT & g = givensLeft( i, j );
			v = v * transpose( g );
			*this = fix( g * *this );
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//縦型長方二重対角行列の対角要素とその上の要素が非０の時に対角行列に近づける
	//追い込みの順序はドキュメント参照
	void chasing( matrixT & v, matrixT & u )
	{
		if ( column() <= 1 ) return;

		const matrixT & g01 = givensRight( 0, 1 );
		u = u * g01;
		*this = fix( *this * g01 );

		const matrixT & g10 = givensLeft( 1, 0 );
		v = v * transpose( g10 );
		*this = fix( g10 * *this );

		const int c = column();
		const int r = row();
		int i = 0, j = 2;
		for ( ; i < r && j < c; ) {
			const matrixT & gr = givensRight( i, j, i+1 );
			u = u * gr;
			*this = fix( *this * gr );
			i += 2; j -= 1;

			if ( ! ( i < r && j < c ) ) break;

			const matrixT & gl = givensLeft( i, j );
			v = v * transpose( gl );
			*this = fix( gl * *this );
			i -= 1; j += 2;
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//二重対角化行列用の特異値分解
	//A=V D U†に分解 Dは自身に入る
	void svd4diag2( matrixT & v, matrixT & u )
	{
		//零要素を検索
		const int C = column();

		//１列の二重対角行列の場合には，特異値分解が終了している
		if ( C == 1 ) return;

		while ( ! diagMatrix() ) {
			//右下の対角要素をチェック
			if ( absolute( mat[C-1][C-1] ) < epsilon ) {
				chasingRightBottom( u );//左側から追い込んでその上の要素を全て零にする
			}
			for ( int i = C - 2; 0 <= i; --i ) {
				// i, iの対角要素をチェック
				if ( absolute( mat[i][i] ) < epsilon ) {
					chasingDiag( v, i );//右側から追い込んでその右の要素を全て零にする
				}
				// i, i+1をチェック
				if ( absolute( mat[i][i+1] ) < epsilon ) {
					//この状態でi,iで４つに分けた時には右上も零行列になるため、２つに分けて処理を行う

					// i,iまでとi+1,i+1からで二つにわける
					matrixT d1 = minor( 0, 0, i + 1, i + 1 );
					matrixT v1 = identity( d1.row(), d1.row() );
					matrixT u1 = identity( d1.column(), d1.column() );
					d1.svd4diag2( v1, u1 );

					matrixT d2 = minor( i + 1, i + 1, row(), column() );
					matrixT v2 = identity( d2.row(), d2.row() );
					matrixT u2 = identity( d2.column(), d2.column() );
					d2.svd4diag2( v2, u2 );

					*this = zero( row(), column() );
					copy( 0, 0, d1 );
					copy( d1.row(), d1.column(), d2 );

					matrixT nv = zero( row(), row() );
					nv.copy( 0, 0, v1 );
					nv.copy( v1.row(), v1.column(), v2 );
					v = v * nv;

					matrixT nu = zero( column(), column() );
					nu.copy( 0, 0, u1 );
					nu.copy( u1.row(), u1.column(), u2 );
					u = u * nu;
					return;
				}
			}
			//追い込み
			chasing( v, u );
		}
	}
public:
	////////////////////////////////////////////////////////////////////////////////
	//特異値分解用に特異値の絶対値の大きい順にソート
	static void sort4svd( matrixT & v, matrixT & d, matrixT & u )
	{
		const int N = minimum( d.row(), d.column() );
		//選択ソート
		for ( int i = 0; i < N; ++i ) {
			//最大を探す
			checkMaximum<T> mx;
			for ( int j = i; j < N; ++j ) {
				mx( absolute( d[j][j] ), j );
			}
			if ( i != mn.sub ) {
				swap( d[i][i], d[mx.sub][mx.sub] );
				swap( v[i], v[mx.sub] );
				swap( u[i], u[mx.sub] );
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//特異値分解
	//A=V D U†に分解
	//二重対角化を行い，Givens変換により非対角化要素を追い込みで零に近づけて，分解を行う手法
	//条件：無し ただし，未ソート
	void singularValueDecomposition( matrixT & v, matrixT & d, matrixT & u ) const
	{
		const int R = row();
		const int C = column();
		if ( C < R ) {//縦長方行列へ変換
			matrixT A = transpose( *this );
			A.singularValueDecomposition( u, d, v );
			u = transpose( u );
			v = transpose( v );
			return;
		}
		//二重対角化行列へ変換
		d = *this;
		d.diag2( u, v );// A -> u A v = A'として二重対角化
		// A = v† A' u†分解としては逆になる
		v = transpose( v );// v A u†形式に変換
		d.svd4diag2( v, u );//分解
		//sort4svd( v, d, u );//ソート
	}
	
	////////////////////////////////////////////////////////////////////////////////
	//Moore-Penroseの一般逆行列
	const matrixT generalInverse() const
	{
		matrixT v, d, u;// A=v d u†と分解
		singularValueDecomposition( v, d, u );
		// A^+ = u 1/d v†
		return u * d.inverseDiag() * transpose( v );
	}

	////////////////////////////////////////////////////////////////////////////////
	//デバッグ用文字出力
	static void debug( const matrixT & m, const char * tag = NULL )
	{
		if ( tag ) {
			OutputDebugString( tag );
			OutputDebugString( ":[matrix]\n" );
		}
		for ( int i = 0; i < m.row(); ++i ) {
			vectorT<T>::debug( m[i] );
		}
	}
};

//四則演算
template<typename T>
const matrixT<T> operator+( const T & s, const matrixT<T> & m ) { return m + s; }
template<typename T>
const matrixT<T> operator-( const T & s, const matrixT<T> & m ) { return - m + s; }
template<typename T>
const matrixT<T> operator*( const T & s, const matrixT<T> & m ) { return m * s; }
template<typename T>
const matrixT<T> operator/( const T & s, const matrixT<T> & m )
{
	matrixT<T> result( m );
	for ( int r = 0; r < result.row(); ++r ) {
		for ( int c = 0; c < result.column(); ++c ) {
			result[r][c] = s / m[r][c];
		}
	}
	return r;
}

template<typename T>
const vectorT<T> operator*( const vectorT<T> v, const matrixT<T> & m )
{
	vectorT<T> result = vectorT<T>::zero( column() );
	for ( int c = 0; c < column(); ++c ) {
		T & sum = result[c];
		for ( int r = 0; r < row() && r < v.dimension(); ++r ) {
			sum += v[r] * m[r][c];
		}
	}
	return result;
}
