////////////////////////////////////////////////////////////////////////////////
// Matrix �^�Ɨv�f���̃e���v���[�g�Łi���l�v�Z�p�̂��ߐ��x�Ǝ������K�v�Ŗ����Ƃ��ɂ͗p���Ȃ��j
// ������ BUG DEBUG

template<typename T>
class matrixT
{
	array< vectorT<T> > mat;
	// ���̍s(row)���x�N�g���Ƃ��Ă����A�c�̗�(column)�����̐�
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
	//�s(row)�̐��A�Ⴕ���͗�(column)�̗v�f��
	int row() const { return mat.size; }
	//��(column)�̐��A�Ⴕ���͍s(row)�̗v�f��
	int column() const { if ( ! row() ) return 0; return mat.first().dimension(); }
	//���Z
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
	//���x�N�g���Ƃ��Ď��o���B
	const vectorT<T> bra( int r ) const
	{
		return mat[r];
	}
	//�c�x�N�g���Ƃ��Ď��o���B
	const vectorT<T> ket( int c ) const
	{
		vectorT<T> result( row() );
		for ( int r = 0; r < row(); ++r ) {
			result[r] = mat[r][c];
		}
		return result;
	}
	//���x�N�g�����s��`���ɕϊ�
	static matrixT bra( const vectorT<T> & v )
	{
		matrixT result( 1, v.dimension() );
		result[0] = v;
		return result;
	}
	//�c�x�N�g�����s��`���ɕϊ�
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
	//�s�̌���
	void swapbra( int i, int j )
	{
		for ( int c = 0; c < column(); ++c ) swap( mat[i][c], mat[j][c] );
	}
	//��̌���
	void swapket( int i, int j )
	{
		for ( int r = 0; r < row(); ++r ) swap( mat[r][i], mat[r][j] );
	}
	//�s��ʂ̍s�ɌW�����|���đ���
	void addbra( int from, int to, T v )
	{
		for ( int c = 0; c < column(); ++c ) mat[to][c] += v * mat[from][c];
	}
	//�L�������ׂ�
	bool valid() const
	{
		for ( int r = 0; r < row(); ++r ) if ( ! mat[r].valid() ) return false;
		return true;
	}
	//0�ɋ߂��v�f���O�ɂ���
	static const matrixT fix( const matrixT & m )
	{
		matrixT result( m );
		for ( int i = 0; i < m.row(); ++i ) {
			result[i] = vectorT<T>::fix( result[i] );
		}
		return result;
	}
	//�]�u
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
	//����
	static T dot( const matrixT & left, const matrixT & right )
	{
		T result = 0;
		for ( int r = 0; r < left.row() && r < right.row(); ++r ) {
			result += vectorT<T>::dot( left[r], right[r] );
		}
		return result;
	}
	//�m����
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
	//�Ίp�̐�
	const T trace() const
	{
		T result = 1;
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result *= mat[r][r];
		}
		return result;
	}
	//�Ίp�s��
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
	//���O�p�s��
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
	//��O�p�s��
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
	//�����s��
	bool squareMatrix() const
	{
		if ( row() != column() ) return false;
		return true;
	}
	//�Ώ̍s��
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
	//���s����擾
	//�c�����Ɖ������̍ŏ��ƍŌ�̗v�f���w��
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
	//���s����擾
	const matrixT minor( int start, int end ) const
	{
		return minor( start, start, end, end );
	}
	//���s����R�s�[���č���
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
	//�Ίp�v�f�݂̂��������s����쐬
	const matrixT diag() const
	{
		matrixT result = identity( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r][r] = mat[r][r];
		}
		return result;
	}
	//�Ίp�v�f�̃x�N�g�����쐬
	const vectorT<T> diagVector() const
	{
		vectorT<T> result = vectorT<T>::zero( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r] = mat[r][r];
		}
		return result;
	}
	//�Ίp�s��̈�ʋt�s���߂�
	const matrixT inverseDiag() const
	{
		matrixT result = identity( minimum( row(), column() ) );
		for ( int r = 0; r < row() && r < column(); ++r ) {
			result[r][r] = zerodivide<T>( 1, mat[r][r] );
		}
		return result;
	}
	//LU����
	//���͂����s���lu��������
	//L�����O�p�s��̑Ίp�������P�CU�E��O�p�s���L * U�̌`�ɍs��𕪉�����
	//�K�E�X�̏����@�ŗp�����Ίp�v�f�Ɋ|�����l��L�s��̂O�ɂ������ɑ�����Ēu������L���v�Z�ł���D
	//U�̓K�E�X�̏����@�̌��ʂł���D
	//�\�[�X�̌��̓A���S���Y�����T

	//���@�K�E�X�̏����@�F�����̗v�f���O�ɂ��邽�߂�
	//������̑Ίp�v�f�̒l�ɂ���l���|���Ă��̍s�S�̂������D
	//��������ォ�獶����E�ցC���̌�ォ�牺�֏��ɍs���Ă�����@
	// LU�����̌��ʂ�p����m*x=b����x������
	bool luFactorization( matrixT & lu, vectorT<int> & exchange ) const
	{
		if ( ! squareMatrix() ) return false;
		int N = row();
		vectorT<T> weight( N );
		exchange = vectorT<int>( N );
		for ( int k = 0; k < N; k++ ) {  // �e��ɂ���
			exchange[k] = k;// �s�������̏����l
			checkMaximum<T> mx;// ���̍s�̐�Βl�ő�̗v�f�����߂�
			for ( int j = 0; j < N; j++ ) {
				mx( absolute( mat[k][j] ) );
			}
			if ( mx() == 0 ) return false; // 0 �Ȃ�s���LU�����ł��Ȃ�
			weight[k] = T( 1.0 / mx() ); // �ő��Βl�̋t��
		}
		lu = *this;
		for ( int k = 0; k < N; k++ ) {  // �e�s�ɂ���
			// ��艺�̊e�s�ɂ��ďd�݁~��Βl ���ő�̍s��������
			{
				checkMaximum<T> mx;
				for ( int i = k; i < N; i++ ) {
					const int ii = exchange[i];
					mx( absolute( lu[ii][k] ) * weight[ii], i );
				}
				// �s�ԍ�������
				swap( exchange[mx.sub], exchange[k] );
			}
			const int ik = exchange[k];//�����������k
			const T vkk = lu[ik][k];//�Ίp����
			if ( vkk == 0 ) return false;// 0 �Ȃ�s���LU�����ł��Ȃ�
			for ( int i = k + 1; i < N; i++) { // Gauss�����@
				const int ii = exchange[i];//�����������i
				lu[ii][k] /= vkk;//L�̗v�f
				const T vik = lu[ii][k];
				for ( int j = k + 1; j < N; j++) {
					lu[ii][j] -= vik * lu[ik][j];
				}
			}
		}
		return true;
	}
	// ���O�p�s��Ƃ��āA�K�E�X�̓f���o���@���s��
	// L x = b
	bool lSolve( const vectorT<T> & b, vectorT<T> & x, const vectorT<int> * exchange = NULL ) const
	{
		if ( ! squareMatrix() ) return false;
		const int N = row();
		if ( exchange && exchange->dimension() != N ) return false;
		if ( b.dimension() != N ) return false;
		x = vectorT<T>::zero( N );
		// l * x = b������
		for ( int i = 0; i < N; i++ ) {
			const int ii = exchange ? (*exchange)[i] : i;
			T t = b[ii];
			for ( int j = 0; j < i; j++ ) {
				t -= mat[ii][j] * x[j];
			}
			x[i] = zerodivide( t, mat[ii][i] );//0�Ȃ�Εs��
		}
		return true;
	}
	// ��O�p�s��Ƃ��āA�K�E�X�̓f���o���@���s��
	// R x = b
	bool rSolve( const vectorT<T> & b, vectorT<T> & x, const vectorT<int> * exchange = NULL ) const
	{
		if ( ! squareMatrix() ) return false;
		const int N = row();
		if ( exchange && exchange->dimension() != N ) return false;
		if ( b.dimension() != N ) return false;
		x = vectorT<T>::zero( N );
		// r * x = b������
		for ( int i = N - 1; i >= 0; i-- ) {
			const int ii = exchange ? (*exchange)[i] : i;
			T t = b[ii];
			for ( int j = i + 1; j < N; j++ ) {
				t -= mat[ii][j] * x[j];
			}
			x[i] = zerodivide( t, mat[ii][i] );//0�Ȃ�Εs��
		}
		return true;
	}
	//LU�����ɂĕ������������ithis=lu�jLU x = b
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
		// L y = b�������AU x = y������
		vectorT<T> y;
		if ( ! L.lSolve( b, y, exchange ) ) return false;
		if ( ! R.rSolve( y, x, exchange ) ) return false;
		return true;
	}
	//GramSchmidt
	//�����x�N�g�����v�Z����B
	//�����F�e��x�N�g����1���Ɨ��łȂ��ꍇ�ɂ́A���͏��Ȃ��Ȃ�B
	void gramSchmidt( matrixT & q ) const
	{
		if ( row() < column() ) {//�����Ȃ�΁A�]�u���Čv�Z����
			matrixT AT = transpose( *this );
			AT.gramSchmidt( q );
			q = transpose( q );
			return;
		}
		//�c���Ȃ̂ŁA��x�N�g���𒼌�����悤�ɋ��߂�
		const int R = row();
		const int C = column();
		//�O�����V���~�b�g�̕��@
		matrixT AT = transpose( *this );
		array< vectorT<T> > orthogonal;
		orthogonal.reserve( C );
		for ( int c = 0; c < C; ++c ) {
			const vectorT<T> & vc = AT[c];//c��̃x�N�g��
			vectorT<T> vo = vc;//��������x�N�g��
			//����܂łɋ��߂��x�N�g���Q�ƕ��s�Ȑ����͑S�č폜����
			for ( int i = 0; i < orthogonal.size; ++i ) {
				vo -= orthogonal[i] * vectorT<T>::dot( vc, orthogonal[i] );
			}
			//�����Ȑ��������邩���ׂ�
			T n = vectorT<T>::norm( vo );
			if ( n < epsilon ) continue;//�����Ȑ����������̂ŁA�Ɨ��ł͂Ȃ����ߍ폜
			orthogonal.push_back( vo / n );//���K�����ĕۑ�
		}
		q.allocate( orthogonal.size, R );
		for ( int c = 0; c < orthogonal.size; ++c ) {
			q[c] = orthogonal[c];
		}
		q = transpose( q );
	}
	//�s��̊K���@SVD�ɂ���@�����ʂ݂��������A1���Ɨ��ȃx�N�g�����������邩���v�Z���ċ��߂�
	int rank() const
	{
		matrixT q;
		gramSchmidt( q );
		return minimum( q.row(), q.column() );
	}
	//QR����
	//�s��A�𒼌��s��Q�ƉE��O�p�s��R�ɕ�������D
	//�����F�e��x�N�g����1���Ɨ��A�������c���̍s��ł��邱��
	void qrFactorization( matrixT & q, matrixT & r ) const
	{
		//if ( row() < column() ) return false;//�c���̍s��̎��̂݌v�Z�\
		const int R = row();
		const int C = column();
		//�C���O�����V���~�b�g�̕��@
		matrixT qT = transpose( *this );
		r = identity( C );
		for ( int k = 0; k < C; ++k ) {
			//�Ίp�v�f�ɂ���
			T & rkk = r[k][k];
			rkk = vectorT<T>::norm( qT[k] );
			//��x�N�g���ł��v�Z�𑱂���
			qT[k] /= rkk;//rkk=0->qTk=0
			for ( int j = k + 1; j < C; ++j ) {//���̉E
				T & rkj = r[k][j];
				rkj = vectorT<T>::dot( qT[k], qT[j] );//rkk=0->rkj=0
				qT[j] -= qT[k] * rkj;
			}
		}
		q = transpose( qT );
	}
	//�s���QR�ϊ����A�f���o���@�ŕ�����������
	bool qrSolve( const vectorT<T> & b, vectorT<T> & x ) const
	{
		matrixT q, matrixT r;
		qrFactorization( q, r );
		// q r x = b
		// r x = qT b
		return r.rSolve( transpose( q ) * b, x );
	}
	//QR�ϊ�
	//A=QR�ƕ������AA'=RQ�ƕϊ�����B����́AA'=Q'AQ�ő����ϊ�
	//P^-1AP�Ƒ����ϊ����s������P���߂�
	//�����F�����s��AQR�����\�i�e��x�N�g����1���Ɨ��j
	bool qrTransform( matrixT & P )
	{
		if ( ! squareMatrix() ) return false;
		matrixT q, r;
		qrFactorization( q, r );
		*this = r * q;
		P = transpose( q );
		return true;
	}
	//�R���X�L�[���� mat=L (L*)�̌`�ɕ���
	//�����C����l�Ώ̐����i�S�Ă̌ŗL�l�����j�C�S�Ă̎�����s�񎮂����
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
	//�C���R���X�L�[���� mat=L D (L*)�̌`�ɕ���
	//�����C�Ώ̐����C�S�Ă̎�����s�񎮂����
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
	//�t�s��
	bool inverse( matrixT<T> & result ) const
	{
		if ( ! squareMatrix() ) {
			result = generalInverse();//���ْl�������K�v
			return false;
		}
		const int N = row();
		result = identity( N );
		//LU����
		matrixT lu;//�v�Z�p
		vectorT<int> exchange;//�s�{�b�g�I���̂��߂̃e�[�u���@�s�����ւ���
		if ( ! luFactorization( lu, exchange ) ) {
			result = generalInverse();//���ْl�������K�v
			return false;
		}
		//���ɋt�s��̂P�񂪓���悤�ɂ���P�ʍs�񂩂甲���o���Čv�Z����
		vectorT<T> b;
		vectorT<T> x;
		for ( int i = 0; i < N; ++i ) {
			b = vectorT<T>::zero( N );
			b[i] = 1;
			if ( ! lu.luSolve( b, x, & exchange ) ) {
				result = generalInverse();//���ْl�������K�v
				return false;
			}
			for ( int j = 0; j < N; ++j ) result[j][i] = x[j];
		}
		return true;
	}

	//�s��l
	static T determinant( const matrixT<T> & m )
	{
		if ( ! m.squareMatrix() ) return 0;
		const int N = m.row();
		if ( N == 1 ) return m[0][0];
		//�������傫���Ƃ��ɂ�LU�������C�Ίp�v�f�̐ς���s�񎮂��v�Z����
		matrixT lu;//�v�Z�p
		vectorT<int> exchange;//�s�{�b�g�I���̂��߂̃e�[�u���@�s�����ւ���
		if ( ! m.luFactorization( lu, exchange ) ) return 0;
		T result = 1;
		for ( int i = 0; i < N; ++i ) result *= lu[exchange[i]][i];
		return result;
	}

	//���K��
	static const matrixT normalize( const matrixT & m ) 
	{
		T l = determinant( m );
		if ( absolute( l - 1 ) < epsilon ) return m;
		return m / l;
	}
	//Householder�ϊ�
	//��x�N�g��v�ɐ����ȕ��ʋ��ɉf������x�N�g��x�̑�y��
	//u=normalize(v)
	//y=x-2(u�x)u
	//y=(E-2v(vT)/((vT)v)   �s��`��

	//x(x1,...,xn)T��y(x1,...,xi,y,0,...,0)T �Ɉڂ�Householder�ϊ� 0<=i<=N-2
	//s=length(xi+1,...,xn) �������v�Z
	//y=-sign( x[i] )*|s| �����ɋC�����āAy�̗v�f���v�Z
	//v=c(x-y)�̂��� v(0,...,0,c( (xi+1) - y ), c(xi+2),...,c xn)
	//c��v����Q�̒����������߂̌W��
	//v�Ev = s^2 + y^2 - 2 xi y = 2 ( s^2 + | xi | * |s| ) = 2 c
	//c=s^2 + | xi | * |s|�Ŋ���΂����B
	//i: number of constant element
	void householder( const vectorT<T> & x, vectorT<T> & v, T & y, int constant )
	{
		const int N = x.dimension();
		*this = identity( N );
		v = vectorT<T>::zero( N );
		y = 0;
		if ( N <= 0 || ! between( 0, constant, N - 2 ) ) {//���ϊ�
			return;
		}
		T s = 0;//�ύX�\�ȗv�f�̃��[�N���b�h�m�������v�Z
		for ( int j = constant; j < N; ++j ) {
			s += square( x[j] );
		}
		s = sqrt( s );
		if ( s == 0 ) {//�������O�Ȃ̂ŁC�S�ĂO�D�ϊ��͒P�ʍs��
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
		*this -= ket( v ) * bra( v );//�c�x�N�g�������x�N�g���ōs��ɂ��āC�P�ʍs�񂩂����
	}

	//x(x1,...,xn)T��y(y1,0,...,0)T |�ɂ����ϊ�
	//��d�Ίp���ŗp����
	void householder1( const vectorT<T> & x, vectorT<T> & v, T & y1 )
	{
		householder( x, v, y1, 0 );
	}

	//x(x1,...,xn)T��y(x1,y2,0,...,0)T |�ɂ����ϊ�
	//�O�d�Ίp���AHessenberg���ŗp����
	void householder2( const vectorT<T> & x, vectorT<T> & v, T & y2 )
	{
		householder( x, v, y2, 1 );
	}

	//Givens�ϊ�
	// ( c, -s )
	// ( s,  c )
	//�P�ʍs��ɂ����āA��ϊ��������Q�̍s�A��̑Ή�����ʒu��cos�ƂƁ}sin�Ƃ̗v�f������]�ϊ�
	//�ʏ�Q�̍s�܂��͗�̂�������v�f���ɂ���悤�ɉ�]����B
	
	//G (a,b)T�Ca���ɂ���悤��Givens�ϊ��̊p�x��Ԃ�
	static const T givensLeftA( const T a, const T b )
	{ return atan2( a, b ); }
	//G (a,b)T�Cb���ɂ���悤��Givens�ϊ��̊p�x��Ԃ�
	static const T givensLeftB( const T a, const T b )
	{ return atan2( -b, a ); }
	//(a,b) G�Ca���ɂ���悤��Givens�ϊ��̊p�x��Ԃ�
	static const T givensRightA( const T a, const T b )
	{ return atan2( -a, b ); }
	//(a,b) G�Cb���ɂ���悤��Givens�ϊ��̊p�x��Ԃ�
	static const T givensRightB( const T a, const T b )
	{ return atan2( b, a ); }

	//��������|���āCi��k�̍s��ύX��i,j�̗v�f���O�ɂ���悤��Givens�ϊ��s���Ԃ�
	const matrixT givensLeft( const int i, const int j, const int k )
	{
		matrixT g = identity( row(), column() );
		const T & sita = ( j < i ) ? //�����v�f���Ίp������ǂ���ɂ��邩���w�W�ƂȂ�
			givensLeftB( mat[k][j], mat[i][j] ) ://�����ɂ���ꍇ
			givensLeftA( mat[i][j], mat[k][j] );//�E��ɂ���ꍇ
		const T & c = cos( sita );
		const T & s = sin( sita );
		//�����̗v�f�ɐ���s������悤�ɂ���
		if ( k < i ) {
			g[k][k] = c; g[k][i] = -s;
			g[i][k] = s; g[i][i] = c;
		} else {
			g[i][i] = c; g[i][k] = -s;
			g[k][i] = s; g[k][k] = c;
		}
		return g;
	}
	//�E������|���āCk��j�̗��ύX��i,j�̗v�f���O�ɂ���悤��Givens�ϊ��s���Ԃ�
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
	//��������|���āCi��j�̍s��ύX��i,j�̗v�f���O�ɂ���悤��Givens�ϊ��s���Ԃ�
	const matrixT givensLeft( const int i, const int j )
	{
		return givensLeft( i, j, j );
	}
	//�E������|���āCi��j�̗��ύX��i,j�̗v�f���O�ɂ���悤��Givens�ϊ��s���Ԃ�
	const matrixT givensRight( const int i, const int j )
	{
		return givensRight( i, j, i );
	}
	//�O�d�Ίp��
	//A->P�� D3 P�Ƒ����ϊ����s������P���߂�
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
			//�E���̏��s�񂾂����C�ɂ���
			x = vectorT<T>::zero( N - i );//�O�ɂ����̗v�f���������o��
			for ( int j = i; j < N; ++j ) {
				x[j-i] = mat[j][i];
			}
			Pi.householder2( x, vi, y2 );
			//���������킹��
			for ( int j = 0; j < i; ++j ) v[j] = 0;
			for ( int j = i; j < N; ++j ) v[j] = vi[j-i];
			Pi = identity( N ) - ket( v ) * bra( v );
			P = P * Pi;//hessenberg���̂Ƃ��̎ˉe�͐��K�����s��̂��߁A�]�u�͂���Ȃ�
			*this = fix( Pi * *this * Pi );
			//��ɂȂ��Ă�������O�ɏ��������Ă���
			for ( int j = i + 2; j < N; ++j ) {
				if ( U ) mat[i][j] = 0;//U��false�̎��ɂ́C�E��ɗv�f������hessenberg�����s���Ă���
				mat[j][i] = 0;
			}
		}
	}
	//Hessenberg��
	void hessenberg( matrixT & P )
	{
		diag3( P, false );
	}
	//��d�Ίp��
	//householder�ϊ��ɂ��C���E����قȂ钼��ϊ��œ�d�Ίp�����s���D
	//�����F�����s��
	//A->L D2 R=diag2�ƒ���ϊ����s������L,R���߂�
	void diag2( matrixT & L, matrixT & R )
	{
		*this = fix( *this );
		L = identity( row() );
		R = identity( column() );
		vectorT<T> x, v;
		T y;
		matrixT P;
		for ( int i = 0; i < row() - 1 && i < column() - 1; ++i ) {
			//[i][i]��艺�̒l���O�Ƃ���
			x = ket( i );
			P.householder( x, v, y, i );
			L = P * L;
			*this = fix( P * *this );

			if ( i == column() - 2 ) break;//�����ŏI������
			
			//[i][i+1]���E�̒l���O�Ƃ���
			x = bra( i );
			P.householder( x, v, y, i+1 );
			R = R * P;
			*this = fix( *this * P );
		}
	}
