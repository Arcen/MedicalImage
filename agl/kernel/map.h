////////////////////////////////////////////////////////////////////////////////
// Option

template<class Key,class Value,class SearchKey = Key>
class map : public btree<Key,Value,SearchKey>
{
public:
	typedef btree<Key,Value,SearchKey> parent;
	virtual ~map()
	{
	}
	bool exist( SearchKey key )
	{
		return search( key ) ? true : false;
	}
	Value & operator[]( SearchKey key )
	{
		node * n = search( key );
		if ( ! n ) {
			n = new node( Key( key ), Value() );
			push( n );
		}
		return n->value;
	}
	bool get( SearchKey key, Value & value )
	{
		node * n = search( key );
		if ( ! n ) return false;
		value = n->value;
		return true;
	}
	bool set( SearchKey key, const Value & value )
	{
		node * n = search( key );
		if ( ! n ) return false;
		n->value = value;
		return true;
	}
	bool kill( SearchKey key )
	{
		node * n = search( key );
		if ( ! n ) return false;
		pop( n );
		return true;
	}
	class iterator
	{
	public:
		parent::iterator it;
		iterator( const map & _base ) : it( ( parent ) _base ){}
		void operator++(){ ++it; }
		operator node * const() const { return ( node * ) it; }
		////////////////////////////////////////////////////////////////////////////////
		//Œ^•ÏŠ·
		const Value * operator->() const { return & it(); }
		Value * operator->() { return & it(); }
		const Value & operator*() const { return it(); }
		Value & operator*() { return it(); }
		const Value & operator()() const { return it(); }
		Value & operator()() { return it(); }
	};
};
