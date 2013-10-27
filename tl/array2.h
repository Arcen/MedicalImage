////////////////////////////////////////////////////////////////////////////////
// Array 2

template<class T>
class array2 : public array<T>
{
public:
	class iterator
	{
		array<T>::iterator it;
	public:
		iterator( const array2 & src, bool head = true ) 
			: it( src, head ) {}
		iterator( const array2 & src, int offset ) 
			: it( src, offset ) {}
		iterator( const iterator & src ) 
			: it( src.it ) {}
		iterator & operator=( const iterator & src )
		{
			it = src.it;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//��������p�^�ϊ�
		operator T * () { return it; }
		operator const T * () const { return it; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		T & operator()()				{ return it(); }
		const T & operator()() const	{ return it(); }
		////////////////////////////////////////////////////////////////////////////////
		//�O��
		iterator & operator--() { --it; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//����
		iterator & operator++() { ++it; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
	};
	array<T*> ys;//�c�����̃|�C���^�e�[�u��
	int w, h;
	////////////////////////////////////////////////////////////////////////////////
	//�R���X�g���N�^
	array2() : w( 0 ), h( 0 ) {}
	array2( int _w, int _h ) : w( 0 ), h( 0 )
	{ allocate( _w, _h ); }
	array2( const array2 & src ) : w( 0 ), h( 0 )
	{ *this = src; }
	////////////////////////////////////////////////////////////////////////////////
	//�f�X�g���N�^
	virtual ~array2() {}
	////////////////////////////////////////////////////////////////////////////////
	//������
	virtual void initialize() { array<T>::initialize(); allocate( 0, 0 ); }
	////////////////////////////////////////////////////////////////////////////////
	//�m��
	virtual void allocate( int _w, int _h )
	{
		if ( _w == w && _h == h ) return;
		if ( _w == 0 || _h == 0 ) {
			_w = _h = 0;
		}
		array<T>::allocate( _w * _h );
		ys.allocate( _h );
		w = _w; h = _h;
		for ( int y = 0; y < h; ++y ) ys[y] = & (*this)[w*y];
	}
	////////////////////////////////////////////////////////////////////////////////
	//���
	virtual void release()
	{
		array<T>::release();
		ys.release();
		w = h = 0;
	}
	T & operator()( int x, int y )
	{
		assert( between( 0, x, w - 1 ) && between( 0, y, h - 1 ) );
		return ys[y][x];
	}
	const T & operator()( int x, int y ) const
	{
		assert( between( 0, x, w - 1 ) && between( 0, y, h - 1 ) );
		return ys[y][x];
	}
	array2 & operator=( const array2 & src )
	{
		if ( this == & src ) return *this;
		if ( w != src.w || h != src.h ) allocate( src.w, src.h );
		for ( iterator itdst( *this ), itsrc( src ); itdst; ++itdst, ++itsrc ) 
			*itdst = *itsrc;
		return *this;
	}

	OperatorSet( array2, T );
};
