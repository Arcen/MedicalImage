class progressbar : public window
{
public:
	int index;
	int minimum;
	int maximum;
	progressbar() : index( 0 ), minimum( 0 ), maximum( 0 ) {}
	progressbar( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd ), index( 0 ), minimum( 0 ), maximum( 0 )
	{}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindow( PROGRESS_CLASS, text, style | PBS_SMOOTH | default_style, x, y, w, h, parent, ( HMENU ) id, instance, NULL );
	}
	void set( int _minimum, int _maximum )
	{
		PBRANGE range;
		send( PBM_GETRANGE, FALSE , LPARAM( & range ) );
		minimum = range.iLow;
		maximum = range.iHigh;
		if ( minimum != _minimum || maximum != _maximum ) {
			send( PBM_SETRANGE32, _minimum, _maximum );
			minimum = _minimum;
			maximum = _maximum;
		}
	}
	void set( int _index )
	{
		_index = clamp( minimum, _index, maximum );
		index = send( PBM_GETPOS );
		if ( index != _index ) {
			send( PBM_SETPOS, _index );
			index = _index;
		}
	}
	int get()
	{
		return index = send( PBM_GETPOS );
	}
	void reset()
	{
		set( 0 );
		set( 0, 0 );
	}
	void expand( int value )
	{
		set( minimum, maximum + value );
	}
	void progress()
	{
		set( clamp( minimum, index + 1, maximum ) );
	}
	int operator()() { return get(); }
	void operator()( int src ) { set( src ); }
	void operator++() { set( clamp( minimum, index + 1, maximum ) ); }
	void operator++( int ) { set( clamp( minimum, index + 1, maximum ) ); }
	void operator--() { set( clamp( minimum, index - 1, maximum ) ); }
	void operator--( int ) { set( clamp( minimum, index - 1, maximum ) ); }
};
