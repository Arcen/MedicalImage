class editbox : public window
{
public:
	editbox(){}
	editbox( HWND _parent, int _id, HWND _hwnd = NULL )
		: window( _parent, _id, _hwnd )
	{
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		if ( message == WM_COMMAND ) {
			if ( HIWORD( wParam ) == EN_CHANGE ||
				HIWORD( wParam ) == EN_KILLFOCUS ) {
				get();
				if ( changed ) changed( this );
			}
		}
		return false;
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindow( "EDIT", text, style | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	void set( int src )
	{
		if ( atoi( get() ) == src && src != 0 ) return;
		set( string( src ) );
	}
	void set( double src )
	{
		if ( atof( get() ) == src ) return;
		set( string( src ) );
	}
	void set( const char * src )
	{
		if ( text != src ) {
			DWORD start, end;
			send( EM_GETSEL, ( WPARAM ) & start, ( LPARAM ) & end );
			SetWindowText( hwnd, src );
			send( EM_SETSEL, start, start );
		}
	}
	char * get()
	{
		text.reserve( 1024 );
		GetWindowText( hwnd, text, 1024 );
		return text;
	}
	char * operator()() { return get(); }
	void operator()( int src ) { set( src ); }
	void operator()( double src ) { set( src ); }
	void operator()( const char * src ) { set( src ); }
};



