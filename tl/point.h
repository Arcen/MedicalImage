////////////////////////////////////////////////////////////////////////////////
// Point
template<class T>
class point2
{
public:
	T x, y;
	point2( T _x = 0, T _y = 0 ) : x( _x ), y( _y ) {}
	point2( const point2 & p ) : x( p.x ), y( p.y ) {}
	point2 & operator=( const point2 & p ) { x = p.x; y = p.y; return *this; }
	bool operator==( const point2 & p ) const { return ( x == p.x && y == p.y ); }
	bool operator!=( const point2 & p ) const { return ! ( *this == p ); }
	point2 operator-() const { return point2( - x, - y ); }
	point2 operator+( const point2 & p ) const { return point2( x + p.x, y + p.y ); }
	point2 operator-( const point2 & p ) const { return point2( x - p.x, y - p.y ); }
	point2 operator*( const point2 & p ) const { return point2( x * p.x, y * p.y ); }
	point2 operator/( const point2 & p ) const { return point2( x / p.x, y / p.y ); }
	point2 & operator+=( const point2 & p ) { x += p.x; y += p.y; return *this; }
	point2 & operator-=( const point2 & p ) { x -= p.x; y -= p.y; return *this; }
	point2 & operator*=( const point2 & p ) { x *= p.x; y *= p.y; return *this; }
	point2 & operator/=( const point2 & p ) { x /= p.x; y /= p.y; return *this; }
	point2 operator+( const T & p ) const { return point2( x + p, y + p ); }
	point2 operator-( const T & p ) const { return point2( x - p, y - p ); }
	point2 operator*( const T & p ) const { return point2( x * p, y * p ); }
	point2 operator/( const T & p ) const { return point2( x / p, y / p ); }
	point2 & operator+=( const T & p ) { x += p; y += p; return *this; }
	point2 & operator-=( const T & p ) { x -= p; y -= p; return *this; }
	point2 & operator*=( const T & p ) { x *= p; y *= p; return *this; }
	point2 & operator/=( const T & p ) { x /= p; y /= p; return *this; }
};

template<class T>
class point3
{
public:
	T x, y, z;
	point3( T _x = 0, T _y = 0, T _z = 0 ) : x( _x ), y( _y ), z( _z ) {}
	point3( const point3 & p ) : x( p.x ), y( p.y ), z( p.z ) {}
	point3 & operator=( const point3 & p ) { x = p.x; y = p.y; z = p.z; return *this; }
	bool operator==( const point3 & p ) const { return ( x == p.x && y == p.y && z == p.z ); }
	bool operator!=( const point3 & p ) const { return ! ( *this == p ); }
	point3 operator-() const { return point3( - x, - y, - z ); }
	point3 operator+( const point3 & p ) const { return point3( x + p.x, y + p.y, z + p.z ); }
	point3 operator-( const point3 & p ) const { return point3( x - p.x, y - p.y, z - p.z ); }
	point3 operator*( const point3 & p ) const { return point3( x * p.x, y * p.y, z * p.z ); }
	point3 operator/( const point3 & p ) const { return point3( x / p.x, y / p.y, z / p.z ); }
	point3 & operator+=( const point3 & p ) { x += p.x; y += p.y; z += p.z; return *this; }
	point3 & operator-=( const point3 & p ) { x -= p.x; y -= p.y; z -= p.z; return *this; }
	point3 & operator*=( const point3 & p ) { x *= p.x; y *= p.y; z *= p.z; return *this; }
	point3 & operator/=( const point3 & p ) { x /= p.x; y /= p.y; z /= p.z; return *this; }
	point3 operator+( const T & p ) const { return point3( x + p, y + p, z + p ); }
	point3 operator-( const T & p ) const { return point3( x - p, y - p, z - p ); }
	point3 operator*( const T & p ) const { return point3( x * p, y * p, z * p ); }
	point3 operator/( const T & p ) const { return point3( x / p, y / p, z / p ); }
	point3 & operator+=( const T & p ) { x += p; y += p; z += p; return *this; }
	point3 & operator-=( const T & p ) { x -= p; y -= p; z -= p; return *this; }
	point3 & operator*=( const T & p ) { x *= p; y *= p; z *= p; return *this; }
	point3 & operator/=( const T & p ) { x /= p; y /= p; z /= p; return *this; }
};
