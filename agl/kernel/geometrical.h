////////////////////////////////////////////////////////////////////////////////
// 距離計算
////////////////////////////////////////////////////////////////////////////////
// 未テスト BUG DEBUG

class geometricalPoint;
class geometricalLine;
class geometricalSurface;
class geometricalVolume;

////////////////////////////////////////////////////////////////////////////////
// 点
class geometricalPoint
{
public:
	typedef geometricalPoint componentType;
	vector3 position;
	geometricalPoint()
	{
	}
	geometricalPoint( const geometricalPoint & p ) : position( p.position )
	{
	}
	geometricalPoint( const vector3 & p ) : position( p )
	{
	}
	const vector3 operator()() const
	{
		return position;
	}
	void component( array<componentType> & result ) const {};
	void component( int index, componentType & t ) const {};
	bool limited() const { return false; }
	bool valid() const { return true; }
	const decimal distance_unlimited( const geometricalPoint & p ) const
	{
		return vector3::length3( (*this)() - p() );
	}
	const decimal distance_unlimited( geometricalLine & l ) const;
	const decimal distance_unlimited( geometricalSurface & s ) const;
	const decimal distance_unlimited( geometricalVolume & v ) const;
};

////////////////////////////////////////////////////////////////////////////////
// 直線・半直線・線分
class geometricalLine
{
public:
	typedef geometricalPoint componentType;
	vector3 position;
	vector3 direction;
	decimal parameter;
	enum {
		Line, Ray, Segment,
	};
	int type;
	geometricalLine() : parameter( 0 ), type( Line )
	{
	}
	geometricalLine( const geometricalLine & l ) 
		: position( l.position ), direction( l.direction ), parameter( l.parameter ), 
		type( l.type )
	{
	}
	geometricalLine( const vector3 & p, const vector3 & d, int t = Line ) : position( p ), direction( d ), 
		parameter( 0 ), type( t )
	{
	}
	const vector3 operator()() const
	{
		return position + direction * parameter;
	}
	void line() { type = Line; }
	void ray() { type = Ray; }
	void segment() { type = Segment; }
	bool limited() const { return type != Line; }
	bool valid() const
	{
		const bool minimum0 = ( type == Ray || type == Segment );
		const bool maximum1 = ( type == Segment );
		if ( minimum0 && parameter < 0 ) return false;
		if ( maximum1 && 1 < parameter ) return false;
		return true;
	}
	void component( array<componentType> & result ) const
	{
		switch ( type ) {
		case Ray:
			result.push_back( geometricalPoint( position ) );
			break;
		case Segment:
			result.push_back( geometricalPoint( position ) );
			result.push_back( geometricalPoint( position + direction ) );
			break;
		}
	};
	void component( int index, componentType & p )
	{
		switch ( type ) {
		case Ray:
			parameter = 0;
			break;
		case Segment:
			parameter = index;
			break;
		}
	}
	const decimal distance_unlimited( const geometricalPoint & p )
	{
		const vector3 & a = direction;
		const vector3 & b = position;
		const vector3 & u = -p.position;
		decimal & x = parameter;
		// Q(x)=|ax+b+u|^2とする
		//Q'(x)=2a(ax+b+u)=0
		//x=-a(b+u)/aa
		x = - vector3::dot3( a, b + u ) / vector3::dot3( a, a );
		return vector3::length3( p() - (*this)() );
	}
	const decimal distance_unlimited( geometricalLine & l )
	{
		const vector3 & a = direction;
		const vector3 & b = position;
		const vector3 & u = -l.direction;
		const vector3 & v = -l.position;
		decimal & x = parameter;
		decimal & y = l.parameter;
		// Q(x,y)=|(ax+b)+(uy+v)|^2とする
		//Q'(x,y)=2(ax+uy+b+v)・(a,u)
		//(x,y)(aa,au)=-(a(b+v), u(b+v))
		//     (au,uu)
		const vector3 bv = b+v;
		const decimal aa = vector3::dot3( a, a );
		const decimal au = vector3::dot3( a, u );
		const decimal uu = vector3::dot3( u, u );
		matrix22 m( aa, au, au, uu );
		if ( absolute( matrix22::determinant( m ) ) < epsilon ) {
			//平行なので、パラメータは片方は何でもいい
			//Q'(0,y)=2(uy+b+v)・(a,u)
			//(uy+b+v)a=0
			//y=-(b+v)a/au
			x = 0;
			y = -vector3::dot3( bv, a ) / au;
		} else {
			vector2 t = - vector2( vector3::dot3( a, bv ), vector3::dot3( u, bv ) ) * 
				matrix22::inverse( m );
			x = t.x;
			y = t.y;
		}
		return vector3::length3( (*this)() - l() );
	}
	const decimal distance_unlimited( geometricalSurface & s );
	const decimal distance_unlimited( geometricalVolume & v );
};

