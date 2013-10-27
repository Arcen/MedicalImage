// ÉÅÉÇÉäÅ[ÇÃè[ìU
inline void memorySet( void * _dst, const int dst_length, const void * _src, const int src_length )
{
	unsigned char * src = ( unsigned char * ) _src, * dst = ( unsigned char * ) _dst;
	if ( dst_length == 0 || src_length == 0 || dst == NULL || src == NULL ) return;
	if ( src_length == 1 ) {
		memset( dst, *src, dst_length );
		return;
	}
	if ( src != dst ) {
		if ( dst_length <= src_length ) {
			memcpy( dst, src, dst_length );
			return;
		}
		memcpy( dst, src, src_length );
	}
	if ( dst_length <= src_length ) return;
	int offset = src_length;
	while ( offset < dst_length ) {
		if ( offset + offset < dst_length ) {
			memcpy( dst + offset, dst, offset );
			offset *= 2;
		} else {
			memcpy( dst + offset, dst, dst_length - offset );
			offset += dst_length - offset;
		}
	}
}


inline bool type_has_integer( const float * ) { return false; }
inline bool type_has_integer( const double * ) { return false; }
inline bool type_has_integer( const long double * ) { return false; }

template<typename T>
bool type_has_integer( const T * )
{
	return true;
}

inline bool type_has_sign( const uint8 * ) { return false; }
inline bool type_has_sign( const uint16 * ) { return false; }
inline bool type_has_sign( const uint32 * ) { return false; }
inline bool type_has_sign( const uint64 * ) { return false; }
inline bool type_has_sign( const unsigned int * ) { return false; }

template<typename T>
bool type_has_sign( const T * t )
{
	return true;
}

template<typename T>
unsigned int type_minimum( const T * t )
{
	if ( ! type_has_integer( t ) ) return 0;
	if ( ! type_has_sign( t ) ) return 0;
	if ( sizeof( T ) == 1 ) return _I8_MAX;
	if ( sizeof( T ) == 2 ) return _I16_MAX;
	return _I32_MAX;
}

template<typename T>
unsigned int type_maximum( const T * t )
{
	if ( ! type_has_integer( t ) ) return 0;
	if ( type_has_sign( t ) ) {
		if ( sizeof( T ) == 1 ) return _I8_MAX;
		if ( sizeof( T ) == 2 ) return _I16_MAX;
		return _I32_MAX;
	} else {
		if ( sizeof( T ) == 1 ) return _UI8_MAX;
		if ( sizeof( T ) == 2 ) return _UI16_MAX;
		return _UI32_MAX;
	}
}

template<typename T>
unsigned int type_range( const T * t )
{
	if ( ! type_has_integer( t ) ) return 1;
	if ( sizeof( T ) == 1 ) return _UI8_MAX;
	if ( sizeof( T ) == 2 ) return _UI16_MAX;
	return _UI32_MAX;
}

