class window;

typedef void ( * procedure )( window * value );

class window
{
	static list< retainer<window> > nodes;
public:
	HWND parent;
	HWND hwnd;
    HACCEL hAccel;
	enum Type {
		RightType = 1, MiddleType = 2, LeftType = 4,
		default_style = WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE,
	};
	int id;
	string text;
	static HINSTANCE instance;
	int style, styleEx;
	procedure changed, clicked, paint;
	POINT scrollDocumentSize, scrollPosition;
	POINT scrollDelta;
	bool flatScrollbar;
	void * externalData;
	window() : externalData( NULL ), parent( NULL ), clicked( NULL ), paint( NULL ), id( 0 ), hwnd( NULL ), hAccel( NULL ), style( 0 ), changed( NULL ), flatScrollbar( false ), styleEx( 0 )
	{
		push( this );
		scrollDocumentSize.x = 1;
		scrollDocumentSize.y = 1;
		scrollPosition.x = 0;
		scrollPosition.y = 0;
		scrollDelta.x = scrollDelta.y = 1;
	}
	window( HWND _parent, int _id, HWND _hwnd = NULL )
		: externalData( NULL ), parent( NULL ), id( 0 ), hwnd( NULL ), style( 0 ), changed( NULL ), flatScrollbar( false ), styleEx( 0 )
	{
		push( this );
		scrollDocumentSize.x = 1;
		scrollDocumentSize.y = 1;
		scrollPosition.x = 0;
		scrollPosition.y = 0;
		scrollDelta.x = scrollDelta.y = 1;
		initialize( _parent, _id, _hwnd );
	}
	virtual ~window()
	{
		pop( this );
	}
	operator bool () const { return hwnd ? true : false; }
	virtual void initialize( HWND _parent, int _id, HWND _hwnd = NULL )
	{
		parent = _parent;
		id = _id;
		hwnd = _hwnd;
		if ( ! hwnd ) hwnd = GetDlgItem( parent, id );
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT ){};
	virtual void create( char * className, char * title, int style, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		this->style = style;
		hwnd = CreateWindow( className, title, style | default_style, x, y, w, h, parent, NULL, instance, NULL );
	}
	virtual void createRoot( char * className, char * title, int style, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT, int menuid = 0 )
	{
		this->style = style;
		HMENU menu = menuid ? LoadMenu( instance, MAKEINTRESOURCE( menuid ) ) : NULL;
		hwnd = CreateWindow( className, title, style | WS_CLIPCHILDREN | WS_VISIBLE, x, y, w, h, NULL, menu, instance, NULL );
	}
	void flatScroll()
	{
		if ( ! ( ( style & WS_HSCROLL ) || ( style & WS_VSCROLL ) ) ) return;
		if ( ! flatScrollbar ) {
			BOOL initialized = InitializeFlatSB( hwnd );
			if ( ! initialized ) return;
			flatScrollbar = true;
		}
		if ( style & WS_HSCROLL ) FlatSB_SetScrollProp( hwnd, WSB_PROP_HSTYLE , FSB_FLAT_MODE, TRUE );
		if ( style & WS_VSCROLL ) FlatSB_SetScrollProp( hwnd, WSB_PROP_VSTYLE , FSB_FLAT_MODE, TRUE );
	}
	void getScrollInfo( SCROLLINFO & si, bool horizontal )
	{
		if ( flatScrollbar ) {
			FlatSB_GetScrollInfo( hwnd, horizontal ? SB_HORZ : SB_VERT, & si );
		} else {
			GetScrollInfo( hwnd, horizontal ? SB_HORZ : SB_VERT, & si );
		}
	}
	void setScrollInfo( SCROLLINFO & si, bool horizontal )
	{
		if ( flatScrollbar ) {
			FlatSB_SetScrollInfo( hwnd, horizontal ? SB_HORZ : SB_VERT, & si, TRUE );
		} else {
			SetScrollInfo( hwnd, horizontal ? SB_HORZ : SB_VERT, & si, TRUE );
		}
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM /*lParam*/ )
	{
		if ( hWnd == hwnd ) {
			switch ( message ) {
			case WM_CREATE:
			break;
			case WM_HSCROLL:
				{
					if ( ! ( style & WS_HSCROLL ) ) break;
					int dv = 0;
					SCROLLINFO si = { sizeof( si ) };
					si.fMask = SIF_ALL;
					getScrollInfo( si, true );
					scrollPosition.x = si.nPos;
					switch ( LOWORD( wParam ) ) {
		//			case SB_LEFT: dv = si.nMin - si.nPos; break;
					case SB_LINELEFT: dv = - scrollDelta.x; break;
					case SB_PAGELEFT: dv = - static_cast<int>( si.nPage ); break;
		//			case SB_RIGHT: dv = ( si.nMax - maximum<int>( si.nPage - 1, 0 ) ) - si.nPos; break;
					case SB_LINERIGHT: dv = scrollDelta.x; break;
					case SB_PAGERIGHT: dv = static_cast<int>( si.nPage ); break;
					case SB_THUMBTRACK: dv = static_cast<int>( HIWORD(wParam) ) - static_cast<int>( si.nPos ); break;
					case SB_THUMBPOSITION: dv = static_cast<int>( HIWORD(wParam) ) - static_cast<int>( si.nPos ); break;
					case SB_ENDSCROLL: InvalidateRect( hWnd, NULL, FALSE ); break;
					}
					if ( dv == 0 ) break;
					scrollPosition.x = clamp<int>( si.nMin, scrollPosition.x + dv, si.nMax - maximum<int>( si.nPage - 1, 0 ) );
					if ( scrollPosition.x == si.nPos ) break;
					int oldPosition = si.nPos;
					if ( LOWORD( wParam ) == SB_THUMBTRACK ) {
						si.nPos = scrollPosition.x;
					} else {
						si.nPos = scrollPosition.x;
					}
					setScrollInfo( si, true );
					invalidate();
					update();
				}
				break;
			case WM_VSCROLL:
				{
					if ( ! ( style & WS_VSCROLL ) ) break;
					int dv = 0;
					SCROLLINFO si = { sizeof( si ) };
					si.fMask = SIF_ALL;
					getScrollInfo( si, false );
					scrollPosition.y = si.nPos;

					switch ( LOWORD( wParam ) ) {
		//			case SB_TOP: dv = si.nMin - si.nPos; break;
					case SB_LINEUP: dv = - scrollDelta.y; break;
					case SB_PAGEUP: dv = - static_cast<int>( si.nPage ); break;
		//			case SB_BOTTOM: dv = ( si.nMax - maximum<int>( si.nPage - 1, 0 ) ) - si.nPos; break;
					case SB_LINEDOWN: dv = scrollDelta.y; break;
					case SB_PAGEDOWN: dv = static_cast<int>( si.nPage ); break;
					case SB_THUMBTRACK: dv = static_cast<int>( HIWORD(wParam) ) - static_cast<int>( si.nPos ); break;
					case SB_THUMBPOSITION: dv = static_cast<int>( HIWORD(wParam) ) - static_cast<int>( si.nPos ); break;
					case SB_ENDSCROLL: InvalidateRect( hWnd, NULL, FALSE ); break;
					}
					if ( dv == 0 ) break;
					scrollPosition.y = clamp<int>( si.nMin, scrollPosition.y + dv, si.nMax - maximum<int>( si.nPage - 1, 0 ) );
					if ( scrollPosition.y == si.nPos ) break;
					int oldPosition = si.nPos;
					if ( LOWORD( wParam ) == SB_THUMBTRACK ) {
						si.nPos = scrollPosition.y;
					} else {
						si.nPos = scrollPosition.y;
					}
					setScrollInfo( si, false );
					invalidate();
					update();
				}
				break;
			}
		}
		//コントロールに対してのフィルタ
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
	void setText( const char * t )
	{
		text = t;
		SetWindowText( hwnd, text.chars() );
	}
	void clearStyle() { style = 0; }
	void getStyle() { if ( hwnd ) style = GetWindowLong( hwnd, GWL_STYLE ); }
	void setStyle() { if ( hwnd ) SetWindowLong( hwnd, GWL_STYLE, style ); }
	void setStyle( int flag, bool value = true ) { getStyle(); if ( value ) { style |= flag; } else { style &= ~ flag; } setStyle(); }
	void getStyleEx() { if ( hwnd ) styleEx = GetWindowLong( hwnd, GWL_EXSTYLE ); }
	void setStyleEx() { if ( hwnd ) SetWindowLong( hwnd, GWL_EXSTYLE, styleEx ); }
	void setStyleEx( int flag, bool value = true ) { getStyleEx(); if ( value ) { styleEx |= flag; } else { styleEx &= ~ flag; } setStyleEx(); }
	virtual void show( bool value ){ if ( hwnd ) ShowWindow( hwnd, value ? SW_SHOW : SW_HIDE ); };
	bool show(){ return IsWindowVisible( hwnd ) ? true : false; }
	void validate(){ ValidateRect( hwnd, NULL ); }
	void invalidate(){ InvalidateRect( hwnd, NULL, FALSE ); }
	void enable( bool value ){ EnableWindow( hwnd, value ? TRUE : FALSE ); }
	void resize( int x, int y, int w, int h, int flag = 0 )
	{
		SetWindowPos( hwnd, HWND_TOP, x, y, w, h, 
			( ( x == -1 || y == -1 ) ? SWP_NOMOVE : 0 ) | 
			( ( w == -1 || h == -1 ) ? SWP_NOSIZE : 0 ) | 
			flag | SWP_NOZORDER | SWP_NOACTIVATE );
	}
	void client( int w, int h )
	{
		RECT cr = client(), wr = rect();
		resize( -1, -1, w + ( wr.right - wr.left ) - ( cr.right - cr.left ),
			h + ( wr.bottom - wr.top ) - ( cr.bottom - cr.top ) );
	}
	RECT client(){ RECT r; GetClientRect( hwnd, & r ); return r; }
	RECT rect(){ RECT r; GetWindowRect( hwnd, & r ); return r; }
	HDC getDC(){ return GetDC( hwnd ); }
	bool release( HDC hdc ){ return ReleaseDC( hwnd, hdc ) ? true : false; }
	LRESULT send( UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 ){ return SendMessage( hwnd, msg, wParam, lParam ); }
	bool post( UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 ){ return TRUE == PostMessage( hwnd, msg, wParam, lParam ); }
	void update(){ UpdateWindow( hwnd ); }
	//スクロール情報を設定
	static void setScrollInfo( SCROLLINFO & si, LONG window, LONG document, LONG & position )
	{
		si.cbSize = sizeof( si );
		si.fMask = ( SIF_ALL & ( ~ SIF_TRACKPOS ) );// | SIF_DISABLENOSCROLL;
		si.nMin = 0;
		si.nMax = document - 1;
		si.nPage = clamp<UINT>( 0, document, window );
		si.nPos = clamp<int>( si.nMin, position, 
			si.nMax - maximum<int>( static_cast<int>( si.nPage ) - 1, 0 ) );
		position = static_cast<int>( si.nPos );
	}
	void updateScrollRange()
	{
		ShowScrollBar( hwnd, SB_BOTH, FALSE );
		for ( int i = 0; i < 2; ++i ) {
			RECT c = client();
			POINT scrollWindowSize;
			scrollWindowSize.x = c.right - c.left;
			scrollWindowSize.y = c.bottom - c.top;
			SCROLLINFO xsi, ysi;
			setScrollInfo( xsi, scrollWindowSize.x, scrollDocumentSize.x, scrollPosition.x );
			setScrollInfo( ysi, scrollWindowSize.y, scrollDocumentSize.y, scrollPosition.y );
			setScrollInfo( xsi, true );
			setScrollInfo( ysi, false );
			getScrollInfo( xsi, true );
			getScrollInfo( ysi, false );
			scrollPosition.x = xsi.nPos;
			scrollPosition.y = ysi.nPos;
		}
	}
	static void initialize()
	{
		INITCOMMONCONTROLSEX ctrl = { sizeof( ctrl ) };
		ctrl.dwICC = 
			ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES | 
			ICC_DATE_CLASSES | ICC_HOTKEY_CLASS | ICC_INTERNET_CLASSES | 
			ICC_LISTVIEW_CLASSES | ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS | 
			ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS | 
			ICC_USEREX_CLASSES | ICC_WIN95_CLASSES;

		InitCommonControlsEx( & ctrl );
	}
	static LRESULT processAll( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		for ( list< retainer<window> >::iterator it( nodes ); it; ++it ) {
			it()->process( hWnd, message, wParam, lParam );
		}
		return 0;
	}
	static void push( window * value ) { nodes.push_back( value ); }
	static void pop( window * value ) 
	{
		retainer<window> src( value );
		retainer<window> dst = nodes.loot( src );
		src.loot();
		dst.loot();
	}
	static void release() { release(); }
	static window * search( HWND hwnd )
	{
		for ( list< retainer<window> >::iterator it( nodes ); it; ++it ) {
			if ( it()->hwnd == hwnd ) return & it()();
		}
		return NULL;
	}
	static window * search( int id )
	{
		for ( list< retainer<window> >::iterator it( nodes ); it; ++it ) {
			if ( it()->id == id ) return & it()();
		}
		return NULL;
	}
	typedef void ( *callbackEnumWindows )( window * w );
	static BOOL CALLBACK EnumWndProc( HWND hwnd, LPARAM lparam )
	{
		callbackEnumWindows func = ( callbackEnumWindows ) lparam;
		window w( NULL, 0, hwnd );
		func( & w );
		return TRUE;
	}
	static void enumWindows( callbackEnumWindows func )
	{
		EnumWindows( ( WNDENUMPROC ) EnumWndProc, ( LPARAM ) func );
	}
	static ATOM registerClass( WNDPROC wndProc, const char * className, int menuID = 0, int iconID = 0, int iconSmallID = 0, int backGround = 0 )
	{
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) }; 
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)wndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= window::instance;
		wcex.hIcon			= iconID ? LoadIcon(window::instance, (LPCTSTR)iconID) : 0;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= backGround ? (HBRUSH)(backGround+1) : 0;
		wcex.lpszMenuName	= menuID ? (LPCSTR)menuID : 0;
		wcex.lpszClassName	= className;
		wcex.hIconSm		= iconSmallID ? LoadIcon(window::instance, (LPCTSTR)iconSmallID) : 0;
		
		return RegisterClassEx( &wcex );
	}
	void loop( procedure idle = NULL, int accelerators = 0 )
	{
		hAccel = ( accelerators == 0 ? NULL : LoadAccelerators( instance, MAKEINTRESOURCE( accelerators ) ) );
		MSG msg;
		while ( 1 ) {
			if ( PeekMessage( & msg, NULL, 0, 0, PM_NOREMOVE ) ) {
				BOOL r = GetMessage( & msg, NULL, 0, 0 );
				if ( r == 0 ) return;
				if ( ! hAccel || ! TranslateAccelerator( hwnd, hAccel, & msg ) ) {
					TranslateMessage( &msg );
				    DispatchMessage( &msg );
				}
			} else {
				if ( idle ) idle( NULL );
				Sleep( 1 );
			}
		}
	}
	void process_message()
	{
		MSG msg;
		if ( PeekMessage( & msg, NULL, 0, 0, PM_REMOVE ) ) {
			if ( ! hAccel || ! TranslateAccelerator( hwnd, hAccel, & msg ) ) {
				TranslateMessage( &msg );
			    DispatchMessage( &msg );
			}
		}
	}
	//ファイル入出力
	static bool fileOpen( char * filename, char * defaultName, char * mask, char * title, HWND parent )
	{
		strcpy( filename, "" );
		if ( defaultName ) strcpy( filename, defaultName );

		OPENFILENAME openfilename; memset( & openfilename, 0, sizeof( openfilename ) );
		openfilename.lStructSize = sizeof( OPENFILENAME );
		openfilename.hwndOwner = parent;
		openfilename.hInstance = instance;
		openfilename.lpstrFilter = mask;
		openfilename.lpstrFile = filename;
		openfilename.nMaxFile = MAX_PATH;
		openfilename.lpstrTitle = title;
		openfilename.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		if ( ! GetOpenFileName( & openfilename ) ) return false;
		return true;
	}
	static bool fileSave( char * filename, char * defaultName, char * mask, char * title, HWND parent, char * defext = NULL )
	{
		strcpy( filename, "" );
		if ( defaultName ) strcpy( filename, defaultName );

		OPENFILENAME openfilename; memset( & openfilename, 0, sizeof( openfilename ) );
		openfilename.lStructSize = sizeof( OPENFILENAME );
		openfilename.hwndOwner = parent;
		openfilename.hInstance = instance;
		openfilename.lpstrFilter = mask;
		openfilename.lpstrFile = filename;
		openfilename.nMaxFile = MAX_PATH;
		openfilename.lpstrTitle = title;
		openfilename.lpstrDefExt = defext;
		openfilename.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT;

		if ( ! GetSaveFileName( & openfilename ) ) return false;
		return true;
	}
};

