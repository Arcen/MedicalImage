class listbox : public window
{
public:
	procedure leftClick, rightClick, leftDoubleClick, rightDoubleClick, selected;
	array<string> items;
	int current;
	listbox() : current( -1 ), leftClick( NULL ), rightClick( NULL ), leftDoubleClick( NULL ), rightDoubleClick( NULL ), selected( NULL )
	{
	}
	listbox( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), current( -1 ), leftClick( NULL ), rightClick( NULL ), leftDoubleClick( NULL ), rightDoubleClick( NULL ), selected( NULL )
	{
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindowEx( 0, "LISTBOX", text, style | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		switch ( message ) {
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch ( pnmh->code ) {
				case LBN_SELCHANGE:
				{
					current = send( LB_GETCURSEL );
					if ( current == LB_ERR ) current = -1;
				}
				break;
				case NM_CLICK: if ( leftClick ) leftClick( this ); break;
				case NM_RCLICK: if ( rightClick ) rightClick( this ); break;
				case NM_DBLCLK: if ( leftDoubleClick ) leftDoubleClick( this ); break;
				case NM_RDBLCLK: if ( rightDoubleClick ) rightDoubleClick( this ); break;
				}
			} break;
		}
		return false;
	}
	void push_back( const string & item )
	{
		items.push_back( item );
	}
	void release()
	{
		items.release();
		send( LB_INITSTORAGE, 0 );
	}
	void update()
	{
		send( LB_INITSTORAGE, items.size, 1024 * items.size );

		for ( int i = 0; i < items.size; ++i ) {
			send( LB_ADDSTRING, 0, LPARAM( items[i].chars() ) );
		}
	}
	void select( int index )
	{
		send( LB_SETCURSEL, index );
	}
	int get()
	{
		current = send( LB_GETCURSEL );
		if ( current == LB_ERR ) current = -1;
		return current;
	}
	int operator()() { return get(); }
	void operator()( int src ) { select( src ); }
};

