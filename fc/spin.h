class spin : public window
{
public:
	int minimum;
	int maximum;
	editbox * edit;
	spin() : minimum( 0 ), maximum( 0 ), edit( NULL ) {}
	spin( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd )
	{
		minimum = maximum = 0;
		edit = NULL;
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateUpDownControl( style | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_NOTHOUSANDS | ( edit ? UDS_SETBUDDYINT : 0 ) | WS_CHILD | WS_VISIBLE, x, y, w, h, parent, id, instance, ( edit ) ? edit->hwnd : 0, maximum, minimum, minimum );
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		return false;
	}
	void set( int _minimum, int _maximum )
	{
		send( UDM_GETRANGE32, WPARAM( & minimum ), LPARAM( & maximum ) );
		if ( minimum != _minimum || maximum != _maximum ) {
			send( UDM_SETRANGE32, _minimum, _maximum );
			minimum = _minimum;
			maximum = _maximum;
		}
	}
};
