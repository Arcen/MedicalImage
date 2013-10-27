class menu : public window
{
public:
	HMENU m;
	menu() : m( NULL )
	{
	}
	menu( HWND _parent ) : window( _parent, 0 ), m( NULL )
	{
		m = GetMenu( _parent );
	}
	virtual void initialize( HWND _parent, int _id = 0, HWND _hwnd = NULL )
	{
		m = GetMenu( _parent );
	}
	void check( int id, bool value )
	{
		CheckMenuItem( m, id, value ? MF_CHECKED : MF_UNCHECKED );
	}
	void enable( int id, bool value )
	{
		EnableMenuItem( m, id, value ? MF_ENABLED : MF_GRAYED );
	}
};

