////////////////////////////////////////////////////////////////////////////////
// Vector 2

class matrix22;
class vector3;
class vector4;

class vector2
{
	struct instance { decimal x, y; };
public:
	const static vector2 identity;
	const static vector2 zero, one;
	union {
		decimal array[2];
		struct { decimal x, y; };
	};
	vector2() {}
	vector2( const vector2 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); }
//	vector2( const vector3 & v );
//	vector2( const vector4 & v );
	vector2( const decimal & _x, const decimal & _y = 0 ) { x = _x; y = _y; }

	vector2 & operator=( const vector2 & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); return *this; }

	const vector2 operator-() const { return vector2( -x, -y ); }
	const vector2 operator+( const vector2 & v ) const { return vector2( x+v.x, y+v.y ); }
	const vector2 operator-( const vector2 & v ) const { return vector2( x-v.x, y-v.y ); }
	const vector2 operator*( const vector2 & v ) const { return vector2( x*v.x, y*v.y ); }
	const vector2 operator/( const vector2 & v ) const { return vector2( zerodivide<decimal>( x, v.x ), zerodivide<decimal>( y, v.y ) ); }
	void operator+=( const vector2 & v ) { x+=v.x; y+=v.y; }
	void operator-=( const vector2 & v ) { x-=v.x; y-=v.y; }
	void operator*=( const vector2 & v ) { x*=v.x; y*=v.y; }
	void operator/=( const vector2 & v ) { x = zerodivide<decimal>( x, v.x ); y = zerodivide<decimal>( y, v.y ); }
	const vector2 operator+( const decimal & v ) const { return vector2( x+v, y+v ); }
	const vector2 operator-( const decimal & v ) const { return vector2( x-v, y-v ); }
	const vector2 operator*( const decimal & v ) const { return vector2( x*v, y*v ); }
	const vector2 operator/( const decimal & v ) const { return vector2( zerodivide<decimal>( x, v ), zerodivide<decimal>( y, v ) ); }
	void operator+=( const decimal & v ) { x+=v; y+=v; }
	void operator-=( const decimal & v ) { x-=v; y-=v; }
	void operator*=( const decimal & v ) { x*=v; y*=v; }
	void operator/=( const decimal & v ) { x = zerodivide<decimal>( x, v ); y = zerodivide<decimal>( y, v ); }
	bool operator==( const vector2 & v ) const { return ( x == v.x && y == v.y ); }
	bool operator!=( const vector2 & v ) const { return ( x != v.x || y != v.y ); }
	operator decimal * () { return array; }
	operator const decimal * () const { return array; }
	const vector2 operator*( const matrix22 & m ) const;
	static decimal dot2( const vector2 & l, const vector2 & r ) { return l.x * r.x + l.y * r.y; }
	static decimal length2( const vector2 & v ) { return sqrt( dot2( v, v ) ); }
	static const vector2 normalize2( const vector2 & v ) { decimal l = length2( v ); if ( l == 1 ) return v; if ( l < epsilon ) return vector2( 1, 0 ); return v / l; }
	static const vector2 abs( const vector2 & v ) { return vector2( v.x < 0 ? -v.x : v.x, v.y < 0 ? -v.y : v.y ); }
	void normalize2() { *this = normalize2( *this ); }
	static bool same( const vector2 & v ) { return v.x == v.y; }
	static decimal argument2( const vector2 & l, const vector2 & r ) { return acos( dot2( normalize2( l ), normalize2( r ) ) ); }
	//—^‚¦‚ç‚ê‚½‚PŽŸ“Æ—§‚ÈƒxƒNƒgƒ‹‚ð•â[‚µ‚Ä³‹K’¼ŒðŠî’êƒxƒNƒgƒ‹‚ðì¬‚·‚é
	static bool gramschmidt( ::array<vector2> & v )
	{
		for ( int i = 0; i < v.size; ++i ) {
			v[i].normalize2();
		}
		const vector2 u[2] = { vector2( 1, 0 ), vector2( 0, 1 ) };
		while ( v.size < 2 ) {
			checkMaximum<decimal> mx;
			for ( int i = 0; i < 2; ++i ) {
				checkMinimum<decimal> mn;
				for ( int j = 0; j < v.size; ++j ) {
					mn( length2( u[i] - v[j] ) );
				}
				if ( mn ) mx( mn(), i );
			}
			if ( mx ) {
				v.push_back( u[mx.sub] );
			} else {
				v.push_back( u[0] );
			}
		}
		for ( int i = 1; i < 2; ++i ) {
			vector2 xi = v[i];
			for ( int j = 0; j < i; ++j ) {
				v[i] -= v[j] * dot2( v[j], xi );
			}
			v[i].normalize2();
		}
	}
};
inline const vector2 operator+( const decimal & s, const vector2 & v ) { return v + s; }
inline const vector2 operator-( const decimal & s, const vector2 & v ) { return - v + s; }
inline const vector2 operator*( const decimal & s, const vector2 & v ) { return v * s; }
inline const vector2 operator/( const decimal & s, const vector2 & v ) { return vector2( zerodivide<decimal>( 1, v.x ), zerodivide<decimal>( 1, v.y ) ) * s; }

#ifdef __GLOBAL__
const vector2 vector2::identity( 0, 0 );
const vector2 vector2::zero( 0, 0 ), vector2::one( 1, 1 );
#endif
