////////////////////////////////////////////////////////////////////////////////
// Chain

//�P�������X�g
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
		//��������p�^�ϊ�
		operator const node * () const { return n; }
		operator node * () { return n; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		T & operator()()				{ assert( n ); return n->data; }
		const T & operator()() const	{ assert( n ); return n->data; }
		////////////////////////////////////////////////////////////////////////////////
		//����
		iterator & operator++()
		{
			if ( n ) n = n->next;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//�O��
	private:
		iterator & operator--();
	public:
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
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
	//������
	virtual void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//�I����
	virtual void finalize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//�m��
	virtual void allocate( int ){}
	////////////////////////////////////////////////////////////////////////////////
	//���
	virtual void release() { while ( head ) pop( head ); }
	////////////////////////////////////////////////////////////////////////////////
	//�v�f�P�����ɒǉ�
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
	//�v�f�P���܂��ɒǉ�
	virtual T & push_front( const T & src ) { return push_front( new node( src ) ); }
	T & push_front( node * src )
	{
		src->next = head;
		head = src;
		size++;
		return src->data;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�v�f�P����납��폜
	virtual void pop_back(){ if ( head ) pop( lastNode() ); }
	////////////////////////////////////////////////////////////////////////////////
	//�v�f�P��O����폜
	virtual void pop_front() { if ( head ) pop( head, NULL ); }
	////////////////////////////////////////////////////////////////////////////////
	// node�̎��O���Ɣj��
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
	//����
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
	//�ꏊ
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
	//����v�f
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
