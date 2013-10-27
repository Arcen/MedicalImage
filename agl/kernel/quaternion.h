////////////////////////////////////////////////////////////////////////////////
// Quaternion

//íPà élå≥êîÇéÂÇ…àµÇ§
class quaternion
{
	struct instance { decimal x, y, z, w; };
public:
	const static quaternion identity;
	const static quaternion zero, one;
	const static quaternion x90, x180, x270, y90, y180, y270, z90, z180, z270;
	union {
		decimal array[4];
		struct { decimal x, y, z, w; };
	};
	quaternion() {}
	quaternion( const quaternion & q ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & q ); }
	quaternion( const decimal & _x, const decimal & _y, const decimal & _z, const decimal & _w ) { x = _x; y = _y; z = _z; w = _w; }
	quaternion( const vector3 & v ) { *this = quaternion( v.x, v.y, v.z, 0 ); }
	quaternion( const vector4 & v ) { *this = quaternion( v.x, v.y, v.z, 0 ); }
	quaternion( const decimal * v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( v ); }
	quaternion( const vector3 & axis, decimal angle )//[rad]
	{
		decimal halfAngle = static_cast<decimal>( angle * 0.5 );
		decimal cosHalfAngle = cos( halfAngle );
		decimal sinHalfAngle = sin( halfAngle );
		if ( fabs( cosHalfAngle ) < epsilon ) {
			cosHalfAngle = 0;
			if ( sinHalfAngle < 0 ) {
				sinHalfAngle = -1;
			} else {
				sinHalfAngle = 1;
			}
		}
		if ( cosHalfAngle < 0 ) {
			cosHalfAngle = - cosHalfAngle;
			sinHalfAngle = - sinHalfAngle;
		}
		*this = axis * sinHalfAngle;
		w = cosHalfAngle;
		*this = validate( *this );
	}
	static quaternion validate( quaternion q )
	{
		if ( between<decimal>( 0, q.w, 1 ) ) {
		} else if ( between<decimal>( -1, q.w, 0 ) ) {
			return validate( - q );
		} else {
			return identity;
		}
		if ( q.w == 1 ) return identity;
		decimal factor = sqrt( 1 - agl::square( q.w ) );
		vector3 v = vector3::normalize3( vector3( q.x, q.y, q.z ) ) * factor;
		q = quaternion( v.x, v.y, v.z, q.w );
		assert( _finite( q.x ) && _finite( q.y ) && _finite( q.z ) && _finite( q.w ) );
		return q;
	}

	quaternion & operator=( const quaternion & v ) { *reinterpret_cast<instance*>( this ) = *reinterpret_cast<const instance*>( & v ); return *this; }

	quaternion operator-() const { return quaternion( -x, -y, -z, -w ); }
	quaternion operator+( const quaternion & v ) const { return quaternion( x+v.x, y+v.y, z+v.z, w+v.w ); }
	quaternion operator-( const quaternion & v ) const { return quaternion( x-v.x, y-v.y, z-v.z, w-v.w ); }
	void operator+=( const quaternion & v ) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; }
	void operator-=( const quaternion & v ) { x-=v.x; y-=v.y; z-=v.z; w-=v.w; }
	quaternion operator+( const decimal & v ) const { return quaternion( x+v, y+v, z+v, w+v ); }
	quaternion operator-( const decimal & v ) const { return quaternion( x-v, y-v, z-v, w-v ); }
	quaternion operator*( const decimal & v ) const { return quaternion( x*v, y*v, z*v, w*v ); }
	quaternion operator/( const decimal & v ) const { return quaternion( zerodivide( x, v ), zerodivide( y, v ), zerodivide( z, v ), zerodivide( w, v ) ); }
	void operator+=( const decimal & v ) { x+=v; y+=v; z+=v; w+=v; }
	void operator-=( const decimal & v ) { x-=v; y-=v; z-=v; w-=v; }
	void operator*=( const decimal & v ) { x*=v; y*=v; z*=v; w*=v; }
	void operator/=( const decimal & v ) { x=zerodivide(x,v); y=zerodivide(y,v); z=zerodivide(z,v); w=zerodivide(w,v); }
	bool operator==( const quaternion & v ) const { return ( x == v.x && y == v.y && z == v.z && w == v.w ); }
	bool operator!=( const quaternion & v ) const { return ( x != v.x || y != v.y || z != v.z || w != v.w ); }
	operator decimal * () { return array; }
	operator const decimal * () const { return array; }

	static decimal dot3( const quaternion & l, const quaternion & r ) { return l.x * r.x + l.y * r.y + l.z * r.z; }
	static decimal dot4( const quaternion & l, const quaternion & r ) { return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w; }
	static decimal length3( const quaternion & v ) { return sqrt( dot3( v, v ) ); }
	static decimal length4( const quaternion & v ) { return sqrt( dot4( v, v ) ); }
	static quaternion normalize( const quaternion & v ) { decimal l = length4( v ); if ( l == 1 ) return validate( v ); if ( l < epsilon ) return quaternion( 0, 0, 0, 1 ); return validate( v / l ); }
	static quaternion conjugation( const quaternion & v ) { return quaternion( -v.x, -v.y, -v.z, v.w ); }
	// ÉxÉNÉ^Å[Ç∆ÇÃêœÇ…ÇÊÇÈâÒì]ÇÕ q * ( v.x, v.y, v.z, 0 ) * q-1 Ç∆íËã`Ç≥ÇÍÇƒÇ¢ÇÈ
	//êœ
	quaternion operator*( const quaternion &q ) const 
	{
		return quaternion( 
			w * q.x + q.w * x + y * q.z - z * q.y,
			w * q.y + q.w * y + z * q.x - x * q.z,
			w * q.z + q.w * z + x * q.y - y * q.x,
			w * q.w - x * q.x - y * q.y - z * q.z );
	}
	void operator*=( const quaternion & q ) { *this = *this * q; }
	//è§
	quaternion operator/( const quaternion &q ) const 
	{
		const quaternion c = conjugation( q );
		const quaternion r = *this * c;
		const quaternion s = square( c );
		return validate( r / s.w );
	}
	void operator/=( const quaternion & q ) { *this = *this / q; }
	//ÇQèÊ
	static quaternion square( const quaternion & q )
	{
		decimal tt = 2 * q.w;
		return normalize( quaternion( q.x * tt, q.y * tt, q.z * tt, q.w * q.w - q.x * q.x - q.y * q.y - q.z * q.z ) );
	}
	//ÇQèÊç™
	static quaternion squareRoot( const quaternion & q )
	{
		decimal len = length4( q );
		if ( len ) {
			len = 1 / len;
		} else {
			len = 1;
		}
		quaternion r( q.x * len, q.z * len, 0, q.w * len );
		decimal m = 1 / sqrt( r.w * r.w + r.x * r.x );
		decimal r1 = sqrt( 0.5f + r.y * 0.5f );
		decimal r2 = sqrt( 0.5f - r.y * 0.5f );
		len = sqrt( len );
		return normalize( quaternion( 
			len * r2 * r.x * m,
			len * r1,
			q.z,
			len * r1 * r.w * m ) );
	}
	//exponent
	static quaternion exp( const quaternion & q )
	{
		decimal len = length3( q );
		decimal t = ( len > 0 ) ? sin( len ) / len : 1;
		return quaternion( q.x * t, q.y * t, q.z * t, cos( len ) );
	}
	//ëŒêî
	// q = cos angle + axis sin angle = exp( axis angle )
	// log(q) = log( exp( axis angle ) ) = axis angle
	static quaternion log( const quaternion & q )
	{
		decimal len = ( q.w ) ? atan( length3( q ) / q.w ) : pi_2;
		return quaternion( q.x * len, q.y * len, q.z * len, 0 );
	}
	//äKèÊ
	static quaternion pow( const quaternion & q, const decimal & t )
	{
		vector3 axis;
		decimal angle;
		extract( q, axis, angle );
		return quaternion( axis, angle * t );
	}
	// ãÖñ ê¸å`ï‚ä‘ Spherical-Linier-intERPolation
	static quaternion slerp( const decimal & alpha, const quaternion & _q1, const quaternion & q2 )
	{
		//Slerp(q1,q2,t) := q1 * sin(É∆(1-t))/sinÉ∆ + q2 * sin(É∆t) /sinÉ∆
		//cosÉ∆:=dotProduct(q1,q2)
		if( alpha < roughEpsilon ) return _q1;
		if( alpha > 1.0f - roughEpsilon ) return q2;
		
		decimal costheta = dot4( _q1, q2 );
		quaternion q1;
		if ( costheta < 0 ) {
			costheta = -costheta;
			q1 = -_q1;
		} else {
			q1 = _q1;
		}
		decimal p1 = 1 - alpha, p2 = alpha;
		if ( fabs( 1.0f - costheta ) > roughEpsilon ) { // if theta == 0 -> sin(x) == x
			const decimal theta = acos( costheta );
			const decimal oosintheta = ( 1.0f / sin( theta ) );
			p1 = sin( p1 * theta ) * oosintheta;
			p2 = sin( p2 * theta ) * oosintheta;
			if ( ( ! _finite( p1 ) ) || ( ! _finite( p2 ) ) ) {
				p1 = 1 - alpha;
				p2 = alpha;
			}
		}

		// calculate final values
		return validate( q1 * p1 + q2 * p2 );
	}
	//SlerpÇÃî˜ï™
	static quaternion derivativeSlerp( const decimal & alpha, const quaternion & q1, const quaternion & q2 )
	{
		quaternion q = conjugation( q1 ) * q2;
		return q1 * pow( q, alpha ) * log( q );
	}
	// Spherical-Cubic-interpolation
	static quaternion slerpForSquad( const decimal & alpha, const quaternion & q1, const quaternion & q2 )
	{
		//Slerp(q1,q2,t) := q1 * sin(É∆(1-t))/sinÉ∆ + q2 * sin(É∆t) /sinÉ∆
		//cosÉ∆:=dotProduct(q1,q2)
		if( alpha < roughEpsilon ) return q1;
		if( alpha > 1.0f - roughEpsilon ) return q2;
		
		decimal costheta = dot4( q1, q2 );
		decimal p1 = 1 - alpha, p2 = alpha;
		if ( fabs( 1.0f - costheta ) > roughEpsilon ) { // if theta == 0 -> sin(x) == x
			const decimal theta = acos( costheta );
			const decimal oosintheta = ( 1.0f / sin( theta ) );
			p1 = sin( p1 * theta ) * oosintheta;
			p2 = sin( p2 * theta ) * oosintheta;
			if ( ( ! _finite( p1 ) ) || ( ! _finite( p2 ) ) ) {
				p1 = 1 - alpha;
				p2 = alpha;
			}
		}
		return q1 * p1 + q2 * p2;
	}
	//SQUADÅiÇSå≥êîÇÃí[ì_à íuÇ∆ê⁄ï˚å¸Ç≈ÇÃï‚ä‘Åj
	static quaternion squad( const decimal & alpha, const quaternion & q1, const quaternion & a1, const quaternion & a2, const quaternion & q2 )
	{
		return validate( slerpForSquad( 2 * alpha * ( 1 - alpha ), slerpForSquad( alpha, q1, q2 ), slerpForSquad( alpha, a1, a2 ) ) );
	}
	//ÇOÇÃà íuÇ≈ÇÃî˜ï™
	static quaternion derivativeSquad0( const quaternion & q1, const quaternion & a1, const quaternion & a2, const quaternion & q2 )
	{
		return q1 * ( log( conjugation( q1 ) * q2 ) + log( conjugation( q1 ) * a1 ) * 2 );
	}
	//ÇPÇÃà íuÇ≈ÇÃî˜ï™
	static quaternion derivativeSquad1( const quaternion & q1, const quaternion & a1, const quaternion & a2, const quaternion & q2 )
	{
		return q2 * ( log( conjugation( q1 ) * q2 ) - log( conjugation( q2 ) * a2 ) * 2 );
	}
	//ÅiRoll,Pitch,YawÅjÇ©ÇÁÇSå≥êîÇåvéZ
	static quaternion euler( const decimal & roll, const decimal & pitch, const decimal & yaw );
	//ÇSå≥êîÇ©ÇÁÅiRoll,Pitch,YawÅjÇåvéZ
	static void euler( const quaternion & q, decimal & roll, decimal & pitch, decimal & yaw );
	//âÒì]é≤Ç∆âÒì]äpìxÇíäèo
	static void extract( const quaternion & q, vector3 & axis, decimal & angle )
	{
		axis = vector3( q.x, q.y, q.z );
		decimal len = vector3::length3( axis );
		if ( len < epsilon ) {
			axis = vector3( 0, 0, 1 );
			angle = 0;
			return;
		}
		axis /= len;
		angle = 2 * acos( q.w );
	}
	// x * from = toÇ∆Ç»ÇÈÇÊÇ§Ç…ç≈íZÇÃâÒì]äpìxÇ∆Ç»ÇÈxÇåvéZ x = to / from
	static quaternion fromTo( const vector3 & from, const vector3 & to )
	{
		decimal cost = vector3::dot3( from, to );
		// same direction
		if ( 1 - epsilon <= cost ) return identity;
		// opposite direction
		if ( cost <= -1 + epsilon ) {
			vector3 t = vector3( 0, from.x, - from.y );
			decimal len = vector3::length3( t );
			if ( len < epsilon ) t = vector3( - from.z, 0, from.x );
			t = vector3::normalize3( t );
			return quaternion( t.x, t.y, t.z, 0 );
		}
		// axis
		vector3 t = vector3::normalize3( vector3::cross( from, to ) );
		// half-angle formulae sin^2 t = ( 1 - cos 2t ) / 2
		t *= sqrt( 0.5f - cost * 0.5f );
		// half-angle formulae cos^2 t = ( 1 + cos 2t ) / 2
		return validate( quaternion( t.x, t.y, t.z, sqrt( 0.5f + cost * 0.5f ) ) );
	}
};

