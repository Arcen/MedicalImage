////////////////////////////////////////////////////////////////////////////////
// Array

template<class T>
class array
{
public:
	class iterator
	{
		T * pointer, * sentinel;
	public:
		iterator( const array & a ) : pointer( a.size ? a.data : NULL ), sentinel( a.data + a.size ){}
		iterator( const iterator & it ) : pointer( it.pointer ), sentinel( it.sentinel ){}
		void operator=( const iterator & value )
		{
			pointer = value.pointer;
			sentinel = value.sentinel;
		}
		////////////////////////////////////////////////////////////////////////////////
		//ğŒ”»’è—pŒ^•ÏŠ·
		operator T * () { return pointer; }
		operator const T * () const { return pointer; }
		////////////////////////////////////////////////////////////////////////////////
		//Œ^•ÏŠ·
		const T * operator->() const { assert( pointer ); return pointer; }
		T * operator->() { assert( pointer ); return pointer; }
		const T & operator*() const { assert( pointer ); return *pointer; }
		T & operator*() { assert( pointer ); return *pointer; }
		const T & operator()() const { assert( pointer ); return *pointer; }
		T & operator()() { assert( pointer ); return *pointer; }
		////////////////////////////////////////////////////////////////////////////////
		//Ÿ‚Ö
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator & operator++()
		{
			if ( pointer ) {
				++pointer;
				if ( sentinel == pointer ) pointer = 0;
			}
			return *this;
		}
	private:
		////////////////////////////////////////////////////////////////////////////////
		//‘O‚Ö “®ì‚³‚¹‚È‚¢
		iterator operator--(int);
		iterator & operator--();
	};
	T * data;
	int size, instanceSize;
	array() : data( 0 ), size( 0 ), instanceSize( 0 )
	{
	}
	array( int _size ) : data( 0 ), size( 0 ), instanceSize( 0 )
	{
		allocate( _size );
	}
	array( const array & index ) : data( 0 ), size( 0 ), instanceSize( 0 )
	{
		*this = index;
	}
	virtual ~array()
	{
		delete [] data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//‰Šú‰»
	void initialize() { allocate( 0 ); }
	////////////////////////////////////////////////////////////////////////////////
	//Šm•Û
	void allocate( int _size )
	{
		if ( _size <= instanceSize ) {
			size = _size;
			return;
		}
		release();
		instanceSize = size = _size;
		if ( size ) data = new T[size];
	}
	////////////////////////////////////////////////////////////////////////////////
	//‰ğ•ú
	void release()
	{
		delete [] data;
		data = 0;
		size = 0;
		instanceSize = 0;
	}
	////////////////////////////////////////////////////////////////////////////////
	//€”õ
	void reserve( int _size ) { ready( _size ); }
	void ready( int _size )
	{
		allocate( _size );
		allocate( 0 );
	}
	////////////////////////////////////////////////////////////////////////////////
	//“à•”Šm•Û”
	int capacity() const { return instanceSize; }
	////////////////////////////////////////////////////////////////////////////////
	//“à•”•Û‘¶ÄŠm•Û
	void resize( int _size, int additionalSize = 1024 )
	{
		if ( _size <= instanceSize ) {
			size = _size;
			return;
		}

		array wk( *this );
		allocate( _size + additionalSize );
		allocate( _size );
		copy( data, wk.data, wk.size );
	}
	////////////////////////////////////////////////////////////////////////////////
	//‹ó
	bool empty() const { return size ? false : true; }
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ‘O‚É’Ç‰Á
	T & push_front( const T & value )
	{
		array<T> wk( *this );
		setSize( wk.size + 1 );
		copy( data + 1, wk.data, wk.size );
		return ( data[0] = value );
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğŒã‚ë‚É’Ç‰Á
	T & push_back( const T & value )
	{
		const int index = size;
		resize( size + 1 );
		return ( data[index] = value );
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ‘O‚É’Ç‰Á
	T & push_front()
	{
		array<T> wk( *this );
		setSize( wk.size + 1 );
		copy( data + 1, wk.data, wk.size );
		return ( data[0] );
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğŒã‚ë‚É’Ç‰Á
	T & push_back()
	{
		const int index = size;
		resize( size + 1 );
		return ( data[index] );
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ‘O‚©‚çíœ
	void pop_front()
	{
		assert( size );
		array wk( *this );
		allocate( wk.size - 1 );
		copy( data, wk.data + 1, size );
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğŒã‚ë‚©‚çíœ
	void pop_back()
	{
		assert( size );
		resize( size - 1 );
	}
	////////////////////////////////////////////////////////////////////////////////
	//ŒŸõ
	iterator search( const T & value ) const
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( *it == value ) break;
		}
		return it;
	}
	////////////////////////////////////////////////////////////////////////////////
	//•¡Ê
	virtual void copy( T * dst, T * src, int _size )
	{
		const T * sentinel = dst + _size;
		while ( dst != sentinel ) {
			copy( dst, src );
			++dst; ++src;
		}
	}
	virtual void copy( T * dst, T * src )
	{
		*dst = *src;
	}
	////////////////////////////////////////////////////////////////////////////////
	//íœ
	void kill( const T & value )
	{
		iterator it = search( value );
		if ( ! it ) return;
		int i = index( it );
		copy( data + i, data + i + 1, size - i - 1 );
		allocate( size - 1 );
	}
	////////////////////////////////////////////////////////////////////////////////
	//êŠ
	int index( const iterator & src ) const { return index( ( const T * ) src ); }
	int index( const T * src ) const
	{
		if ( src < data ) return size;
		if ( data + size <= src ) return size;
		return int( int( src ) - int( data ) ) / sizeof( T );
	}
	////////////////////////////////////////////////////////////////////////////////
	//“Áê—v‘f
	T & index( int i )
	{
		assert( between( 0, i, size - 1 ) );
		return data[i];
	}
	const T & index( int i ) const
	{
		assert( between( 0, i, size - 1 ) );
		return data[i];
	}
	T & first() { return index( 0 ); } const T & first() const { return index( 0 ); }
	T & second() { return index( 1 ); } const T & second() const { return index( 1 ); }
	T & third() { return index( 2 ); } const T & third() const { return index( 2 ); }
	T & last() { return index( size - 1 ); } const T & last() const { return index( size - 1 ); }
	T & prelast() { return index( size - 2 ); } const T & prelast() const { return index( size - 2 ); }

	T & operator[]( int index )
	{
		assert( between( 0, index, size - 1 ) );
		return data[index];
	}
	const T & operator[]( int index ) const
	{
		assert( between( 0, index, size - 1 ) );
		return data[index];
	}
	operator T * ()
	{
		return data;
	}
	operator const T * () const
	{
		return data;
	}
	array & operator=( const array & index )
	{
		if ( this == &index ) return *this;
		if ( size != index.size ) allocate( index.size );
		for ( iterator itdst( *this ), itsrc( index ); itdst; ++itdst, ++itsrc ) 
			*itdst = *itsrc;
		return *this;
	}

	OperatorSetForArray( array, T );
};
