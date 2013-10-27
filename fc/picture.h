class picture : public window
{
public:
	picture(){}
	picture( HWND _parent, int _id, HWND _hwnd = NULL )
		: window( _parent, _id, _hwnd )
	{
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		if ( message == WM_NOTIFY ) {
			if ( HIWORD( wParam ) == NM_CLICK ) {
				NMHDR * nmhdr = ( NMHDR * ) lParam;
				if ( clicked ) clicked( this );
			}
		}
		return false;
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindow( "STATIC", text, style | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
};



