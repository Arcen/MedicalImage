class dialog : public window
{
public:
	DLGPROC proc;
	dialog() : proc( NULL )
	{
	}
	dialog( HWND _parent, int _id, DLGPROC _proc ) : window( _parent, _id, NULL ), proc( _proc )
	{
	}
	virtual ~dialog()
	{
	}
	virtual void initialize( DLGPROC _proc )
	{
		proc = _proc;
	}
	bool call()
	{
		if ( ! proc ) return false;
		return DialogBoxParam( window::instance, MAKEINTRESOURCE( id ), parent, (DLGPROC)proc, (LPARAM)this ) ? true : false;
	}
	virtual void create( int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int w = CW_USEDEFAULT, int h = CW_USEDEFAULT )
	{
		hwnd = CreateDialogParam( window::instance, MAKEINTRESOURCE( id ), parent, (DLGPROC)proc, (LPARAM)this );
	}
private:
	static int align( int value, int size )
	{
		if ( value % size ) return static_cast<int>( ( value + size - 1 ) / size ) * size;
		return value;
	}
	static void * align4( void * value )
	{
		const unsigned int addr = reinterpret_cast<unsigned int>( value );
		char * const cp = static_cast<char*>( value );
		if ( addr & 0x3 ) {
			return reinterpret_cast<void *>( cp + 4 - static_cast<int>( addr & 0x3 ) );
		}
		return value;
	}
	static const array<WCHAR> toUNICODE( string s )
	{
		int length = MultiByteToWideChar( CP_ACP, 0, s.chars(), -1, NULL, 0 );
		array<WCHAR> result( length );
		int converted = MultiByteToWideChar( CP_ACP, 0, s.chars(), -1, result.data, result.size );
		assert( converted == length );
		assert( converted != 0 );
		return result;
	}
public:
	//ダイアログテンプレート構造作成
	static void setTemplate( array<char> & buffer, WORD cdit = 1, 
		short x = 0, short y = 0, 
		short cx = 640, short cy = 480, 
		string title = "", string menu = "", 
		DWORD style = WS_SYSMENU | WS_CAPTION | WS_POPUP | WS_VISIBLE |
		DS_MODALFRAME | DS_NOIDLEMSG | DS_CENTER | DS_SETFOREGROUND, 
		DWORD dwExtendedStyle = 0, 
		string classname = "",
		WORD fontsize = 10, string fontname = "" )
	{
		//先頭はDWORD align,各要素はWORD align
		// DLGTEMPLATE(4A), menu(UNICODE/0xFFFF+value(2),2A), 
		// classname(UNICODE/0xFFFF+value(2),2A), title(UNICODE,2A), 
		// if ( style & DS_SETFONT ) + point size(2,2A), typename(UNICODE,2A), 
		int size = align( sizeof( DLGTEMPLATE ), sizeof( WORD ) );
		//メニュー menu string / 0x0000 (nothing) / 0xFFFF + menu id
		const int offset_menu = size;
		WORD menu_value = 0x0000;
		if ( menu.length() == 6 ) {
			char * wk = NULL;
			menu_value = static_cast<unsigned short>( strtol( menu.chars(), & wk, 0 ) );
			menu = "";
		}
		array<WCHAR> menuU = toUNICODE( menu );
		size += ( menu_value ? sizeof( WORD ) + sizeof( WORD ) : menuU.size * sizeof( WCHAR ) );
		//クラス名 classname string(windowclass) / 0x0000 (システムが利用している事前定義されたクラスID) / 0xFFFF + id(事前定義されたクラスID)
		const int offset_classname = size;
		WORD classname_value = 0x0000;
		if ( classname.length() == 6 ) {
			char * wk = NULL;
			classname_value = static_cast<unsigned short>( strtol( classname.chars(), & wk, 0 ) );
			classname = "";
		}
		array<WCHAR> classnameU = toUNICODE( classname );
		size += ( classname_value ? sizeof( WORD ) + sizeof( WORD ) : classnameU.size * sizeof( WCHAR ) );
		//タイトル
		const int offset_title = size;
		array<WCHAR> titleU = toUNICODE( title );
		size += titleU.size * sizeof( WCHAR );
		//フォント
		const int offset_font = size;
		const int offset_fontname = size + sizeof( WORD );
		array<WCHAR> fontnameU;
		if ( style & DS_SETFONT ) {
			fontnameU = toUNICODE( fontname );
			size += sizeof( WORD );
			size += fontnameU.size * sizeof( WCHAR );
		}
		size = align( size, sizeof( DWORD ) );
		buffer.allocate( size );
		ZeroMemory( buffer.data, buffer.size );
		DLGTEMPLATE * tmpl = reinterpret_cast<DLGTEMPLATE*>( buffer.data );
		tmpl->cdit = cdit;
		tmpl->cx = cx;
		tmpl->cy = cy;
		tmpl->dwExtendedStyle = dwExtendedStyle;
		tmpl->style = style;
		tmpl->x = x;
		tmpl->y = y;
		//メニュー
		{
			WORD * p = reinterpret_cast<WORD*>( buffer.data + offset_menu );
			if ( menu_value ) {
				p[0] = 0xFFFF;
				p[1] = menu_value;
			} else {
				CopyMemory( p, menuU.data, menuU.size * sizeof( WCHAR ) );
			}
		}
		//クラス名
		{
			WORD * p = reinterpret_cast<WORD*>( buffer.data + offset_classname );
			if ( classname_value ) {
				p[0] = 0xFFFF;
				p[1] = classname_value;
			} else {
				CopyMemory( p, classnameU.data, classnameU.size * sizeof( WCHAR ) );
			}
		}
		//タイトル　UNICODEの文字無しは0x0000なので，そのまま代用
		CopyMemory( buffer.data + offset_title, titleU.data, titleU.size * sizeof( WCHAR ) );
		//フォント
		if ( style & DS_SETFONT ) {
			*reinterpret_cast<WORD*>( buffer.data + offset_font ) = fontsize;
			WORD * p = reinterpret_cast<WORD*>( buffer.data + offset_fontname );
			CopyMemory( p, fontnameU.data, fontnameU.size * sizeof( WCHAR ) );
		}
	}
	//ダイアログテンプレート用アイテム構造作成
	static void setTemplateItem( array<char> & buffer, WORD id, 
		short x, short y, short cx, short cy, 
		string title = "", string classname = "", 
		DWORD style = WS_CHILD | WS_VISIBLE, 
		DWORD dwExtendedStyle = 0, 
		array<char> creation = array<char>( 0 ) )
	{
		//先頭はDWORD align,各要素はWORD align
		// DLGITEMTEMPLATE(4A), class(UNICODE/0xFFFF+WORD,2A), 
		// title(UNICODE/0xFFFF+WORD,2A), creation((size)WORD+byte data,2A)
		int size = align( sizeof( DLGITEMTEMPLATE ), 2 );

		//クラス名 classname string(windowclass) / 0x0000 (システムが利用している事前定義されたクラスID) / 0xFFFF + id(事前定義されたクラスID)
		const int offset_classname = size;
		WORD classname_value = 0x0000;
		if ( classname.length() == 6 ) {
			char * wk = NULL;
			classname_value = static_cast<unsigned short>( strtol( classname.chars(), & wk, 0 ) );
			classname = "";
		}
		array<WCHAR> classnameU = toUNICODE( classname );
		size += ( classname_value ? sizeof( WORD ) + sizeof( WORD ) : classnameU.size * sizeof( WCHAR ) );
		//タイトル
		const int offset_title = size;
		array<WCHAR> titleU = toUNICODE( title );
		size += titleU.size * sizeof( WCHAR );
		//生成情報
		const int offset_creation = size;
		size = size + sizeof( WORD ) + creation.size;

		size = align( size, 4 );
		buffer.allocate( size );
		ZeroMemory( buffer.data, buffer.size );
		DLGITEMTEMPLATE * item = reinterpret_cast<DLGITEMTEMPLATE*>( buffer.data );
		item->cx = cx;
		item->cy = cy;
		item->dwExtendedStyle = dwExtendedStyle;
		item->id = id;
		item->style = style;
		item->x = x;
		item->y = y;
		//クラス名
		{
			WORD * p = reinterpret_cast<WORD*>( buffer.data + offset_classname );
			if ( classname_value ) {
				p[0] = 0xFFFF;
				p[1] = classname_value;
			} else {
				CopyMemory( p, classnameU.data, classnameU.size * sizeof( WCHAR ) );
			}
		}
		//タイトル　UNICODEの文字無しは0x0000なので，そのまま代用
		CopyMemory( buffer.data + offset_title, titleU.data, titleU.size * sizeof( WCHAR ) );
		//生成情報
		*reinterpret_cast<WORD*>( buffer.data + offset_creation ) = creation.size;
		if ( creation.size ) {
			CopyMemory( buffer.data + offset_creation + sizeof( WORD ), creation.data, creation.size );
		}
	}
private:
	static bool align( array<char> & aligned, const list< array<char> > & templates )
	{
		int size = 0;
		for ( list< array<char> >::iterator it( templates ); it; ++it ) {
			size += align( it->size, sizeof( DWORD ) );
		}
		if ( ! size ) return false;
		aligned.allocate( size + 8 );
		ZeroMemory( aligned.data, aligned.size );
		char * p = reinterpret_cast<char*>( align4( aligned.data ) );
		for ( list< array<char> >::iterator it( templates ); it; ++it ) {
			CopyMemory( p, it->data, it->size );
			p += align( it->size, sizeof( DWORD ) );
		}
		return true;
	}
public:
	bool call( const list< array<char> > & templates )
	{
		if ( ! proc ) return false;
		array<char> aligned;
		if ( ! align( aligned, templates ) ) return false;
		DLGTEMPLATE * p = reinterpret_cast<DLGTEMPLATE*>( align4( aligned.data ) );
		return DialogBoxIndirectParam( window::instance, p, parent, proc, reinterpret_cast<LPARAM>( this ) ) == IDOK;
	}
	bool create( const list< array<char> > & templates )
	{
		if ( ! proc ) return false;
		array<char> aligned;
		if ( ! align( aligned, templates ) ) return false;
		DLGTEMPLATE * p = reinterpret_cast<DLGTEMPLATE*>( align4( aligned.data ) );
		hwnd = CreateDialogIndirectParam( window::instance, p, parent, proc, reinterpret_cast<LPARAM>( this ) );
	}
	void loop( procedure idle = NULL, int accelerators = 0 )
	{
		HACCEL hAccel = ( accelerators == 0 ? NULL : LoadAccelerators( instance, MAKEINTRESOURCE( accelerators ) ) );
		MSG msg;
		while ( 1 ) {
			if ( PeekMessage( & msg, NULL, 0, 0, PM_NOREMOVE ) ) {
				BOOL r = GetMessage( & msg, NULL, 0, 0 );
				if ( r == 0 ) return;
				if ( hAccel == NULL || 
					! TranslateAccelerator( hwnd, hAccel, &msg ) ) {
					if ( ! IsDialogMessage( hwnd, & msg ) ) {
						TranslateMessage( &msg );
						DispatchMessage( &msg );
					}
				}
			} else {
				if ( idle ) idle( NULL );
				Sleep( 1 );
			}
		}
	}
};
