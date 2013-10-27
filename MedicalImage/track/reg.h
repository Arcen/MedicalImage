inline bool regGet( const char * name, string & value )
{
	return windows::get( "SOFTWARE\\Akira", "Track", name, value );
}
inline bool regSet( const char * name, const string & value )
{
	return windows::set( "SOFTWARE\\Akira", "Track", name, value );
}
inline bool regsKill( const char * directory, const list< pair<string> > & value )
{
	for ( list< pair<string> >::iterator it( value ); it; ++it ) {
		windows::kill( "SOFTWARE\\Akira\\Track", directory, it().left );
	}
	return true;
}
inline bool regsSet( const char * directory, const list< pair<string> > & value )
{
	string path = string( "SOFTWARE\\Akira\\Track" );
	if ( directory ) {
		path = path + string( "\\" ) + string( directory );
	}
	for ( list< pair<string> >::iterator it( value ); it; ++it ) {
		windows::set( "SOFTWARE\\Akira\\Track", directory, it().left, it().right );
	}
	return true;
}
inline bool regsGet( const char * directory, list< pair<string> > & value )
{
	string path = string( "SOFTWARE\\Akira\\Track" );
	if ( directory ) {
		path = path + string( "\\" ) + string( directory );
	}
	HKEY reg = NULL;
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, path, 0, KEY_ALL_ACCESS, & reg ) ) return false;
	for ( int index = 0; ; index++ ) {
		char name[1024];
		DWORD nameLength = 1024;
		DWORD type = 0;
		BYTE data[1024];
		DWORD dataSize = 1024;
		if ( RegEnumValue( reg, index, name, & nameLength, NULL, & type, data, & dataSize ) ) break;
		switch ( type ) {
		case REG_SZ: value.push_back( pair<string>( name, string( ( const char * ) data ) ) ); break;
		}
	}
	RegCloseKey( reg );
	return ! value.empty();
}
