class combobox : public window
{
public:
	array<string> nodes;
	int maxLength;
	int index;
	combobox() : maxLength( 0 ), index( 0 )
	{
	}
	combobox( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), maxLength( 0 ), index( 0 )
	{
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		switch ( message ) {
		case WM_COMMAND:
			{
				WORD event = HIWORD( wParam );
				switch ( event ) {
				case CBN_SELCHANGE:
					select();
					if ( changed ) changed( this );
					break;
				}
			} break;
		}
		return false;
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindow( "COMBOBOX", text, style | CBS_SIMPLE | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	void release()
	{
		nodes.release();
		index = 0;
		maxLength = 0;
		refresh();
	}
	void push_back( const char * value )
	{
		nodes.push_back( string( value ) );
		int length = strlen( value ) + 1;
		if ( maxLength < length ) maxLength = length;
		refresh();
	}
	void refresh()
	{
		send( CB_RESETCONTENT );
		send( CB_INITSTORAGE, nodes.size, maxLength );
		for ( int i = 0; i < nodes.size; i++ ) {
			send( CB_ADDSTRING, 0, ( LPARAM ) ( const char * ) nodes[ i ] );
		}
		if ( nodes.size ) select( 0 );
	}
	void select( int value )
	{
		index = value;
		send( CB_SETCURSEL, value );
	}
	int select() { return ( index = send( CB_GETCURSEL ) ); }
	void setSimple(){ setStyle( 0xf, false ); setStyle( CBS_SIMPLE, true ); }
	void setDropDown(){ setStyle( 0xf, false ); setStyle( CBS_DROPDOWN, true ); }
	void setDropDownList(){ setStyle( 0xf, false ); setStyle( CBS_DROPDOWNLIST, true ); }
	int operator()() { return select(); }
	void operator()( int src ) { select( src ); }
};



