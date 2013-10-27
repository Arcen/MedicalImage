////////////////////////////////////////////////////////////////////////////////
// smart
// ポインタを保持して、deleteの責任を負う。ただし参照カウンタを用いる
template<class T>
class smart
{
	mutable T * pointer;//対象のポインタ。const状態でも取り外せるようにmutable。
	mutable list<smart*> references;//参照されているsmartのポインタリスト
public:
	////////////////////////////////////////////////////////////////////////////////
	//コンストラクタ・デストラクタ
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
	//略奪（delete無しの取り外し）
	T * loot() const 
	{
		T * result = const_cast<T*>( pointer );
		finalize();
		return result;
	}
	////////////////////////////////////////////////////////////////////////////////
	//型変換
	T & operator()() { assert( pointer ); return *pointer; }
	const T & operator()() const { assert( pointer ); return *pointer; }
	T & operator*() { return ( * this )(); } const T & operator*() const { return ( * this )(); }
	T * operator->() { return & ( * this )(); } const T * operator->() const { return & ( * this )(); }
	////////////////////////////////////////////////////////////////////////////////
	//比較
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
	//存在
	operator bool () const { return pointer ? true : false; }
};
