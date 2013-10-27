////////////////////////////////////////////////////////////////////////////////
// Array

template<class T>
class array : public container<T>
{
public:
	class iterator
	{
		T * pointer, * prev, * post;
	public:
		////////////////////////////////////////////////////////////////////////////////
		//�����E����
		iterator()
			: pointer( NULL ), prev( NULL ), post( NULL ) {}
		iterator( const array & src, bool head = true ) 
			: pointer( src.size ? ( head ? src.data : src.data + src.size - 1 ) : NULL ), 
			prev( src.size ? src.data - 1 : NULL ), 
			post( src.size ? src.data + src.size : NULL ){}
		iterator( const array & src, int offset ) 
			: pointer( between( 0, offset, src.size - 1 ) ? src.data + offset : NULL ), 
			prev( src.size ? src.data - 1 : NULL ), 
			post( src.size ? src.data + src.size : NULL ){}
		iterator( const iterator & it ) 
			: pointer( it.pointer ), prev( it.prev ), post( it.post ){}
		iterator & operator=( const iterator & src )
		{
            if ( this != & src )
            {
			    pointer = src.pointer;
			    prev = src.prev;
			    post = src.post;
            }
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//��������p�^�ϊ�
		operator T * () { return pointer; }
		operator const T * () const { return pointer; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		T & operator()()				{ assert( pointer ); return *pointer; }
		const T & operator()() const	{ assert( pointer ); return *pointer; }
		////////////////////////////////////////////////////////////////////////////////
		//�O��
		iterator & operator--() { if ( pointer && prev == --pointer ) pointer = NULL; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//����
		iterator & operator++() { if ( pointer && post == ++pointer ) pointer = NULL; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
	};
	T * data;
	int instance;
	////////////////////////////////////////////////////////////////////////////////
	//�R���X�g���N�^
	array() : data( NULL ), instance( 0 ){}
	array( int _size ) : data( NULL ), instance( 0 )
	{ allocate( _size ); }
	array( const array & index ) : data( NULL ), instance( 0 )
	{ *this = index; }
	////////////////////////////////////////////////////////////////////////////////
	//�f�X�g���N�^
	virtual ~array() { finalize(); }
	////////////////////////////////////////////////////////////////////////////////
	//������
	virtual void initialize() { data = NULL; size = 0; instance = 0; allocate( 0 ); }
	////////////////////////////////////////////////////////////////////////////////
	//�I����
	virtual void finalize() { delete [] data; }
	////////////////////////////////////////////////////////////////////////////////
	//�m��
	virtual void allocate( int _size )
	{
		if ( _size <= instance ) {
			size = _size;
			return;
		}
		release();
		size = instance = _size;
		if ( size ) data = new T[size];
	}
	////////////////////////////////////////////////////////////////////////////////
	//���
	virtual void release()
	{
		delete [] data;
		data = NULL;
		size = instance = 0;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�����m�ې�
	virtual int capacity() const { return instance; }
	////////////////////////////////////////////////////////////////////////////////
	//�v�f�P��O�ɒǉ�
	virtual T & push_front( const T & src )
	{
		array<T> wk( *this );
		allocate( wk.size + 1 );
		copy( data + 1, wk.data, wk.size );
		return ( data[0] = src );
	}
	////////////////////////////////////////////////////////////////////////////////
	//�v�f�P�����ɒǉ�
	virtual T & push_back( const T & src )
	{
		const int index = size;
		resize( size + 1 );
		return ( data[index] = src );
	}
	////////////////////////////////////////////////////////////////////////////////
	//�v�f�P��O����폜
	virtual void pop_front()
	{
		assert( size );
		array wk( *this );
		allocate( wk.size - 1 );
		copy( data, wk.data + 1, size );
	}
	////////////////////////////////////////////////////////////////////////////////
	//�v�f�P����납��폜
	virtual void pop_back()
	{
		assert( size );
		resize( size - 1 );
	}
	////////////////////////////////////////////////////////////////////////////////
	//����
	void copy( T * dst, T * src, int _size )
	{
		for ( const T * sentinel = dst + _size; dst != sentinel; ++dst, ++src ) {
			*dst = *src;
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//�폜
	virtual void pop( const T & src )
	{
		iterator it = search( src );
		if ( ! it ) return;
		int i = index( it );
		copy( data + i, data + i + 1, size - i - 1 );
		resize( size - 1 );
	}
	virtual T loot( const T & src )
	{
		T wk;
		iterator it = search( src );
		if ( ! it ) return wk;
		wk = it();
		int i = index( it );
		copy( data + i, data + i + 1, size - i - 1 );
		resize( size - 1 );
		return wk;
	}
	void popByIndex( int i )
	{
		assert( between( 0, i, size - 1 ) );
		copy( data + i, data + i + 1, size - i - 1 );
		resize( size - 1 );
	}
	T lootByIndex( int i )
	{
		assert( between( 0, i, size - 1 ) );
		T wk = data[i];
		copy( data + i, data + i + 1, size - i - 1 );
		resize( size - 1 );
		return wk;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�ꏊ
	int index( const iterator & src ) const { return index( src() ); }
	virtual int index( const T & src ) const
	{
		if ( & src < data ) return size;
		if ( data + size <= & src ) return size;
		return ( reinterpret_cast<int>( & src ) - reinterpret_cast<int>( data ) ) / sizeof( T );
	}
	////////////////////////////////////////////////////////////////////////////////
	//����v�f
	virtual T & value( int src )
	{
		assert( between( 0, src, size - 1 ) );
		return data[src];
	}
	virtual const T & value( int src ) const
	{
		assert( between( 0, src, size - 1 ) );
		return data[src];
	}
	T & operator[]( int src ) { return value( src ); }
	const T & operator[]( int src ) const { return value( src ); }
	operator T * () { return data; }
	operator const T * () const { return data; }
	array & operator=( const array & index )
	{
		if ( this == &index ) return *this;
		if ( size != index.size ) allocate( index.size );
		for ( iterator itdst( *this ), itsrc( index ); itdst; ++itdst, ++itsrc ) 
			*itdst = *itsrc;
		return *this;
	}

	OperatorSet( array, T );

	////////////////////////////////////////////////////////////////////////////////
	//����
	iterator search( const T & src ) const
	{
		return iterator( *this, index( src ) );
	}
	////////////////////////////////////////////////////////////////////////////////
	//�����ۑ��E�Ċm��
	void resize( int _size, int increment = 16 )
	{
		if ( _size <= instance ) {
			size = _size;
			return;
		}
		array wk( *this );
		allocate( _size + increment );
		allocate( _size );
		copy( data, wk.data, wk.size );
	}
};
