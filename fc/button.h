class button : public window
{
public:
	enum {
		unchecked = 0,
		checked = 1,
		grayed = 2
	};
	int state;
	button() : state( 0 )
	{}
	button( HWND _parent, int _id, HWND _hwnd = NULL )
		: window( _parent, _id, _hwnd ), state( 0 )
	{}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( message == WM_DRAWITEM && wParam == WPARAM( id ) && paint ) paint( this );
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		if ( message == WM_COMMAND ) {
			if ( HIWORD( wParam ) == BN_CLICKED ) {
				state = SendMessage( ( HWND ) lParam, BM_GETCHECK, 0, 0 );
				if ( clicked ) clicked( this );
			} else if ( HIWORD( wParam ) == BN_PAINT ) {
				if ( paint ) paint( this );
			}
		}
		return false;
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindow( "BUTTON", text, style | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	void set( int value )
	{
		send( BM_SETCHECK, value & 0x3 );
	}
	int get()
	{
		return send( BM_GETCHECK ) & 0x3;
	}
	void setStyleButton(){ setStyle( 0xf, false ); setStyle( BS_PUSHBUTTON, true ); }
	void setStyleCheckBox(){ setStyle( 0xf, false ); setStyle( BS_AUTOCHECKBOX, true ); }
	void setStyleRadioButton(){ setStyle( 0xf, false ); setStyle( BS_AUTORADIOBUTTON, true ); }
	bool operator()() { return get() & BST_CHECKED ? true : false; }
	void operator()( bool check ) { set( check ? ( get() | BST_CHECKED ) : ( get() & ( ~ BST_CHECKED ) ) ); }
};

