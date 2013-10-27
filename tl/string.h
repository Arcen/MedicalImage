////////////////////////////////////////////////////////////////////////////////
// String

//以下のファイルにstring =<data.data>を追加すると文字列が見える
//C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\AUTOEXP.DAT
class string
{
	array<char> data;
public:
	string( const char *src = "" ) { * this = src; }
	string( const string & src ) { * this = src; }
	string( const int & src ) { print( "%d", src ); }
	string( const long double & src ) { print( ( src && -1e-2 < src && src < 1e-2 ) ? "%.16e" : "%f", ( double ) src ); }
	string( const double & src ) { print( ( src && -1e-2 < src && src < 1e-2 ) ? "%.12e" : "%f", src ); }
	string( const float & src ) { print( ( src && -1e-2 < src && src < 1e-2 ) ? "%.10e" : "%f", src ); }
	void reserve( int len )
	{
		data.reserve( len + 1 );
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
		if( ! src ) return *this = "";
		if ( src == data.data ) return *this;
		int len = strlen( src );
		if ( between<const char *>( data.data, src, data.data + len ) ) return *this = string( src );
		data.allocate( len + 1 );
		strcpy( data.data, src );
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
	string operator+( const string &src ) const { return *this + ( const char * ) src; }
	string & operator+=( char src )
	{
		data.push_back( '\0' );
		data.prelast() = src;
		return *this;
	}
	string & operator+=( const char *src ) { return *this = *this + src; }
	string & operator+=( const string & src ) { return *this = *this + ( const char * ) src; }

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
	string operator-( const string &src ) const { return *this - ( const char * ) src; }
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
	string & operator-=( const string & src ) { return *this = *this - ( const char * ) src; }
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
	int toi() const { if ( *this == "" ) return 0; return atoi( data ); }
	double tof() const { if ( *this == "" ) return 0; return atof( data ); }
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
	static bool isnumber( char c, const int basis )
	{
		if ( basis == 10 ) return isinteger( c );
		if ( basis == 8 ) return isoctal( c );
		if ( basis == 16 ) return ishexadecimal( c );
		if ( basis < 10 ) {
			if ( between<int>( '0', c, '9' - ( 10 - basis ) ) ) return true;
		}
		if ( basis > 36 ) return false;
		if ( isinteger( c ) ) return true;
		if ( between<int>( 'a', c, 'z' - ( 35 - basis ) ) ||
			between<int>( 'A', c, 'Z' - ( 35 - basis ) ) ) return true;
		return false;
	}
	static int number( char c )
	{
		if ( isinteger( c ) ) return c - '0';
		if ( between( 'a', c, 'z' ) ) return c - 'a' + 10;
		if ( between( 'A', c, 'Z' ) ) return c - 'A' + 10;
		return 0;
	}
	static bool ismultibyte( const char c )
	{
		return between<unsigned int>( 0x81, static_cast<unsigned char>( c ), 0x9F ) || 
			between<unsigned int>( 0xE0, static_cast<unsigned char>( c ), 0xFC );
	}
	static bool isupper( const char c )
	{
		return between( 'A', c, 'Z' );
	}
	static bool islower( const char c )
	{
		return between( 'a', c, 'z' );
	}
	void toupper()
	{
		const char * const sentinel = data.data + maximum( 0, data.size - 1 );
		for ( char * p = data.data; p != sentinel; ++p ) {
			if ( ismultibyte( *p ) ) {
				++p;
			} else if ( islower( *p ) ) {
				*p = *p - 'a' + 'A';
			}
		}
	}
	void tolower()
	{
		const char * const sentinel = data.data + maximum( 0, data.size - 1 );
		for ( char * p = data.data; p != sentinel; ++p ) {
			if ( ismultibyte( *p ) ) {
				++p;
			} else if ( isupper( *p ) ) {
				*p = *p - 'A' + 'a';
			}
		}
	}
	bool isinteger() const
	{
		if ( data.size <= 1 ) return false;
		const char * const sentinel = data.data + maximum( 0, data.size - 1 );
		for ( const char * p = data.data; p != sentinel; ++p ) {
			if ( ! isinteger( *p ) ) return false;
		}
		return true;
	}
private:
	// [+-][0-z]*{[.][0-z]*}{[eE][0-9]*}
	template<class T>
	static bool scan_number( T & a, const char * & b, int width, const int basis, bool enablesign, bool enabledecimal )
	{
		if ( 0 == width || *b == '\0' ) return false;
		a = 0;
		int sign = 1;
		if ( enablesign ) {
			if ( *b && issign( *b ) ) {
				sign = ( *b == '-' ? -1 : 1 );
				b++;
				if ( 0 < width && 0 == --width ) return false;
			}
		}
		while ( *b && isnumber( *b, basis ) ) {
			a = a * basis + number( *b );
			b++;
			if ( 0 < width && 0 == --width ) break;
		}
		if ( enabledecimal ) {
			if ( *b == '.' ) {
				T base = T( 1 );
				b++;
				if ( 0 < width && 0 == --width ) return true;
				while ( *b && isnumber( *b, basis ) ) {
					base /= T( basis );
					a += number( *b ) * base;
					b++;
					if ( 0 < width && 0 == --width ) return true;
				}
			}
			a *= sign;
			if ( basis == 10 && ( *b == 'e' || *b == 'E' ) ) {
				b++;
				int index = 0;
				if ( ! scan_number( index, b, width, 10, true, false ) ) return true;
				if ( index < 0 ) {
					index = -index;
					for ( int i = 0; i < index; i++ ) a /= T( 10 );
				} else {
					for ( int i = 0; i < index; i++ ) a *= T( 10 );
				}
			}
		} else {
			a *= sign;
		}
		return true;
	}
	template<class T>
	static bool scan_d( T & a, const char * & b, int width )
	{
		return scan_number( a, b, width, 10, true, false );
	}
	template<class T>
	static bool scan_o( T & a, const char * & b, int width )
	{
		return scan_number( a, b, width, 8, true, false );
	}
	template<class T>
	static bool scan_x( T & a, const char * & b, int width )
	{
		return scan_number( a, b, width, 16, true, false );
	}
	template<class T>
	static bool scan_i( T & a, const char * & b, int width )
	{
		if ( 0 == width || *b == '\0' ) return false;
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
		return scan_number( a, b, width, 10, false, false );
	}
	template<class T>
	static bool scan_f( T & a, const char * & b, int width )
	{
		return scan_number( a, b, width, 10, true, true );
	}
	static bool scan_c( char & a, const char * & b, int width )
	{
		if ( 0 == width || *b == '\0' ) return false;
		a = *b++;
		return true;
	}
	static bool scan_s( char * & a, const char * & b, int width )
	{
		if ( 0 == width || *b == '\0' ) return false;
		while ( ! isspace( *b ) && *b != '\0' ) {
			if ( a ) { *a++ = *b++; } else { b++; }
			if ( 0 < width && 0 == --width ) break;
		}
		if ( a ) *a = '\0';
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
				if ( *b == '\0' ) throw true;
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
										default: { float * a = va_arg( args, float* ); if ( ! scan_f( *a, b, width ) ) throw false; } break;
										}
									}
								} else if ( *prefix == 'I' ) {
									if ( ignore ) {
										switch ( *type ) {
										case 'd': { int64 a; if ( ! scan_d( a, b, width ) ) throw false; } break;
										case 'x': case 'X': { int64 a; if ( ! scan_x( a, b, width ) ) throw false; } break;
										case 'o': { int64 a; if ( ! scan_o( a, b, width ) ) throw false; } break;
										case 'i': { int64 a; if ( ! scan_i( a, b, width ) ) throw false; } break;
										case 'u': { uint64 a; if ( ! scan_u( a, b, width ) ) throw false; } break;
										case 'c': { char a; if ( ! scan_c( a, b, width ) ) throw false; } break;
										case 's': { char * a = NULL; if ( ! scan_s( a, b, width ) ) throw false; } break;
										default: { float a; if ( ! scan_f( a, b, width ) ) throw false; } break;
										}
									} else {
										switch ( *type ) {
										case 'd': { int64 * a = va_arg( args, int64* ); if ( ! scan_d( *a, b, width ) ) throw false; } break;
										case 'x': case 'X': { int64 * a = va_arg( args, int64* ); if ( ! scan_x( *a, b, width ) ) throw false; } break;
										case 'o': { int64 * a = va_arg( args, int64* ); if ( ! scan_o( *a, b, width ) ) throw false; } break;
										case 'i': { int64 * a = va_arg( args, int64* ); if ( ! scan_i( *a, b, width ) ) throw false; } break;
										case 'u': { uint64 * a = va_arg( args, uint64* ); if ( ! scan_u( *a, b, width ) ) throw false; } break;
										case 'c': { char * a = va_arg( args, char* ); if ( ! scan_c( *a, b, width ) ) throw false; } break;
										case 's': { char * a = va_arg( args, char* ); if ( ! scan_s( a, b, width ) ) throw false; } break;
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
									default: { float * a = va_arg( args, float* ); if ( ! scan_f( *a, b, width ) ) throw false; } break;
									}
								}
							}
						} else {
							if ( *f == '\0' ) throw false;
							if ( *f == '[' ) {//%[？]で括られた文字列とバッファを比較する
								++f;
								while ( *f != ']' && f != '\0' && *b ) {
									if ( *f++ != *b++ ) throw false;
								}
								if ( *f == ']' ) ++f;
							} else {//%？%で括られた文字列とバッファを比較する
								if ( *f != *b++ ) throw false;
								while ( *f != '%' && f != '\0' && *b ) {
									if ( *f++ != *b++ ) throw false;
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
		} catch ( bool result ) {
			r = result;
		}
		va_end( args );
		return r ? reinterpret_cast<unsigned int>( b ) - reinterpret_cast<unsigned int>( data.data ) : -1;
	}
};

#undef FOR_DEBUG