inline quaternion operator+( const decimal & s, const quaternion & v ) { return v + s; }
inline quaternion operator-( const decimal & s, const quaternion & v ) { return - v + s; }
inline quaternion operator*( const decimal & s, const quaternion & v ) { return v * s; }
inline quaternion operator/( const decimal & s, const quaternion & v ) { return quaternion( 1/v.x, 1/v.y, 1/v.z, 1/v.w ) * s; }

#ifdef __GLOBAL__
const quaternion quaternion::identity( 0, 0, 0, 1 );
const quaternion quaternion::zero( 0, 0, 0, 1 ), quaternion::one( 0, 0, 0, 1 );
const quaternion quaternion::x90( 1 / root2, 0, 0, 1 / root2 );
const quaternion quaternion::x180( 1, 0, 0, 0 );
const quaternion quaternion::x270( -1 / root2, 0, 0, 1 / root2 );
const quaternion quaternion::y90( 0, 1 / root2, 0, 1 / root2 );
const quaternion quaternion::y180( 0, 1, 0, 0 );
const quaternion quaternion::y270( 0, -1 / root2, 0, 1 / root2 );
const quaternion quaternion::z90( 0, 0, 1 / root2, 1 / root2 );
const quaternion quaternion::z180( 0, 0, 1, 0 );
const quaternion quaternion::z270( 0, 0, -1 / root2, 1 / root2 );
#endif