class memoryDC
{
public:
	HDC hdc;
	HBITMAP object, wk;
	BITMAPINFO bmpinfo;
	LONG & width, & height;
	memoryDC() : hdc( CreateCompatibleDC( NULL ) ), object( NULL ), wk( NULL ), width( bmpinfo.bmiHeader.biWidth ), height( bmpinfo.bmiHeader.biHeight )
	{
		memset( & bmpinfo, 0, sizeof( bmpinfo ) );
	}
	bool initialize( int width, int height )
	{
		if ( this->width == width && this->height == height ) return true;

		finalize();
		if ( width <= 0 || height <= 0 ) return false;
		memset( & bmpinfo, 0, sizeof( bmpinfo ) );
		bmpinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bmpinfo.bmiHeader.biPlanes = 1;
		bmpinfo.bmiHeader.biCompression = BI_RGB;
		bmpinfo.bmiHeader.biWidth = ( LONG ) width;
		bmpinfo.bmiHeader.biHeight = ( LONG ) height;
		bmpinfo.bmiHeader.biBitCount = ( WORD ) 32;
		object = CreateDIBSection( hdc, &bmpinfo, DIB_RGB_COLORS, NULL, NULL, 0 );
		if ( ! object ) return false;
		wk = ( HBITMAP ) SelectObject( hdc, object );
		return true;
	}
	bool finalize()
	{
		if ( object ) {
			SelectObject( hdc, wk );
			DeleteObject( object );
			object = NULL;
			wk = NULL;
		}
		return true;
	}
	~memoryDC()
	{
		finalize();
		DeleteDC( hdc );
	}
	memoryDC & operator=( const memoryDC & mdc )
	{
		initialize( mdc.width, mdc.height );
		BitBlt( hdc, 0, 0, width, height, mdc.hdc, 0, 0, SRCCOPY );
		return *this;
	}
};
