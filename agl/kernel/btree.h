////////////////////////////////////////////////////////////////////////////////
// Balanced Tree

// balanced tree��branch��half�`half*2�̗L����node��size�����A
// size+1�̃u�����`��S�Ď��}���A�S�Ď����Ȃ��t�ł���B
// root�͗�O��0�`half*2�̃m�[�h�������Ƃ��ł���
// �܂��A�S�Ă̊K�w�̐[���͓����ł���B

// SearchKey�͈ȉ��𖞂����K�v������B�������̂��ߎ��������D
// SearchKey <= ( SearchKey ) Key
// Key <= new Key( SearchKey )
template<class Key, class Value, class SearchKey = Key, int half = 2>
class btree
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
		Value value;//�l
		node( const node & src ) : key( src.key ), value( src.value ){}
		node( const Key & _key, Value _value ) : key ( _key ), value( _value ){}
		node & operator=( node & src ){ key = src.key; value = src.value; return *this; }
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
			for ( i = size + 1; ib < i; i-- ) b[i] = b[i-1];
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
			for ( i = ib; i < size; i++ ) b[i] = b[i+1];
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
		// kill
		//�߂�l�̓T�C�Y���K���艺��������Ƃ�\��
		// _n�͎�菜���ׂ��t�̈ʒu�̃m�[�h�B�폜����m�[�h�ł͂Ȃ�
		bool kill( node * _n )
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
			if ( b[index] && b[index]->kill( _n ) ) {
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
			for ( i = index; i < num_node; i++ ) _n[i+1] = n[i];

			for ( i = 0; i < index+1; i++ ) _b[i] = b[i];
			_b[index+1] = tb;
			for ( i = index+1; i < num_branch; i++ ) _b[i+1] = b[i];
			//����
			//half == 2�̂Ƃ����������
			//  n0  n1  n2  n3  n4  ��  n0  n1  ��n2��  n3  n4  �ɂȂ�
			//b0  b1  b2  b3  b4  b5  b0  b1  b2      b3  b4  b5
			//�O��
			size = half;
			for ( i = 0; i < half; i++ ) n[i] = _n[i];
			for ( i = 0; i < half + 1; i++ ) b[i] = _b[i];
			for ( i = half; i < num_node; i++ ) n[i] = 0;
			for ( i = half + 1; i < num_branch; i++ ) b[i] = 0;
			//����
			tn = _n[half];
			//�㔼
			tb = new branch();
			tb->size = half;
			for ( i = 0; i < half; i++ ) tb->n[i] = _n[i+half+1];
			for ( i = 0; i < half + 1; i++ ) tb->b[i] = _b[i+half+1];
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
				stack.add( b );
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
					stack.deleteLast();
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
		iterator( const btree & _base ) : b( _base.root ), index( 0 )
		{
			goDown();
		}
		////////////////////////////////////////////////////////////////////////////////
		//����
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator & operator++()
		{
			if ( b ) {
				index++;
				goDown();
				goUp();
			}
			return *this;
		}
	private:
		////////////////////////////////////////////////////////////////////////////////
		//�O�� ���삳���Ȃ�
		iterator operator--(int);
		iterator & operator--();
	public:
		////////////////////////////////////////////////////////////////////////////////
		//��������p�^�ϊ�
		operator node * ()
		{
			if ( b && index < b->size ) {
				return b->n[index];
			}
			return 0;
		}
		operator const node * () const 
		{
			if ( b && index < b->size ) {
				return b->n[index];
			}
			return 0;
		}
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		const Value * operator->() const { assert( b && index < b->size ); return & ( b->n[index]->value ); }
		Value * operator->() { assert( b && index < b->size ); return & ( b->n[index]->value ); }
		const Value & operator*() const { assert( b && index < b->size ); return b->n[index]->value; }
		Value & operator*() { assert( b && index < b->size ); return b->n[index]->value; }
		const Value & operator()() const { assert( b && index < b->size ); return b->n[index]->value; }
		Value & operator()() { assert( b && index < b->size ); return b->n[index]->value; }
	};
	branch * root;//���[�g
	int size;//�S�m�[�h��
	////////////////////////////////////////////////////////////////////////////////
	//�R���X�g���N�^
	btree() : root( 0 ), size( 0 )
	{
	}
	////////////////////////////////////////////////////////////////////////////////
	//�f�X�g���N�^
	virtual ~btree()
	{
		release();
	}
	////////////////////////////////////////////////////////////////////////////////
	//������
	void initialize() { release(); }
	////////////////////////////////////////////////////////////////////////////////
	//�����m�ې�
	int capacity() const { return size; }
	////////////////////////////////////////////////////////////////////////////////
	//��
	bool empty() const { return size ? false : true; }
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
		n = 0;
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
		b = 0;
	}
