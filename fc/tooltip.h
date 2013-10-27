class tooltip : public window
{
public:
	tooltip()
	{
	}
	tooltip( HWND _parent, int _id, HWND _hwnd = NULL ) : 
		window( _parent, _id, _hwnd )
	{
	}
	~tooltip()
	{
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateWindowEx( 0, TOOLTIPS_CLASS, NULL, style | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, instance, NULL );
	}
	virtual bool process( HWND hWnd, UINT message, WPARAM wParam, LPARAM /*lParam*/ )
	{
		return false;
	}
	void add( window * w, char * text )
	{
		TOOLINFO ti = { sizeof( TOOLINFO ) };
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.hwnd = parent;
		ti.lpszText = text;
		ti.uId = UINT( w->hwnd );
		send( TTM_ADDTOOL, 0, LPARAM( & ti ) );
	}
};
