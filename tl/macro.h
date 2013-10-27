////////////////////////////////////////////////////////////////////////////////
// Macro

typedef char int8;
typedef short int16;
typedef long int32;
typedef _int64 int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned _int64 uint64;

//#define for if(0){}else for

template<class T> bool between( const T & minimum, const T & v, const T & maximum )
{ return ( minimum <= v ) && ( v <= maximum ) && ( minimum <= maximum ); }

template<class T> const T & clamp( const T & minimum, const T & v, const T & maximum )
{ return v < minimum ? minimum : ( maximum < v ? maximum : v ); }

template<class T> void swap( T & left, T & right )
{ T wk( left ); left = right; right = wk; }

template<class T> const T & minimum( const T & v1, const T & v2 )
{ return v1 < v2 ? v1 : v2; }

template<class T> const T & maximum( const T & v1, const T & v2 )
{ return v1 > v2 ? v1 : v2; }

template<class T, class U> const T round( const U & v )
{ return T( v + 0.5 ); }

template<class T> const T sign( const T & v )
{ return v == T( 0 ) ? T( 0 ) : v < T( 0 ) ? T( -1 ) : T( 1 ); }

template<class T, class U = T>
class pair
{
public:
	T left;
	U right;
	pair(){}
	pair( const T & _left, const U & _right ) : left( _left ), right( _right ) {}
	pair( const pair & _pair ) : left( _pair.left ), right( _pair.right ) {}
	pair & operator=( const pair & _pair ){ left = _pair.left; right = _pair.right; return *this; }
	bool operator==( const pair & _pair ){ return left == _pair.left && right == _pair.right; }
	bool operator!=( const pair & _pair ){ return left != _pair.left || right != _pair.right; }
};

