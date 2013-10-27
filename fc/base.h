class base;

typedef void ( * procedure )( base * value );

class base
{
	static list< retainer<base> > nodes;
public:
	HWND parent;
	HWND hwnd;
	enum Type {
		RightType = 1, MiddleType = 2, LeftType = 4,
		default_style = WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE,
	};
	int id;
	string text;
	static HINSTANCE instance;
	int style;
	procedure changed;
	void * externalData;
	base( HWND _parent, int _id, HWND _hwnd = NULL );
	virtual ~base();
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT ){};
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM /*lParam*/ )
	{
		if ( hWnd != parent ) return false;
		switch ( message ) {
		case WM_COMMAND:
			if ( id != LOWORD( wParam ) ) return false;
			break;
		case WM_NOTIFY:
			if ( WPARAM( id ) != wParam ) return false;
			break;
		}
		return true;
	}
	void clearStyle() { style = 0; }
	void getStyle() { if ( hwnd ) style = GetWindowLong( hwnd, GWL_STYLE ); }
	void setStyle() { if ( hwnd ) SetWindowLong( hwnd, GWL_STYLE, style ); }
	void setStyle( int flag, bool value = true ) { getStyle(); if ( value ) { style |= flag; } else { style &= ~ flag; } setStyle(); }
	void show( bool value ){ if ( hwnd ) ShowWindow( hwnd, value ? SW_SHOW : SW_HIDE ); };
	void validate(){ ValidateRect( hwnd, NULL ); }
	void invalidate(){ InvalidateRect( hwnd, NULL, FALSE ); }
	void enable( bool value ){ EnableWindow( hwnd, value ? TRUE : FALSE ); }

	static void initialize()
	{
		INITCOMMONCONTROLSEX ctrl;
		ctrl.dwSize = sizeof( ctrl );
		ctrl.dwICC = 
			ICC_ANIMATE_CLASS | 
			ICC_BAR_CLASSES | 
			ICC_COOL_CLASSES | 
			ICC_DATE_CLASSES | 
			ICC_HOTKEY_CLASS | 
			ICC_INTERNET_CLASSES | 
			ICC_LISTVIEW_CLASSES | 
			ICC_PAGESCROLLER_CLASS | 
			ICC_PROGRESS_CLASS | 
			ICC_TAB_CLASSES | 
			ICC_TREEVIEW_CLASSES | 
			ICC_UPDOWN_CLASS | 
			ICC_USEREX_CLASSES | 
			ICC_WIN95_CLASSES;

		InitCommonControlsEx( & ctrl );
	}
	static LRESULT processAll( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		for ( list< retainer<base> >::iterator it( nodes ); it; ++it ) {
			it()->process( hWnd, message, wParam, lParam );
		}
		return 0;
	}
	static void push( base * value ) { nodes.push_back( value ); }
	static void pop( base * value ) 
	{
		retainer<base> src( value );
		retainer<base> dst = nodes.loot( src );
		src.loot();
		dst.loot();
	}
	static void release() { release(); }
	static base * search( HWND hwnd )
	{
		for ( list< retainer<base> >::iterator it( nodes ); it; ++it ) {
			if ( it()->hwnd == hwnd ) return & it()();
		}
		return NULL;
	}
	static base * search( int id )
	{
		for ( list< retainer<base> >::iterator it( nodes ); it; ++it ) {
			if ( it()->id == id ) return & it()();
		}
		return NULL;
	}
	void resize( int x, int y, int w, int h )
	{
		SetWindowPos( hwnd, HWND_TOP, x, y, w, h, 
			( ( x == -1 || y == -1 ) ? SWP_NOMOVE : 0 ) | 
			( ( w == -1 || h == -1 ) ? SWP_NOSIZE : 0 ) | 
			SWP_NOZORDER | SWP_NOACTIVATE );
	}
};
