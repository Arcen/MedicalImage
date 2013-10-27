class stepParameter
{
public:
	int start, end, step;
	stepParameter() : start( 0 ), end( 0 ), step( 1 )
	{
	}
	int count()
	{
		if ( step < 1 ) return 1;
		int c = 0;
		for ( int i = start; i <= end; i += step ) ++c;
		return c;
	}
	void set( int _start, int _end, int _step = 1 )
	{
		start = _start;
		end = _end;
		step = _step;
	}
	void set( int _start )
	{
		set( _start, _start );
	}
	class iterator
	{
		const stepParameter * sp;
		int i;
	public:
		iterator() : sp( NULL ), i( 0 ){}
		iterator( const stepParameter & src ) : sp( & src ), i( sp->start ){}
		iterator( const iterator & it ) : sp( it.sp ), i( sp->start ){}
		iterator & operator=( const iterator & it ){ sp = it.sp; i = it.i; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//��������p�^�ϊ�
		operator bool () const {
			if ( ! sp ) return false;
			if ( sp->step < 1 ) return sp->start == i;
			return ( i <= sp->end );
		}
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		int operator()() const { return i; }
		////////////////////////////////////////////////////////////////////////////////
		//����
		iterator & operator++()
		{
			if ( sp->step < 1 ) {
				i = sp->end + 1;
			} else {
				i += sp->step;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//�O��
		iterator & operator--()
		{
			if ( sp->step < 1 ) {
				i = sp->end + 1;
			} else {
				i -= sp->step;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
	};
};

