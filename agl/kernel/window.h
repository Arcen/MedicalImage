
//ウィンドウズ用のツール

//BMPとしてアクセスできるウィンドウ
class windowSurfaceBitmap
{
public:
	HDC hdc;
	HBITMAP object, wk;
	BITMAPINFO bmpinfo;
	LONG & width, & height;
	windowSurfaceBitmap() : hdc( CreateCompatibleDC( NULL ) ), object( NULL ), wk( NULL ), width( bmpinfo.bmiHeader.biWidth ), height( bmpinfo.bmiHeader.biHeight )
	{
		memset( & bmpinfo, 0, sizeof( bmpinfo ) );
	}
	bool initialize( int width, int height )
	{
		if ( this->width == width && this->height == height ) return true;

		finalize();
		if ( width <= 0 || height <= 0 ) return false;
		memset( & bmpinfo, 0, sizeof( bmpinfo ) );
		bmpinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bmpinfo.bmiHeader.biPlanes = 1;
		bmpinfo.bmiHeader.biCompression = BI_RGB;
		bmpinfo.bmiHeader.biWidth = ( LONG ) width;
		bmpinfo.bmiHeader.biHeight = ( LONG ) height;
		bmpinfo.bmiHeader.biBitCount = ( WORD ) 32;
		object = CreateDIBSection( hdc, &bmpinfo, DIB_RGB_COLORS, NULL, NULL, 0 );
		if ( ! object ) return false;
		wk = ( HBITMAP ) SelectObject( hdc, object );
		return true;
	}
	bool finalize()
	{
		if ( object ) {
			SelectObject( hdc, wk );
			DeleteObject( object );
			object = NULL;
			wk = NULL;
		}
		return true;
	}
	~windowSurfaceBitmap()
	{
		finalize();
		DeleteDC( hdc );
	}
	windowSurfaceBitmap & operator=( const windowSurfaceBitmap & surface )
	{
		initialize( surface.width, surface.height );
		BitBlt( hdc, 0, 0, width, height, surface.hdc, 0, 0, SRCCOPY );
		return *this;
	}
	bool operator=( const image & img )
	{
		initialize( img.width, img.height );
		BITMAPINFO bmpsrcinfo;
		memset( & bmpsrcinfo, 0, sizeof( bmpsrcinfo ) );
		bmpsrcinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bmpsrcinfo.bmiHeader.biPlanes = 1;
		bmpsrcinfo.bmiHeader.biCompression = BI_RGB;
		bool shrink = ( bmpinfo.bmiHeader.biWidth != img.width || bmpinfo.bmiHeader.biHeight != img.height ) ? true : false;
		if ( shrink == false ) {
			bmpsrcinfo.bmiHeader.biBitCount = 32;
			bmpsrcinfo.bmiHeader.biWidth = img.width;
			bmpsrcinfo.bmiHeader.biHeight = img.height;
	//		SetStretchBltMode( hdc, HALFTONE );
			StretchDIBits( hdc, 0, 0, bmpinfo.bmiHeader.biWidth, bmpinfo.bmiHeader.biHeight, 
				0, 0, bmpsrcinfo.bmiHeader.biWidth, bmpsrcinfo.bmiHeader.biHeight, 
				img.data, & bmpsrcinfo, DIB_RGB_COLORS, SRCCOPY );
			return true;
		} else {
			bmpsrcinfo.bmiHeader.biWidth = img.width;
			bmpsrcinfo.bmiHeader.biHeight = img.height;
			bmpsrcinfo.bmiHeader.biBitCount = 32;//４バイト長で画像を作るのでパディングがいらなくなる
			image image_src;
			image_src.create( bmpinfo.bmiHeader.biWidth, bmpinfo.bmiHeader.biHeight );
			image_src.copy( img );
			SetDIBitsToDevice( hdc, 0, 0, bmpinfo.bmiHeader.biWidth, bmpinfo.bmiHeader.biHeight, 
				0, 0, bmpinfo.bmiHeader.biWidth, bmpinfo.bmiHeader.biHeight, 
				image_src.data, &bmpsrcinfo, DIB_RGB_COLORS );
		}
		return true;
	}
};

