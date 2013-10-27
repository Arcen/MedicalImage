////////////////////////////////////////////////////////////////////////////////
// Rectangle

template<class T>
class rectangle2
{
public:
	T left, top, right, bottom;
	rectangle2() : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) {}
	rectangle2( T _left, T _top, T _right, T _bottom ) : left( _left ), top( _top ), right( _right ), bottom( _bottom ) {}
	rectangle2 & operator=( const rectangle2 & r ) { left = r.left; top = r.top; right = r.right; bottom = r.bottom; return *this; }
	bool operator==( const rectangle2 & r ) const { return( left == r.left && right == r.right && top == r.top && bottom == r.bottom ); }
	bool operator!=( const rectangle2 & r ) const { return ! ( *this == r ); }
	void initialize() { left = top = right = bottom = 0; }
	T width() const { return right - left; }
	T height() const { return bottom - top; }
	double area() const { return double( width() ) * height(); }
	void first( T l, T t ){ left = l; top = t; }
	void second( T w, T h ){ right = left + w; bottom = top + h; }
	void single(){ right = left + 1; bottom = top + 1; }
	void expand( T band ){ left -= band; right += band; top -= band; bottom += band; }
	void offset( T x, T y ) { left += x; right += x; top += y; bottom += y; }
	bool in( T x, T y ) const { return between<T>( left, x, right - 1 ) && between<T>( top, y, bottom - 1 ); }
	static rectangle2 pixel( T x, T y ){ rectangle2 r; r.left = x; r.top = y; r.single(); return r; }
	static rectangle2 size( T w, T h ){ return rectangle2( 0, 0, w, h ); }
	const rectangle2 operator|( const rectangle2 & src )
	{
		rectangle2 r;
		r.left = minimum( left, src.left );
		r.right = maximum( right, src.right );
		r.top = minimum( top, src.top );
		r.bottom = maximum( bottom, src.bottom );
		return r;
	}
	const rectangle2 operator&( const rectangle2 & src )
	{
		rectangle2 r;
		if ( src.right <= left || right <= src.left ||
			src.bottom <= top || bottom <= src.top ) return r;
		r.left = maximum( left, src.left );
		r.right = minimum( right, src.right );
		r.top = maximum( top, src.top );
		r.bottom = minimum( bottom, src.bottom );
		if ( r.right <= r.left || r.bottom <= r.top ) return rectangle2();
		return r;
	}
};

template<class T>
class rectangle3
{
public:
	T left, top, front, right, bottom, back;
	rectangle3() : left( 0 ), top( 0 ), front( 0 ), right( 0 ), bottom( 0 ), back( 0 ) {}
	rectangle3( T _left, T _top, T _front, T _right, T _bottom, T _back ) : left( _left ), top( _top ), front( _front ), right( _right ), bottom( _bottom ), back( _back ) {}
	rectangle3 & operator=( const rectangle3 & r ) { left = r.left; top = r.front; top = r.front; right = r.right; bottom = r.bottom; back = r.back; return *this; }
	bool operator==( const rectangle3 & r ) const { return( left == r.left && right == r.right && top == r.top && bottom == r.bottom && front == r.front && back == r.back ); }
	bool operator!=( const rectangle3 & r ) const { return ! ( *this == r ); }
	void initialize() { left = top = right = bottom = front = back = 0; }
	T width() const { return right - left; }
	T height() const { return bottom - top; }
	T depth() const { return back - front; }
	double volume() const { return double( width() ) * height() * depth(); }
	bool in( T x, T y, T z ) const { return between<T>( left, x, right - 1 ) && between<T>( top, y, bottom - 1 ) && between( front, z, back - 1 ); }
	void first( T l, T t, T f ){ left = l; top = t; front = f; }
	void second( T w, T h, T d ){ right = left + w; bottom = top + h; back = front + d; }
	void single(){ right = left + 1; bottom = top + 1; back = front + 1; }
	void expand( T band ){ left -= band; right += band; top -= band; bottom += band; front -= band; back += band; }
	void offset( T x, T y, T z ) { left += x; right += x; top += y; bottom += y; front += z; back += z; }
	static rectangle3 voxel( T x, T y, T z ){ rectangle3 r; r.left = x; r.top = y; r.front = z; r.single(); return r; }
	static rectangle3 size( T w, T h, T d ){ return rectangle3( 0, 0, 0, w, h, d ); }
};
