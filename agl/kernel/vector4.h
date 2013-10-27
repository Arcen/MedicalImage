////////////////////////////////////////////////////////////////////////////////
// Vector 4

class matrix44;

class vector4
{
	struct instance { decimal x, y, z, w; };
public:
	const static vector4 identity;
	const static vector4 zero, one;
	const static vector4 black, white, gray, red, green, blue, cyan, magenta, yellow;
	union {
		decimal array[4];
		struct { decimal x, y, z, w; };
	};
	vector4() {}
	vector4( const vector2 & v ) { x = v.x; y = v.y; z = 0; w = 1; }
	vector4( const vector3 & v ) { x = v.x; y = v.y; z = v.z; w = 1; }
	vector4( const vector4 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); }
	vector4( const decimal & _x, const decimal & _y = 0, const decimal & _z = 0, const decimal & _w = 1 ) { x = _x; y = _y; z = _z; w = _w; }
	vector4( const decimal * v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( v ); }

	vector4 & operator=( const vector4 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); return *this; }

	const vector4 operator-() const { return vector4( -x, -y, -z, -w ); }
	const vector4 operator+( const vector4 & v ) const { return vector4( x+v.x, y+v.y, z+v.z, w+v.w ); }
	const vector4 operator-( const vector4 & v ) const { return vector4( x-v.x, y-v.y, z-v.z, w-v.w ); }
	const vector4 operator*( const vector4 & v ) const { return vector4( x*v.x, y*v.y, z*v.z, w*v.w ); }
	const vector4 operator/( const vector4 & v ) const { return vector4( zerodivide<decimal>( x, v.x ), zerodivide<decimal>( y, v.y ), zerodivide<decimal>( z, v.z ), zerodivide<decimal>( w, v.w ) ); }
	void operator+=( const vector4 & v ) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; }
	void operator-=( const vector4 & v ) { x-=v.x; y-=v.y; z-=v.z; w-=v.w; }
	void operator*=( const vector4 & v ) { x*=v.x; y*=v.y; z*=v.z; w*=v.w; }
	void operator/=( const vector4 & v ) { x = zerodivide<decimal>( x, v.x ); y = zerodivide<decimal>( y, v.y ); z = zerodivide<decimal>( z, v.z ); w = zerodivide<decimal>( w, v.w ); }
	const vector4 operator+( const decimal & v ) const { return vector4( x+v, y+v, z+v, w+v ); }
	const vector4 operator-( const decimal & v ) const { return vector4( x-v, y-v, z-v, w-v ); }
	const vector4 operator*( const decimal & v ) const { return vector4( x*v, y*v, z*v, w*v ); }
	const vector4 operator/( const decimal & v ) const { return vector4( zerodivide<decimal>( x, v ), zerodivide<decimal>( y, v ), zerodivide<decimal>( z, v ), zerodivide<decimal>( w, v ) ); }
	void operator+=( const decimal & v ) { x+=v; y+=v; z+=v; w+=v; }
	void operator-=( const decimal & v ) { x-=v; y-=v; z-=v; w-=v; }
	void operator*=( const decimal & v ) { x*=v; y*=v; z*=v; w*=v; }
	void operator/=( const decimal & v ) { x = zerodivide<decimal>( x, v ); y = zerodivide<decimal>( y, v ); z = zerodivide<decimal>( z, v ); w = zerodivide<decimal>( w, v ); }
	bool operator==( const vector4 & v ) const { return ( x == v.x && y == v.y && z == v.z && w == v.w ); }
	bool operator!=( const vector4 & v ) const { return ( x != v.x || y != v.y || z != v.z || w != v.w ); }
	operator decimal * () { return array; }
	operator const decimal * () const { return array; }
	const vector4 operator*( const matrix44 & m ) const;
	static decimal dot2( const vector4 & l, const vector4 & r ) { return l.x * r.x + l.y * r.y; }
	static decimal dot3( const vector4 & l, const vector4 & r ) { return l.x * r.x + l.y * r.y + l.z * r.z; }
	static decimal dot4( const vector4 & l, const vector4 & r ) { return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w; }
	static const vector4 cross( const vector4 & l, const vector4 & r ) { return vector4( l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x ); }
	static decimal argument3( const vector4 & l, const vector4 & r ) { return acos( dot3( normalize3( l ), normalize3( r ) ) ); }
	static decimal length2( const vector4 & v ) { return sqrt( dot2( v, v ) ); }
	static decimal length3( const vector4 & v ) { return sqrt( dot3( v, v ) ); }
	static decimal length4( const vector4 & v ) { return sqrt( dot4( v, v ) ); }
