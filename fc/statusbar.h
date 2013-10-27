class statusbar : public window
{
	bool multiPart;
public:
	class itemData
	{
		int _minimum, _maximum, _value;
	public:
		char text[1024];
		int minimum, maximum, value;
		itemData() : minimum( 0 ), maximum( 1 ), value( 0 ), _minimum( -1 ), _maximum( -1 ), _value( -1 )
		{
			strcpy( text, "" );
		}
	};
	itemData * multiPartItem;
	int multiPartSize;
	int height;
	statusbar() : multiPart( false ), multiPartItem( NULL ), multiPartSize( 0 ), height( 0 ) {}
	statusbar( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), 
		multiPart( false ), multiPartItem( NULL ), multiPartSize( 0 ), height( 0 )
	{
		style |= SBARS_SIZEGRIP | CCS_BOTTOM;
	}
	~statusbar()
	{
		release();
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateStatusWindow( style | default_style, text, parent, id );
		INT borders[3];
		send( SB_GETBORDERS, 0, LPARAM( borders ) );
		RECT r;
		send( SB_GETRECT, 0, LPARAM( & r ) );
		height = r.bottom - r.top + borders[1];
		setMultiPart( 1 );
	}
	bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch ( message ) {
		case WM_SIZE:
			{
				send( WM_SIZE, wParam, lParam );
			}
			break;
		case WM_DRAWITEM:
			{
				if ( this->id != int( wParam ) ) break;
				LPDRAWITEMSTRUCT ds = LPDRAWITEMSTRUCT( lParam );
				statusbar * sb = ( statusbar * ) window::search( ds->hwndItem );
				if ( ! sb ) break;
				char * msg = sb->multiPartItem[ ds->itemID ].text;
				const int & minimum = sb->multiPartItem[ ds->itemID ].minimum, & maximum = sb->multiPartItem[ ds->itemID ].maximum, & value = sb->multiPartItem[ ds->itemID ].value;
				if ( minimum <= maximum && minimum <= value && value <= maximum ) {
					RECT & r = ds->rcItem;
					RECT leftRect = { r.left, r.top, r.left, r.bottom };
					RECT rightRect = { r.left, r.top, r.right, r.bottom };
					if ( minimum < maximum && minimum < value && value <= maximum ) {
						rightRect.left = leftRect.right = r.left + LONG( ( r.right - r.left ) * ( double( value - minimum ) / ( maximum - minimum ) ) );
					}
					if ( leftRect.left != leftRect.right )
						FillRect( ds->hDC, & leftRect, HBRUSH( COLOR_HIGHLIGHT + 1 ) ); 
					if ( rightRect.left != rightRect.right )
						FillRect( ds->hDC, & rightRect, HBRUSH( COLOR_MENU + 1 ) );
			        int oldBkMode = SetBkMode( ds->hDC, TRANSPARENT );
					HFONT font = CreateFont( r.bottom - r.top - 2, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, NULL );
					HFONT oldFont = ( HFONT ) SelectObject( ds->hDC, font );
					TextOut( ds->hDC, r.left, r.top + 1, msg, strlen( msg ) );
					SelectObject( ds->hDC, oldFont );
					DeleteObject( font );
					SetBkMode( ds->hDC, oldBkMode );
				}
			}
			break;
		}
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		return false;
	}
	void set( char * value, int index = 0 )
	{
		if ( multiPart ) {
			if ( value ) strncpy( multiPartItem[index].text, value, 1024-1 );
			send( SB_SETTEXT, SBT_OWNERDRAW | index, LPARAM( value ) );
		} else {
			send( SB_SETTEXT, 0, LPARAM( value ) );
		}
	}
	void post( char * value, int index = 0 )
	{
		if ( multiPart ) {
			if ( value ) strncpy( multiPartItem[index].text, value, 1024-1 );
            window::post( SB_SETTEXT, SBT_OWNERDRAW | index, LPARAM( value ) );
		} else {
			window::post( SB_SETTEXT, 0, LPARAM( value ) );
		}
	}
	void setStyleMovable( bool value ){ setStyle( SBARS_SIZEGRIP, value ); }
	void release()
	{
		delete [] multiPartItem;
		multiPartItem = NULL;
		multiPartSize = 0;
	}
	void setMultiPart( int num )
	{
		release();
		multiPartSize = num;
		multiPartItem = new itemData[multiPartSize];
		int multiPartWidth[256];
		int i;
		for ( i = 0; i < multiPartSize; i++ ) {
			multiPartWidth[i] = 0xffff;
		}
		multiPart = true;
		send( SB_SETPARTS, multiPartSize, LPARAM( multiPartWidth ) );
		for ( i = 0; i < multiPartSize; i++ ) {
			set( "", i );
		}
	}
private:
	void setPB( int _minimum, int _maximum, int index )
	{
		multiPartItem[index].value = 0;
		multiPartItem[index].minimum = _minimum;
		multiPartItem[index].maximum = _maximum;
		set( NULL, index );
	}
	void postPB( int _minimum, int _maximum, int index )
	{
		multiPartItem[index].value = 0;
		multiPartItem[index].minimum = _minimum;
		multiPartItem[index].maximum = _maximum;
		post( NULL, index );
	}
	void setPB( int _index, int index )
	{
		multiPartItem[index].value = _index;
		set( NULL, index );
	}
	void postPB( int _index, int index )
	{
		multiPartItem[index].value = _index;
		post( NULL, index );
	}
public:
	void reset( int index = 0 )
	{
		setPB( 0, index );
		setPB( 0, 0, index );
	}
	void expand( int value, int index = 0 )
	{
		setPB( multiPartItem[index].minimum, multiPartItem[index].maximum + value, index );
	}
	void progress( int index = 0 )
	{
		setPB( clamp( multiPartItem[index].minimum, multiPartItem[index].value + 1, multiPartItem[index].maximum ), index );
	}
	void progress_multithread( int index = 0 )
	{
		postPB( clamp( multiPartItem[index].minimum, multiPartItem[index].value + 1, multiPartItem[index].maximum ), index );
	}
	void loop( int index = 0 )
	{
		setPB( clamp( multiPartItem[index].minimum, multiPartItem[index].value + 1, multiPartItem[index].maximum ), index );
		if ( multiPartItem[index].value + 1 == multiPartItem[index].maximum ) {
			multiPartItem[index].value = multiPartItem[index].minimum;
		}
	}
};