//ウィンドウズ関連
class windows
{
public:
	static HINSTANCE instance;
	const char * className;
	HWND hwnd;
	WNDPROC wndProc;
	int left, top;
	int width, height;
	bool fullscreen;
	HDC hdc;
	HWND parent;
	HACCEL accelerator;
	windows( HINSTANCE _instance, const char * _className, WNDPROC _wndProc, HWND _parent = NULL ) 
		: className( _className ), hwnd( NULL ), wndProc( _wndProc ),
		width( 640 ), height( 480 ), fullscreen( false ), left( 0 ), top( 0 ), parent( _parent ), accelerator( NULL )
	{
		windows::instance = _instance;
	}
	bool registerClass()
	{
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = fullscreen ? 0 : CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = wndProc;
		wcex.hInstance = instance;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.lpszClassName = className;
		RegisterClassEx( &wcex );
		return true;
	}
	bool createWindow( void * lParam = NULL, HMENU menu = NULL, HACCEL _accelerator = NULL )
	{
		hwnd = CreateWindow( className, className, ( fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW ) | WS_CLIPCHILDREN | WS_VISIBLE, 
			left, top, width, height, parent, menu, instance, NULL );
		accelerator = _accelerator;
		if( ! hwnd ) return false;
		SetWindowLong( hwnd, GWL_USERDATA, ( LPARAM ) lParam );
		RECT c;
		GetClientRect( hwnd, &c );
		if ( c.right != width || c.bottom != height ) {
			SetWindowPos( hwnd, NULL, 0, 0, 2 * width - c.right, 2 * height - c.bottom, SWP_NOMOVE | SWP_NOZORDER );
		}
		ShowWindow( hwnd, SW_SHOW );
		UpdateWindow( hwnd );
		hdc = GetDC( hwnd );
		return true;
	}
	bool setWindow( void * lParam, HWND _hwnd, HACCEL _accelerator = NULL )
	{
		hwnd = _hwnd;
		accelerator = _accelerator;
		if( ! hwnd ) return false;
		SetWindowLong( hwnd, GWL_USERDATA, ( LPARAM ) lParam );
		RECT c;
		GetClientRect( hwnd, &c );
		width = c.right - c.left;
		height = c.bottom - c.top;
		hdc = GetDC( hwnd );
		return true;
	}
	bool createDialog( LPCTSTR res, DLGPROC proc, LPARAM param )
	{
		hwnd = CreateDialogParam( instance, res, parent, proc, param );
		if( ! hwnd ) return false;
		SetWindowPos( hwnd, HWND_TOP, left, top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW );
		UpdateWindow( hwnd );
		return true;
	}
	void loop()
	{
		MSG msg;
		while( GetMessage(&msg, NULL, 0, 0) ) 
		{
			if ( accelerator && TranslateAccelerator( hwnd, accelerator, & msg ) ) {
			} else {
				TranslateMessage( &msg );
			}
			DispatchMessage( &msg );
		}
	}
	//ファイル入出力
	static bool fileOpen( char * filename, char * defaultName, char * mask, char * title, HWND parent )
	{
		strcpy( filename, "" );
		if ( defaultName ) strcpy( filename, defaultName );

		OPENFILENAME openfilename; memset( & openfilename, 0, sizeof( openfilename ) );
		openfilename.lStructSize = sizeof( OPENFILENAME );
		openfilename.hwndOwner = parent;
		openfilename.hInstance = instance;
		openfilename.lpstrFilter = mask;
		openfilename.lpstrFile = filename;
		openfilename.nMaxFile = MAX_PATH;
		openfilename.lpstrTitle = title;
		openfilename.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		if ( ! GetOpenFileName( & openfilename ) ) return false;
		return true;
	}
	static bool fileSave( char * filename, char * defaultName, char * mask, char * title, HWND parent, char * defext = NULL )
	{
		strcpy( filename, "" );
		if ( defaultName ) strcpy( filename, defaultName );

		OPENFILENAME openfilename; memset( & openfilename, 0, sizeof( openfilename ) );
		openfilename.lStructSize = sizeof( OPENFILENAME );
		openfilename.hwndOwner = parent;
		openfilename.hInstance = instance;
		openfilename.lpstrFilter = mask;
		openfilename.lpstrFile = filename;
		openfilename.nMaxFile = MAX_PATH;
		openfilename.lpstrTitle = title;
		openfilename.lpstrDefExt = defext;
		openfilename.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT;

		if ( ! GetSaveFileName( & openfilename ) ) return false;
		return true;
	}
    static const REGSAM KEY_READ_BITS = KEY_READ | KEY_EXECUTE;
    static const REGSAM KEY_WRITE_BITS = KEY_ALL_ACCESS;
    
	//レジストリ操作
	static bool get( const char * directory, const char * page, const char * name, string & value )
	{
		bool result = false;
		FILETIME time;
		HKEY reg = NULL;
        if ( RegOpenKeyEx( HKEY_CURRENT_USER, directory, 0, KEY_READ_BITS, & reg ) ) {
		    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, directory, 0, KEY_READ_BITS, & reg ) ) return false;
        }
		for ( int index = 0; ; index++ ) {
			char _page[1024];
			DWORD pageLength = 1024;
			if ( RegEnumKeyEx( reg, index, _page, & pageLength, NULL, NULL, NULL, & time ) ) break;
			if ( strcmp( _page, page ) ) continue;
			HKEY subreg = NULL;
			if ( RegOpenKeyEx( reg, page, 0, KEY_QUERY_VALUE | KEY_READ, & subreg ) ) break;
			for ( int subIndex = 0; ; subIndex++ ) {
				char _name[1024];
				DWORD nameLength = 1024;
				DWORD type = 0;
				BYTE data[1024];
				DWORD dataSize = 1024;
				if ( RegEnumValue( subreg, subIndex, _name, & nameLength, NULL, & type, data, & dataSize ) ) break;
				if ( strcmp( _name, name ) ) continue;
				switch ( type ) {
				case REG_SZ: value = ( const char * ) data; result = true; break;
				}
			}
			RegCloseKey( subreg );
		}
		RegCloseKey( reg );
		return result;
	}
	static bool set( const char * directory, const char * page, const char * name, const string & value )
	{
		HKEY reg = NULL;
		string path = string( directory ) + "\\" + page;
		DWORD dwDisposition = 0;
		if ( RegCreateKeyEx( HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE_BITS, NULL, & reg, & dwDisposition ) ) return false;
		RegSetValueEx( reg, name, 0, REG_SZ, ( const BYTE * ) value.chars(), value.length() );
		RegCloseKey( reg );
		return true;
	}
	static bool kill( const char * directory, const char * page, const char * name )
	{
		HKEY reg = NULL;
		string path = string( directory ) + "\\" + page;
		if ( RegOpenKeyEx( HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, & reg ) == ERROR_SUCCESS )
        {
		    RegDeleteKey( reg, name );
		    RegCloseKey( reg );
        }
		if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, path, 0, KEY_ALL_ACCESS, & reg ) == ERROR_SUCCESS )
        {
		    RegDeleteKey( reg, name );
		    RegCloseKey( reg );
        }
	    return true;
	}
};
#ifdef __GLOBAL__
HINSTANCE windows::instance = NULL;
#endif
