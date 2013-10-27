////////////////////////////////////////////////////////////////////////////////
// tree

//�؍\��
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
		//���[�g�m�[�h�𓾂�
		const node * root() const 
		{
			const node * r = this;
			while ( r->parent ) r = r->parent;
			return r;
		}
		//�e����Ɨ�����
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
		//�q�����폜
		void killChildren()
		{
			while ( head ) pop( head );
		}
		//�e�q��S�����A�e�͐ڑ���؂�A�q�����폜
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
	//�c�`�T���Ɖ��`�T�����ЂƂ̃N���X�ōs���B�C���N�������g�A�f�N�������g�ɒ��ӁB
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
		//��������p�^�ϊ�
		operator node * () { return n; }
		operator const node * () const { return n; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		T & operator()() { assert( n ); return n->data; }
		const T & operator()() const { assert( n ); return n->data; }
		////////////////////////////////////////////////////////////////////////////////
		//���ցi�c�`�T���j
		iterator & operator++()
		{
			//���ɂ͉�������ړ��ł��Ă���̂ŁA��x��ɏオ�����Ƃ��ɂ́A���̃m�[�h�ȉ��͒T���ς݂ł���B
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
					down = false;//�����A���͒T���K�v���Ȃ��B
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//�O�ցi�c�`�T���j
		iterator & operator--()
		{
			if ( n->prev ) {//�O�ɖ߂�鎞�ɂ́A�߂��Ă��̈�ԉE�����ɂȂ�
				n = n->prev;
				while ( n->tail ) {
					n = n->tail;
				}
			} else if ( n->parent ) {//�O���Ȃ��āA�オ��������A�オ��O
				n = n->parent;
			} else {
				n = NULL;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//���ցi���`�T���j
		iterator & operator++(int)
		{
			int depth = 0;//���݂���̒T���[��
			bool foundRoot = false;
			//�ʏ�ł́A���͖����ł�����B�T���ׂ��͈͂������܂�ł���悤�Ȃ�΁A���ɂ��������悤�ɂ���B
			bool down = ( depth == 0 ? false : true );
			while ( n ) {
				if ( n->head && down ) {//���ɂ�����Ȃ�΁A���ɍs���B
					n = n->head;
					--depth;
					if ( depth == 0 ) break;//�����K�w�Ȃ�΁A���������B
				} else if ( n->next ) {//���ɂ���Ȃ�΁A�i��
					n = n->next;
					if ( depth == 0 ) break;//�����K�w�Ȃ�΁A���������B
					down = true;//���ɂ��T���Ȃ���΂Ȃ�Ȃ��B
				} else {//��x��ɖ߂��āA�T���Ȃ����K�v������B
					if ( ! n->parent ) {
						if ( foundRoot ) {//�Q�x���ǂ蒅�����Ȃ�΁A�����T���[���͂Ȃ����ƂɂȂ�B
							n = NULL;
							return *this;
						}
						//��x�������������Ȃ�΁A�[���������ĒT���Ȃ����B
						depth++;
						foundRoot = true;
						down = true;
					} else {
						depth++;
						n = n->parent;
						down = false;//��x�A����������Anext�ɐi�ނ܂ł́A������K�v���Ȃ��B
					}
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//�O�ցi���`�T���j
		iterator & operator--(int)
		{
			int depth = 0;//���݂���̒T���[��
			//�ʏ�ł́A���͖����ł�����B�T���ׂ��͈͂������܂�ł���悤�Ȃ�΁A���ɂ��������悤�ɂ���B
			bool down = ( depth == 0 ? false : true );
			while ( n ) {
				if ( n->tail && down ) {//���ɂ�����Ȃ�΁A���ɍs���B
					n = n->tail;
					--depth;
					if ( depth == 0 ) break;//�����K�w�Ȃ�΁A���������B
				} else if ( n->prev ) {//���ɂ���Ȃ�΁A�߂�
					n = n->prev;
					if ( depth == 0 ) break;//�����K�w�Ȃ�΁A���������B
					down = true;//���ɂ��T���Ȃ���΂Ȃ�Ȃ��B
				} else {//��x��ɖ߂��āA�T���Ȃ����K�v������B
					if ( ! n->parent ) {
						if ( depth == 0 ) {//�ŏ�����n�܂��Ă��鎞�ɂ́A����ōŌ�B
							n = NULL;
							return *this;
						}
						//�[�����グ�ĒT���Ȃ����B
						down = true;
						--depth;
						if ( depth == 0 ) break;//�����K�w�Ȃ�΁A���������B
					} else {
						depth++;
						n = n->parent;
						down = false;//��x�A����������Aprev�ɐi�ނ܂ł́A������K�v���Ȃ��B
					}
				}
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
	};
	node * root;
	tree() : root( NULL ) {}
	tree( const tree & index ) : root( NULL )
	{ *this = index; }
	virtual ~tree() { finalize(); }
	////////////////////////////////////////////////////////////////////////////////
	//������
	virtual void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//�I����
	virtual void finalize() { delete root; }
	////////////////////////////////////////////////////////////////////////////////
	//�m��
	virtual void allocate( int ){}
	////////////////////////////////////////////////////////////////////////////////
	//���
	virtual void release() 
	{
		delete root;
		root = NULL;
		size = 0;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�A�N�Z�X�֎~
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
	//���[�g��ǉ�
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
	//�e���w�肵�āA�q���̌��ɒǉ�
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
	//�e���w�肵�āA�q���̑O�ɒǉ�
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
	//���[�g���폜
	void pop()
	{
		if ( ! root ) return;
		delete root;
		root = NULL;
		size = 0;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�m�[�h���폜
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
	//�e���w�肵�āA�q���̌����폜
	void pop_back( node * parent ) { pop( parent->tail ); }
	////////////////////////////////////////////////////////////////////////////////
	//�e���w�肵�āA�q���̑O���폜
	void pop_front( node * parent ) { pop( parent->head ); }
	////////////////////////////////////////////////////////////////////////////////
	//���[�g�����O��
	T loot()
	{
		if ( root ) return loot( root );
		T wk;
		return wk;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�m�[�h�����O��
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
	//�e���w�肵�āA�q���̌������O��
	T loot_back( node * parent ) { return loot( parent->tail ); }
	////////////////////////////////////////////////////////////////////////////////
	//�e���w�肵�āA�q���̑O�����O��
	T loot_front( node * parent ) { return loot( parent->head ); }
	////////////////////////////////////////////////////////////////////////////////
	//����
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
	//�ꏊ
	virtual int index( const T & src ) const
	{
		int i = 0;
		for ( iterator it( *this ); it; ++it, ++i ) {
			if ( it() == src ) break;
		}
		return i;
	}
	////////////////////////////////////////////////////////////////////////////////
	//����v�f
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
