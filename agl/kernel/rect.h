class rect
{
public:
	int left, top, right, bottom;
	rect() : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) {}
	rect( int _left, int _top, int _right, int _bottom ) : left( _left ), top( _top ), right( _right ), bottom( _bottom ) {}
	rect & operator=( const rect & r ) { left = r.left; top = r.top; right = r.right; bottom = r.bottom; return *this; }
	bool operator==( const rect & r ) const { return( left == r.left && right == r.right && top == r.top && bottom == r.bottom ); }
	bool operator!=( const rect & r ) const { return ! ( *this == r ); }
	int width() const { return right - left; }
	int height() const { return bottom - top; }
	int area() const { return width() * height(); }
	void size( int w, int h ){ right = left + w; bottom = top + h; }
	void single(){ right = left + 1; bottom = top + 1; }
	void expand( int band ){ left -= band; right += band; top -= band; bottom += band; }
	void offset( int x, int y ) { left += x; right += x; top += y; bottom += y; }
	static rect pixel( int x, int y ){ rect r; r.left = x; r.top = y; r.single(); return r; }
};

class rect3
{
public:
	int left, top, front, right, bottom, back;
	rect3() : left( 0 ), top( 0 ), front( 0 ), right( 0 ), bottom( 0 ), back( 0 ) {}
	rect3( int _left, int _top, int _front, int _right, int _bottom, int _back ) : left( _left ), top( _top ), front( _front ), right( _right ), bottom( _bottom ), back( _back ) {}
	rect3 & operator=( const rect3 & r ) { left = r.left; top = r.front; top = r.front; right = r.right; bottom = r.bottom; back = r.back; return *this; }
	bool operator==( const rect3 & r ) const { return( left == r.left && right == r.right && top == r.top && bottom == r.bottom && front == r.front && back == r.back ); }
	bool operator!=( const rect3 & r ) const { return ! ( *this == r ); }
	int width() const { return right - left; }
	int height() const { return bottom - top; }
	int depth() const { return back - front; }
	int volume() const { return width() * height() * depth(); }
	void size( int w, int h, int d ){ right = left + w; bottom = top + h; back = front + d; }
	void single(){ right = left + 1; bottom = top + 1; back = front + 1; }
	void expand( int band ){ left -= band; right += band; top -= band; bottom += band; front -= band; back += band; }
	void offset( int x, int y, int z ) { left += x; right += x; top += y; bottom += y; front += z; back += z; }
	static rect3 voxel( int x, int y, int z ){ rect3 r; r.left = x; r.top = y; r.front = z; r.single(); return r; }
};
