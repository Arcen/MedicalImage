class popupmenu : public window
{
public:
	class item {
	public:
		popupmenu * parent;
		MENUITEMINFO info;
		string name;
		item( int id, const char * _name ) : parent( NULL )
		{
			memset( & info, 0, sizeof( info ) );
			info.cbSize = sizeof( info );
			info.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE;
			info.fState = MFS_ENABLED;
			info.fType = MFT_STRING;
			info.wID = id;
			name = _name;
			if ( name == "-" ) {
				info.fType = MFT_MENUBARBREAK;
			} else {
				info.dwTypeData = name;
			}
		}
		void enable( bool value )
		{
			info.fState = value ? MFS_ENABLED : MFS_DISABLED;
			SetMenuItemInfo( parent->menu, info.wID, FALSE, & info );
		}
	};
	list< retainer<item> > items;

	HMENU menu;
	procedure select;
	item * current;
	popupmenu() : select( NULL ), current( NULL ), menu( NULL )
	{
	}
	popupmenu( HWND _parent ) : window( _parent, 0 ), select( NULL ), current( NULL ), menu( CreatePopupMenu() )
	{
	}
	~popupmenu()
	{
		if ( menu ) DestroyMenu( menu );
	}
	virtual void initialize( HWND _parent, int _id = 0, HWND _hwnd = NULL )
	{
		if ( menu ) return;
		window::initialize( _parent, 0 );
		menu = CreatePopupMenu();
	}
	void push_back( item * i )
	{
		i->parent = this;
		items.push_back( retainer<item>( i ) );
		InsertMenuItem( menu, items.size - 1, TRUE, & i->info );
	}
	item * call( int x, int y )
	{
		if ( ! items.size ) return NULL;
		DWORD id = TrackPopupMenu( menu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_NONOTIFY, x, y, 0, parent, NULL );
		for ( list< retainer<item> >::iterator it( items ); it; ++it ) {
			if ( it()->info.wID == id ) {
				current = & ( it()() );
				if ( select ) select( this );
				return current;
			}
		}
		return NULL;
	}
};

