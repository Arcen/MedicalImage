////////////////////////////////////////////////////////////////////////////////
// Balanced Tree

// balanced treeはbranchにhalf～half*2の有効なnodeをsize個もち、
// size+1個のブランチを全て持つ枝か、全て持たない葉である。
// rootは例外で0～half*2のノードをもつことができる
// また、全ての階層の深さは同じである。

// SearchKeyは以下を満たす必要がある。高速化のため実装した．
// SearchKey <= ( SearchKey ) Key
// Key <= new Key( SearchKey )
template<class Key, class T, class SearchKey = Key, int half = 2>
class btree : container<T>
{
public:
	enum {
		num_node = half * 2, 
		num_branch = half * 2 + 1,
	};
	class node
	{
	public:
		Key key;//検索用キーの実体
		T data;//値
		node( const node & src ) : key( src.key ), data( src.data ){}
		node( const Key & _key, T _value ) : key ( _key ), data( _value ){}
		node & operator=( node & src ){ key = src.key; data = src.data; return *this; }
		// KeyとSearchKeyの型変換と演算
		bool operator==( SearchKey & sk ) const { return key == sk; }
		bool operator!=( SearchKey & sk ) const { return key != sk; }
		bool operator<( SearchKey & sk ) const { return key < sk; }
		bool operator>( SearchKey & sk ) const { return key > sk; }
		bool operator<=( SearchKey & sk ) const { return key <= sk; }
		bool operator>=( SearchKey & sk ) const { return key >= sk; }
		operator SearchKey() const { return ( SearchKey )( key ); }
	};
	class branch
	{
	public:
		int size;//ノードの数
		branch * b[num_branch];//子のブランチへのポインタ
		node * n[num_node];//ノードへのポインタ
		////////////////////////////////////////////////////////////////////////////////
		//コンストラクタ
		branch() : size( 0 )
		{
			memset( n, 0, sizeof( node * ) * num_node );
			memset( b, 0, sizeof( branch * ) * num_branch );
		}
		branch( node * _n, branch * low, branch * high ) : size( 1 )
		{
			memset( n, 0, sizeof( node * ) * num_node );
			memset( b, 0, sizeof( branch * ) * num_branch );
			n[0] = _n;
			b[0] = low;
			b[1] = high;
		}
		////////////////////////////////////////////////////////////////////////////////
		// ノードを探す、またはindexを検索キー以上のノードを指すようにする
		node * search( SearchKey sk, int & index )
		{
			while ( index < size && * ( n[index] ) < sk ) index++;
			if ( index < size && * ( n[index] ) == sk ) 
				return n[index];
			return NULL;
		}
		////////////////////////////////////////////////////////////////////////////////
		// ノードを探す
		node * get_front()
		{
			if ( b[0] ) return b[0]->search_front();
			if ( size && n[0] ) return n[0];
			return NULL;
		}
		node * get_back()
		{
			if ( b[size] ) return b[size]->search_back();
			if ( size && n[size-1] ) return n[size-1];
			return NULL;
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[in],b[ib]に挿入
		void insert( int in, int ib, node * _n, branch * _b )
		{
			for ( int i = size; in < i; i-- ) n[i] = n[i-1];
			for ( int i = size + 1; ib < i; i-- ) b[i] = b[i-1];
			n[in] = _n;
			b[ib] = _b;
			size++;
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[in],b[ib]を削除
		void remove( int in, int ib, node * & _n, branch * & _b )
		{
			_n = n[in];
			_b = b[ib];
			for ( int i = in; i < size - 1; i++ ) n[i] = n[i+1];
			for ( int i = ib; i < size; i++ ) b[i] = b[i+1];
			n[size-1] = 0;
			b[size] = 0;
			size--;
		}
		////////////////////////////////////////////////////////////////////////////////
		// n[index],b[index+1]を消して、前に詰める
		bool shorten( int index )
		{
			node * dn; branch * db; // dummy
			remove( index, index + 1, dn, db );
			return size < half ? true : false;//再構成が必要かどうか
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[index],b[index+1]をb[index]に結合し、前に詰める
		bool combine( int index )
		{
			while ( 0 <= b[index+1]->size ) moveFromHigh( index );
			delete b[index+1];//destroyで破壊しない
			return shorten( index );
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[index-1]とb[index-1]の上位をb[index]の下位に挿入
		void moveFromLow( int index )
		{
			node * dn; branch * db; // dummy
			b[index-1]->remove( b[index-1]->size - 1, b[index-1]->size, dn, db );
			b[index]->insert( 0, 0, n[index-1], db );
			n[index-1] = dn;
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[index]とb[index+1]の下位をb[index]の上位に挿入
		//indexは移動する下位のbranch
		void moveFromHigh( int index )
		{
			node * dn; branch * db; // dummy
			b[index+1]->remove( 0, 0, dn, db );
			b[index]->insert( b[index]->size, b[index]->size + 1, n[index], db );
			n[index] = dn;
		}
		////////////////////////////////////////////////////////////////////////////////
		// pop
		//戻り値はサイズが規定より下回ったことを表す
		// _nは取り除くべき葉の位置のノード。削除するノードではない
		bool pop( node * _n )
		{
			int index = 0;
			if ( search( ( SearchKey ) * _n, index ) ) {
				//n[index] == _nである
				branch * s = b[index+1];//high側のブランチ
				if ( ! s ) {
					//thisは葉であった
					//その場所を削除する。このときはbranchは全部からである。
					return shorten( index );
				}
				//枝だった
				while ( s->b[0] ) s = s->b[0];//一番下のブランチの最下位のブランチが無くなるまでサーチ
				//n[index]とs->n[0]を入れ替えてs->n[0]を削除すれば葉のノードを削除することが出来る
				//そこの最初のノードをターゲットに変更
				n[index] = s->n[0];
				s->n[0] = _n;
				index++;//high側に対象があるので、変更
			}
			//見つからなかった。ただ探すべきブランチは特定できた
			if ( b[index] && b[index]->pop( _n ) ) {
				//削除したブランチを調整
				//b[index]は１個ノードがすくないので補充できるなら補充
				if ( 0 < index && b[index-1]->size > half ) {
					moveFromLow( index );
				} else if ( index < size && b[index+1]->size > half ) {
					moveFromHigh( index );
				} else {
					//下、もしくは上と結合
					return combine( index ? index - 1 : 0 );
				}
			}
			return false;
		}
		////////////////////////////////////////////////////////////////////////////////
		// insert
		// 戻り値は上のブランチで再挿入する必要がないことを表す
		bool insert( node * & tn, branch * & tb )
		{
			int index = 0;
			search( ( SearchKey ) * tn, index );//見つかることはないはず

			if ( b[index] && b[index]->insert( tn, tb ) ) return true;

			if ( size < 2 * half ) {//１つ増やしても影響がないかどうか
				// node[index], branch[index+1]の個所に挿入する
				insert( index, index + 1, tn, tb );
				tb = 0; tn = 0;
				return true;//上で再挿入は必要ない
			}
			//分割する必要がある
			// node:half,branch:half+1のbranchを２つ、中間のnode１つに分割する。
			// 前半をthis、中間をtn、後半をtbに入れる

			//挿入
			//half == 2のときtn,tbをindex == 2に挿入するとすると
			//  n0  n1  n2  n3  が  n0  n1  tn  n2  n3    となる
			//b0  b1  b2  b3  b4  b0  b1  b2  tb  b3  b4
			node * _n[num_node+1];
			branch * _b[num_branch+1];
			for ( int i = 0; i < index; i++ ) _n[i] = n[i];
			_n[index] = tn;
			for ( int i = index; i < num_node; i++ ) _n[i+1] = n[i];

			for ( int i = 0; i < index+1; i++ ) _b[i] = b[i];
			_b[index+1] = tb;
			for ( int i = index+1; i < num_branch; i++ ) _b[i+1] = b[i];
			//分離
			//half == 2のとき分離すると
			//  n0  n1  n2  n3  n4  が  n0  n1  とn2と  n3  n4  になる
			//b0  b1  b2  b3  b4  b5  b0  b1  b2      b3  b4  b5
			//前半
			size = half;
			for ( int i = 0; i < half; i++ ) n[i] = _n[i];
			for ( int i = 0; i < half + 1; i++ ) b[i] = _b[i];
			for ( int i = half; i < num_node; i++ ) n[i] = 0;
			for ( int i = half + 1; i < num_branch; i++ ) b[i] = 0;
			//中間
			tn = _n[half];
			//後半
			tb = new branch();
			tb->size = half;
			for ( int i = 0; i < half; i++ ) tb->n[i] = _n[i+half+1];
			for ( int i = 0; i < half + 1; i++ ) tb->b[i] = _b[i+half+1];
			return false;//上で再挿する
		}
	};
	class iterator
	{
		branch * b;
		int index;
		array<branch *> stack;
		////////////////////////////////////////////////////////////////////////////////
		//下の階層に進むことができれば進む
		void goDown()
		{
			if ( ! b ) return;
			while ( b->b[index] ) {
				stack.push_back( b );
				b = b->b[index];
				index = 0;
			}
		}
		////////////////////////////////////////////////////////////////////////////////
		//上の階層に戻る必要があれば戻る
		void goUp()
		{
			while ( b && b->size <= index ) { // need or not to up
				// need to up
				bool found = false;
				if ( stack.size ) {
					branch * parent = stack.last();
					stack.pop_back();
					for ( int i = 0; i <= parent->size; i++ ) {
						if ( b == parent->b[i] ) {
							b = parent;
							index = i;
							found = true;
							break;
						}
					}
				}
				if ( found ) continue;
				b = 0;
				index = 0;
			}
		}
	public:
		////////////////////////////////////////////////////////////////////////////////
		//コンストラクタ
		iterator( const btree & src ) : b( src.root ), index( 0 )
		{
			goDown();
		}
		iterator( const iterator & src ) : b( src.b ), index( src.index ), stack( src.stack ){}
		iterator & operator=( const iterator & src )
		{
			b = src.b;
			index = src.index;
			stack = src.stack;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		iterator & operator++()
		{
			if ( b ) {
				index++;
				goDown();
				goUp();
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//前へ 動作させない
		private: iterator & operator--();
	public:
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator node * () { return ( b && index < b->size ) ? b->n[index] : NULL; }
		operator const node * () const { return ( b && index < b->size ) ? b->n[index] : NULL; }
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		T & operator()() { assert( b && index < b->size ); return b->n[index]->data; }
		const T & operator()() const { assert( b && index < b->size ); return b->n[index]->data; }
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		private: iterator operator--(int);
	};
	branch * root;//ルート
	////////////////////////////////////////////////////////////////////////////////
	//コンストラクタ
	btree() : root( NULL )
	{
	}
	btree( const btree & index ) : root( NULL )
	{ *this = index; }
	////////////////////////////////////////////////////////////////////////////////
	//デストラクタ
	virtual ~btree()
	{
		release();
	}
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
	//開放
	void release()
	{
		destroy( root );
		size = 0;
	}
private:
	////////////////////////////////////////////////////////////////////////////////
	//ノードの破壊
	virtual void destroy( node * & n )
	{
		delete n;
		n = NULL;
	}
	////////////////////////////////////////////////////////////////////////////////
	//ブランチの破壊
	virtual void destroy( branch * & b )
	{
		if ( ! b ) return;
		int i;
		for ( i = 0; i < b->size; i++ ) {
			destroy( b->n[i] );
		}
		for ( i = 0; i <= b->size; i++ ) {
			destroy( b->b[i] );
		}
		delete b;
		b = NULL;
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
	//ノードの検索
	node * search( SearchKey src )
	{
		branch * b = root;
		while ( b ) {
			int index = 0;
			node * found = b->search( src, index );
			if ( found ) return found;
			b = b->b[index];
		}
		return NULL;
	}
	/*
	node * get_front()
	{
		if ( root ) return root->get_front();
		return NULL;
	}
	node * get_back()
	{
		if ( root ) return root->get_back();
		return NULL;
	}
	*/
	////////////////////////////////////////////////////////////////////////////////
	//戻り値は挿入が行われたかどうかを表す。行われないときにはノードの削除を行う
	bool push( const Key & k, T & v )
	{
		return push( new node( k, v ) );
	}
	bool push( node * n )
	{
		node * r = search( n->key );
		if ( r ) {
			destroy( n );
			return false;
		}
		size++;
		node * targetNode = n;
		branch * targetBranch = 0;
		if ( ! root ) {
			root = new branch( targetNode, 0, 0 );
		} else {
			if ( ! root->insert( targetNode, targetBranch ) ) {
				root = new branch( targetNode, root, targetBranch );
			}
		}
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////
	//削除
	void pop( node * n )
	{
		if ( root ) root->pop( n );
		if ( root && ! root->size ) {//rootの中身がなくなった時
			branch * b = root;
			root = root->b[0];
			delete b;
		}
		destroy( n );
		size--;
	}
	void popByKey( const SearchKey & src )
	{
		node * n = search( src );
		if ( n ) pop( n );
	}
	virtual void pop( const T & src )
	{
		for ( iterator it( *this ); it; ++it ) {
			node * n = it;
			if ( n->data == src ) {
				pop( n );
				return;
			}
		}
	}
	T loot( node * n )
	{
		T wk( n->data );
		if ( root ) root->pop( n );
		if ( root && ! root->size ) {//rootの中身がなくなった時
			branch * b = root;
			root = root->b[0];
			delete b;
		}
		destroy( n );
		size--;
		return wk;
	}
	void lootByKey( const SearchKey & src )
	{
		node * n = search( src );
		if ( n ) return loot( n );
		T wk;
		return wk;
	}
	virtual T loot( const T & src )
	{
		for ( iterator it( *this ); it; ++it ) {
			node * n = it;
			if ( n->data == src ) return loot( n );
		}
		T wk;
		return wk;
	}
	////////////////////////////////////////////////////////////////////////////////
	//場所
	virtual int index( const T & src ) const
	{
		int i = 0;
		for ( iterator it( *this ); it; ++it, ++i ) {
			if ( ( ( node * ) it )->data == src ) break;
		}
		return i;
	}
	int indexByKey( const SearchKey & src ) const
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
		assert( between( 0, src, size - 1 ) && "btree bad access" );
		for ( iterator it( *this ); it; ++it ) {
			if ( src == 0 ) return it();
			src--;
		}
		static T wk; return wk;
	}
	virtual const T & value( int src ) const
	{
		assert( between( 0, src, size - 1 ) && "btree bad access" );
		for ( iterator it( *this ); it; ++it ) {
			if ( src == 0 ) return it();
			src--;
		}
		static T wk; return wk;
	}
	btree & operator=( const btree & src )
	{
		if ( this == &src ) return *this;
		release();
		for ( iterator it( src ); it; ++it ) {
			node * n = it;
			insert( n->key, n->data );
		}
		return *this;
	}
	OperatorSet( btree, T );
};
