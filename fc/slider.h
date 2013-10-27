class slider : public window
{
public:
	int index;
	int minimum;
	int maximum;
	editbox * edit;
	slider() : index( 0 ), minimum( 0 ), maximum( 0 ), edit( NULL ) {}
	slider( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), index( 0 ), minimum( 0 ), maximum( 0 ), edit( NULL )
	{
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindow( TRACKBAR_CLASS, text, style | TBS_BOTH | TBS_NOTICKS | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		if ( edit && edit->window::process( hWnd, message, wParam, lParam ) ) {
			switch ( HIWORD( wParam ) ) {
			case EN_KILLFOCUS:
			case EN_CHANGE:
				{
					int value = atoi( edit->get() );
					if ( ! between( minimum, value, maximum ) ) {
						value = clamp( minimum, value, maximum );
						edit->set( value );
					}
					set( value );
					return true;
				}
				break;
			}
		}
		if ( ! window::process( hWnd, message, wParam, lParam ) ) return false;
		switch ( message ) {
		case WM_NOTIFY:
			{
				LPNMHDR pnmh = (LPNMHDR) lParam;
	 			index = send( TBM_GETPOS );
				if ( changed ) changed( this );
				if ( edit ) {
					if ( index != atoi( edit->get() ) ) {
						edit->set( index );
						return true;
					}
				}
			} break;
		}
		return false;
	}
	void set( int _minimum, int _maximum )
	{
		minimum = send( TBM_GETRANGEMIN );
		if ( minimum != _minimum ) {
			send( TBM_SETRANGEMIN, TRUE, _minimum );
			minimum = _minimum;
		}
		maximum = send( TBM_GETRANGEMAX );
		if ( maximum != _maximum ) {
			send( TBM_SETRANGEMAX, TRUE, _maximum );
			maximum = _maximum;
		}
	}
	void set( int _index )
	{
		if ( edit ) edit->set( _index );
		_index = clamp( minimum, _index, maximum );
		index = send( TBM_GETPOS );
		if ( index != _index ) {
			send( TBM_SETPOS, TRUE, _index );
			index = _index;
		}
	}
	int get()
	{
		return index = send( TBM_GETPOS );
	}
	int operator()() { return get(); }
	void operator()( int src ) { set( src ); }
	void operator++() { set( clamp( minimum, index + 1, maximum ) ); }
	void operator++( int ) { set( clamp( minimum, index + 1, maximum ) ); }
	void operator--() { set( clamp( minimum, index - 1, maximum ) ); }
	void operator--( int ) { set( clamp( minimum, index - 1, maximum ) ); }
};

