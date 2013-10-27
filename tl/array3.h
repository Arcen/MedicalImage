////////////////////////////////////////////////////////////////////////////////
// Array 3

template<class T>
class array3 : public array<T>
{
public:
	class iterator
	{
		array<T>::iterator it;
	public:
		iterator( const array3 & src, bool head = true ) 
			: it( src, head ) {}
		iterator( const array3 & src, int offset ) 
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
	array<T**> zs;//�������̃|�C���^�e�[�u��
	int w, h, d;
	////////////////////////////////////////////////////////////////////////////////
	//�R���X�g���N�^
	array3() : w( 0 ), h( 0 ), d( 0 ) {}
	array3( int _w, int _h, int _d ) : w( 0 ), h( 0 ), d( 0 )
	{ allocate( _w, _h, _d ); }
	array3( const array3 & src ) : w( 0 ), h( 0 ), d( 0 )
	{ *this = src; }
	////////////////////////////////////////////////////////////////////////////////
	//�f�X�g���N�^
	virtual ~array3() {}
	////////////////////////////////////////////////////////////////////////////////
	//������
	virtual void initialize() { array<T>::initialize(); allocate( 0, 0, 0 ); }
	////////////////////////////////////////////////////////////////////////////////
	//�m��
	virtual void allocate( int _w, int _h, int _d )
	{
		if ( _w == w && _h == h && _d == d ) return;
		if ( _w == 0 || _h == 0 || _d == 0 ) {
			_w = _h = _d = 0;
		}
		array<T>::allocate( _w * _h * _d );
		ys.allocate( _h * _d );
		zs.allocate( _d );
		w = _w; h = _h; d = _d;
		int a = w * h;
		for ( int z = 0; z < d; ++z ) {
			for ( int y = 0; y < h; ++y ) {
				ys[y+z*h] = & (*this)[w*y+a*z];
			}
			zs[z] = & ys[h*z];
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//���
	virtual void release()
	{
		array<T>::release();
		zs.release();
		ys.release();
		w = h = d = 0;
	}
	T & operator()( int x, int y, int z )
	{
		assert( between( 0, x, w - 1 ) && between( 0, y, h - 1 ) && between( 0, z, d - 1 ) );
		return zs[z][y][x];
	}
	const T & operator()( int x, int y, int z ) const
	{
		assert( between( 0, x, w - 1 ) && between( 0, y, h - 1 ) && between( 0, z, d - 1 ) );
		return zs[z][y][x];
	}
	array3 & operator=( const array3 & src )
	{
		if ( this == & src ) return *this;
		if ( w != src.w || h != src.h || d != src.d ) allocate( src.w, src.h, src.d );
		for ( iterator itdst( *this ), itsrc( src ); itdst; ++itdst, ++itsrc ) 
			*itdst = *itsrc;
		return *this;
	}

	OperatorSet( array3, T );
};
