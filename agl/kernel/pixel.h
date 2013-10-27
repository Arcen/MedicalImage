//TŒ^‚Ì‰æ‘f
template<class T>
class pixelT
{
public:
	T r, g, b, a;
	typedef T element;
	enum {
		band = sizeof(T), //Œ^‚ÌƒTƒCƒY
	};
	pixelT( const T & _r = 0, const T & _g = 0, const T & _b = 0, const T & _a = T( type_maximum( (const T*)NULL ) ) ) : r( _r ), g( _g ), b( _b ), a( _a ) {}
	pixelT( const pixelT & p ) : r( p.r ), g( p.g ), b( p.b ), a( p.a ) {}
	pixelT & operator=( const pixelT & p ){ r = p.r; g = p.g; b = p.b; a = p.a; return *this; }
	template<class U>
	void convert( const pixelT<U> & src )
	{
		RGBA( src.R(), src.G(), src.B(), src.A() );
	}
	decimal R() const { return clamp<decimal>( 0, ( decimal( r ) - type_minimum( (const T*)NULL ) ) / type_range( (const T*)NULL ), 1 ); }
	decimal G() const { return clamp<decimal>( 0, ( decimal( g ) - type_minimum( (const T*)NULL ) ) / type_range( (const T*)NULL ), 1 ); }
	decimal B() const { return clamp<decimal>( 0, ( decimal( b ) - type_minimum( (const T*)NULL ) ) / type_range( (const T*)NULL ), 1 ); }
	decimal A() const { return clamp<decimal>( 0, ( decimal( a ) - type_minimum( (const T*)NULL ) ) / type_range( (const T*)NULL ), 1 ); }

	void R( decimal _r ){ r = T( clamp<decimal>( 0, _r, 1 ) * type_range( (const T*)NULL ) + type_minimum( (const T*)NULL ) ); }
	void G( decimal _g ){ g = T( clamp<decimal>( 0, _g, 1 ) * type_range( (const T*)NULL ) + type_minimum( (const T*)NULL ) ); }
	void B( decimal _b ){ b = T( clamp<decimal>( 0, _b, 1 ) * type_range( (const T*)NULL ) + type_minimum( (const T*)NULL ) ); }
	void A( decimal _a ){ a = T( clamp<decimal>( 0, _a, 1 ) * type_range( (const T*)NULL ) + type_minimum( (const T*)NULL ) ); }

	void RGBA( decimal _r, decimal _g, decimal _b, decimal _a ){ R( _r ); G( _g ), B( _b ); A( _a ); }
	pixelT opacity() const { return pixelT( r, g, b, type_maximum( (const T*)NULL ) ); }
	pixelT semitransparent( T _a ) const { return pixelT( r, g, b, _a ); }
};

typedef pixelT<uint8>	pixelT8;
typedef pixelT<uint16>	pixelT16;
typedef pixelT<uint32>	pixelT32;

//‚q‚f‚a‚`Œ^
template<class T> class pixelRedGreenBlueAlpha
{
public:
	T r, g, b, a;
	typedef T element;
	enum {
		band = pixelT<T>::band, 
		channel = 4, dot = band * channel, color = 1, alpha = 1
	};

	pixelRedGreenBlueAlpha( const T & _r = 0, const T & _g = 0, const T & _b = 0, const T & _a = T( type_maximum( (const T*)NULL ) ) ) : r( _r ), g( _g ), b( _b ), a( _a ) {}
	pixelRedGreenBlueAlpha & operator=( const pixelRedGreenBlueAlpha & p ){ r = p.r; g = p.g; b = p.b; a = p.a; return *this; }

	pixelRedGreenBlueAlpha( const pixelT<T> & p ){ *this = p; }
	pixelRedGreenBlueAlpha & operator=( const pixelT<T> & p ){ r = p.r; g = p.g; b = p.b; a = p.a; return *this; }
	operator const pixelT<T> () const { return pixelT<T>( r, g, b, a ); }
	bool operator==( const pixelRedGreenBlueAlpha & src ) const { return r == src.r && g == src.g && b == src.b && a == src.a; }
	bool operator!=( const pixelRedGreenBlueAlpha & src ) const { return r != src.r || g != src.g || b != src.b || a != src.a; }
	pixelRedGreenBlueAlpha opacity() const { return pixelRedGreenBlueAlpha( r, g, b, type_maximum( (const T*)NULL ) ); }
	pixelRedGreenBlueAlpha semitransparent( T _a ) const { return pixelRedGreenBlueAlpha( r, g, b, _a ); }
};

//‚a‚f‚q‚`Œ^
template<class T> class pixelBlueGreenRedAlpha
{
public:
	T b, g, r, a;
	typedef T element;
	enum {
		band = pixelT<T>::band, 
		channel = 4, dot = band * channel, color = 1, alpha = 1
	};

	pixelBlueGreenRedAlpha( const T & _r = 0, const T & _g = 0, const T & _b = 0, const T & _a = T( type_maximum( (const T*)NULL ) ) ) : r( _r ), g( _g ), b( _b ), a( _a ) {}
	pixelBlueGreenRedAlpha & operator=( const pixelBlueGreenRedAlpha & p ){ r = p.r; g = p.g; b = p.b; a = p.a; return *this; }

	pixelBlueGreenRedAlpha( const pixelT<T> & p ){ *this = p; }
	pixelBlueGreenRedAlpha & operator=( const pixelT<T> & p ){ r = p.r; g = p.g; b = p.b; a = p.a; return *this; }
	operator const pixelT<T> () const { return pixelT<T>( r, g, b, a ); }
	bool operator==( const pixelBlueGreenRedAlpha & src ) const { return r == src.r && g == src.g && b == src.b && a == src.a; }
	bool operator!=( const pixelBlueGreenRedAlpha & src ) const { return r != src.r || g != src.g || b != src.b || a != src.a; }
	pixelBlueGreenRedAlpha opacity() const { return pixelBlueGreenRedAlpha( r, g, b, type_maximum( (const T*)NULL ) ); }
	pixelBlueGreenRedAlpha semitransparent( T _a ) const { return pixelBlueGreenRedAlpha( r, g, b, _a ); }
};

