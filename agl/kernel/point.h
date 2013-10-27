class point
{
public:
	int x, y;
	point( int _x = 0, int _y = 0 ) : x( _x ), y( _y ) {}
	point( const point & p ) : x( p.x ), y( p.y ) {}
	point & operator=( const point & p ) { x = p.x; y = p.y; return *this; }
	bool operator==( const point & p ) const { return ( x == p.x && y == p.y ); }
	bool operator!=( const point & p ) const { return ! ( *this == p ); }
	point operator+( const point & p ) const { return point( x + p.x, y + p.y ); }
	point operator-( const point & p ) const { return point( x - p.x, y - p.y ); }
	point operator*( const point & p ) const { return point( x * p.x, y * p.y ); }
	point operator/( const point & p ) const { return point( x / p.x, y / p.y ); }
	point & operator+=( const point & p ) { x += p.x; y += p.y; return *this; }
	point & operator-=( const point & p ) { x -= p.x; y -= p.y; return *this; }
	point & operator*=( const point & p ) { x *= p.x; y *= p.y; return *this; }
	point & operator/=( const point & p ) { x /= p.x; y /= p.y; return *this; }
	point operator+( const int & p ) const { return point( x + p, y + p ); }
	point operator-( const int & p ) const { return point( x - p, y - p ); }
	point operator*( const int & p ) const { return point( x * p, y * p ); }
	point operator/( const int & p ) const { return point( x / p, y / p ); }
	point & operator+=( const int & p ) { x += p; y += p; return *this; }
	point & operator-=( const int & p ) { x -= p; y -= p; return *this; }
	point & operator*=( const int & p ) { x *= p; y *= p; return *this; }
	point & operator/=( const int & p ) { x /= p; y /= p; return *this; }
};

class point3
{
public:
	int x, y, z;
	point3( int _x = 0, int _y = 0, int _z = 0 ) : x( _x ), y( _y ), z( _z ) {}
	point3( const point3 & p ) : x( p.x ), y( p.y ), z( p.z ) {}
	point3 & operator=( const point3 & p ) { x = p.x; y = p.y; z = p.z; return *this; }
	bool operator==( const point3 & p ) const { return ( x == p.x && y == p.y && z == p.z ); }
	bool operator!=( const point3 & p ) const { return ! ( *this == p ); }
	point3 operator+( const point3 & p ) const { return point3( x + p.x, y + p.y, z + p.z ); }
	point3 operator-( const point3 & p ) const { return point3( x - p.x, y - p.y, z - p.z ); }
	point3 operator*( const point3 & p ) const { return point3( x * p.x, y * p.y, z * p.z ); }
	point3 operator/( const point3 & p ) const { return point3( x / p.x, y / p.y, z / p.z ); }
	point3 & operator+=( const point3 & p ) { x += p.x; y += p.y; z += p.z; return *this; }
	point3 & operator-=( const point3 & p ) { x -= p.x; y -= p.y; z -= p.z; return *this; }
	point3 & operator*=( const point3 & p ) { x *= p.x; y *= p.y; z *= p.z; return *this; }
	point3 & operator/=( const point3 & p ) { x /= p.x; y /= p.y; z /= p.z; return *this; }
	point3 operator+( const int & p ) const { return point3( x + p, y + p, z + p ); }
	point3 operator-( const int & p ) const { return point3( x - p, y - p, z - p ); }
	point3 operator*( const int & p ) const { return point3( x * p, y * p, z * p ); }
	point3 operator/( const int & p ) const { return point3( x / p, y / p, z / p ); }
	point3 & operator+=( const int & p ) { x += p; y += p; z += p; return *this; }
	point3 & operator-=( const int & p ) { x -= p; y -= p; z -= p; return *this; }
	point3 & operator*=( const int & p ) { x *= p; y *= p; z *= p; return *this; }
	point3 & operator/=( const int & p ) { x /= p; y /= p; z /= p; return *this; }
};
