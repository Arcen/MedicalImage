class treeview : public window
{
public:
	procedure leftClick, rightClick, leftDoubleClick, rightDoubleClick, selected;
	class item {
	public:
		string name;
		HTREEITEM handle;
		void * data;
		item( char * _name = "default" ) : handle( NULL ), data( NULL ), name( _name ){}
		item( const item & src ) : handle( src.handle ), data( src.data ), name( src.name ){}
		item & operator=( const item & src )
		{
			name = src.name;
			handle = src.handle;
			data = src.data;
			return *this;
		}
		bool operator==( const item & src ) const { return this == & src; }
		bool operator!=( const item & src ) const { return ! ( *this == src ); }
	};
	tree<item> nodes;
	typedef tree<item>::node node;
	typedef tree<item>::iterator iterator;
	node * current, * root;
	treeview() : current( NULL ), root( NULL ), leftClick( NULL ), rightClick( NULL ), leftDoubleClick( NULL ), rightDoubleClick( NULL ), selected( NULL )
	{
		root = nodes.search( nodes.push( item( "root" ) ) );
	}
	treeview( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), current( NULL ), root( NULL ), leftClick( NULL ), rightClick( NULL ), leftDoubleClick( NULL ), rightDoubleClick( NULL ), selected( NULL )
	{
		root = nodes.search( nodes.push( item( "root" ) ) );
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindowEx( 0, WC_TREEVIEW, text, style | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		switch ( message ) {
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
				switch ( pnmh->code ) {
				case TVN_SELCHANGED:
				{
					LPNMTREEVIEW pnmtv = (LPNMTREEVIEW) pnmh;
					TVITEM & tvitem = pnmtv->itemNew;
					if ( tvitem.mask & TVIF_PARAM ) {
						current = reinterpret_cast<node *>( tvitem.lParam );
					} else {
						current = NULL;
					}
					if ( selected ) selected( this );
					break;
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
	node * push( item & i, node * parent = NULL )
	{
		node * n = nodes.search( nodes.push_back( i, parent ? parent : nodes.root ) );

		HTREEITEM parent_handle = ( parent && parent->data.handle ) ? parent->data.handle : (HTREEITEM) TVI_ROOT; 

		TVINSERTSTRUCT tvins;

		TVITEM & tvi = tvins.item;
		//    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
		tvi.mask = TVIF_TEXT | TVIF_PARAM; 

		tvi.lParam = ( LPARAM ) ( void * ) n;

		tvi.pszText = n->data.name;
		tvi.cchTextMax = n->data.name.length();

		tvins.hInsertAfter = TVI_LAST;
		tvins.hParent = parent_handle;

		n->data.handle = (HTREEITEM) send( TVM_INSERTITEM, 0, (LPARAM) (LPTVINSERTSTRUCT) &tvins );
		return n;
	} 
	void pop( node * n )
	{
		if ( n->data.handle ) {
			send( TVM_DELETEITEM, 0, (LPARAM) (HTREEITEM) n->data.handle );
		}
		n->data.handle = NULL;
		nodes.pop( n );
	}
	void release()
	{
		if ( ! nodes.root ) return;
		node * next = NULL;
		for ( node * n = nodes.root->head; n; n = next ) {
			next = n->next;
			pop( n );
		}
	}
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
};

