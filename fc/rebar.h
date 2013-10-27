class rebar : public window
{
public:
	int height;
	rebar() : height( 0 )
	{
		style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_AUTOSIZE | CCS_NODIVIDER;
	}
	rebar( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), height( 0 )
	{
		style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS | RBS_AUTOSIZE | CCS_NODIVIDER;
	}
	~rebar()
	{
	}
	virtual void create()
	{
		hwnd = CreateWindowEx( WS_EX_TOOLWINDOW, REBARCLASSNAME, NULL,
			style | WS_CHILD | WS_VISIBLE, 
			0, 0, 0, 0, parent,
			( HMENU ) id, instance, NULL );

		RECT r = rect();
		height = r.bottom - r.top;
	}
	void add( window * w, int id, char * text = NULL, int cx = -1, int cy = -1, int position = -1 )
	{
		REBARBANDINFO rbbi = { sizeof( rbbi ) };
		rbbi.fMask  = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE | RBBIM_ID;
		rbbi.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;

		rbbi.hwndChild = w->hwnd;
		RECT r = w->rect();
		rbbi.cx = r.right - r.left;
		rbbi.cxMinChild = cx == -1 ? rbbi.cx : cx;
		rbbi.cyMinChild = cy == -1 ? 24 : cy;
		rbbi.lpText = text;
		if ( text ) rbbi.fMask |= RBBIM_TEXT;
		rbbi.wID = id;
		send( RB_INSERTBAND, (WPARAM) position, (LPARAM) & rbbi );
		w->parent = parent;//rebarは自分に来た通知を上に投げるため，フィルターで削除されないように，rebarを見えなくしておく．
	}
	bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch ( message ) {
		case WM_SIZE:
			{
				if ( hWnd == parent ) send( WM_SIZE, wParam, lParam );
				RECT r = rect();
				height = r.bottom - r.top;
			}
			break;
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				if ( pnmh->hwndFrom == hwnd && pnmh->code == RBN_HEIGHTCHANGE ) {
					InvalidateRect( hWnd, NULL, TRUE);
					UpdateWindow( hWnd );
				}
            }
		}
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		switch ( message ) {
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch ( pnmh->code ) {
				case RBN_HEIGHTCHANGE:
					SendMessage( parent, WM_SIZE, 0, 0 );
					break;
				}
			} break;
		}
		return false;
	}
};

