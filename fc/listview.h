class listview : public window
{
public:
	procedure leftClick, rightClick, leftDoubleClick, rightDoubleClick, selected;
	class item {
	public:
		string name;
		array<string> values;
		LPLVITEM handle;
		void * data;
		item( char * _name = "default" ) : handle( NULL ), values( NULL ), name( _name ), data( NULL ){}
		item( const item & src ) : handle( src.handle ), values( src.values ), name( src.name ), data( src.data ){}
		item & operator=( const item & src )
		{
			name = src.name;
			handle = src.handle;
			data = src.data;
			values = src.values;
			return *this;
		}
		bool operator==( const item & src ) const { return this == & src; }
		bool operator!=( const item & src ) const { return ! ( *this == src ); }
	};
	class label {
	public:
		string name;
		int width;
		int format;//LVCFMT_CENTER, LVCFMT_LEFT, LVCFMT_RIGHT
		label( char * _name = "default" ) : name( _name ), width( 100 ), format( LVCFMT_CENTER ){}
		label( const label & src ) : name( src.name ), width( src.width ), format( src.format ){}
		label & operator=( const label & src )
		{
			name = src.name;
			width = src.width;
			format = src.format;
			return *this;
		}
		bool operator==( const label & src ) const { return this == & src; }
		bool operator!=( const label & src ) const { return ! ( *this == src ); }
	};
	array<label> labels;
	list<item> items;
	typedef list<item>::node node;
	typedef list<item>::iterator iterator;
	int current, currentSub;
	int column;
	listview() : column( 1 ), current( -1 ), currentSub( -1 ), leftClick( NULL ), rightClick( NULL ), leftDoubleClick( NULL ), rightDoubleClick( NULL ), selected( NULL )
	{
	}
	listview( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), column( 1 ), current( -1 ), currentSub( -1 ), leftClick( NULL ), rightClick( NULL ), leftDoubleClick( NULL ), rightDoubleClick( NULL ), selected( NULL )
	{
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindowEx( 0, WC_LISTVIEW, text, style | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		switch ( message ) {
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch ( pnmh->code ) {
				case LVN_GETDISPINFO:
				{
					NMLVDISPINFO * nmlv = (NMLVDISPINFO*) pnmh;
					nmlv->item.pszText = this->items[nmlv->item.iItem].values[nmlv->item.iSubItem].chars();
				}
				case NM_CLICK: if ( leftClick ) leftClick( this ); break;
				case NM_RCLICK: if ( rightClick ) rightClick( this ); break;
				case NM_DBLCLK: if ( leftDoubleClick ) leftDoubleClick( this ); break;
				case NM_RDBLCLK: if ( rightDoubleClick ) rightDoubleClick( this ); break;
				}
			} break;
		}
		return false;
	}
	void push_back_label( const label & l )
	{
		labels.push_back( l );
		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvc.fmt = labels.last().format;
		lvc.cx = labels.last().width;
		lvc.pszText = labels.last().name.chars();
		lvc.cchTextMax = labels.last().name.length();
		lvc.iSubItem = labels.size - 1;
		send( LVM_INSERTCOLUMN, labels.size - 1, LPARAM( & lvc ) );
	}
	void release_label()
	{
		labels.release();
	}
	void update_label()
	{
		//全ラベル削除
		while ( send( LVM_DELETECOLUMN, 0 ) == TRUE );

		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		for ( int i = 0; i < labels.size; ++i ) {
			lvc.fmt = labels[i].format;
			lvc.cx = labels[i].width;
			lvc.pszText = labels[i].name.chars();
			lvc.cchTextMax = labels[i].name.length();
			lvc.iSubItem = i;
			send( LVM_INSERTCOLUMN, i, LPARAM( & lvc ) );
		}
	}
	void release()
	{
		items.release();
		send( LVM_DELETEALLITEMS, 0 );
	}
	void push_back( item & i )
	{
		item & it = items.push_back( i );

		LV_ITEM lvi;
		lvi.mask = LVIF_TEXT | LVIF_STATE;
		lvi.state = 0;
		lvi.stateMask = 0;
		lvi.iItem = items.size - 1;
		lvi.iSubItem = 0;
		lvi.pszText = LPSTR_TEXTCALLBACK;//コールバックで設定
		send( LVM_INSERTITEM, 0, LPARAM( & lvi ) );
	}
	/*
	void select( node * n, bool caret = true , bool drophilite = false, bool firstvisible = false )
	{
		send( TVM_SELECTITEM, 
			( caret ? TVGN_CARET : 0 ) | 
			( drophilite ? TVGN_DROPHILITE : 0 ) | 
			( firstvisible ? TVGN_FIRSTVISIBLE : 0 ), 
			(LPARAM) (HTREEITEM) ( n ? n->data.handle : NULL ) );
	}
	node * get()
	{
		HTREEITEM handle = ( HTREEITEM ) send( TVM_GETNEXTITEM, TVGN_CARET );
		for ( iterator it( nodes ); it; ++it ) {
			if ( it->handle == handle ) return it;
		}
		return NULL;
	}
	node * operator()() { return get(); }
	void operator()( node * src ) { select( src ); }
	*/
};

