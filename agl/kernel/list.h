////////////////////////////////////////////////////////////////////////////////
// List

//‘o•ûŒüƒŠƒXƒg
template<class T>
class list
{
public:
	class node
	{
	public:
		T data;
		node * prev, * next;
		node( const T & _data ) : data( _data ), prev( 0 ), next( 0 ){}
	};
	class iterator
	{
		node * n;
	public:
		iterator() : n( 0 ){}
		iterator( node * _n ) : n( _n ){}
		iterator( const list & base ) : n( base.head ){}
		iterator( const iterator & it ) : n( it.n ){}
		void operator=( const iterator & it ){ n = it.n; }
		////////////////////////////////////////////////////////////////////////////////
		//ğŒ”»’è—pŒ^•ÏŠ·
		operator const node * () const { return n; }
		operator node * () { return n; }
		////////////////////////////////////////////////////////////////////////////////
		//Œ^•ÏŠ·
		const T * operator->() const { assert( n ); return & n->data; }
		T * operator->() { assert( n ); return & n->data; }
		T & operator*() { assert( n ); return n->data; }
		const T & operator*() const { assert( n ); return n->data; }
		T & operator()() { assert( n ); return n->data; }
		const T & operator()() const { assert( n ); return n->data; }
		////////////////////////////////////////////////////////////////////////////////
		//Ÿ‚Ö
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator & operator++()
		{
			if ( n ) n = n->next;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//‘O‚Ö
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
		iterator & operator--()
		{
			if ( n ) n = n->prev;
			return *this;
		}
	};
	node * head, * tail;
	int size;
	list() : head( 0 ), tail( 0 ), size( 0 ){}
	virtual ~list() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//‰Šú‰»
	void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//‰ğ•ú
	void release() { while ( head ) kill( head ); }
	////////////////////////////////////////////////////////////////////////////////
	//“à•”Šm•Û”
	int capacity() const { return size; }
	////////////////////////////////////////////////////////////////////////////////
	//‹ó
	bool empty() const { return size ? false : true; }
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğŒã‚ë‚É’Ç‰Á
	T & push_back( const T & data ){ return push_back( new node( data ) ); }
	T & push_back( node * n )
	{
		if ( tail ) {
			tail->next = n;
			n->prev = tail;
		}
		tail = n;
		if ( ! head ) head = n;
		size++;
		return n->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ‚Ü‚¦‚É’Ç‰Á
	T & push_front( const T & data ) { return push_front( new node( data ) ); }
	T & push_front( node * n )
	{
		if ( head ) {
			head->prev = n;
			n->next = head;
		}
		head = n;
		if ( ! tail ) tail = n;
		size++;
		return n->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğŒã‚ë‚©‚çíœ
	void pop_back(){ if ( tail ) kill( tail ); }
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ‘O‚©‚çíœ
	void pop_front() { if ( head ) kill( head ); }
	////////////////////////////////////////////////////////////////////////////////
	// node‚Ìæ‚èŠO‚µ‚Æ”j‰ó
	void kill( node * n )
	{
		if ( n->prev ) {
			n->prev->next = n->next;
		} else {
			head = n->next;
		}
		if ( n->next ) {
			n->next->prev = n->prev;
		} else {
			tail = n->prev;
		}
		n->prev = n->next = NULL;
		destroy( n );
		size--;
	}
	////////////////////////////////////////////////////////////////////////////////
	// node‚Ì”j‰ó
	virtual void destroy( node * n )
	{
		delete n;
	}
	////////////////////////////////////////////////////////////////////////////////
	//ŒŸõ
	iterator search( const T & data )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == data ) break;
		}
		return it;
	}
	bool exist( node * data )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it == data ) return true;
		}
		return false;
	}
	////////////////////////////////////////////////////////////////////////////////
	//íœ
	void kill( T data )
	{
		kill( search( data ) );
	}
	void kill( iterator & it )
	{
		if ( it ) kill( ( node * ) it );
	}
	virtual T loot_front()
	{
		assert( size );
		T data( head->data );
		pop_front();
		return data;
	}
	virtual T loot_back()
	{
		assert( size );
		T data( tail->data );
		pop_back();
		return data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//êŠ
	int index( const iterator & src ) const { index( ( const T * ) src ); }
	int index( const T * src ) const
	{
		iterator it = search( *src );
		int i = 0;
		for ( iterator it( *this ); it; ++it, ++i ) {
			if ( it() == *src ) break;
		}
		if ( it ) return i;
		return -1;
	}
	iterator operator[]( int index )
	{
		if ( index < 0 ) return iterator();
		for ( iterator it( *this ); it; ++it ) {
			if ( index == 0 ) break;
			index--;
		}
		return it;
	}
	const iterator operator[]( int index ) const
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( index == 0 ) break;
			index--;
		}
		return it;
	}
	////////////////////////////////////////////////////////////////////////////////
	//“Áê—v‘f
	T & index( int i )
	{
		assert( between( 0, i, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) break;
			i--;
		}
		return ( T & ) it;
	}
	const T & index( int i ) const
	{
		assert( between( 0, i, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) break;
			i--;
		}
		return ( T & ) it;
	}
	T & first()
	{
		assert( between( 0, 0, size - 1 ) );
		return head->data;
	}
	const T & first() const
	{
		assert( between( 0, 0, size - 1 ) );
		return head->data;
	}
	T & second()
	{
		assert( between( 0, 1, size - 1 ) );
		return head->next->data;
	}
	const T & second() const
	{
		assert( between( 0, 1, size - 1 ) );
		return head->next->data;
	}
	T & third()
	{
		assert( between( 0, 2, size - 1 ) );
		return head->next->next->data;
	}
	const T & third() const
	{
		assert( between( 0, 2, size - 1 ) );
		return head->next->next->data;
	}
	T & last()
	{
		assert( between( 0, size - 1, size - 1 ) );
		return tail->data;
	}
	const T & last() const
	{
		assert( between( 0, size - 1, size - 1 ) );
		return tail->data;
	}
	T & prelast()
	{
		assert( between( 0, size - 2, size - 1 ) );
		return tail->prev->data;
	}
	const T & prelast() const
	{
		assert( between( 0, size - 2, size - 1 ) );
		return tail->prev->data;
	}
	list & operator=( const list & index )
	{
		if ( this == &index ) return *this;
		release();
		for ( iterator itsrc( index ); itsrc; ++itsrc ) push_back( itsrc() );
		return *this;
	}
	OperatorSet( list, T );
};

