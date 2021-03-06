////////////////////////////////////////////////////////////////////////////////
// Chain

//単方向リスト
template<class T>
class chain : public container<T>
{
public:
	class node
	{
	public:
		T data;
		node * next;
		node( const T & src ) : data( src ), next( NULL ){}
	};
	class iterator
	{
		node * n;
	public:
		iterator() : n( NULL ){}
		iterator( node * src ) : n( src ){}
		iterator( const node & src ) : n( & src ){}
		iterator( const chain & base ) : n( base.head ){}
		iterator( const iterator & it ) : n( it.n ){}
		iterator & operator=( const iterator & it ){ n = it.n; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator const node * () const { return n; }
		operator node * () { return n; }
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		T & operator()()				{ assert( n ); return n->data; }
		const T & operator()() const	{ assert( n ); return n->data; }
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		iterator & operator++()
		{
			if ( n ) n = n->next;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//前へ
	private:
		iterator & operator--();
	public:
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		private: iterator operator--(int);
	};
	node * head;
	chain() : head( NULL ){}
	chain( const chain & index ) : head( NULL )
	{ *this = index; }
	virtual ~chain() { finalize(); }
	////////////////////////////////////////////////////////////////////////////////
	//初期化
	virtual void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//終了化
	virtual void finalize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//確保
	virtual void allocate( int ){}
	////////////////////////////////////////////////////////////////////////////////
	//解放
	virtual void release() { while ( head ) pop( head ); }
	////////////////////////////////////////////////////////////////////////////////
	//要素１つを後ろに追加
	virtual T & push_back( const T & src ){ return push_back( new node( src ) ); }
	T & push_back( node * src )
	{
		src->next = NULL;
		if ( head ) {
			node * tail = lastNode();
			tail->next = src;
		}
		if ( ! head ) head = src;
		size++;
		return src->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//要素１つをまえに追加
	virtual T & push_front( const T & src ) { return push_front( new node( src ) ); }
	T & push_front( node * src )
	{
		src->next = head;
		head = src;
		size++;
		return src->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//要素１つを後ろから削除
	virtual void pop_back(){ if ( head ) pop( lastNode() ); }
	////////////////////////////////////////////////////////////////////////////////
	//要素１つを前から削除
	virtual void pop_front() { if ( head ) pop( head, NULL ); }
	////////////////////////////////////////////////////////////////////////////////
	// nodeの取り外しと破壊
	void pop( node * src, node * prev )
	{
		if ( prev ) {
			prev->next = src->next;
		} else {
			head = src->next;
		}
		src->next = NULL;
		size--;
		delete src;
	}
	virtual void pop( const T & src )
	{
		node * prev = NULL;
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == src ) {
				pop( it, prev );
				return;
			}
			prev = it;
		}
	}
	void pop( node * src )
	{
		node * prev = NULL;
		for ( iterator it( *this ); it; ++it ) {
			if ( it == src ) {
				pop( it, prev );
				return;
			}
			prev = it;
		}
	}
	T loot( node * src, node * prev )
	{
		T wk( src->data );
		if ( prev ) {
			prev->next = src->next;
		} else {
			head = src->next;
		}
		src->next = NULL;
		size--;
		delete src;
		return wk;
	}
	virtual T loot( const T & src )
	{
		node * prev = NULL;
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == src ) {
				return loot( it, prev );
			}
			prev = it;
		}
		T wk;
		return wk;
	}
	T loot( node * src )
	{
		node * prev = NULL;
		for ( iterator it( *this ); it; ++it ) {
			if ( it == src ) {
				return loot( it, prev );
			}
			prev = it;
		}
		T wk;
		return wk;
	}
	////////////////////////////////////////////////////////////////////////////////
	//検索
	iterator search( const T & src )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == src ) return it;
		}
		return it;
	}
	bool exist( node * src )
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it == src ) return true;
		}
		return false;
	}
	////////////////////////////////////////////////////////////////////////////////
	//場所
	int index( const iterator & src ) const { index( src() ); }
	virtual int index( const T & src ) const
	{
		int i = 0;
		for ( iterator it( *this ); it; ++it, ++i ) {
			if ( it() == src ) break;
		}
		return i;
	}
	////////////////////////////////////////////////////////////////////////////////
	//特殊要素
	virtual T & value( int src )
	{
		assert( between( 0, src, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( src == 0 ) break;
			src--;
		}
		return it();
	}
	virtual const T & value( int src ) const
	{
		assert( between( 0, src, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( src == 0 ) break;
			src--;
		}
		return it();
	}
	T & operator[]( int src ) { return value( src ); }
	const T & operator[]( int src ) const { return value( src ); }

	chain & operator=( const chain & index )
	{
		if ( this == &index ) return *this;
		release();
		for ( iterator itsrc( index ); itsrc; ++itsrc ) push_back( itsrc() );
		return *this;
	}
	OperatorSet( chain, T );
};
