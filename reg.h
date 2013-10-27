inline bool regGet( const char * name, string & value )
{
	return windows::get( "SOFTWARE\\Akira", "MedicalImage", name, value );
}
inline bool regSet( const char * name, const string & value )
{
	return windows::set( "SOFTWARE\\Akira", "MedicalImage", name, value );
}
inline bool regsKill( const char * directory, const list< pair<string> > & value )
{
	for ( list< pair<string> >::iterator it( value ); it; ++it ) {
		windows::kill( "SOFTWARE\\Akira\\MedicalImage", directory, it().left );
	}
	return true;
}
inline bool regsSet( const char * directory, const list< pair<string> > & value )
{
	string path = string( "SOFTWARE\\Akira\\MedicalImage" );
	if ( directory ) {
		path = path + string( "\\" ) + string( directory );
	}
	for ( list< pair<string> >::iterator it( value ); it; ++it ) {
		windows::set( "SOFTWARE\\Akira\\MedicalImage", directory, it().left, it().right );
	}
	return true;
}
inline bool regsGet( const char * directory, list< pair<string> > & value )
{
	string path = string( "SOFTWARE\\Akira\\MedicalImage" );
	if ( directory ) {
		path = path + string( "\\" ) + string( directory );
	}
	HKEY reg = NULL;
    if ( RegOpenKeyEx( HKEY_CURRENT_USER, path, 0, windows::KEY_READ_BITS, & reg ) != ERROR_SUCCESS ) {
        if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, path, 0, windows::KEY_READ_BITS, & reg ) != ERROR_SUCCESS ) return false;
    }
	for ( int index = 0; ; index++ ) {
		char name[1024];
		DWORD nameLength = 1024;
		DWORD type = 0;
		BYTE data[1024];
		DWORD dataSize = 1024;
		if ( RegEnumValue( reg, index, name, & nameLength, NULL, & type, data, & dataSize ) != ERROR_SUCCESS ) break;
		switch ( type ) {
		case REG_SZ: value.push_back( pair<string>( name, string( ( const char * ) data ) ) ); break;
		}
	}
	RegCloseKey( reg );
	return ! value.empty();
}
