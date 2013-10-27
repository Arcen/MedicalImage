////////////////////////////////////////////////////////////////////////////////
// Vector 3

class vector4;
class matrix33;
class matrix44;

class vector3
{
	struct instance { decimal x, y, z; };
public:
	const static vector3 identity;
	const static vector3 zero, one;
	const static vector3 black, white, gray, red, green, blue, cyan, magenta, yellow;
	union {
		decimal array[3];
		struct { decimal x, y, z; };
	};
	vector3() {}
	vector3( const vector2 & v ) { x = v.x; y = v.y; z = 0; }
	vector3( const vector3 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); }
//	vector3( const vector4 & v );
	vector3( const decimal & _x, const decimal & _y = 0, const decimal & _z = 0 ) { x = _x; y = _y; z = _z; }
	vector3( const decimal * v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( v ); }

	vector3 & operator=( const vector3 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); return *this; }

	const vector3 operator-() const { return vector3( -x, -y, -z ); }
	const vector3 operator+( const vector3 & v ) const { return vector3( x+v.x, y+v.y, z+v.z ); }
	const vector3 operator-( const vector3 & v ) const { return vector3( x-v.x, y-v.y, z-v.z ); }
	const vector3 operator*( const vector3 & v ) const { return vector3( x*v.x, y*v.y, z*v.z ); }
	const vector3 operator/( const vector3 & v ) const { return vector3( zerodivide<decimal>( x, v.x ), zerodivide<decimal>( y, v.y ), zerodivide<decimal>( z, v.z ) ); }
	void operator+=( const vector3 & v ) { x+=v.x; y+=v.y; z+=v.z; }
	void operator-=( const vector3 & v ) { x-=v.x; y-=v.y; z-=v.z; }
	void operator*=( const vector3 & v ) { x*=v.x; y*=v.y; z*=v.z; }
	void operator/=( const vector3 & v ) { x = zerodivide<decimal>( x, v.x ); y = zerodivide<decimal>( y, v.y ); z = zerodivide<decimal>( z, v.z ); }
	const vector3 operator+( const decimal & v ) const { return vector3( x+v, y+v, z+v ); }
	const vector3 operator-( const decimal & v ) const { return vector3( x-v, y-v, z-v ); }
	const vector3 operator*( const decimal & v ) const { return vector3( x*v, y*v, z*v ); }
	const vector3 operator/( const decimal & v ) const { return vector3( zerodivide<decimal>( x, v ), zerodivide<decimal>( y, v ), zerodivide<decimal>( z, v ) ); }
	void operator+=( const decimal & v ) { x+=v; y+=v; z+=v; }
	void operator-=( const decimal & v ) { x-=v; y-=v; z-=v; }
	void operator*=( const decimal & v ) { x*=v; y*=v; z*=v; }
	void operator/=( const decimal & v ) { x = zerodivide<decimal>( x, v ); y = zerodivide<decimal>( y, v ); z = zerodivide<decimal>( z, v ); }
	bool operator==( const vector3 & v ) const { return ( x == v.x && y == v.y && z == v.z ); }
	bool operator!=( const vector3 & v ) const { return ( x != v.x || y != v.y || z != v.z ); }
	operator decimal * () { return array; }
	operator const decimal * () const { return array; }
	const vector3 operator*( const matrix33 & m ) const;
	const vector3 operator*( const matrix44 & m ) const;
	static decimal dot2( const vector3 & l, const vector3 & r ) { return l.x * r.x + l.y * r.y; }
	static decimal dot3( const vector3 & l, const vector3 & r ) { return l.x * r.x + l.y * r.y + l.z * r.z; }
	static const vector3 cross( const vector3 & l, const vector3 & r ) { return vector3( l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x ); }
	static decimal length2( const vector3 & v ) { return sqrt( dot2( v, v ) ); }
	static decimal length3( const vector3 & v ) { return sqrt( dot3( v, v ) ); }
private:
	//強制的にZの値を変更する
	static const vector3 z0( const vector3 & v ) { return vector3( v.x, v.y, 0 ); }
	static const vector3 z1( const vector3 & v ) { return vector3( v.x, v.y, 1 ); }
public:
	static const vector3 normalize2( const vector3 & v ) { decimal l = length2( v ); if ( l == 1 ) return v; if ( l < epsilon ) return vector3( 1, 0, 0 ); return z0( v / l ); }
	static const vector3 normalize3( const vector3 & v ) { decimal l = length3( v ); if ( l == 1 ) return v; if ( l < epsilon ) return vector3( 1, 0, 0 ); return v / l; }
	void normalize2() { *this = normalize2( *this ); }
	void normalize3() { *this = normalize3( *this ); }
	static const vector3 abs( const vector3 & v ) { return vector3( v.x < 0 ? -v.x : v.x, v.y < 0 ? -v.y : v.y, v.z < 0 ? -v.z : v.z ); }
	static bool same( const vector3 & v ) { return v.x == v.y && v.x == v.z; }
	const vector2 getVector2() const { return vector2( x, y ); }
	static decimal argument2( const vector3 & l, const vector3 & r ) { return acos( dot2( normalize2( l ), normalize2( r ) ) ); }
	static decimal argument3( const vector3 & l, const vector3 & r ) { return acos( dot3( normalize3( l ), normalize3( r ) ) ); }
	//与えられた１次独立なベクトルを補充して正規直交基底ベクトルを作成する
	static bool gramschmidt( ::array<vector3> & v )
	{
		for ( int i = 0; i < v.size; ++i ) {
			v[i].normalize3();
		}
		const vector3 u[3] = { vector3( 1, 0, 0 ), vector3( 0, 1, 0 ), vector3( 0, 0, 1 ) };
		while ( v.size < 3 ) {
			checkMaximum<decimal> mx;
			for ( int i = 0; i < 3; ++i ) {
				checkMinimum<decimal> mn;
				for ( int j = 0; j < v.size; ++j ) {
					mn( length3( u[i] - v[j] ) );
				}
				if ( mn ) mx( mn(), i );
			}
			if ( mx ) {
				v.push_back( u[mx.sub] );
			} else {
				v.push_back( u[0] );
			}
		}
		for ( int i = 1; i < 3; ++i ) {
			vector3 xi = v[i];
			for ( int j = 0; j < i; ++j ) {
				v[i] -= v[j] * dot3( v[j], xi );
			}
			v[i].normalize3();
		}
	}
};
inline const vector3 operator+( const decimal & s, const vector3 & v ) { return v + s; }
inline const vector3 operator-( const decimal & s, const vector3 & v ) { return - v + s; }
inline const vector3 operator*( const decimal & s, const vector3 & v ) { return v * s; }
inline const vector3 operator/( const decimal & s, const vector3 & v ) { return vector3( zerodivide<decimal>( 1, v.x ), zerodivide<decimal>( 1, v.y ), zerodivide<decimal>( 1, v.z ) ) * s; }
#ifdef __GLOBAL__
const vector3 vector3::identity( 0, 0, 0 );
const vector3 vector3::zero( 0, 0, 0 ), vector3::one( 1, 1, 1 );
const vector3 vector3::black( 0, 0, 0 ), vector3::white( 1, 1, 1 ), vector3::gray( 0.5, 0.5, 0.5 ), vector3::red( 1, 0, 0 ), vector3::green( 0, 1, 0 ), vector3::blue( 0, 0, 1 ), vector3::cyan( 0, 1, 1 ), vector3::magenta( 1, 0, 1 ), vector3::yellow( 1, 1, 0 );
#endif