public:
	////////////////////////////////////////////////////////////////////////////////
	//�m�[�h�̌���
	node * search( SearchKey sk )
	{
		branch * b = root;

		while ( b ) {
			int index = 0;
			node * found = b->search( sk, index );
			if ( found ) return found;
			b = b->b[index];
		}
		return 0;
	}
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
	////////////////////////////////////////////////////////////////////////////////
	//�߂�l�͑}�����s��ꂽ���ǂ�����\���B�s���Ȃ��Ƃ��ɂ̓m�[�h�̍폜���s��
	bool insert( const Key & k, Value & v )
	{
		return insert( new node( k, v ) );
	}
	bool insert( node * n )
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
	void kill( node * n )
	{
		if ( root ) root->kill( n );
		if ( root && ! root->size ) {//root�̒��g���Ȃ��Ȃ�����
			branch * b = root;
			root = root->b[0];
			delete b;
		}
		destroy( n );
		size--;
	}
	////////////////////////////////////////////////////////////////////////////////
	//����v�f
	Value & index( int i )
	{
		ASSERT( between( 0, i, size - 1 ) && "btree bad access" );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) break;
			i--;
		}
		return *it;
	}
	const Value & index( int i ) const
	{
		ASSERT( between( 0, i, size - 1 ) && "btree bad access" );
		for ( iterator it( *this ); it; ++it ) {
			if ( i == 0 ) break;
			i--;
		}
		return *it;
	}
	Value & first() { return index( 0 ); } const Value & first() const { return index( 0 ); }
	Value & second() { return index( 1 ); } const Value & second() const { return index( 1 ); }
	Value & third() { return index( 2 ); } const Value & third() const { return index( 2 ); }
	Value & last() { return index( size - 1 ); } const Value & last() const { return index( size - 1 ); }
	Value & prelast() { return index( size - 2 ); } const Value & prelast() const { return index( size - 2 ); }
	btree & operator=( const btree & src )
	{
		if ( this == &src ) return *this;
		release();
		for ( iterator it( src ); it; ++it ) {
			node * n = it;
			insert( n->key, n->value );
		}
		return *this;
	}
	OperatorSet( btree, Value );
};
////////////////////////////////////////////////////////////////////////////////
//Value�Ƀ|�C���^�^���g���Ē��g�̔j���ۏ؂���Ƃ��Ɏg���N���X
template<class Key, class Value, class SearchKey = Key, int half = 2>
class btreeRetain : public btree<Key, Value*, SearchKey, half>
{
private:
	virtual void destroy( node * & n )
	{
		delete n->value;
		delete n;
		n = 0;
	}
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
		b = 0;
	}
public:
	virtual ~btreeRetain()
	{
		release();
	}
	btreeRetain & operator=( const btreeRetain & src )
	{
		if ( this == &src ) return *this;
		release();
		for ( iterator it( src ); it; ++it ) {
			node * n = it;
			insert( n->key, n->value );
		}
		return *this;
	}
	node * loot_front()
	{
		return loot( get_front() );
	}
	node * loot_back()
	{
		return loot( get_back() );
	}
	node * loot( node * n )
	{
		if ( ! n ) return NULL;
		node * result = new node( *n );
		n->value = NULL;
		kill( n );
		return result;
	}
	btree & operator=( const btree & src )
	{
		if ( this == &src ) return *this;
		release();
		while ( src.size ) {
			node * n = src.loot_front();
			insert( n );
		}
		return *this;
	}
	OperatorSet( btreeRetain, Value* );
};