template<class T>
class listRetain : public list<T*>
{
public:
	typedef list<T*> parent;
	typedef list<T*>::iterator parentIterator;
	class iterator
	{
		node * n;
	public:
		iterator( node * _n ) : n( _n ){}
		iterator( const listRetain & base ) : n( base.head ){}
		iterator( const iterator & it ) : n( it.n ){}
		void operator=( const iterator & it ){ n = it.n; }
		////////////////////////////////////////////////////////////////////////////////
		//ğŒ”»’è—pŒ^•ÏŠ·
		operator const node * () const { return n; }
		operator node * () { return n; }
		////////////////////////////////////////////////////////////////////////////////
		//Œ^•ÏŠ·
		const T * operator->() const { assert( n ); return n->data; }
		T * operator->() { assert( n ); return n->data; }
		T & operator*() { assert( n ); return *(n->data); }
		const T & operator*() const { assert( n ); return *(n->data); }
		T * & operator()() { assert( n ); return n->data; }
		const T * & operator()() const { assert( n ); return n->data; }
		////////////////////////////////////////////////////////////////////////////////
		//Ÿ‚Ö
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator & operator++()
		{
			if ( n ) n = n->next;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//‘O‚Ö
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
		iterator & operator--()
		{
			if ( n ) n = n->prev;
			return *this;
		}
	};
	listRetain()
	{
	}
	virtual ~listRetain()
	{
		release();
	}
	virtual void destroy( node * n )
	{
		delete n->data;
		delete n;
	}
	T * operator[]( int index )
	{
		parent * p = ( parent * ) this;
		parentIterator pit = ( *p )[ index ];
		if ( ! pit ) return 0;
		return pit();
	}
	const T * const operator[]( int index ) const
	{
		const parent * p = ( const parent * ) this;
		parentIterator pit = ( *p )[ index ];
		if ( ! pit ) return 0;
		return pit();
	}
	virtual T* loot_front()
	{
		if ( size < 1 ) return 0;
		T * data = head->data;
		head->data = 0;
		pop_front();
		return data;
	}
	virtual T* loot_back()
	{
		if ( size < 1 ) return 0;
		T * data = tail->data;
		tail->data = 0;
		pop_back();
		return data;
	}
	T* loot( T * data )
	{
		if ( size < 1 ) return 0;
		node * n = search( data );
		if ( ! n ) return 0;
		n->data = 0;
		kill( n );
		return data;
	}
	listRetain & operator=( const listRetain & index )
	{
		if ( this == &index ) return *this;
		release();
		while ( index.size ) push_back( index.loot_front() );
		return *this;
	}
	OperatorSet( listRetain, T* );
};
