////////////////////////////////////////////////////////////////////////////////
// Balanced Tree

// balanced tree��branch��half�`half*2�̗L����node��size�����A
// size+1�̃u�����`��S�Ď��}���A�S�Ď����Ȃ��t�ł���B
// root�͗�O��0�`half*2�̃m�[�h�������Ƃ��ł���
// �܂��A�S�Ă̊K�w�̐[���͓����ł���B

// SearchKey�͈ȉ��𖞂����K�v������B�������̂��ߎ��������D
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
		Key key;//�����p�L�[�̎���
		T data;//�l
		node( const node & src ) : key( src.key ), data( src.data ){}
		node( const Key & _key, T _value ) : key ( _key ), data( _value ){}
		node & operator=( node & src ){ key = src.key; data = src.data; return *this; }
		// Key��SearchKey�̌^�ϊ��Ɖ��Z
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
		int size;//�m�[�h�̐�
		branch * b[num_branch];//�q�̃u�����`�ւ̃|�C���^
		node * n[num_node];//�m�[�h�ւ̃|�C���^
		////////////////////////////////////////////////////////////////////////////////
		//�R���X�g���N�^
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
		// �m�[�h��T���A�܂���index�������L�[�ȏ�̃m�[�h���w���悤�ɂ���
		node * search( SearchKey sk, int & index )
		{
			while ( index < size && * ( n[index] ) < sk ) index++;
			if ( index < size && * ( n[index] ) == sk ) 
				return n[index];
			return NULL;
		}
		////////////////////////////////////////////////////////////////////////////////
		// �m�[�h��T��
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
		//n[in],b[ib]�ɑ}��
		void insert( int in, int ib, node * _n, branch * _b )
		{
			for ( int i = size; in < i; i-- ) n[i] = n[i-1];
			for ( int i = size + 1; ib < i; i-- ) b[i] = b[i-1];
			n[in] = _n;
			b[ib] = _b;
			size++;
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[in],b[ib]���폜
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
		// n[index],b[index+1]�������āA�O�ɋl�߂�
		bool shorten( int index )
		{
			node * dn; branch * db; // dummy
			remove( index, index + 1, dn, db );
			return size < half ? true : false;//�č\�����K�v���ǂ���
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[index],b[index+1]��b[index]�Ɍ������A�O�ɋl�߂�
		bool combine( int index )
		{
			while ( 0 <= b[index+1]->size ) moveFromHigh( index );
			delete b[index+1];//destroy�Ŕj�󂵂Ȃ�
			return shorten( index );
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[index-1]��b[index-1]�̏�ʂ�b[index]�̉��ʂɑ}��
		void moveFromLow( int index )
		{
			node * dn; branch * db; // dummy
			b[index-1]->remove( b[index-1]->size - 1, b[index-1]->size, dn, db );
			b[index]->insert( 0, 0, n[index-1], db );
			n[index-1] = dn;
		}
		////////////////////////////////////////////////////////////////////////////////
		//n[index]��b[index+1]�̉��ʂ�b[index]�̏�ʂɑ}��
		//index�͈ړ����鉺�ʂ�branch
		void moveFromHigh( int index )
		{
			node * dn; branch * db; // dummy
			b[index+1]->remove( 0, 0, dn, db );
			b[index]->insert( b[index]->size, b[index]->size + 1, n[index], db );
			n[index] = dn;
		}
		////////////////////////////////////////////////////////////////////////////////
		// pop
		//�߂�l�̓T�C�Y���K���艺��������Ƃ�\��
		// _n�͎�菜���ׂ��t�̈ʒu�̃m�[�h�B�폜����m�[�h�ł͂Ȃ�
		bool pop( node * _n )
		{
			int index = 0;
			if ( search( ( SearchKey ) * _n, index ) ) {
				//n[index] == _n�ł���
				branch * s = b[index+1];//high���̃u�����`
				if ( ! s ) {
					//this�͗t�ł�����
					//���̏ꏊ���폜����B���̂Ƃ���branch�͑S������ł���B
					return shorten( index );
				}
				//�}������
				while ( s->b[0] ) s = s->b[0];//��ԉ��̃u�����`�̍ŉ��ʂ̃u�����`�������Ȃ�܂ŃT�[�`
				//n[index]��s->n[0]�����ւ���s->n[0]���폜����Ηt�̃m�[�h���폜���邱�Ƃ��o����
				//�����̍ŏ��̃m�[�h���^�[�Q�b�g�ɕύX
				n[index] = s->n[0];
				s->n[0] = _n;
				index++;//high���ɑΏۂ�����̂ŁA�ύX
			}
			//������Ȃ������B�����T���ׂ��u�����`�͓���ł���
			if ( b[index] && b[index]->pop( _n ) ) {
				//�폜�����u�����`�𒲐�
				//b[index]�͂P�m�[�h�������Ȃ��̂ŕ�[�ł���Ȃ��[
				if ( 0 < index && b[index-1]->size > half ) {
					moveFromLow( index );
				} else if ( index < size && b[index+1]->size > half ) {
					moveFromHigh( index );
				} else {
					//���A�������͏�ƌ���
					return combine( index ? index - 1 : 0 );
				}
			}
			return false;
		}
		////////////////////////////////////////////////////////////////////////////////
		// insert
		// �߂�l�͏�̃u�����`�ōđ}������K�v���Ȃ����Ƃ�\��
		bool insert( node * & tn, branch * & tb )
		{
			int index = 0;
			search( ( SearchKey ) * tn, index );//�����邱�Ƃ͂Ȃ��͂�

			if ( b[index] && b[index]->insert( tn, tb ) ) return true;

			if ( size < 2 * half ) {//�P���₵�Ă��e�����Ȃ����ǂ���
				// node[index], branch[index+1]�̌��ɑ}������
				insert( index, index + 1, tn, tb );
				tb = 0; tn = 0;
				return true;//��ōđ}���͕K�v�Ȃ�
			}
			//��������K�v������
			// node:half,branch:half+1��branch���Q�A���Ԃ�node�P�ɕ�������B
			// �O����this�A���Ԃ�tn�A�㔼��tb�ɓ����

			//�}��
			//half == 2�̂Ƃ�tn,tb��index == 2�ɑ}������Ƃ����
			//  n0  n1  n2  n3  ��  n0  n1  tn  n2  n3    �ƂȂ�
			//b0  b1  b2  b3  b4  b0  b1  b2  tb  b3  b4
			node * _n[num_node+1];
			branch * _b[num_branch+1];
			for ( int i = 0; i < index; i++ ) _n[i] = n[i];
			_n[index] = tn;
			for ( int i = index; i < num_node; i++ ) _n[i+1] = n[i];

			for ( int i = 0; i < index+1; i++ ) _b[i] = b[i];
			_b[index+1] = tb;
			for ( int i = index+1; i < num_branch; i++ ) _b[i+1] = b[i];
			//����
			//half == 2�̂Ƃ����������
			//  n0  n1  n2  n3  n4  ��  n0  n1  ��n2��  n3  n4  �ɂȂ�
			//b0  b1  b2  b3  b4  b5  b0  b1  b2      b3  b4  b5
			//�O��
			size = half;
			for ( int i = 0; i < half; i++ ) n[i] = _n[i];
			for ( int i = 0; i < half + 1; i++ ) b[i] = _b[i];
			for ( int i = half; i < num_node; i++ ) n[i] = 0;
			for ( int i = half + 1; i < num_branch; i++ ) b[i] = 0;
			//����
			tn = _n[half];
			//�㔼
			tb = new branch();
			tb->size = half;
			for ( int i = 0; i < half; i++ ) tb->n[i] = _n[i+half+1];
			for ( int i = 0; i < half + 1; i++ ) tb->b[i] = _b[i+half+1];
			return false;//��ōđ}����
		}
	};
	class iterator
	{
		branch * b;
		int index;
		array<branch *> stack;
		////////////////////////////////////////////////////////////////////////////////
		//���̊K�w�ɐi�ނ��Ƃ��ł���ΐi��
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
		//��̊K�w�ɖ߂�K�v������Ζ߂�
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
		//�R���X�g���N�^
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
		//����
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
		//�O�� ���삳���Ȃ�
		private: iterator & operator--();
	public:
		////////////////////////////////////////////////////////////////////////////////
		//��������p�^�ϊ�
		operator node * () { return ( b && index < b->size ) ? b->n[index] : NULL; }
		operator const node * () const { return ( b && index < b->size ) ? b->n[index] : NULL; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		T & operator()() { assert( b && index < b->size ); return b->n[index]->data; }
		const T & operator()() const { assert( b && index < b->size ); return b->n[index]->data; }
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
		T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		private: iterator operator--(int);
	};
	branch * root;//���[�g
	////////////////////////////////////////////////////////////////////////////////
	//�R���X�g���N�^
	btree() : root( NULL )
	{
	}
	btree( const btree & index ) : root( NULL )
	{ *this = index; }
	////////////////////////////////////////////////////////////////////////////////
	//�f�X�g���N�^
	virtual ~btree()
	{
		release();
	}
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
	//�J��
	void release()
	{
		destroy( root );
		size = 0;
	}
private:
	////////////////////////////////////////////////////////////////////////////////
	//�m�[�h�̔j��
	virtual void destroy( node * & n )
	{
		delete n;
		n = NULL;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�u�����`�̔j��
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
	//�m�[�h�̌���
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
	//�߂�l�͑}�����s��ꂽ���ǂ�����\���B�s���Ȃ��Ƃ��ɂ̓m�[�h�̍폜���s��
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
	//�폜
	void pop( node * n )
	{
		if ( root ) root->pop( n );
		if ( root && ! root->size ) {//root�̒��g���Ȃ��Ȃ�����
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
		if ( root && ! root->size ) {//root�̒��g���Ȃ��Ȃ�����
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
	//�ꏊ
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
	//����v�f
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
