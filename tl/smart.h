////////////////////////////////////////////////////////////////////////////////
// smart
// �|�C���^��ێ����āAdelete�̐ӔC�𕉂��B�������Q�ƃJ�E���^��p����
template<class T>
class smart
{
	mutable T * pointer;//�Ώۂ̃|�C���^�Bconst��Ԃł����O����悤��mutable�B
	mutable list<smart*> references;//�Q�Ƃ���Ă���smart�̃|�C���^���X�g
public:
	////////////////////////////////////////////////////////////////////////////////
	//�R���X�g���N�^�E�f�X�g���N�^
	smart() : pointer( NULL )
	{
	}
	smart( const smart & src ) : pointer( NULL )
	{
		*this = src;
	}
	smart( const T * src ) : pointer( NULL )
	{
		*this = src;
	}
	~smart()
	{
		finalize();
	}
	void initialize( const T * src )
	{
		finalize();
		if ( ! src ) return;
		pointer = const_cast<T*>( src );
		references.push_back( this );
	}
	void initialize( const smart & src )
	{
		finalize();
		if ( ! src.pointer ) return;
		pointer = const_cast<T*>( src.pointer );
		references = src.references;
		for ( list<smart*>::iterator it( references ); it; ++it ) {
			it()->references.push_back( this );
		}
		references.push_back( this );
	}
	void finalize()
	{
		if ( pointer ) {
			for ( list<smart*>::iterator it( references ); it; ++it ) {
				if ( it() != this ) it()->references.pop( this );
			}
			references.pop( this );
			if ( references.empty() ) {
				delete pointer;
				pointer = NULL;
			}
			references.release();
		}
	}
	smart & operator=( const smart & src )
	{
		initialize( src );
		return *this;
	}
	smart & operator=( const T * src )
	{
		initialize( src );
		return *this;
	}
	////////////////////////////////////////////////////////////////////////////////
	//���D�idelete�����̎��O���j
	T * loot() const 
	{
		T * result = const_cast<T*>( pointer );
		finalize();
		return result;
	}
	////////////////////////////////////////////////////////////////////////////////
	//�^�ϊ�
	T & operator()() { assert( pointer ); return *pointer; }
	const T & operator()() const { assert( pointer ); return *pointer; }
	T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
	T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
	////////////////////////////////////////////////////////////////////////////////
	//��r
	bool operator==( const smart & src ) const { return pointer == src.pointer; }
	bool operator!=( const smart & src ) const { return pointer != src.pointer; }
	bool operator<( const smart & src ) const { return pointer < src.pointer; }
	bool operator>( const smart & src ) const { return pointer > src.pointer; }
	bool operator<=( const smart & src ) const { return pointer <= src.pointer; }
	bool operator>=( const smart & src ) const { return pointer >= src.pointer; }
	bool operator==( const T * src ) const { return pointer == src; }
	bool operator!=( const T * src ) const { return pointer != src; }
	bool operator<( const T * src ) const { return pointer < src; }
	bool operator>( const T * src ) const { return pointer > src; }
	bool operator<=( const T * src ) const { return pointer <= src; }
	bool operator>=( const T * src ) const { return pointer >= src; }
	bool operator==( const T & src ) const { return pointer == & src; }
	bool operator!=( const T & src ) const { return pointer != & src; }
	bool operator<( const T & src ) const { return pointer < & src; }
	bool operator>( const T & src ) const { return pointer > & src; }
	bool operator<=( const T & src ) const { return pointer <= & src; }
	bool operator>=( const T & src ) const { return pointer >= & src; }
	////////////////////////////////////////////////////////////////////////////////
	//����
	operator bool () const { return pointer ? true : false; }
};
