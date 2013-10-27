////////////////////////////////////////////////////////////////////////////////
// List

//‘o•ûŒüƒŠƒXƒg
template<class T>
class list : public container<T>
{
public:
	class node
	{
	public:
		T data;
		node * prev, * next;
		node( const T & src ) : data( src ), prev( NULL ), next( NULL ){}
	};
	class iterator
	{
		node * n;
	public:
		iterator() : n( NULL ){}
		iterator( node * src ) : n( src ){}
		iterator( const node & src ) : n( & src ){}
		iterator( const list & base, bool head = true ) : n( head ? base.head : base.tail ){}
		iterator( const iterator & it ) : n( it.n ){}
		iterator & operator=( const iterator & it ){ n = it.n; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//ğŒ”»’è—pŒ^•ÏŠ·
		operator const node * () const { return n; }
		operator node * () { return n; }
		////////////////////////////////////////////////////////////////////////////////
		//Œ^•ÏŠ·
		T & operator()() { assert( n ); return n->data; }
		const T & operator()() const { assert( n ); return n->data; }
		////////////////////////////////////////////////////////////////////////////////
		//Ÿ‚Ö
		iterator & operator++()
		{
			if ( n ) n = n->next;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//‘O‚Ö
		iterator & operator--()
		{
			if ( n ) n = n->prev;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//‚»‚Ì‘¼‚ÌƒCƒ“ƒ^ƒtƒF[ƒX
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
		const iterator next() const { iterator wk(*this); ++wk; return wk; }
		const iterator prev() const { iterator wk(*this); --wk; return wk; }
	};
	node * head, * tail;
	list() : head( NULL ), tail( NULL ){}
	list( const list & index ) : head( NULL ), tail( NULL )
	{ *this = index; }
	virtual ~list() { finalize(); }
	////////////////////////////////////////////////////////////////////////////////
	//‰Šú‰»
	virtual void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//I—¹‰»
	virtual void finalize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//Šm•Û
	virtual void allocate( int ){}
	////////////////////////////////////////////////////////////////////////////////
	//‰ğ•ú
	virtual void release() { while ( head ) pop( head ); }
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğŒã‚ë‚É’Ç‰Á
	virtual T & push_back( const T & src ){ return push_back( new node( src ) ); }
	T & push_back( node * src )
	{
		src->next = NULL;
		if ( tail ) {
			tail->next = src;
			src->prev = tail;
		}
		tail = src;
		if ( ! head ) head = src;
		size++;
		return src->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ‚Ü‚¦‚É’Ç‰Á
	virtual T & push_front( const T & src ) { return push_front( new node( src ) ); }
	T & push_front( node * src )
	{
		src->prev = NULL;
		if ( head ) {
			head->prev = src;
			src->next = head;
		}
		head = src;
		if ( ! tail ) tail = src;
		size++;
		return src->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ’Ç‰Á
	virtual T & push( const T & src, int index ) { return push( new node( src ), index ); }
	T & push( node * src, int index )
	{
		if ( ! between( 0, index, size ) || index == 0 ) {
			return push_front( src );
		}
		if ( index == size ) {
			return push_back( src );
		}
		node * current = head;
		for ( ; index; --index ) {
			if ( current ) current = current->next;
		}
		assert( current );
		src->prev = current->prev;
		src->next = current;
		src->prev->next = src;
		src->next->prev = src;
		size++;
		return src->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğŒã‚ë‚©‚çíœ
	virtual void pop_back(){ if ( tail ) pop( tail ); }
	////////////////////////////////////////////////////////////////////////////////
	//—v‘f‚P‚Â‚ğ‘O‚©‚çíœ
	virtual void pop_front() { if ( head ) pop( head ); }
	////////////////////////////////////////////////////////////////////////////////
	// node‚Ìæ‚èŠO‚µ‚Æ”j‰ó
	virtual void pop( const T & src )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == src ) {
				pop( it );
				return;
			}
		}
	}
	void pop( node * src )
	{
		if ( src->prev ) {
			src->prev->next = src->next;
		} else {
			head = src->next;
		}
		if ( src->next ) {
			src->next->prev = src->prev;
		} else {
			tail = src->prev;
		}
		src->prev = src->next = NULL;
		size--;
		delete src;
	}
	T loot( node * src )
	{
		T wk( src->data );
		if ( src->prev ) {
			src->prev->next = src->next;
		} else {
			head = src->next;
		}
		if ( src->next ) {
			src->next->prev = src->prev;
		} else {
			tail = src->prev;
		}
		src->prev = src->next = NULL;
		size--;
		delete src;
		return wk;
	}
	virtual T loot( const T & src )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == src ) return loot( it );
		}
		T wk;
		return wk;
	}
	////////////////////////////////////////////////////////////////////////////////
	//ŒŸõ
	iterator search( const T & src )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == src ) return it;
		}
		return iterator();
	}
	bool exist( node * src )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it == src ) return true;
		}
		return false;
	}
	////////////////////////////////////////////////////////////////////////////////
	//êŠ
	int index( const iterator & src ) const { return index( src() ); }
	virtual int index( const T & src ) const
	{
		int i = 0;
		for ( iterator it( *this ); it; ++it, ++i ) {
			if ( it() == src ) break;
		}
		return i;
	}
	////////////////////////////////////////////////////////////////////////////////
	//“Áê—v‘f
	virtual T & value( int src )
	{
		assert( between( 0, src, size - 1 ) );
		if ( src < size / 2 ) {
			for ( iterator it( *this ); it; ++it ) {
				if ( src-- == 0 ) return it();
			}
			static T wk; return wk;
		}
		for ( iterator it( *this, false ); it; --it ) {
			if ( ++src == size ) return it();
		}
		static T wk; return wk;
	}
	virtual const T & value( int src ) const
	{
		assert( between( 0, src, size - 1 ) );
		if ( src < size / 2 ) {
			for ( iterator it( *this ); it; ++it ) {
				if ( src-- == 0 ) return it();
			}
			static T wk; return wk;
		}
		for ( iterator it( *this, false ); it; --it ) {
			if ( ++src == size ) return it();
		}
		static T wk; return wk;
	}
	T & operator[]( int src ) { return value( src ); }
	const T & operator[]( int src ) const { return value( src ); }

	list & operator=( const list & index )
	{
		if ( this == &index ) return *this;
		release();
		for ( iterator itsrc( index ); itsrc; ++itsrc ) push_back( itsrc() );
		return *this;
	}
	OperatorSet( list, T );
};
