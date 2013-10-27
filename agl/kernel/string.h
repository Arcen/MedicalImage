////////////////////////////////////////////////////////////////////////////////
// String

class string
{
#ifndef NDEBUG
	const char * & raw;//デバッグ時に文字列を見やすくするためだけのポインタ変数
#define FOR_DEBUG	: raw( data.data )
#else
#define FOR_DEBUG
#endif

	array<char> data;
public:
	string( const char *src = "" ) FOR_DEBUG { * this = src; }
	string( const string & src ) FOR_DEBUG { * this = src; }
	string( const int & src ) FOR_DEBUG
	{
		print( "%d", src );
	}
	string( const double & src ) FOR_DEBUG
	{
		print( "%f", src );
	}
	string( const float & src ) FOR_DEBUG
	{
		print( "%f", src );
	}
	static string identity;
	void ready( int len )
	{
		data.ready( len + 1 );
		data.allocate( 1 );
		data[0] = '\0';
	}
	void allocate( int len )
	{
		data.allocate( len + 1 );
		data[0] = '\0';
	}
	void release()
	{
		*this = "";
	}
	string & operator=( const char *src )
	{
		if( src ) {
			if ( src == data ) return *this;
			int len = strlen( src );
			if ( between<const char *>( data.data, src, data.data + len ) ) return *this = string( src );
			data.allocate( len + 1 );
			strcpy( data, src );
		} else {
			return *this = "";
		}
		return *this;
	}
	string & operator=( const string &src )
	{
		return *this = ( const char * ) src;
	}
	string operator+( char src ) const { return string( *this ) += src; }
	string operator+( const char *src ) const
	{
		string dst;
		dst.data.allocate( length() + strlen( src ) + 1 );
		strcpy( dst.data, data );
		strcat( dst.data, src );
		return dst;
	}
	string operator+( const string &src ) const 
	{
		return *this+( const char * ) src;
	}
	string & operator+=( char src )
	{
		data.push_back( '\0' );
		data.prelast() = src;
		return *this;
	}
	string & operator+=( const char *src ) { return *this = *this + src; }
	string & operator+=( const string & src ) { return *this = *this + src; }

