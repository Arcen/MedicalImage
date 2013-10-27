////////////////////////////////////////////////////////////////////////////////
// Map
// キーからデータへのマッピングを行うクラス
template<class Key,class T,class SearchKey = Key,class Container = btree<Key,T,SearchKey> >
class map : public Container
{
public:
	typedef typename Container::iterator iterator;
	virtual ~map(){}
	bool exist( SearchKey key )
	{
		return search( key ) ? true : false;
	}
	T & operator[]( SearchKey key )
	{
		node * n = search( key );
		if ( ! n ) {
			n = new node( Key( key ), T() );
			push( n );
		}
		return n->data;
	}
	bool get( SearchKey key, T & data )
	{
		node * n = search( key );
		if ( ! n ) return false;
		data = n->data;
		return true;
	}
	bool set( SearchKey key, const T & data )
	{
		node * n = search( key );
		if ( ! n ) return false;
		n->data = data;
		return true;
	}
	bool pop( SearchKey key )
	{
		node * n = search( key );
		if ( ! n ) return false;
		Container::pop( n );
		return true;
	}
};
