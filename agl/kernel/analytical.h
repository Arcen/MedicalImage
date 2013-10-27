//�K�E�X�̏����@�Fm*x=b����x������
template<typename T>
bool gaussianElimination( const matrixT<T> & m, const vectorT<T> & b, vectorT<T> & x )
{
	matrixT<T> lu;//�v�Z�p
	vectorT<int> exchange;//�s�{�b�g�I���̂��߂̃e�[�u���@�s�����ւ���
	if ( ! m.luFactorization( lu, exchange ) ) return false;
	if ( ! lu.luSolve( exchange, b, x ) ) return false;
	return true;
}

//�}���J�[�g�@(�it �i+�ɂc)��=-�it �d
template<typename T>//N:�p�����[�^��,E:�덷�̐�
class levenbergMarquardt
{
public:
	T tiny;//�������s���Ƃ��̔�����
	T final_error;//�덷�̏I������
	matrixT<T> scaleParameter;//�X�P�[�����O�Ίp�s��
	typedef T ( * functionError )( vectorT<T> & error, const vectorT<T> & parameter, void * ext );
	void initialize( const int N, T final_error = T( 10e-8 ), T tiny = 1 )
	{
		this->tiny = tiny;
		this->final_error = final_error;
		scaleParameter = matrixT<T>::identity( N );
	}
	//���R�r�A��
	void jacobian( matrixT<T> & jcbn, const vectorT<T> & parameter, const vectorT<T> & error, functionError functor, void * ext )
	{
		const int N = parameter.dimension();
		const int E = error.dimension();
		jcbn = matrixT<T>::zero( E, N );
		//�s���F�덷�̗v�f���C�񐔁F�p�����[�^��
		//���R�r�A���F�덷�̃x�N�g���֐��̊e�v�f���e�p�����[�^�Ŕ��������l��v�f�Ƃ��Ď��s��
		vectorT<T> pi( N );
		vectorT<T> ei( E );
		for ( int i = 0; i < N; ++i ) {
			pi = parameter;
			pi[i] += tiny;
			functor( ei, pi, ext );
			for ( int j = 0; j < E; ++j ) {
				jcbn[j][i] = ( ei[j] - error[j] ) / tiny;
			}
		}
	}
	//marquardt  �߂�l��true�Ȃ�΁C�I��
	bool operator()( vectorT<T> & parameter, functionError functor, void * ext, T lamda = T( 0.1 ) )
	{
		const int N = parameter.dimension();

		T scale = T( 10e-10 );//�����_�̍ő���ʁD
		//lamda //�}���J�[�g�@�̃ɂ̒l�D
		matrixT<T> jcbn, jcbnT, left, leftInverse, hessian;
		vectorT<T> right, delta, error;
		for ( int t = 0; t < 1; ++t ) {
			//���݂̃G���[���v�Z���C�I���������m�F����
			T sum_error = functor( error, parameter, ext );
			const int E = error.dimension();
			if ( sum_error < final_error ) {
				return true;
			}
			//���R�r�A���v�Z
			jacobian( jcbn, parameter, error, functor, ext );

			jcbnT = matrixT<T>::transpose( jcbn );//�]�u�v�Z

			//�w�b�V�A���v�Z
			// �όv�Z�����C�o�O�̂���%�ɂāC�����s��ȊO�̍s��̐όv�Z�������Ȃ��Ă���D
			hessian = jcbnT * jcbn;//�덷���܂� 
			//�s�񎮂�(�����j�捪�v�Z���ăɂɊ|����
			T det = matrixT<T>::determinant( hessian );//�����s��Ȃ̂ŁC�s�񎮂��v�Z�\
			T sgn = sign( det );//���ɂȂ�ꍇ�ɕ�����ۑ����Ă���
			if ( det == 0 || ! _isnan( det ) ) det = 1;//�s�񎮂��s���̏ꍇ�ɂ́C�e������菜��
			det = absolute( det );
			T powereddet = sgn * pow( det, 1.0 / N );//�X�P�[���p�����[�^�ւ̌W�����v�Z
			if ( ! _isnan( powereddet ) ) powereddet = 1;//�s�񎮂��s���̏ꍇ�ɂ́C�e������菜��

			//�}���J�[�g�@(�it �i+�ɂc)��=-�it �d
			//�i�F���R�r�A��,�it�F�]�u���R�r�A��
			//�ɁFlamda,�c�F�X�P�[���p�����[�^
			//�F�~������
			//�d�F�G���[
			int count = 0;//�t�s��v�Z�̌J��Ԃ���
			bool ok = false;//�t�s�񂪋��܂邩�H
			do {//�t�s��������_��h�炵�Ȃ�����Ƃ܂镨��I������
				//�t�s��v�Z�����s�������Ȃǂ̓ɂɎ����ꂽ�肵�ĉ��Ƃ�����B
				//�����_�̒l��h�炷
				//lamda = maximum<T>( 0, lamda + scale * ( rand() % 201 - 100 ) / 100.0 );
				lamda += scale * ( ( rand() % 201 - 100 ) / 100.0 );
				//���ӂ̍s����v�Z
				left = hessian + scaleParameter * ( lamda * powereddet );
				//�t�s����v�Z
				if ( left.inverse( leftInverse ) ) ok = true;//�t�s�񂪑��݂��邩�`�F�b�N
				count++;
				if ( 100 < count ) return true;//���s
			} while ( ! ok );//�t�s��v�Z�̎��s
			//�E�ӂ̍s����v�Z
			right = ( jcbnT * error ) * -1;
			//���v�Z
			delta = leftInverse * right;
			//���l���L�����`�F�b�N
			if ( ! delta.valid() ) return true;

			//�����̑傫���Ɏ�����Č���

			//�������t�C�Ⴕ���͑傫�����ē����Ȃ��Ƃ��ɂ͏���������D
			T old_error = sum_error, trial_error;//�p�����[�^�𓮂����Ƃ��ɁC�덷���傫���Ȃ�����ɓ����Ȃ��悤�Ɍ덷��ۑ����Ă���
			vectorT<T> trialparameter( N );//�e�X�g�p�̃p�����[�^
			count = 0;
			do {
				if ( count ) delta *= T( 0.1 );//�Q��ڈȍ~�͈ړ��ʂ����Ȃ�����
				trialparameter = parameter + delta;
				trial_error = functor( error, trialparameter, ext );
				//�G���[��������ꍇ���]�������Č���
				if ( old_error <= trial_error ) {
					trialparameter = parameter - delta;
					trial_error = functor( error, trialparameter, ext );
					if ( trial_error < old_error ) delta *= -1;//��������C���]
				}
				count++;
			} while ( old_error < trial_error && count < 10 );
			//�K���ȑ傫���ɂȂ��������ցC�ŋ}�~���@�̗l�ɍ~������D
			count = 0;
			while ( trial_error < old_error && count < 10000 ) {
				parameter = trialparameter;//�p�����[�^�̍X�V
				old_error = trial_error;
				//���ʂ̒��x���������悤�Ȃ�C���x��������
				if ( count % 10 == 0 ) delta *= 2.0;
				trialparameter += delta;
				trial_error = functor( error, trialparameter, ext );
				count++;
			};
		}
		return false;
	}
};

