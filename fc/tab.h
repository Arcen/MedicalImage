class tab;

class sheet : public dialog
{
public:
	tab & t;
	sheet( tab & _t ) : t( _t ) {}
	sheet( tab & _t, int _id, const char * _name, DLGPROC _proc = NULL ) : t( _t )
	{
		initialize( _id, _name, _proc );
	}
	virtual ~sheet()
	{
		if ( hwnd ) { DestroyWindow( hwnd ); hwnd = NULL; }
	}
	virtual void initialize( int id, const char * _name, DLGPROC _proc = NULL );
	virtual void show( bool value );
};

class tab : public window
{
public:
	array<sheet*> sheets;
	int prev, index;
	tab() : prev( 0 ), index( 0 )
	{}
	tab( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), prev( 0 ), index( 0 )
	{}
	virtual ~tab()
	{
	}
	void reflesh()
	{
		send( TCM_DELETEALLITEMS );

		TCITEM tie;
		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = -1;
		for ( int i = 0; i < sheets.size; i++ ) {
			tie.pszText = sheets[ i ]->text;
			send( TCM_INSERTITEM, i, LPARAM( &tie ) );
		}
		index = 0;
		if ( sheets.size ) {
			sheets[index]->show( true );
		}
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch ( message ) {
		case WM_SIZE:
			if ( between( 0, index, sheets.size ) ) sheets[index]->show( true );
			break;
		}
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return 0;
		switch ( message ) {
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				prev = index;
 				index = SendMessage( pnmh->hwndFrom, TCM_GETCURSEL, 0, 0 );
				if ( prev != index ) {
					sheets[ prev ]->show( false );
					sheets[ index ]->show( true );
					if ( changed ) changed( this );
				}
			} break;
		}
		return 0;
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindow( WC_TABCONTROL, text, style | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	static BOOL CALLBACK wndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		window::processAll( hWnd, message, wParam, lParam );
		return 0;
	}
	int get()
	{
 		return index = send( TCM_GETCURSEL );
	}
	void set( int src )
	{
		send( TCM_SETCURSEL, WPARAM( src ) );
 		index = get();
	}
	int operator()() { return get(); }
	void operator()( int src ) { set( src ); }
};

inline void sheet::show( bool value )
{
	if ( value ) {
		RECT r = t.client();
		t.send( TCM_ADJUSTRECT, FALSE, LPARAM( & r ) );
		resize( r.left, r.top, r.right - r.left, r.bottom - r.top, 0 );
	}
	window::show( value );
}
inline void sheet::initialize( int id, const char * _name, DLGPROC _proc )
{
	window::initialize( t.hwnd, id );
	dialog::initialize( _proc ? _proc : t.wndProc );
	text = _name;
	create();
	window::show( false );
	t.sheets.push_back( this );
}