private:
	////////////////////////////////////////////////////////////////////////////////
	//�O�d�Ίp���Ώ̍s���[3][0],[0][3]�̔�Ίp�v�f���O�̎��ɂR�d�Ίp�s��ɖ߂����߂̒ǂ�����
	//�ǂ����݂̏����̓h�L�������g�Q��
	void chasingDiag3( matrixT & u )
	{
		if ( column() <= 2 ) return;
		const int c = column();
		for ( int i = 2; i < column(); ++i ) {
			if ( mat[i][i-2] == 0 ) return;
			const matrixT & g = givensLeft( i, i - 2, i - 1 );
			// ( U�� G�� ) G A G�� ( G U )
			u = g * u;
			*this = fix( g * *this * transpose( g ) );
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//�O�d�Ίp���Ώ̍s��p�̌ŗL�l����
	//A=U�� D U�ɕ��� D�͎��g�ɓ���
	//U A U��=D
	void evd4diag3( matrixT & u )
	{
		// U�� D3 U ->U'�� D U'

		const int C = column();

		//�P��̎O�d�Ίp�s��̏ꍇ�ɂ́C�ŗL�l�������I�����Ă���
		if ( C == 1 ) return;

		while ( ! diagMatrix() ) {
			//�ŏ���Givens�ϊ������߂�B
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
			// G�� A G�Ƃ��ĕό`����
			matrixT GT = transpose( G );
			u = GT * u;// ( U�� G ) G�� A G ( G�� U )
			*this = fix( GT * *this * G );
			chasingDiag3( u );

			//����̔�Ίp�v�f���`�F�b�N
			if ( absolute( mat[1][0] ) < epsilon ) {
				//���̏�Ԃ�i,i�łS�ɕ��������ɂ͉E�����s��ɂȂ邽�߁A�Q�ɕ����ď������s��
				//C==2�Ȃ�A���łɑΊp���ς݂ł���B
				if ( C == 2 ) return;

				// �ŗL�l����Œ肵�A�E���̍s��݂̂Ōv�Z����
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
	//�ŗL�l�̉�͗p�ɌŗL�l�̐�Βl�̑傫�����Ƀ\�[�g
	static void sort4evd( vectorT<T> & values, matrixT & vectors )
	{
		const int N = values.dimension();
		//�I���\�[�g
		for ( int i = 0; i < N; ++i ) {
			//�ő��T��
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
	//���Ώ̍s��̌ŗL�l�ƌŗL�x�N�g��
	//Householder-QR�@�ɂ��
	//��Ώێ��s��̌ŗL�l�E�ŗL�x�N�g���͕��f���ɂȂ�A��ʋt�s������߂邾���Ȃ�Γ��ْl�����ŏ\���Ȃ��߁A���Ώ̍s��݂̂Ƃ���B
	bool eigen( vectorT<T> & values, matrixT & vectors ) const
	{
		if ( ! symmetricMatrix() ) return false;
		//�O�d�Ίp�s��ɕύX
		matrixT A = *this;
		// A->P�� D3 P
		A.diag3( vectors );
		//�ŗL�l����
		// P�� D3 P ->U�� D U
		A.evd4diag3( vectors );
		values = A.diagVector();
		vectors = transpose( vectors );//evec eval evec���ɂ���
		//sort4evd( values, vectors );
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�p��@
	//�ő�̌ŗL�l�ƑΉ�����ŗL�x�N�g�������߂�
	void powerMethod( T & eigenvalue, vectorT<T> & eigenvector ) const
	{
		eigenvector = vectorT<T>::normalize( vectorT<T>::one( column() ) );
		do {
			eigenvalue = vectorT<T>::dot( eigenvector, *this * eigenvector );
			eigenvector = vectorT<T>::normalize( *this * eigenvector );
		} while ( absolute( eigenvalue - vectorT<T>::dot( eigenvector, *this * eigenvector ) ) < tiny );
	}
	////////////////////////////////////////////////////////////////////////////////
	//���C���[���t�����@
	//�ŗL�x�N�g���̋ߎ��l����ŗL�l�𓾁A�ŗL�x�N�g�����ߎ����Ă���
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
	//���C���[��
	//�ߎ��I�ȌŗL�x�N�g������C�ŗL�l���v�Z����
	const T rayleighQuotient( const vectorT<T> & eigenvector ) const
	{
		return zerodivide( vectorT<T>::dot( *this * eigenvector, eigenvector ), 
			vectorT<T>::dot( eigenvector, eigenvector ) );
	}
private:
	////////////////////////////////////////////////////////////////////////////////
	//���ْl����
	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////
	//�c�^������d�Ίp�s��̉E���̑Ίp�v�f���O�̎��ɂ��̏�̗v�f���O�ɂ��邽�߂�Givens�ϊ�
	//�ǂ����݂̏����̓h�L�������g�Q��
	void chasingRightBottom( matrixT & u )
	{
		if ( column() <= 1 ) return;
		const int c = column();
		for ( int i = c - 2; 0 <= i; --i ) {
			if ( mat[i][c-1] == 0 ) return;
			const matrixT & g = givensRight( i, c - 1 );
			u = u * g;// X * G * G�� * U��=X * G * (U*G)��
			*this = fix( *this * g );
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//�c�^������d�Ίp�s��̉E���łȂ��Ίp�v�f���O�̎��ɂ��̉E�̗v�f���O�ɂ��邽�߂�Givens�ϊ�
	//�ǂ����݂̏����̓h�L�������g�Q��
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
	//�c�^������d�Ίp�s��̑Ίp�v�f�Ƃ��̏�̗v�f����O�̎��ɑΊp�s��ɋ߂Â���
	//�ǂ����݂̏����̓h�L�������g�Q��
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
	//��d�Ίp���s��p�̓��ْl����
	//A=V D U���ɕ��� D�͎��g�ɓ���
	void svd4diag2( matrixT & v, matrixT & u )
	{
		//��v�f������
		const int C = column();

		//�P��̓�d�Ίp�s��̏ꍇ�ɂ́C���ْl�������I�����Ă���
		if ( C == 1 ) return;

		while ( ! diagMatrix() ) {
			//�E���̑Ίp�v�f���`�F�b�N
			if ( absolute( mat[C-1][C-1] ) < epsilon ) {
				chasingRightBottom( u );//��������ǂ�����ł��̏�̗v�f��S�ė�ɂ���
			}
			for ( int i = C - 2; 0 <= i; --i ) {
				// i, i�̑Ίp�v�f���`�F�b�N
				if ( absolute( mat[i][i] ) < epsilon ) {
					chasingDiag( v, i );//�E������ǂ�����ł��̉E�̗v�f��S�ė�ɂ���
				}
				// i, i+1���`�F�b�N
				if ( absolute( mat[i][i+1] ) < epsilon ) {
					//���̏�Ԃ�i,i�łS�ɕ��������ɂ͉E�����s��ɂȂ邽�߁A�Q�ɕ����ď������s��

					// i,i�܂ł�i+1,i+1����œ�ɂ킯��
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
			//�ǂ�����
			chasing( v, u );
		}
	}
public:
	////////////////////////////////////////////////////////////////////////////////
	//���ْl����p�ɓ��ْl�̐�Βl�̑傫�����Ƀ\�[�g
	static void sort4svd( matrixT & v, matrixT & d, matrixT & u )
	{
		const int N = minimum( d.row(), d.column() );
		//�I���\�[�g
		for ( int i = 0; i < N; ++i ) {
			//�ő��T��
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
	//���ْl����
	//A=V D U���ɕ���
	//��d�Ίp�����s���CGivens�ϊ��ɂ���Ίp���v�f��ǂ����݂ŗ�ɋ߂Â��āC�������s����@
	//�����F���� �������C���\�[�g
	void singularValueDecomposition( matrixT & v, matrixT & d, matrixT & u ) const
	{
		const int R = row();
		const int C = column();
		if ( C < R ) {//�c�����s��֕ϊ�
			matrixT A = transpose( *this );
			A.singularValueDecomposition( u, d, v );
			u = transpose( u );
			v = transpose( v );
			return;
		}
		//��d�Ίp���s��֕ϊ�
		d = *this;
		d.diag2( u, v );// A -> u A v = A'�Ƃ��ē�d�Ίp��
		// A = v�� A' u�������Ƃ��Ă͋t�ɂȂ�
		v = transpose( v );// v A u���`���ɕϊ�
		d.svd4diag2( v, u );//����
		//sort4svd( v, d, u );//�\�[�g
	}
	
	////////////////////////////////////////////////////////////////////////////////
	//Moore-Penrose�̈�ʋt�s��
	const matrixT generalInverse() const
	{
		matrixT v, d, u;// A=v d u���ƕ���
		singularValueDecomposition( v, d, u );
		// A^+ = u 1/d v��
		return u * d.inverseDiag() * transpose( v );
	}

	////////////////////////////////////////////////////////////////////////////////
	//�f�o�b�O�p�����o��
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

//�l�����Z
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