private:
	//強制的にZ,Wの値を変更する
	static const vector4 zw0( const vector4 & v ) { return vector4( v.x, v.y, 0, 0 ); }
	static const vector4 w0( const vector4 & v ) { return vector4( v.x, v.y, v.z, 0 ); }
public:
	static const vector4 normalize2( const vector4 & v ) { decimal l = length2( v ); if ( l == 1 ) return zw0( v ); if ( l < epsilon ) return vector4( 1, 0, 0, 0 ); return zw0( v / l ); }
	static const vector4 normalize3( const vector4 & v ) { decimal l = length3( v ); if ( l == 1 ) return w0( v ); if ( l < epsilon ) return vector4( 1, 0, 0, 0 ); return w0( v / l ); }
	static const vector4 normalize4( const vector4 & v ) { decimal l = length4( v ); if ( l == 1 ) return v; if ( l < epsilon ) return vector4( 1, 0, 0, 0 ); return v / l; }
	void normalize2() { *this = normalize2( *this ); }
	void normalize3() { *this = normalize3( *this ); }
	void normalize4() { *this = normalize4( *this ); }
	static const vector4 abs( const vector4 & v ) { return vector4( v.x < 0 ? -v.x : v.x, v.y < 0 ? -v.y : v.y, v.z < 0 ? -v.z : v.z, v.w < 0 ? -v.w : v.w ); }
	static bool same( const vector4 & v ) { return v.x == v.y && v.x == v.z && v.x == v.w; }
	const vector2 getVector2() const { return vector2( x, y ); }
	const vector3 getVector3() const { return vector3( x, y, z ); }
	//与えられた１次独立なベクトルを補充して正規直交基底ベクトルを作成する
	static bool gramschmidt( ::array<vector4> & v )
	{
		for ( int i = 0; i < v.size; ++i ) {
			v[i].normalize4();
		}
		const vector4 u[4] = { vector4( 1, 0, 0, 0 ), vector4( 0, 1, 0, 0 ), vector4( 0, 0, 1, 0 ), vector4( 0, 0, 0, 1 ) };
		while ( v.size < 4 ) {
			checkMaximum<decimal> mx;
			for ( int i = 0; i < 4; ++i ) {
				checkMinimum<decimal> mn;
				for ( int j = 0; j < v.size; ++j ) {
					mn( length4( u[i] - v[j] ) );
				}
				if ( mn ) mx( mn(), i );
			}
			if ( mx ) {
				v.push_back( u[mx.sub] );
			} else {
				v.push_back( u[0] );
			}
		}
		for ( int i = 1; i < 4; ++i ) {
			vector4 xi = v[i];
			for ( int j = 0; j < i; ++j ) {
				v[i] -= v[j] * dot4( v[j], xi );
			}
			v[i].normalize4();
		}
	}
};
inline const vector4 operator+( const decimal & s, const vector4 & v ) { return v + s; }
inline const vector4 operator-( const decimal & s, const vector4 & v ) { return - v + s; }
inline const vector4 operator*( const decimal & s, const vector4 & v ) { return v * s; }
inline const vector4 operator/( const decimal & s, const vector4 & v ) { return vector4( zerodivide<decimal>( 1, v.x ), zerodivide<decimal>( 1, v.y ), zerodivide<decimal>( 1, v.z ), zerodivide<decimal>( 1, v.w ) ) * s; }
#ifdef __GLOBAL__
const vector4 vector4::identity( 0, 0, 0, 1 );
const vector4 vector4::zero( 0, 0, 0, 0 ), vector4::one( 1, 1, 1, 1 );
const vector4 vector4::black( 0, 0, 0, 1 ), vector4::white( 1, 1, 1, 1 ), vector4::gray( 0.5, 0.5, 0.5, 1 ), vector4::red( 1, 0, 0, 1 ), vector4::green( 0, 1, 0, 1 ), vector4::blue( 0, 0, 1, 1 ), vector4::cyan( 0, 1, 1, 1 ), vector4::magenta( 1, 0, 1, 1 ), vector4::yellow( 1, 1, 0, 1 );
#endif
