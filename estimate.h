////////////////////////////////////////////////////////////////////////////////
//�K�E�V�A������

class gaussianEstimate
{
public:
	enum {
		minimumValue = -4096,//�ŏ��l
		maximumValue = 4096,//�ő�l
		sizeValue = maximumValue - minimumValue + 1,//��蓾��͈͂̐�
	};
	//�e�[�u��
	array<decimal> table;//�Q�̃K�E�X�֐��̑���
	decimal sd;
	//�e�[�u��������
	void initialize( double var )
	{
        sd = ::sqrt( var );
		//�ȉ��̒l���v�Z����
		// 1 / sqrt( 4 pi var ) * exp( - subtract ^ 2 / ( 4 var ) )
		table.allocate( sizeValue );
        const decimal coefficient = 1 / ::sqrt( 4 * pi * var );
		for ( int i = 0; i < sizeValue; ++i ) {
			table[i] = coefficient * exp( - decimal( i ) * decimal( i ) / ( 4 * var ) );
		}
	}
private:
	//�Q�̃K�E�X�֐��̑���
	decimal gaussCorrelation( int subtract )
	{
		return table[ subtract ];
	}
public:
	//�p�^�[���ƃK�E�X�֐����p�^�[���̎��O�v�Z�e�[�u�����v�Z
	void buildup( const array<int16> & pettern, array<decimal> & table )
	{
		table.allocate( sizeValue );
		//���֌v�Z���s��
		for ( int16 v = minimumValue; v <= maximumValue; ++v ) {
			decimal & value = table[v-minimumValue];
			value = 0;
			for ( array<int16>::iterator its( pettern ); its; ++its ) {
				value += gaussCorrelation( abs( v - its() ) );
			}
		}
		//���ςɂ���
		table /= pettern.size;
	}
	//�e�[�u����Е��̎��ȑ��ւ̂Q�捪�Ŋ���
	void normalized( const array<int16> & pettern, array<decimal> & table )
	{
		//���ȑ��ւ̂Q�捪�Ŋ���
        table /= ::sqrt( self( pettern, table ) );
	}
	//�T���v�����玩�ȑ��ւ��v�Z����
	decimal self( const array<int16> & pettern )
	{
		//�ȉ����R�����g�A�E�g����Ɠ����m���ƂȂ�D
		//���֌v�Z���s��
		decimal self = 0;
		for ( array<int16>::iterator it1( pettern ); it1; ++it1 ) {
			const int16 & i1 = it1();
			array<int16>::iterator it2 = it1;
			++it2;
			for ( ; it2; ++it2 ) {
				self += gaussCorrelation( abs( i1 - it2() ) );
			}
		}
		self = self * 2 + gaussCorrelation( 0 ) * pettern.size;
		//���ςɂ���
		return ( self / pettern.size ) / pettern.size;
	}
	//�e�[�u�����g�����ȑ��ւ��v�Z����
	decimal self( const array<int16> & pettern, const array<decimal> & table )
	{
		decimal self = 0;
		for ( array<int16>::iterator its( pettern ); its; ++its ) {
			self += table[its()-minimumValue];
		}
		return self / pettern.size;
	}
	//��������W�{�l�Ǝ��O�v�Z�e�[�u�����A�K�E�V�A��������v�Z
	decimal estimate( const int16 & sample, const array<decimal> & table )
	{
		return table[sample-minimumValue];
	}
	//��������W�{�Ǝ��O�v�Z�e�[�u�����A�K�E�V�A��������v�Z
	decimal estimate( const array<int16> & sample, const array<decimal> & table )
	{
		decimal c = 0;
		for ( array<int16>::iterator it( sample ); it; ++it ) 
#ifdef NDEBUG
			c += table.data[it()-minimumValue];//������
#else
			c += estimate( it(), table );
#endif
		return c / sample.size;
	}
};

extern gaussianEstimate ge;

