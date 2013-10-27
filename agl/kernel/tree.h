////////////////////////////////////////////////////////////////////////////////
// tree

//ñÿç\ë¢
template<class T>
class tree
{
public:
	class node
	{
	public:
		T data;
		node * parent;
		listRetain<node> children;
		node( const T & src ) : data( src ), parent( NULL ){}
		node( const node & src ) : data( src.data ), parent( NULL )
		{
			for ( listRetain<node>::iterator it( src.children ); it; ++it ) {
				node * & c = children.push_back( new node( *( it() ) ) );
				c->parent = this;
			}
		}
		~node()
		{
			if ( parent ) {
				if ( parent->children.exist( this ) ) parent->children.loot( this );
			}
		}
		T & push_front( node * n )
		{
			n->parent = this;
			children.push_front( n );
		}
		T & push_back( node * n )
		{
			n->parent = this;
			children.push_back( n );
		}
		void pop_front()
		{
			children.pop_front( n );
		}
		void pop_back()
		{
			children.pop_back( n );
		}
		node * get_front()
		{
			if ( children.size ) return children.first();
			return NULL;
		}
		node * get_back()
		{
			if ( children.size ) return children.last();
			return NULL;
		}
		int capacity()
		{
			int result = 1;
			for ( listRetain<node>::iterator it( children ); it; ++it ) {
				result += it()->capacity();
			}
			return result;
		}
	};
	class iterator
	{
		list<node*> nodes;
		list<node*>::iterator it;
		void initializeInternal( node * n, bool depthFirst )
		{
			if ( depthFirst ) {
				nodes.push_back( n );
				for ( listRetain<node>::iterator itc( children ); itc; ++itc ) {
					initializeInternal( itc(), true );
				}
			} else {
				list<node*> target;
				target.push_back( n );
				while ( target.size ) {
					list<node*> children;
					while ( target.size ) {
						node * n = target.loot_front();
						nodes.push_back( n );
						for ( listRetain<node>::iterator itc( n->children ); itc; ++itc ) {
							children.push_back( itc() );
						}
					}
					target = children;
				}
			}
		}
	public:
		iterator() : it( nodes ){}
		iterator( const tree & src ) : it( nodes ){ initialize( src ); }
		iterator( const iterator & src ) : nodes( src.nodes ), it( nodes )
		{
			for ( ; it; ++it ) if ( it == src.it ) break;
		}
		void initialize( const tree & base, bool depthFirst = true )
		{
			nodes.release();
			if ( base.root ) initializeInternal( base.root, depthFirst );
			it = list<node*>::iterator( nodes );
		}
		////////////////////////////////////////////////////////////////////////////////
		//èåèîªíËópå^ïœä∑
		operator const node * () const { return it ? it() : NULL; }
		operator node * () { return it ? it() : NULL; }
		////////////////////////////////////////////////////////////////////////////////
		//å^ïœä∑
		const T * operator->() const { assert( it ); return & it()->data; }
		T * operator->() { assert( it ); return & it()->data; }
		T & operator*() { assert( it ); return it()->data; }
		const T & operator*() const { assert( it ); return it()->data; }
		T & operator()() { assert( it ); return it()->data; }
		const T & operator()() const { assert( it ); return it()->data; }
		////////////////////////////////////////////////////////////////////////////////
		//éüÇ÷
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator & operator++()
		{
			if ( it ) ++it;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//ëOÇ÷
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
		iterator & operator--()
		{
			if ( it ) --it;
			return *this;
		}
	};
	node * root;
	int size;
	tree() : root( NULL ), size( 0 ){}
	virtual ~tree() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//èâä˙âª
	void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//âï˙
	void release() { kill( root ); }
	////////////////////////////////////////////////////////////////////////////////
	//ì‡ïîämï€êî
	int capacity() const { return size; }
	////////////////////////////////////////////////////////////////////////////////
	//ãÛ
	bool empty() const { return size ? false : true; }
	////////////////////////////////////////////////////////////////////////////////
	//óvëfÇPÇ¬Çå„ÇÎÇ…í«â¡
	T & push_back( const T & data ){ return push_back( new node( data ) ); }
	T & push_back( node * n )
	{
		if ( root ) {
			root->push_back( n );
		} else {
			n->parent = NULL;
			root = n;
		}
		return n->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//óvëfÇPÇ¬ÇÇ‹Ç¶Ç…í«â¡
	T & push_front( const T & data ) { return push_front( new node( data ) ); }
	T & push_front( node * n )
	{
		if ( root ) {
			root->push_front( n );
		} else {
			n->parent = NULL;
			root = n;
		}
		return n->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//óvëfÇPÇ¬Çå„ÇÎÇ©ÇÁçÌèú
	void pop_back()
	{
		if ( ! root ) return;
		if ( root->children.size ) {
			root->pop_back();
		} else {
			kill( root );
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//óvëfÇPÇ¬ÇëOÇ©ÇÁçÌèú
	void pop_front()
	{
		if ( ! root ) return;
		if ( root->children.size ) {
			root->pop_front();
		} else {
			kill( root );
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	// nodeÇÃéÊÇËäOÇµÇ∆îjâÛ
	void kill( node * n )
	{
		if ( root == n ) {
			destroy( n );
			size = 0;
			root = NULL;
		} else {
			size -= n->capacity();
			destroy( n );
		}
	}
	void kill( T data ) { kill( search( data ) ); }
	void kill( iterator & it ) { if ( it ) kill( ( node * ) it ); }
	////////////////////////////////////////////////////////////////////////////////
	// nodeÇÃîjâÛ
	virtual void destroy( node * n )
	{
		delete n;
	}
	////////////////////////////////////////////////////////////////////////////////
	//åüçı
	iterator search( const T & data )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == data ) break;
		}
		return it;
	}
	////////////////////////////////////////////////////////////////////////////////
	//èÍèä
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
	//ì¡éÍóvëf
	T & index( int i )
	{
		assert( between( 0, i, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) break;
			--i;
		}
		return ( T & ) it;
	}
	const T & index( int i ) const
	{
		assert( between( 0, i, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) break;
			--i;
		}
		return ( T & ) it;
	}
	T & first() { return index( 0 ); }
	const T & first() const { return index( 0 ); }
	T & second() { return index( 1 ); }
	const T & second() const { return index( 1 ); }
	T & third() { return index( 2 ); }
	const T & third() const { return index( 2 ); }
	T & last() { return index( size - 1 ); }
	const T & last() const { return index( size - 1 ); }
	T & prelast() { return index( size - 2 ); }
	const T & prelast() const { return index( size - 2 ); }
	tree & operator=( const tree & src )
	{
		if ( this == &src ) return *this;
		release();
		if ( src.root ) root = new node( * ( src.root ) );
		return *this;
	}
	OperatorSet( tree, T );
};

template<class T>
class treeRetain : public tree<T*>
{
public:
	typedef tree<T*> parent;
	treeRetain()
	{
	}
	virtual ~treeRetain()
	{
		release();
	}
	virtual void destroy( node * n )
	{
		delete n->data;
		delete n;
	}
	T * loot( T * data )
	{
		if ( size < 1 ) return NULL;
		node * n = search( data );
		if ( ! n ) return NULL;
		n->data = NULL;
		kill( n );
		return data;
	}
	treeRetain & operator=( const treeRetain & src )
	{
		if ( this == &src ) return *this;
		release();
		if ( src.root ) {
			root = new node( src.root );
			for ( iterator it( src ); it; ++it ) it() = NULL;
			src.release();
		}
		return *this;
	}
	OperatorSet( treeRetain, T* );
};
