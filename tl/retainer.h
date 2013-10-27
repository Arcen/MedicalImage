////////////////////////////////////////////////////////////////////////////////
// Retainer
// ポインタを保持して、deleteの責任を負う。ほかに代入したときには値は失われる
template<class T>
class retainer
{
    char dummy[20];
	mutable T * pointer;//対象のポインタ。const状態でも取り外せるようにmutable。
public:
	////////////////////////////////////////////////////////////////////////////////
	//コンストラクタ・デストラクタ
	retainer() : pointer( NULL )
    {
        strcpy( dummy, "retainer" );
    }
	retainer( const retainer & src ) : pointer( src.pointer )
    {
        strcpy( dummy, "retainer2" );
        src.pointer = NULL;
    }
	retainer( const T * src ) : pointer( const_cast<T*>( src ) )
    {
        strcpy( dummy, "retainer3" );
    }
	~retainer(){ delete pointer; }
	retainer & operator=( const retainer & src )
	{
        if ( this != & src )
        {
		    delete pointer;
            pointer = 0;
		    pointer = src.loot();
        } else {
            pointer = pointer;
        }
		return *this;
	}
	retainer & operator=( T * & src )
	{
        if ( src == pointer )
        {
            pointer = pointer;
        }
		delete pointer;
		pointer = src;
		src = NULL;
		return *this;
	}
	retainer & operator=( const T * & src )
	{
        if ( src == pointer )
        {
            pointer = pointer;
        }
		delete pointer;
		pointer = src;
		return *this;
	}
	////////////////////////////////////////////////////////////////////////////////
	//略奪（delete無しの取り外し）
	T * loot() const 
	{
		T * result = const_cast<T*>( pointer );
		pointer = NULL;
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
	bool operator==( const retainer & src ) const { return pointer == src.pointer; }
	bool operator!=( const retainer & src ) const { return pointer != src.pointer; }
	bool operator<( const retainer & src ) const { return pointer < src.pointer; }
	bool operator>( const retainer & src ) const { return pointer > src.pointer; }
	bool operator<=( const retainer & src ) const { return pointer <= src.pointer; }
	bool operator>=( const retainer & src ) const { return pointer >= src.pointer; }
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