////////////////////////////////////////////////////////////////////////////////
// 平面・半平面(0<x)・四分平面(0<x,0<y)
// 帯状平面(0<x<1)・半帯状平面(0<x<1,0<y)
// 矩形(0<x<1,0<y<1)・三角形(0<x,0<y,x+y<1)
class geometricalSurface
{
public:
	typedef geometricalLine componentType;
	vector3 position;
	vector3 direction_x, direction_y;
	vector2 parameter;
	enum {
		Plane, 
		HalfPlane, //0<x
		QuarterPlane, //0<x,0<y
		Belt, //0<x<1
		HalfBelt, //0<x<1,0<y
		Rectangle, //0<x<1,0<y<1
		Triangle, //0<x,0<y,x+y<1
	};
	int type;
	geometricalSurface() : type( Plane )
	{
	}
	geometricalSurface( const vector3 & p, const vector3 & x, const vector3 & y, int t = Plane ) 
		: position( p ), direction_x( x ), direction_y( y ), 
		type( t )
	{
	}
	geometricalSurface( const geometricalSurface & s ) 
		: position( s.position ), direction_x( s.direction_x ), direction_y( s.direction_y ), parameter( s.parameter ), 
		type( s.type )
	{
	}
	const vector3 operator()() const
	{
		return position + direction_x * parameter.x + direction_y * parameter.y;
	}
	void plane() { type = Plane; }
	void halfplane() { type = HalfPlane; }
	void quaterplane() { type = QuarterPlane; }
	void belt() { type = Belt; }
	void halfbelt() { type = HalfBelt; }
	void rectangle() { type = Rectangle; }
	void triangle() { type = Triangle; }
	bool limited() const { return type != Plane; }
	bool valid() const
	{
		const bool x_minimum0 = ( type != Plane );
		const bool x_maximum1 = ( type == Belt || type == HalfBelt || type == Rectangle );
		const bool y_minimum0 = ( type == QuarterPlane || type == HalfBelt || type == Rectangle || type == Triangle );
		const bool y_maximum1 = ( type == Rectangle );
		const bool oblique = ( type == Triangle );
		if ( x_minimum0 && parameter.x < 0 ) return false;
		if ( x_maximum1 && 1 < parameter.x ) return false;
		if ( y_minimum0 && parameter.y < 0 ) return false;
		if ( y_maximum1 && 1 < parameter.y ) return false;
		if ( oblique && 1 < parameter.x + parameter.y ) return false;
		return true;
	}
	void component( array<componentType> & result ) const
	{
		switch ( type ) {
		case HalfPlane://0<x
			result.push_back( geometricalLine( position, direction_y ) );
			break;
		case QuarterPlane://0<x,0<y
			result.push_back( geometricalLine( position, direction_y, geometricalLine::Ray ) );
			result.push_back( geometricalLine( position, direction_x, geometricalLine::Ray ) );
			break;
		case Belt://0<x<1
			result.push_back( geometricalLine( position, direction_y ) );
			result.push_back( geometricalLine( position + direction_x, direction_y ) );
			break;
		case HalfBelt://0<x<1,0<y
			result.push_back( geometricalLine( position, direction_y, geometricalLine::Ray ) );
			result.push_back( geometricalLine( position + direction_x, direction_y, geometricalLine::Ray ) );
			result.push_back( geometricalLine( position, direction_x, geometricalLine::Segment ) );
			break;
		case Rectangle://0<x<1,0<y<1
			result.push_back( geometricalLine( position, direction_y, geometricalLine::Segment ) );
			result.push_back( geometricalLine( position + direction_x, direction_y, geometricalLine::Segment ) );
			result.push_back( geometricalLine( position, direction_x, geometricalLine::Segment ) );
			result.push_back( geometricalLine( position + direction_y, direction_x, geometricalLine::Segment ) );
			break;
		case Triangle://0<x,0<y,x+y<1
			result.push_back( geometricalLine( position, direction_y, geometricalLine::Segment ) );
			result.push_back( geometricalLine( position, direction_x, geometricalLine::Segment ) );
			result.push_back( geometricalLine( position + direction_x, direction_y - direction_x, geometricalLine::Segment ) );
			break;
		}
	};
	void component( int index, componentType & l )
	{
		switch ( type ) {
		case HalfPlane://0<x
			parameter = vector2( 0, l.parameter );
			break;
		case QuarterPlane://0<x,0<y
			switch ( index ) {
			case 0: parameter = vector2( 0, l.parameter ); break;
			case 1: parameter = vector2( l.parameter, 0 ); break;
			}
			break;
		case Belt://0<x<1
			switch ( index ) {
			case 0: parameter = vector2( 0, l.parameter ); break;
			case 1: parameter = vector2( 1, l.parameter ); break;
			}
			break;
		case HalfBelt://0<x<1,0<y
			switch ( index ) {
			case 0: parameter = vector2( 0, l.parameter ); break;
			case 1: parameter = vector2( 1, l.parameter ); break;
			case 2: parameter = vector2( l.parameter, 0 ); break;
			}
			break;
		case Rectangle://0<x<1,0<y<1
			switch ( index ) {
			case 0: parameter = vector2( 0, l.parameter ); break;
			case 1: parameter = vector2( 1, l.parameter ); break;
			case 2: parameter = vector2( l.parameter, 0 ); break;
			case 3: parameter = vector2( l.parameter, 1 ); break;
			}
			break;
		case Triangle://0<x,0<y,x+y<1
			switch ( index ) {
			case 0: parameter = vector2( 0, l.parameter ); break;
			case 1: parameter = vector2( l.parameter, 0 ); break;
			case 2: parameter = vector2( 1 - l.parameter, l.parameter ); break;
			}
			break;
		}
	}
	const decimal distance_unlimited( const geometricalPoint & p )
	{
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = position;
		const vector3 & u = -p.position;
		decimal & x = parameter.x;
		decimal & y = parameter.y;
		// Q(x,y)=|(ax+by+c)+(u)|^2とする
		//Q'(x,y)=2(ax+by+c+u)・(a,b)
		//(x,y)(aa,ab)=-(a(c+u), b(c+u))
		//     (ab,bb)
		const vector3 cu = c+u;
		const decimal aa = vector3::dot3( a, a );
		const decimal ab = vector3::dot3( a, b );
		const decimal bb = vector3::dot3( b, b );
		matrix22 m( aa, ab, ab, bb );
		if ( absolute( matrix22::determinant( m ) ) < epsilon ) {
			//平面は直線である
			geometricalLine t( c, a );
			t.distance_unlimited( p );
			x = t.parameter;
			y = 0;
		} else {
			vector2 t = -vector2( vector3::dot3( a, cu ), vector3::dot3( b, cu ) ) * 
				matrix22::inverse( m );
			x = t.x;
			y = t.y;
		}
		return vector3::length3( (*this)() - p() );
	}
	const decimal distance_unlimited( geometricalLine & l )
	{
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = position;
		const vector3 & u = -l.direction;
		const vector3 & v = -l.position;
		decimal & x = parameter.x;
		decimal & y = parameter.y;
		decimal & z = l.parameter;
		// Q(x,y,z)=|(ax+by+c)+(uz+v)|^2とする
		//Q'(x,y,z)=2(ax+by+uz+c+v)・(a,b,u)
		// (x,y,z)(aa,ab,au)=(a(c+v),b(c+v),u(c+v))
		//        (ab,bb,bu)
		//        (au,bu,uu)
		const vector3 cv = c+v;
		const decimal aa = vector3::dot3( a, a );
		const decimal ab = vector3::dot3( a, b );
		const decimal au = vector3::dot3( a, u );
		const decimal bb = vector3::dot3( b, b );
		const decimal bu = vector3::dot3( b, u );
		const decimal uu = vector3::dot3( u, u );
		matrix33 m( aa, ab, au, ab, bb, bu, au, bu, uu );
		if ( absolute( matrix33::determinant( m ) ) < epsilon ) {
			if ( absolute( aa * bb - ab * ab ) < epsilon ) {
				//平面は直線である
				geometricalLine t( c, a );
				l.distance_unlimited( t );
				x = t.parameter;
				y = 0;
			} else {
				//平行なので、線上の点は適当に
				z = 0;
				geometricalPoint t( v );
				this->distance_unlimited( t );
			}
		} else {
			vector3 t = - vector3( vector3::dot3( a, cv ), vector3::dot3( b, cv ), vector3::dot3( u, cv ) ) * 
				matrix33::inverse( m );
			x = t.x;
			y = t.y;
			z = t.z;
		}
		return vector3::length3( (*this)() - l() );
	}
	const decimal distance_unlimited( geometricalSurface & s )
	{
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = position;
		const vector3 & u = s.direction_x;
		const vector3 & v = s.direction_y;
		const vector3 & w = s.position;
		decimal & x = parameter.x;
		decimal & y = parameter.y;
		decimal & z = s.parameter.x;
		decimal & o = s.parameter.y;
		// Q(x,y,z,o)=|(ax+by+c)+(uz+vo+w)|^2とする
		//Q'(x,y,z,o)=2(ax+by+uz+vo+c+w)・(a,b,u,v)
		// (x,y,z,o)(aa,ab,au,av)=(c+w)・(a,b,u,v)
		//          (ab,bb,bu,bv)
		//          (au,bu,uu,uv)
		//          (av,bv,uv,vv)
		const vector3 cw = c+w;
		const decimal aa = vector3::dot3( a, a );
		const decimal ab = vector3::dot3( a, b );
		const decimal au = vector3::dot3( a, u );
		const decimal av = vector3::dot3( a, v );
		const decimal bb = vector3::dot3( b, b );
		const decimal bu = vector3::dot3( b, u );
		const decimal bv = vector3::dot3( b, v );
		const decimal uu = vector3::dot3( u, u );
		const decimal uv = vector3::dot3( u, v );
		const decimal vv = vector3::dot3( v, v );
		matrix44 m( aa, ab, au, av, ab, bb, bu, bv, au, bu, uu, uv, av, bv, uv, vv );
		//if ( matrix44::determinant( m ) == 0 ) {
		if ( true ) {
			if ( absolute( aa * bb - ab * ab ) < epsilon ) {
				//平面は直線である
				geometricalLine t( c, a );
				s.distance_unlimited( t );
				x = t.parameter;
				y = 0;
			} else if ( absolute( uu * vv - uv * uv ) < epsilon ) {
				//平面は直線である
				geometricalLine t( w, u );
				distance_unlimited( t );
				z = t.parameter;
				o = 0;
			} else {
				const vector3 d = vector3::cross( a, b );
				const vector3 e = vector3::cross( u, v );
				if ( absolute( vector3::dot3( d, d ) * vector3::dot3( e, e ) - square( vector3::dot3( d, e ) ) ) < epsilon ) {
					//平行なので、平面上の点は適当に
					z = 0;
					o = 0;
					geometricalPoint t( w );
					this->distance_unlimited( t );
				} else {
					//交点を適当に求める
					geometricalLine t1( w, u );
					geometricalLine t2( w, v );
					geometricalSurface t3( *this );
					geometricalSurface t4( *this );
					if ( t3.distance_unlimited( t1 ) < t4.distance_unlimited( t2 ) ) {
						parameter = t3.parameter;
						z = t1.parameter;
						o = 0;
					} else {
						parameter = t4.parameter;
						o = t2.parameter;
						z = 0;
					}
				}
			}
		} else {
			//平面の交点が１点に交わることは無いので、ここには来ない
			vector4 t = - vector4( vector3::dot3( a, cw ), vector3::dot3( b, cw ), vector3::dot3( u, cw ), vector3::dot3( v, cw ) ) * 
				matrix44::inverse( m );
			x = t.x;
			y = t.y;
			z = t.z;
			o = t.w;
		}
		return vector3::length3( (*this)() - s() );
	}
	const decimal distance_unlimited( geometricalVolume & v );
};
////////////////////////////////////////////////////////////////////////////////
// 空間
// 直方体(0<x<1,0<y<1,0<z<1)
// 三角柱(0<x,0<y,x+y<1,0<z<1)
// 三角錐(0<x,0<y,0<z,x+y+z<1)
class geometricalVolume
{
public:
	typedef geometricalSurface componentType;
	vector3 position;
	vector3 direction_x, direction_y, direction_z;
	vector3 parameter;
	enum {
		Space, 
		Parallelepiped, //0<x<1,0<y<1,0<z<1
		TriangularPrism, //0<x,0<y,0<z<1,x+y<1
		TriangularPyramid, //0<x,0<y,0<z,x+y+z<1
	};
	int type;
	geometricalVolume() : type( Parallelepiped )
	{
	}
	geometricalVolume( const vector3 & p, const vector3 & x, const vector3 & y, const vector3 & z, int t = Parallelepiped ) 
		: position( p ), direction_x( x ), direction_y( y ), direction_z( z ), 
		type( t )
	{
	}
	geometricalVolume( const geometricalVolume & v ) 
		: position( v.position ), direction_x( v.direction_x ), direction_y( v.direction_y ), direction_z( v.direction_z ), parameter( v.parameter ), 
		type( v.type )
	{
	}
	const vector3 operator()() const
	{
		return position + direction_x * parameter.x + direction_y * parameter.y+ direction_z * parameter.z;
	}
	void space() { type = Space; }
	void parallelepiped() { type = Parallelepiped; }
	void triangularPrism() { type = TriangularPrism; }
	void triangularPyramid() { type = TriangularPyramid; }
	bool limited() const { return type != Space; }
	bool valid() const
	{
		const bool x_minimum0 = ( type == Parallelepiped || type == TriangularPrism || type == TriangularPyramid );
		const bool x_maximum1 = ( type == Parallelepiped );
		const bool y_minimum0 = ( type == Parallelepiped || type == TriangularPrism || type == TriangularPyramid );
		const bool y_maximum1 = ( type == Parallelepiped );
		const bool z_minimum0 = ( type == Parallelepiped || type == TriangularPrism || type == TriangularPyramid );
		const bool z_maximum1 = ( type == Parallelepiped || type == TriangularPrism );
		const bool xy_oblique = ( type == TriangularPrism );
		const bool xyz_oblique = ( type == TriangularPyramid );
		if ( x_minimum0 && parameter.x < 0 ) return false;
		if ( x_maximum1 && 1 < parameter.x ) return false;
		if ( y_minimum0 && parameter.y < 0 ) return false;
		if ( y_maximum1 && 1 < parameter.y ) return false;
		if ( z_minimum0 && parameter.z < 0 ) return false;
		if ( z_maximum1 && 1 < parameter.z ) return false;
		if ( xy_oblique && 1 < parameter.x + parameter.y ) return false;
		if ( xyz_oblique && 1 < parameter.x + parameter.y + parameter.z ) return false;
		return true;
	}
	void component( array<componentType> & result ) const
	{
		switch ( type ) {
		case Parallelepiped://0<x<1,0<y<1,0<z<1
			result.push_back( geometricalSurface( position, direction_y, direction_z, geometricalSurface::Rectangle ) );
			result.push_back( geometricalSurface( position + direction_x, direction_y, direction_z, geometricalSurface::Rectangle ) );
			result.push_back( geometricalSurface( position, direction_z, direction_x, geometricalSurface::Rectangle ) );
			result.push_back( geometricalSurface( position + direction_y, direction_z, direction_x, geometricalSurface::Rectangle ) );
			result.push_back( geometricalSurface( position, direction_x, direction_y, geometricalSurface::Rectangle ) );
			result.push_back( geometricalSurface( position + direction_z, direction_x, direction_y, geometricalSurface::Rectangle ) );
			break;
		case TriangularPrism://0<x,0<y,0<z<1,x+y<1
			result.push_back( geometricalSurface( position, direction_y, direction_z, geometricalSurface::Rectangle ) );
			result.push_back( geometricalSurface( position, direction_z, direction_x, geometricalSurface::Rectangle ) );
			result.push_back( geometricalSurface( position, direction_x, direction_y, geometricalSurface::Triangle ) );
			result.push_back( geometricalSurface( position + direction_z, direction_x, direction_y, geometricalSurface::Triangle ) );
			result.push_back( geometricalSurface( position + direction_x, direction_y - direction_x, direction_z, geometricalSurface::Rectangle ) );
			break;
		case TriangularPyramid://0<x,0<y,0<z,x+y+z<1
			result.push_back( geometricalSurface( position, direction_y, direction_z, geometricalSurface::Triangle ) );
			result.push_back( geometricalSurface( position, direction_z, direction_x, geometricalSurface::Triangle ) );
			result.push_back( geometricalSurface( position, direction_x, direction_y, geometricalSurface::Triangle ) );
			result.push_back( geometricalSurface( position + direction_x, direction_y - direction_x, direction_z - direction_x, geometricalSurface::Triangle ) );
			break;
		}
	};
	void component( int index, componentType & s )
	{
		switch ( type ) {
		case Parallelepiped://0<x<1,0<y<1,0<z<1
			switch ( index ) {
			case 0: parameter = vector3( 0, s.parameter.x, s.parameter.y ); break;
			case 1: parameter = vector3( 1, s.parameter.x, s.parameter.y ); break;
			case 2: parameter = vector3( s.parameter.y, 0, s.parameter.x ); break;
			case 3: parameter = vector3( s.parameter.y, 1, s.parameter.x ); break;
			case 4: parameter = vector3( s.parameter.x, s.parameter.y, 0 ); break;
			case 5: parameter = vector3( s.parameter.x, s.parameter.y, 1 ); break;
			}
			break;
		case TriangularPrism://0<x,0<y,0<z<1,x+y<1
			switch ( index ) {
			case 0: parameter = vector3( 0, s.parameter.x, s.parameter.y ); break;
			case 1: parameter = vector3( s.parameter.y, 0, s.parameter.x ); break;
			case 2: parameter = vector3( s.parameter.x, s.parameter.y, 0 ); break;
			case 3: parameter = vector3( s.parameter.x, s.parameter.y, 1 ); break;
			case 4: parameter = vector3( 1 - s.parameter.x, s.parameter.x, s.parameter.y ); break;
			}
			break;
		case TriangularPyramid://0<x,0<y,0<z,x+y+z<1
			switch ( index ) {
			case 0: parameter = vector3( 0, s.parameter.x, s.parameter.y ); break;
			case 1: parameter = vector3( s.parameter.y, 0, s.parameter.x ); break;
			case 2: parameter = vector3( s.parameter.x, s.parameter.y, 0 ); break;
			case 3: parameter = vector3( 1 - s.parameter.x - s.parameter.y, s.parameter.x, s.parameter.y ); break;
			}
			break;
		}
	}
private:
	bool point() const
	{
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = direction_z;
		//長さがあるか
		if ( a != vector3::zero ) return false;
		if ( b != vector3::zero ) return false;
		if ( c != vector3::zero ) return false;
		return true;
	}
	bool line_x() const
	{
		return ( direction_x != vector3::zero );
	}
	bool line_y() const
	{
		return ( direction_y != vector3::zero );
	}
	bool line_z() const
	{
		return ( direction_z != vector3::zero );
	}
	bool line() const
	{
		//平面であるか
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = direction_z;
		if ( absolute( vector3::dot3( a, a ) * vector3::dot3( b, b ) - square( vector3::dot3( a, b ) ) ) > epsilon ) return false;
		if ( absolute( vector3::dot3( b, b ) * vector3::dot3( c, c ) - square( vector3::dot3( b, c ) ) ) > epsilon ) return false;
		if ( absolute( vector3::dot3( c, c ) * vector3::dot3( a, a ) - square( vector3::dot3( c, a ) ) ) > epsilon ) return false;
		return true;
	}
	bool plane_xy() const
	{
		//平面であるか
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		return ( absolute( vector3::dot3( a, a ) * vector3::dot3( b, b ) - square( vector3::dot3( a, b ) ) ) > epsilon );
	}
	bool plane_yz() const
	{
		//平面であるか
		const vector3 & a = direction_y;
		const vector3 & b = direction_z;
		return ( absolute( vector3::dot3( a, a ) * vector3::dot3( b, b ) - square( vector3::dot3( a, b ) ) ) > epsilon );
	}
	bool plane_zx() const
	{
		//平面であるか
		const vector3 & a = direction_z;
		const vector3 & b = direction_x;
		return ( absolute( vector3::dot3( a, a ) * vector3::dot3( b, b ) - square( vector3::dot3( a, b ) ) ) > epsilon );
	}
	bool plane() const
	{
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = direction_z;
		matrix33 m( a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z );
		//空間であるか
		if ( absolute( matrix33::determinant( m ) ) > epsilon ) return false;
		return true;
	}
	bool volume() const
	{
		if ( point() ) return false;
		if ( line() ) return false;
		if ( plane() ) return false;
		return true;
	}
	geometricalLine line_component()
	{
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = direction_z;
		const vector3 & d = position;
		if ( line_x() ) {
			return geometricalLine( d, a );
		} else if ( line_y() ) {
			return geometricalLine( d, b );
		} else {
			return geometricalLine( d, c );
		}
	}
	void line_component( const geometricalLine & l )
	{
		if ( line_x() ) {
			parameter = vector3( l.parameter, 0, 0 );
		} else if ( line_y() ) {
			parameter = vector3( 0, l.parameter, 0 );
		} else {
			parameter = vector3( 0, 0, l.parameter );
		}
	}
	geometricalSurface surface_component()
	{
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = direction_z;
		const vector3 & d = position;
		if ( plane_xy() ) {
			return geometricalSurface( d, a, b );
		} else if ( plane_yz() ) {
			return geometricalSurface( d, b, c );
		} else {
			return geometricalSurface( d, c, a );
		}
	}
	void surface_component( const geometricalSurface & s )
	{
		if ( plane_xy() ) {
			parameter = vector3( s.parameter.x, s.parameter.y, 0 );
		} else if ( plane_yz() ) {
			parameter = vector3( 0, s.parameter.x, s.parameter.y );
		} else {
			parameter = vector3( s.parameter.y, 0, s.parameter.x );
		}
	}
public:
	const decimal distance_unlimited( const geometricalPoint & t )
	{
		if ( point() ) {
			parameter = vector3::zero;
			geometricalPoint p( position );
			return t.distance_unlimited( p );
		} else if ( line() ) {
			geometricalLine l = line_component();
			decimal dis = t.distance_unlimited( l );
			line_component( l );
			return dis;
		} else if ( plane() ) {
			geometricalSurface s = surface_component();
			decimal dis = t.distance_unlimited( s );
			surface_component( s );
			return dis;
		}
		const vector3 & a = direction_x;
		const vector3 & b = direction_y;
		const vector3 & c = direction_z;
		const vector3 & d = position;
		const vector3 & u = t.position;
		// 位置pになるようにパラメータを計算
		//S:ax+by+cz+d
		//ax+by+cz=u-d
		//a,b,c,d,uを横ベクトルとすると
		//(x,y,z)(a)=u-d
		//       (b)
		//       (c)
		matrix33 m( a.x, a.y, a.z, b.x, b.y, b.z, c.x, c.y, c.z );
		parameter = ( u - d ) * matrix33::inverse( m );
		return 0;
	}
	const decimal distance_unlimited( geometricalLine & t )
	{
		if ( point() ) {
			parameter = vector3::zero;
			geometricalPoint p( position );
			return t.distance_unlimited( p );
		} else if ( line() ) {
			geometricalLine l = line_component();
			decimal dis = t.distance_unlimited( l );
			line_component( l );
			return dis;
		} else if ( plane() ) {
			geometricalSurface s = surface_component();
			decimal dis = t.distance_unlimited( s );
			surface_component( s );
			return dis;
		}
		t.parameter = 0;
		geometricalPoint p( t.position );
		return distance_unlimited( p );
	}
	const decimal distance_unlimited( geometricalSurface & t )
	{
		if ( point() ) {
			parameter = vector3::zero;
			geometricalPoint p( position );
			return t.distance_unlimited( p );
		} else if ( line() ) {
			geometricalLine l = line_component();
			decimal dis = t.distance_unlimited( l );
			line_component( l );
			return dis;
		} else if ( plane() ) {
			geometricalSurface s = surface_component();
			decimal dis = t.distance_unlimited( s );
			surface_component( s );
			return dis;
		}
		t.parameter = vector2::zero;
		geometricalPoint p( t.position );
		return this->distance_unlimited( p );
	}
	const decimal distance_unlimited( geometricalVolume & t )
	{
		if ( point() ) {
			parameter = vector3::zero;
			geometricalPoint p( position );
			return t.distance_unlimited( p );
		} else if ( line() ) {
			geometricalLine l = line_component();
			decimal dis = t.distance_unlimited( l );
			line_component( l );
			return dis;
		} else if ( plane() ) {
			geometricalSurface s = surface_component();
			decimal dis = t.distance_unlimited( s );
			surface_component( s );
			return dis;
		}
		t.parameter = vector3::zero;
		geometricalPoint p( t.position );
		return this->distance_unlimited( p );
	}
};