	string operator-( char src ) const { return string( *this ) -= src; }
	string operator-( const char *src ) const
	{
		string dst( *this );
		int srclen = strlen( src );
		int dstlen = dst.length();
		if ( srclen <= dstlen ) {
			if ( strcmp( dst.data.data + dstlen - srclen, src ) == 0 ) {
				dst.data[dstlen - srclen] = '\0';
				string wk( dst.data );
				dst = wk;
			}
		}
		return dst;
	}
	string operator-( const string &src ) const 
	{
		return *this-( const char * ) src;
	}
	string & operator-=( char src )
	{
		if ( 2 < data.size && data.prelast() == src ) {
			data.prelast() = '\0';
			string wk = data;
			*this = wk;
		}
		return *this;
	}
	string & operator-=( const char *src ) { return *this = *this - src; }
	string & operator-=( const string & src ) { return *this = *this - src; }
	bool operator==( const char *src ) const { return strcmp( data, src ) == 0 ? true : false; }
	bool operator==( const string &src ) const { return *this == ( const char * ) src; }
	bool operator!=( const char *src ) const { return ! ( *this == src ); }
	bool operator!=( const string &src ) const { return ! ( *this == src ); }
	bool operator<( const char * src ) const { return strcmp( data, src ) < 0 ? true : false; }
	bool operator>( const char * src ) const { return strcmp( data, src ) > 0 ? true : false; }
	bool operator<=( const char * src ) const { return strcmp( data, src ) <= 0 ? true : false; }
	bool operator>=( const char * src ) const { return strcmp( data, src ) >= 0 ? true : false; }
	int length() const { return strlen( data ); }
	operator const char*() const { return data.data; }
	operator char*() { return data.data; }
	const char* chars() const { return data.data; }
	char* chars() { return data.data; }
	string replace( const char * const findString, const char replaceChar )
	{
		string result( *this );
		for ( const char * f = findString; *f; f++ ) {
			for ( char * p = result; *p; p++ ) {
				if ( *p == *f ) {
					*p = replaceChar;
				}
			}
		}
		return result;
	}
	bool reverseCompare( const char * target )
	{
		int myLength = length();
		int targetLength = strlen( target );
		if ( myLength < targetLength ) return false;
		return strcmp( target, data.data + myLength - targetLength ) == 0;
	}
	int toi() const { return atoi( data ); }
	float tof() const { return atof( data ); }
	void print( char *fmt, ... )
	{
		string wk;
		wk.allocate( maximum<unsigned int>( strlen( fmt ) * 10, 1024 ) );
		va_list args;
		va_start( args, fmt );
		int r = _vsnprintf( ( char * ) wk, wk.data.size - 1, fmt, args );
		while ( r < 0 ) {
			wk.allocate( wk.data.size * 2 );
			r = _vsnprintf( ( char * ) wk, wk.data.size - 1, fmt, args );
		}
		va_end( args );
		*this = wk;
	}
	static bool issign( char c )
	{
		if ( c == '-' || c == '+' ) return true;
		return false;
	}
	static bool isinteger( char c )
	{
		if ( between( '0', c, '9' ) ) return true;
		return false;
	}
	static bool isdecimal( char c )
	{
		if ( between( '0', c, '9' ) || c == '.' || c == 'e' || c == 'E' || c == '-' ) return true;
		return false;
	}
	static bool isoctal( char c )
	{
		if ( between( '0', c, '7' ) ) return true;
		return false;
	}
	static bool ishexadecimal( char c )
	{
		if ( between( '0', c, '9' ) || between( 'a', c, 'f' ) || between( 'A', c, 'F' ) ) return true;
		return false;
	}
	static int hexadecimal( char c )
	{
		if ( between( '0', c, '9' ) ) return c - '0';
		if ( between( 'a', c, 'f' ) ) return c - 'a' + 10;
		if ( between( 'A', c, 'F' ) ) return c - 'A' + 10;
		return 0;
	}
	private:
	template<class T>
	static bool scan_d( T & a, const char * & b, int width )
	{
		if ( 0 == width ) return false;
		a = 0;
		int sign = 1;
		if ( issign( *b ) ) {
			sign = ( *b == '-' ? -1 : 1 );
			b++;
			if ( 0 < width && 0 == --width ) return false;
		}
		if ( ! isinteger( *b ) ) return false;
		while ( isinteger( *b ) ) {
			a = a * 10 + ( *b - '0' );
			b++;
			if ( 0 < width && 0 == --width ) break;
		}
		a *= sign;
		return true;
	}
	template<class T>
	static bool scan_o( T & a, const char * & b, int width )
	{
		if ( 0 == width ) return false;
		a = 0;
		if ( ! isoctal( *b ) ) return false;
		while ( isoctal( *b ) ) {
			a = a * 8 + ( *b - '0' );
			b++;
			if ( 0 < width && 0 == --width ) break;
		}
		return true;
	}
	template<class T>
	static bool scan_x( T & a, const char * & b, int width )
	{
		if ( 0 == width ) return false;
		a = 0;
		if ( ! ishexadecimal( *b ) ) return false;
		while ( ishexadecimal( *b ) ) {
			a = a * 16 + hexadecimal( *b );
			b++;
			if ( 0 < width && 0 == --width ) break;
		}
		return true;
	}
	template<class T>
	static bool scan_i( T & a, const char * & b, int width )
	{
		if ( 0 == width ) return false;
		if ( *b == '0' && b[1] == 'x' ) {
			if ( 0 < width && width < 2 ) return false;
			b += 2;
			return scan_x( a, b, width - 2 );
		}
		if ( *b == '0' ) return scan_o( a, ++b, width - 1 );
		return scan_d( a, b, width );
	}
	template<class T>
	static bool scan_u( T & a, const char * & b, int width )
	{
		if ( 0 == width ) return false;
		a = 0;
		if ( ! isinteger( *b ) ) return false;
		while ( isinteger( *b ) ) {
			a = a * 10 + ( *b - '0' );
			b++;
			if ( 0 < width && 0 == --width ) break;
		}
		return true;
	}
	static bool scan_c( char & a, const char * & b, int width )
	{
		if ( 0 == width ) return false;
		a = *b++;
		return true;
	}
	static bool scan_s( char * & a, const char * & b, int width )
	{
		if ( 0 == width ) return false;
		while ( ! isspace( *b ) ) {
			if ( a ) { *a++ = *b++; } else { b++; }
			if ( 0 < width && 0 == --width ) break;
		}
		if ( a ) *a = '\0';
		return true;
	}
	// [+-][0-9]*{[.][0-9]*}{[eE][0-9]*}
	template<class T>
	static bool scan_f( T & a, const char * & b, int width )
	{
		a = 0;
		if ( 0 == width ) return false;
		int sign = 1;
		if ( issign( *b ) ) {
			sign = ( *b == '-' ? -1 : 1 );
			b++;
			if ( 0 < width && 0 == --width ) return false;
		}
		if ( ! isinteger( *b ) && *b != '.' ) return false;
		while ( isinteger( *b ) ) {
			a = a * T( 10 ) + ( *b - '0' );
			b++;
			if ( 0 < width && 0 == --width ) { a *= sign; return true; }
		}
		if ( *b == '.' ) {
			T base = T( 1 );
			b++;
			if ( 0 < width && 0 == --width ) { a *= sign; return true; }
			while ( isinteger( *b ) ) {
				base /= T( 10 );
				a += ( *b - '0' ) * base;
				b++;
				if ( 0 < width && 0 == --width ) { a *= sign; return true; }
			}
		}
		a *= sign;
		if ( *b == 'e' || *b == 'E' ) {
			b++;
			if ( 0 < width && 0 == --width ) return true;
			if ( *b == '-' ) {
				b++;
				if ( 0 < width && 0 == --width ) return true;
				int index = 0;
				while ( isinteger( *b ) ) {
					index = index * T( 10 ) + T( *b - '0' );
					b++;
					if ( 0 < width && 0 == --width ) break;
				}
				for ( int i = 0; i < index; i++ ) a /= T( 10 );
			} else {
				int index = 0;
				while ( isinteger( *b ) ) {
					index = index * T( 10 ) + T( *b - '0' );
					b++;
					if ( 0 < width && 0 == --width ) break;
				}
				for ( int i = 0; i < index; i++ ) a *= T( 10 );
			}
		}
		return true;
	}
	static bool scan_n( int & a, const char * b, const char * base )
	{
		a = int( b ) - int( base );
		return true;
	}
	public:
	//低機能版にするのでANSIとは非互換
	int scan( char *fmt, ... )
	{
		va_list args;
		va_start( args, fmt );
		const char * b = data.data;
		const char * f = fmt;
		bool r = true;
		try {
			for ( ; *f; ++f ) {
				switch ( *f ) {
				case ' ':
					while ( isspace( *b ) ) ++b;
					break;
				case '%':
					{
						++f;
						if ( *f == '\0' ) throw false;
						bool ignore = false;
						if ( *f == '*' ) { ignore = true; ++f; }
						int width = -1;
						if ( isinteger( *f ) ) {
							width = *f++ - '0';
							while ( isinteger( *f ) ) {
								width = width * 10 + *f++ - '0';
							}
						}
						const char * prefix = NULL;
						if ( *f == 'l' || *f == 'h' || *f == 'L' ) prefix = f++;
						else if ( *f == 'I' && f[1] == '6' && f[2] == '4' ) {
							prefix = f;
							f += 3;
						}
						const char * type = NULL;
						if ( *f == 'd' || *f == 'i' || *f == 'o' || *f == 'u' || *f == 'x' || *f == 'X' || 
							*f == 'e' || *f == 'E' || *f == 'f' || *f == 'g' || *f == 'G' ||
							*f == 'c' || *f == 's' || *f == 'n' ) type = f++;
						if ( type ) {
							while ( isspace( *b ) ) b++;
							if ( prefix ) {
								if ( *prefix == 'l' || *prefix == 'L' ) {
									if ( ignore ) {
										switch ( *type ) {
										case 'd': { long int a; if ( ! scan_d( a, b, width ) ) throw false; } break;
										case 'x': case 'X': { long int a; if ( ! scan_x( a, b, width ) ) throw false; } break;
										case 'o': { long int a; if ( ! scan_o( a, b, width ) ) throw false; } break;
										case 'i': { long int a; if ( ! scan_i( a, b, width ) ) throw false; } break;
										case 'u': { long unsigned int a; if ( ! scan_u( a, b, width ) ) throw false; } break;
										case 'c': { char a; if ( ! scan_c( a, b, width ) ) throw false; } break;
										case 's': { char * a = NULL; if ( ! scan_s( a, b, width ) ) throw false; } break;
										case 'n': { int a; if ( ! scan_n( a, b, data.data ) ) throw false; } break;
										default: { double a; if ( ! scan_f( a, b, width ) ) throw false; } break;
										}
									} else {
										switch ( *type ) {
										case 'd': { long int * a = va_arg( args, long int* ); if ( ! scan_d( *a, b, width ) ) throw false; } break;
										case 'x': case 'X': { long int * a = va_arg( args, long int* ); if ( ! scan_x( *a, b, width ) ) throw false; } break;
										case 'o': { long int * a = va_arg( args, long int* ); if ( ! scan_o( *a, b, width ) ) throw false; } break;
										case 'i': { long int * a = va_arg( args, long int* ); if ( ! scan_i( *a, b, width ) ) throw false; } break;
										case 'u': { long unsigned int * a = va_arg( args, long unsigned int* ); if ( ! scan_u( *a, b, width ) ) throw false; } break;
										case 'c': { char * a = va_arg( args, char* ); if ( ! scan_c( *a, b, width ) ) throw false; } break;
										case 's': { char * a = va_arg( args, char* ); if ( ! scan_s( a, b, width ) ) throw false; } break;
										case 'n': { int *a = va_arg( args, int* ); if ( ! scan_n( *a, b, data.data ) ) throw false; } break;
										default: { double * a = va_arg( args, double* ); if ( ! scan_f( *a, b, width ) ) throw false; } break;
										}
									}
								} else if ( *prefix == 'h' ) {
									if ( ignore ) {
										switch ( *type ) {
										case 'd': { short int a; if ( ! scan_d( a, b, width ) ) throw false; } break;
										case 'x': case 'X': { short int a; if ( ! scan_x( a, b, width ) ) throw false; } break;
										case 'o': { short int a; if ( ! scan_o( a, b, width ) ) throw false; } break;
										case 'i': { short int a; if ( ! scan_i( a, b, width ) ) throw false; } break;
										case 'u': { short unsigned int a; if ( ! scan_u( a, b, width ) ) throw false; } break;
										case 'c': { char a; if ( ! scan_c( a, b, width ) ) throw false; } break;
										case 's': { char * a = NULL; if ( ! scan_s( a, b, width ) ) throw false; } break;
										case 'n': { int a; if ( ! scan_n( a, b, data.data ) ) throw false; } break;
										default: { float a; if ( ! scan_f( a, b, width ) ) throw false; } break;
										}
									} else {
										switch ( *type ) {
										case 'd': { short int * a = va_arg( args, short int* ); if ( ! scan_d( *a, b, width ) ) throw false; } break;
										case 'x': case 'X': { short int * a = va_arg( args, short int* ); if ( ! scan_x( *a, b, width ) ) throw false; } break;
										case 'o': { short int * a = va_arg( args, short int* ); if ( ! scan_o( *a, b, width ) ) throw false; } break;
										case 'i': { short int * a = va_arg( args, short int* ); if ( ! scan_i( *a, b, width ) ) throw false; } break;
										case 'u': { short unsigned int * a = va_arg( args, short unsigned int* ); if ( ! scan_u( *a, b, width ) ) throw false; } break;
										case 'c': { char * a = va_arg( args, char* ); if ( ! scan_c( *a, b, width ) ) throw false; } break;
										case 's': { char * a = va_arg( args, char* ); if ( ! scan_s( a, b, width ) ) throw false; } break;
										case 'n': { int *a = va_arg( args, int* ); if ( ! scan_n( *a, b, data.data ) ) throw false; } break;
										default: { float * a = va_arg( args, float* ); if ( ! scan_f( *a, b, width ) ) throw false; } break;
										}
									}
								} else if ( *prefix == 'I' ) {
									if ( ignore ) {
										switch ( *type ) {
										case 'd': { _int64 a; if ( ! scan_d( a, b, width ) ) throw false; } break;
										case 'x': case 'X': { _int64 a; if ( ! scan_x( a, b, width ) ) throw false; } break;
										case 'o': { _int64 a; if ( ! scan_o( a, b, width ) ) throw false; } break;
										case 'i': { _int64 a; if ( ! scan_i( a, b, width ) ) throw false; } break;
										case 'u': { unsigned _int64 a; if ( ! scan_u( a, b, width ) ) throw false; } break;
										case 'c': { char a; if ( ! scan_c( a, b, width ) ) throw false; } break;
										case 's': { char * a = NULL; if ( ! scan_s( a, b, width ) ) throw false; } break;
										case 'n': { int a; if ( ! scan_n( a, b, data.data ) ) throw false; } break;
										default: { float a; if ( ! scan_f( a, b, width ) ) throw false; } break;
										}
									} else {
										switch ( *type ) {
										case 'd': { _int64 * a = va_arg( args, _int64* ); if ( ! scan_d( *a, b, width ) ) throw false; } break;
										case 'x': case 'X': { _int64 * a = va_arg( args, _int64* ); if ( ! scan_x( *a, b, width ) ) throw false; } break;
										case 'o': { _int64 * a = va_arg( args, _int64* ); if ( ! scan_o( *a, b, width ) ) throw false; } break;
										case 'i': { _int64 * a = va_arg( args, _int64* ); if ( ! scan_i( *a, b, width ) ) throw false; } break;
										case 'u': { unsigned _int64 * a = va_arg( args, unsigned _int64* ); if ( ! scan_u( *a, b, width ) ) throw false; } break;
										case 'c': { char * a = va_arg( args, char* ); if ( ! scan_c( *a, b, width ) ) throw false; } break;
										case 's': { char * a = va_arg( args, char* ); if ( ! scan_s( a, b, width ) ) throw false; } break;
										case 'n': { int *a = va_arg( args, int* ); if ( ! scan_n( *a, b, data.data ) ) throw false; } break;
										default: { float * a = va_arg( args, float* ); if ( ! scan_f( *a, b, width ) ) throw false; } break;
										}
									}
								}
							} else {
								if ( ignore ) {
									switch ( *type ) {
									case 'd': { int a; if ( ! scan_d( a, b, width ) ) throw false; } break;
									case 'x': case 'X': { int a; if ( ! scan_x( a, b, width ) ) throw false; } break;
									case 'o': { int a; if ( ! scan_o( a, b, width ) ) throw false; } break;
									case 'i': { int a; if ( ! scan_i( a, b, width ) ) throw false; } break;
									case 'u': { unsigned int a; if ( ! scan_u( a, b, width ) ) throw false; } break;
									case 'c': { char a; if ( ! scan_c( a, b, width ) ) throw false; } break;
									case 's': { char * a = NULL; if ( ! scan_s( a, b, width ) ) throw false; } break;
									case 'n': { int a; if ( ! scan_n( a, b, data.data ) ) throw false; } break;
									default: { float a; if ( ! scan_f( a, b, width ) ) throw false; } break;
									}
								} else {
									switch ( *type ) {
									case 'd': { int * a = va_arg( args, int* ); if ( ! scan_d( *a, b, width ) ) throw false; } break;
									case 'x': case 'X': { int * a = va_arg( args, int* ); if ( ! scan_x( *a, b, width ) ) throw false; } break;
									case 'o': { int * a = va_arg( args, int* ); if ( ! scan_o( *a, b, width ) ) throw false; } break;
									case 'i': { int * a = va_arg( args, int* ); if ( ! scan_i( *a, b, width ) ) throw false; } break;
									case 'u': { unsigned int * a = va_arg( args, unsigned int* ); if ( ! scan_u( *a, b, width ) ) throw false; } break;
									case 'c': { char * a = va_arg( args, char* ); if ( ! scan_c( *a, b, width ) ) throw false; } break;
									case 's': { char * a = va_arg( args, char* ); if ( ! scan_s( a, b, width ) ) throw false; } break;
									case 'n': { int *a = va_arg( args, int* ); if ( ! scan_n( *a, b, data.data ) ) throw false; } break;
									default: { float * a = va_arg( args, float* ); if ( ! scan_f( *a, b, width ) ) throw false; } break;
									}
								}
							}
						} else {
							if ( *f == '\0' ) throw false;
							if ( *f == '[' ) {
								++f;
								while ( *f != ']' && f != '\0' ) {
									if ( *f++ != *b++ ) return -1;
								}
								if ( *f == ']' ) ++f;
							} else {
								if ( *f != *b++ ) return false;
								while ( *f != '%' && f != '\0' ) {
									if ( *f++ != *b++ ) return -1;
								}
								if ( *f == '%' ) ++f;
							}
						}
					}
					break;
				default:
					{
						while ( isspace( *b ) ) ++b;
						if ( *f != *b ) throw false;
						++b;
					}
					break;
				}
			}
		} catch ( bool ) {
			r = false;
		}
		va_end( args );
		return r ? reinterpret_cast<unsigned int>( b ) - reinterpret_cast<unsigned int>( data.data ) : -1;
	}
};

#ifdef __GLOBAL__
string string::identity;
#endif

#undef FOR_DEBUG
