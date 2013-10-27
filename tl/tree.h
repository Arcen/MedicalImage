////////////////////////////////////////////////////////////////////////////////
// tree

//木構造
template<class T>
class tree : public container<T>
{
public:
	class node
	{
	public:
		T data;
		node * parent, * prev, * next, * head, * tail;
		node( const T & src ) : data( src ), parent( NULL ), prev( NULL ), next( NULL ), head( NULL ), tail( NULL ){}
		node( const node & src ) : data( src.data ), parent( NULL ), prev( NULL ), next( NULL ), head( NULL ), tail( NULL )
		{
			for ( node * child = src->head; child; child = child->next ) {
				push_back( new node( * child ) );
			}
		}
		~node()
		{
			orphan();
		}
		//ルートノードを得る
		const node * root() const 
		{
			const node * r = this;
			while ( r->parent ) r = r->parent;
			return r;
		}
		//親から独立する
		void independent()
		{
			if ( parent ) {
				if ( prev ) {
					prev->next = next;
				} else {
					parent->head = next;
				}
				if ( next ) {
					next->prev = prev;
				} else {
					parent->tail = prev;
				}
				parent = prev = next = NULL;
			}
		}
		//子供を削除
		void killChildren()
		{
			while ( head ) pop( head );
		}
		//親子を全部取る、親は接続を切り、子供を削除
		void orphan()
		{
			independent();
			killChildren();
		}
		const node * search( const T & data ) const 
		{
			for ( node * child = head; child; child = child->next ) {
				if ( data == child->data ) return child;
				result += child->capacity();
			}
			return NULL;
		}
		T & push_front( node * src )
		{
			src->parent = this;
			src->prev = NULL;
			if ( head ) {
				head->prev = src;
				src->next = head;
			}
			head = src;
			if ( ! tail ) tail = src;
			return src->data;
		}
		T & push_back( node * src )
		{
			src->parent = this;
			src->next = NULL;
			if ( tail ) {
				tail->next = src;
				src->prev = tail;
			}
			tail = src;
			if ( ! head ) head = src;
			return src->data;
		}
		void pop( node * src )
		{
			if ( src->parent != this ) return;
			delete src;
		}
		void pop_front(){ pop( head ); }
		void pop_back(){ pop( tail ); }
		T loot( node * src )
		{
			T wk;
			if ( src->parent != this ) return wk;
			wk = src->data;
			delete src;
			return wk;
		}
		T loot_front(){ loot( head ); }
		T loot_back(){ loot( tail ); }
		int capacity()
		{
			int result = 1;
			for ( node * child = head; child; child = child->next ) {
				result += child->capacity();
			}
			return result;
		}
	};
	//縦形探索と横形探索をひとつのクラスで行う。インクリメント、デクリメントに注意。
	class iterator
	{
		node * n;
	public:
		iterator() : n( NULL ){}
		iterator( const tree & src ) : n( src.root ){}
		iterator( const iterator & src ) : n( src.n ){}
		iterator & operator=( const iterator & src )
		{
			n = src.n;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator node * () { return n; }
		operator const node * () const { return n; }
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		T & operator()() { assert( n ); return n->data; }
		const T & operator()() const { assert( n ); return n->data; }
		////////////////////////////////////////////////////////////////////////////////
		//次へ（縦形探索）
		iterator & operator++()
		{
			//横には下側から移動できているので、一度上に上がったときには、そのノード以下は探索済みである。
			bool down = true;
			while ( n ) {
				if ( n->head && down ) {
					n = n->head;
					break;
				} else if ( n->next ) {
					n = n->next;
					break;
				} else {
					n = n->parent;
					down = false;//もう、下は探す必要がない。
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//前へ（縦形探索）
		iterator & operator--()
		{
			if ( n->prev ) {//前に戻れる時には、戻ってその一番右下隅になる
				n = n->prev;
				while ( n->tail ) {
					n = n->tail;
				}
			} else if ( n->parent ) {//前がなくて、上があったら、上が一つ前
				n = n->parent;
			} else {
				n = NULL;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//次へ（横形探索）
		iterator & operator++(int)
		{
			int depth = 0;//現在からの探す深さ
			bool foundRoot = false;
			//通常では、下は無視でいける。探すべき範囲が下を含んでいるようならば、下にも下がれるようにする。
			bool down = ( depth == 0 ? false : true );
			while ( n ) {
				if ( n->head && down ) {//下にいけるならば、下に行く。
					n = n->head;
					--depth;
					if ( depth == 0 ) break;//同じ階層ならば、見つかった。
				} else if ( n->next ) {//横にあるならば、進む
					n = n->next;
					if ( depth == 0 ) break;//同じ階層ならば、見つかった。
					down = true;//下にも探さなければならない。
				} else {//一度上に戻って、探しなおす必要がある。
					if ( ! n->parent ) {
						if ( foundRoot ) {//２度たどり着いたならば、もう探す深さはないことになる。
							n = NULL;
							return *this;
						}
						//一度見つかっただけならば、深さを下げて探しなおす。
						depth++;
						foundRoot = true;
						down = true;
					} else {
						depth++;
						n = n->parent;
						down = false;//一度、あがったら、nextに進むまでは、下がる必要がない。
					}
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//前へ（横形探索）
		iterator & operator--(int)
		{
			int depth = 0;//現在からの探す深さ
			//通常では、下は無視でいける。探すべき範囲が下を含んでいるようならば、下にも下がれるようにする。
			bool down = ( depth == 0 ? false : true );
			while ( n ) {
				if ( n->tail && down ) {//下にいけるならば、下に行く。
					n = n->tail;
					--depth;
					if ( depth == 0 ) break;//同じ階層ならば、見つかった。
				} else if ( n->prev ) {//横にあるならば、戻る
					n = n->prev;
					if ( depth == 0 ) break;//同じ階層ならば、見つかった。
					down = true;//下にも探さなければならない。
				} else {//一度上に戻って、探しなおす必要がある。
					if ( ! n->parent ) {
						if ( depth == 0 ) {//最初から始まっている時には、これで最後。
							n = NULL;
							return *this;
						}
						//深さを上げて探しなおす。
						down = true;
						--depth;
						if ( depth == 0 ) break;//同じ階層ならば、見つかった。
					} else {
						depth++;
						n = n->parent;
						down = false;//一度、あがったら、prevに進むまでは、下がる必要がない。
					}
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
	};
	node * root;
	tree() : root( NULL ) {}
	tree( const tree & index ) : root( NULL )
	{ *this = index; }
	virtual ~tree() { finalize(); }
	////////////////////////////////////////////////////////////////////////////////
	//初期化
	virtual void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//終了化
	virtual void finalize() { delete root; }
	////////////////////////////////////////////////////////////////////////////////
	//確保
	virtual void allocate( int ){}
	////////////////////////////////////////////////////////////////////////////////
	//解放
	virtual void release() 
	{
		delete root;
		root = NULL;
		size = 0;
	}
	////////////////////////////////////////////////////////////////////////////////
	//アクセス禁止
private:
	virtual T & push_front( const T & src ){ assert( 0 ); static T wk; return wk; }
	virtual T & push_back( const T & src ){ assert( 0 ); static T wk; return wk; }
	virtual void pop_front(){ assert( 0 ); }
	virtual void pop_back(){ assert( 0 ); }
	virtual T & put_front(){ assert( 0 ); static T wk; return wk; }
	virtual T & put_back(){ assert( 0 ); static T wk; return wk; }
	virtual T loot_front(){ assert( 0 ); T wk; return wk; }
	virtual T loot_back(){ assert( 0 ); T wk; return wk; }
public:
	////////////////////////////////////////////////////////////////////////////////
	//ルートを追加
	virtual T & push( const T & data ){ return push( new node( data ) ); }
	T & push( node * n )
	{
		if ( root ) {
			delete n;
			return root->data;
		}
		n->parent = NULL;
		root = n;
		size = root->capacity();
		return n->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//親を指定して、子供の後ろに追加
	T & push_back( const T & data, const T & parent ){ return push_back( new node( data ), search( parent ) ); }
	T & push_back( node * n, const T & parent ){ return push_back( n, search( parent ) ); }
	T & push_back( const T & data, node * parent ){ return push_back( new node( data ), parent ); }
	T & push_back( node * n, node * parent )
	{
		if ( parent->root() != root ) {
			delete n;
			static T wk;
			return wk;
		}
		parent->push_back( n );
		size = root ? root->capacity() : 0;
		return n->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//親を指定して、子供の前に追加
	T & push_front( const T & data, const T & parent ){ return push_front( new node( data ), search( parent ) ); }
	T & push_front( node * n, const T & parent ){ return push_front( n, search( parent ) ); }
	T & push_front( const T & data, node * parent ){ return push_front( new node( data ), parent ); }
	T & push_front( node * n, node * parent )
	{
		if ( parent->root() != root ) {
			delete n;
			static T wk;
			return wk;
		}
		parent->push_front( n );
		size = root ? root->capacity() : 0;
		return n->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//ルートを削除
	void pop()
	{
		if ( ! root ) return;
		delete root;
		root = NULL;
		size = 0;
	}
	////////////////////////////////////////////////////////////////////////////////
	//ノードを削除
	void pop( node * src )
	{
		if ( src->root() != root ) return;
		if ( src->parent ) {
			delete src;
			size = root->capacity();
		} else {
			pop();
		}
	}
	virtual void pop( const T & src )
	{
		node * n = const_cast<node*>( search( src ) );
		if ( n ) pop( n );
	}
	void pop( node * parent, const T & src )
	{
		node * n = parent->search( src );
		if ( n ) pop( n );
	}
	////////////////////////////////////////////////////////////////////////////////
	//親を指定して、子供の後ろを削除
	void pop_back( node * parent ) { pop( parent->tail ); }
	////////////////////////////////////////////////////////////////////////////////
	//親を指定して、子供の前を削除
	void pop_front( node * parent ) { pop( parent->head ); }
	////////////////////////////////////////////////////////////////////////////////
	//ルートを取り外し
	T loot()
	{
		if ( root ) return loot( root );
		T wk;
		return wk;
	}
	////////////////////////////////////////////////////////////////////////////////
	//ノードを取り外し
	T loot( node * src )
	{
		T wk;
		if ( src->root() != root ) return wk;
		wk = src->data;
		pop( src );
		return wk;
	}
	virtual T loot( const T & src )
	{
		node * n = const_cast<node*>( search( src ) );
		if ( n ) return loot( n );
		T wk;
		return wk;
	}
	T loot( node * parent, const T & src )
	{
		node * n = parent->search( src );
		if ( n ) return loot( n );
		T wk;
		return wk;
	}
	////////////////////////////////////////////////////////////////////////////////
	//親を指定して、子供の後ろを取り外し
	T loot_back( node * parent ) { return loot( parent->tail ); }
	////////////////////////////////////////////////////////////////////////////////
	//親を指定して、子供の前を取り外し
	T loot_front( node * parent ) { return loot( parent->head ); }
	////////////////////////////////////////////////////////////////////////////////
	//検索
	node * search( const T & data ) 
	{
		for ( iterator it( *this ); it; ++it ) {
			if ( it() == data ) {
				return it;
			}
		}
		return NULL;
	}
	////////////////////////////////////////////////////////////////////////////////
	//場所
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
	T & value( int i )
	{
		assert( between( 0, i, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) return it();
			--i;
		}
		static T wk;
		return wk;
	}
	const T & value( int i ) const
	{
		assert( between( 0, i, size - 1 ) );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) return it();
			--i;
		}
		static T wk;
		return wk;
	}
	tree & operator=( const tree & src )
	{
		if ( this == &src ) return *this;
		release();
		if ( src.root ) root = new node( * ( src.root ) );
		return *this;
	}
	OperatorSet( tree, T );
};