#ifdef __GLOBAL__

const decimal geometricalPoint::distance_unlimited( geometricalLine & l ) const
{
	return l.distance_unlimited( *this );
}
const decimal geometricalPoint::distance_unlimited( geometricalSurface & s ) const
{
	return s.distance_unlimited( *this );
}
const decimal geometricalPoint::distance_unlimited( geometricalVolume & v ) const
{
	return v.distance_unlimited( *this );
}
const decimal geometricalLine::distance_unlimited( geometricalSurface & s )
{
	return s.distance_unlimited( *this );
}
const decimal geometricalLine::distance_unlimited( geometricalVolume & v )
{
	return v.distance_unlimited( *this );
}
const decimal geometricalSurface::distance_unlimited( geometricalVolume & v )
{
	return v.distance_unlimited( *this );
}

#endif

template<class T1, class T2>
decimal distance( T1 & g1, T2 & g2 )
{
	//制限無しの場合について最短を求める
	const decimal uld = g1.distance_unlimited( g2 );
	//有効範囲内ならば、それを用いる
	if ( g1.valid() && g2.valid() ) return uld;
	//形状を分解する
	array<T1::componentType> c1; g1.component( c1 );
	array<T2::componentType> c2; g2.component( c2 );
	//パラメータを其々保存するために、テンポラリを作成
	array<T1> p1; p1.allocate( c2.size );
	for ( array<T1>::iterator it( p1 ); it; ++it ) it() = g1;
	array<T2> p2; p2.allocate( c1.size );
	for ( array<T2>::iterator it( p2 ); it; ++it ) it() = g2;
	//最短の組み合わせを探す
	checkMinimum<decimal> minimumDistance;
	int index = 0;
	if ( c1.size ) {
		array<T1::componentType>::iterator it1( c1 );
		array<T2>::iterator it2( p2 );
		for ( ; it1; ++it1 ) {
			minimumDistance( distance( it1(), it2() ), index++ );
		}
	}
	if ( c2.size ) {
		array<T1>::iterator it1( p1 );
		array<T2::componentType>::iterator it2( c2 );
		for ( ; it2; ++it2 ) {
			minimumDistance( distance( it1(), it2() ), index++ );
		}
	}
	//両方とも、構成するものが無い＝点同士の場合には、ここには来ないが、バグ防止のためのチェック
	if ( ! minimumDistance ) return infinity;
	//最短の組み合わせのインデックスを計算
	index = minimumDistance.sub;
	//組み合わせから、パラメータを決定
	if ( index < c1.size ) {
		g1.component( index, c1[index] );
		g2 = p2[index];
	} else {
		index -= c1.size;
		g1 = p1[index];
		g2.component( index, c2[index] );
	}
	//最短距離を返す
	if ( minimumDistance() < epsilon ) return 0;
	return minimumDistance();
}
//線の中点
inline vector3 centerLines( geometricalLine & l1, geometricalLine & l2 )
{
	distance( l1, l2 );
	return ( l1() + l2() ) / 2.0;
}
