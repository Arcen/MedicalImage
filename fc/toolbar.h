class toolbar : public window
{
public:
	int height;
	toolbar() : height( 0 )
	{
	}
	toolbar( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), height( 0 )
	{
	}
	~toolbar()
	{
	}
	static int sysIcon( int id, int type )
	{
		return id | 0x80000000 | ( type << 16 );
	}
	int sysIconID( int did )
	{
		return did & 0xFFFF;
	}
	bool sysIconCheck( int did )
	{
		return did & 0x80000000 ? true : false;
	}
	int sysIconGroup( int did )
	{
		return ( did >> 16 ) & 0x7FFF;
	}
	virtual void create( int resourceID, int numberButtonsOfResource, int numberButton, TBBUTTON * buttons )
	{
		style |= TBSTYLE_AUTOSIZE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS;

		hwnd = CreateWindowEx( 0, TOOLBARCLASSNAME, NULL, style | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, parent, (HMENU)id, instance, NULL );
		send( TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON) );//構造体サイズの設定
		//アイコンのロード
		TBADDBITMAP tbab;
		tbab.hInst = instance;
		tbab.nID = resourceID;
		int offsetImage = send( TB_ADDBITMAP, numberButtonsOfResource, LPARAM( & tbab ) );

		//デフォルトアイコンがある場合にはロードして設定
		int offsetStdSmallColorImage = 0;
		int offsetStdLargeColorImage = 0;
		int offsetHistSmallColorImage = 0;
		int offsetHistLargeColorImage = 0;
		int offsetViewSmallColorImage = 0;
		int offsetViewLargeColorImage = 0;
		//tb.dwDataの上位ビットを立てておくとシステムアイコンを使うことにする
		for ( int i = 0; i < numberButton; ++i ) {
			TBBUTTON & tb = buttons[i];
			if ( ! sysIconCheck( tb.iBitmap ) ) {
				tb.iBitmap += offsetImage;
				continue;
			}
			int group = sysIconGroup( tb.iBitmap );
			int sysid = sysIconID( tb.iBitmap );
			TBADDBITMAP tbab;
			tbab.hInst = HINST_COMMCTRL;
			tbab.nID = group;
			if ( group == IDB_STD_SMALL_COLOR ) {
				if ( ! offsetStdSmallColorImage ) {
					offsetStdSmallColorImage = send( TB_ADDBITMAP, 0, LPARAM( & tbab ) );
				}
				tb.iBitmap = sysid + offsetStdSmallColorImage;
			} else if ( group == IDB_STD_LARGE_COLOR ) {
				if ( ! offsetStdLargeColorImage ) {
					offsetStdLargeColorImage = send( TB_ADDBITMAP, 0, LPARAM( & tbab ) );
				}
				tb.iBitmap = sysid + offsetStdLargeColorImage;
			} else if ( tb.dwData == IDB_HIST_SMALL_COLOR ) {
				if ( ! offsetHistSmallColorImage ) {
					offsetHistSmallColorImage = send( TB_ADDBITMAP, 0, LPARAM( & tbab ) );
				}
				tb.iBitmap = sysid + offsetHistSmallColorImage;
			} else if ( tb.dwData == IDB_HIST_LARGE_COLOR ) {
				if ( ! offsetHistLargeColorImage ) {
					offsetHistLargeColorImage = send( TB_ADDBITMAP, 0, LPARAM( & tbab ) );
				}
				tb.iBitmap = sysid + offsetHistLargeColorImage;
			} else if ( tb.dwData == IDB_VIEW_LARGE_COLOR ) {
				if ( ! offsetViewSmallColorImage ) {
					offsetViewSmallColorImage = send( TB_ADDBITMAP, 0, LPARAM( & tbab ) );
				}
				tb.iBitmap = sysid + offsetViewSmallColorImage;
			} else if ( tb.dwData == IDB_VIEW_SMALL_COLOR ) {
				if ( ! offsetViewLargeColorImage ) {
					offsetViewLargeColorImage = send( TB_ADDBITMAP, 0, LPARAM( & tbab ) );
				}
				tb.iBitmap = sysid + offsetViewLargeColorImage;
			} else {
				tb.iBitmap = sysid;
			}
		}
		//tool tipの設定
		//tb.dwDataに渡すアドレスを入れておく
		send( TB_SETMAXTEXTROWS, 32 );

		send( TB_ADDBUTTONS, numberButton, LPARAM( buttons ) );

		RECT r = rect();
		height = r.bottom - r.top;
	}
	bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch ( message ) {
		case WM_SIZE:
			{
				send( WM_SIZE, wParam, lParam );
				RECT r = rect();
				height = r.bottom - r.top;
			}
			break;
		}
		if ( hWnd != parent ) return false;
		switch ( message ) {
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch ( pnmh->code ) {
				case TTN_NEEDTEXT:
					{
						LPTOOLTIPTEXT ttt = (LPTOOLTIPTEXT)pnmh;
						int index = send( TB_COMMANDTOINDEX, pnmh->idFrom );
						
						TBBUTTON tb;
						if ( send( TB_GETBUTTON, index, LPARAM( & tb ) ) ) {
							ttt->lpszText = ( char * ) tb.dwData;
							ttt->uFlags |= TTF_DI_SETITEM;
						}
						break;
					}
				}
			} break;
		}
		return false;
	}
	void enabled( int id, bool p )
	{
		int s = send( TB_GETSTATE, id );
		if ( p ) {
			s |= TBSTATE_ENABLED;
		} else {
			s &= ~ TBSTATE_ENABLED;
		}
		send( TB_SETSTATE, id, MAKELONG( s, 0 ) );
	}
	void pressed( int id, bool p )
	{
		int s = send( TB_GETSTATE, id );
		if ( p ) {
			s |= TBSTATE_PRESSED;
		} else {
			s &= ~ TBSTATE_PRESSED;
		}
		send( TB_SETSTATE, id, MAKELONG( s, 0 ) );
	}
	void checked( int id, bool p )
	{
		int s = send( TB_GETSTATE, id );
		if ( p ) {
			s |= TBSTATE_CHECKED;
		} else {
			s &= ~ TBSTATE_CHECKED;
		}
		send( TB_SETSTATE, id, MAKELONG( s, 0 ) );
	}
};