//‚q‚f‚aŒ^
template<class T> class pixelRedGreenBlue
{
public:
	T r, g, b;
	typedef T element;
	enum {
		band = pixelT<T>::band, 
		channel = 3, dot = band * channel, color = 1, alpha = 0
	};

	pixelRedGreenBlue( const T & _r = 0, const T & _g = 0, const T & _b = 0, const T & _a = T( type_maximum( (const T*)NULL ) ) ) : r( _r ), g( _g ), b( _b ) {}
	pixelRedGreenBlue & operator=( const pixelRedGreenBlue & p ){ r = p.r; g = p.g; b = p.b; return *this; }

	pixelRedGreenBlue( const pixelT<T> & p ){ *this = p; }
	pixelRedGreenBlue & operator=( const pixelT<T> & p ){ r = p.r; g = p.g; b = p.b; return *this; }
	operator const pixelT<T> () const { return pixelT<T>( r, g, b, T( type_maximum( (const T*)NULL ) ) ); }
	bool operator==( const pixelRedGreenBlue & src ) const { return r == src.r && g == src.g && b == src.b; }
	bool operator!=( const pixelRedGreenBlue & src ) const { return r != src.r || g != src.g || b != src.b; }
};

//‚a‚f‚qŒ^
template<class T> class pixelBlueGreenRed
{
public:
	T b, g, r;
	typedef T element;
	enum {
		band = pixelT<T>::band, 
		channel = 3, dot = band * channel, color = 1, alpha = 0
	};

	pixelBlueGreenRed( const T & _r = 0, const T & _g = 0, const T & _b = 0, const T & _a = T( type_maximum( (const T*)NULL ) ) ) : r( _r ), g( _g ), b( _b ) {}
	pixelBlueGreenRed & operator=( const pixelBlueGreenRed & p ){ r = p.r; g = p.g; b = p.b; return *this; }

	pixelBlueGreenRed( const pixelT<T> & p ){ *this = p; }
	pixelBlueGreenRed & operator=( const pixelT<T> & p ){ r = p.r; g = p.g; b = p.b; return *this; }
	operator const pixelT<T> () const { return pixelT<T>( r, g, b, T( type_maximum( (const T*)NULL ) ) ); }
	bool operator==( const pixelBlueGreenRed & src ) const { return r == src.r && g == src.g && b == src.b; }
	bool operator!=( const pixelBlueGreenRed & src ) const { return r != src.r || g != src.g || b != src.b; }
};

//‚x‚`Œ^
template<class T> class pixelLuminanceAlpha
{
public:
	T y, a;
	typedef T element;
	enum {
		band = pixelT<T>::band, 
		channel = 2, dot = band * channel, color = 0, alpha = 1
	};

	pixelLuminanceAlpha( const T & _r = 0, const T & _g = 0, const T & _b = 0, const T & _a = T( type_maximum( (const T*)NULL ) ) ) : y( _r ), a( _a ) {}
	pixelLuminanceAlpha & operator=( const pixelLuminanceAlpha & p ){ y = p.y; a = p.a; return *this; }

	pixelLuminanceAlpha( const pixelT<T> & p ){ *this = p; }
	pixelLuminanceAlpha & operator=( const pixelT<T> & p ){ y = p.r; a = p.a; return *this; }
	operator const pixelT<T> () const { return pixelT<T>( y, y, y, a ); }
	bool operator==( const pixelLuminanceAlpha & src ) const { return y == src.y && a == src.a; }
	bool operator!=( const pixelLuminanceAlpha & src ) const { return y != src.y || a != src.a; }
	pixelLuminanceAlpha opacity() const { return pixelLuminanceAlpha( y, y, y, type_maximum( (const T*)NULL ) ); }
	pixelLuminanceAlpha semitransparent( T _a ) const { return pixelLuminanceAlpha( y, y, y, _a ); }
};

//‚xŒ^
template<class T> class pixelLuminance
{
public:
	T y;
	typedef T element;
	enum {
		band = pixelT<T>::band, 
		channel = 1, dot = band * channel, color = 0, alpha = 0
	};

	pixelLuminance( const T & _r = 0, const T & _g = 0, const T & _b = 0, const T & _a = T( type_maximum( (const T*)NULL ) ) ) : y( _r ) {}
	pixelLuminance & operator=( const pixelLuminance & p ){ y = p.y; return *this; }

	pixelLuminance( const pixelT<T> & p ){ *this = p; }
	pixelLuminance & operator=( const pixelT<T> & p ){ y = p.r; return *this; }
	operator const pixelT<T> () const { return pixelT<T>( y, y, y ); }
	bool operator==( const pixelLuminance & src ) const { return y == src.y; }
	bool operator!=( const pixelLuminance & src ) const { return y != src.y; }
};

typedef pixelRedGreenBlueAlpha<uint8> pixelRGBA;
typedef pixelBlueGreenRedAlpha<uint8> pixelBGRA;
typedef pixelRedGreenBlue<uint8> pixelRGB;
typedef pixelBlueGreenRed<uint8> pixelBGR;
typedef pixelLuminance<uint8> pixelY;
typedef pixelLuminance<uint16> pixelY2;

typedef pixelBGRA	pixel;
