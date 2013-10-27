fc::dialog * mainDialog = NULL;
image viewimage[3];
imageInterface< pixelLuminance<int32> > calculatedimage[3];

BOOL renderImage( image & vi, DRAWITEMSTRUCT & dis )
{
	HDC hDC = dis.hDC;
	RECT cr = dis.rcItem;//��`���
	if ( vi.width && vi.height ) {
		BITMAPINFO bmpsrcinfo;
		memset( & bmpsrcinfo, 0, sizeof( bmpsrcinfo ) );
		bmpsrcinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		bmpsrcinfo.bmiHeader.biPlanes = 1;
		bmpsrcinfo.bmiHeader.biCompression = BI_RGB;
		bmpsrcinfo.bmiHeader.biWidth = vi.width;
		bmpsrcinfo.bmiHeader.biHeight = vi.height;
		bmpsrcinfo.bmiHeader.biBitCount = 32;//�S�o�C�g���ŉ摜�����̂Ńp�f�B���O������Ȃ��Ȃ�
		SetStretchBltMode( hDC, HALFTONE );
		StretchDIBits( hDC, 0, 0, cr.right, cr.bottom, 0, 0, vi.width, vi.height, vi.data, & bmpsrcinfo, DIB_RGB_COLORS, SRCCOPY );
	} else {
		BitBlt( hDC, 0, 0, cr.right, cr.bottom, hDC, 0, 0, WHITENESS );
	}
	return FALSE;
}
void makeImage( image & vi, imageInterface< pixelLuminance<int32> > & ci, list<string> & fileslist, HWND hwnd, HWND hDlg )
{
	//�\���p�̉摜����
	vi.finalize();
	ci.finalize();
	int numimage = 0;
	int selcount = SendMessage( hwnd, LB_GETSELCOUNT, 0, 0 );
	int i = 0;
	imageInterface< pixelLuminance<int16> > imgtemp;
	pixelLuminance<int32> pl32;
	checkMaximum<int32> limitpixel;
	HWND progressbar = GetDlgItem( hDlg, IDC_PROGRESS );
	SendMessage( progressbar, PBM_SETRANGE32, 0, fileslist.size );
	for ( list<string>::iterator it( fileslist ); it; ++it, ++i ) {
		SendMessage( progressbar, PBM_SETPOS, i, 0 );
		//�I�����������C�I������Ă���Ƃ�
		if ( ! ( selcount == 0 || 0 < SendMessage( hwnd, LB_GETSEL, i, 0 ) ) ) continue;
		SendMessage( hwnd, LB_SETSEL, FALSE, i );
		retainer<file> f = file::initialize( it(), false, true );
		if ( ! f ) continue;
		//�t�@�C���ǂݍ���
		if ( ! dcm.read( f(), imgtemp ) ) continue;
		//���a�摜�쐬
		if ( numimage == 0 ) {
			ci.create( imgtemp.width, imgtemp.height );
		} else {
			if ( ci.width != imgtemp.width || 
				ci.height != imgtemp.height ) 
				continue;
		}
		//���Z
		for ( int y = 0; y < imgtemp.height; ++y ) {
			for ( int x = 0; x < imgtemp.width; ++x ) {
				pl32.y = ci.get( x, y ).y + int32( imgtemp.get( x, y ).y );
				limitpixel( pl32.y );
				ci.set( x, y, pl32 );
			}
		}
		++numimage;
		SendMessage( hwnd, LB_SETSEL, TRUE, i );
	}
	SendMessage( progressbar, PBM_SETPOS, 0, 0 );
	if ( numimage == 0 ) return;
	//���ω�
	vi.create( ci.width, ci.height );
	pixel p;
	p.a = 255;
	for ( int y = 0; y < vi.height; ++y ) {
		for ( int x = 0; x < vi.width; ++x ) {
			p.r = p.g = p.b = clamp<uint8>( 0, double( ci.get( x, y ).y ) / limitpixel() * 255, 255 );
			vi.set( x, y, p );
		}
	}
}
void makeImage( HWND hDlg )
{
	EnableWindow( GetDlgItem( hDlg, IDC_SAVE ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_VIEW ), FALSE );
	HWND listwnd[2] = { GetDlgItem( hDlg, IDC_LIST1 ), GetDlgItem( hDlg, IDC_LIST2 ) };
	image & vi = viewimage[2];
	imageInterface< pixelLuminance<int32> > & ci = calculatedimage[2];
	vi.finalize();
	ci.finalize();
	HWND progressbar = GetDlgItem( hDlg, IDC_PROGRESS );
	SendMessage( progressbar, PBM_SETRANGE32, 0, 2 );
	if ( ! calculatedimage[0].enable() ) return;
	if ( ! calculatedimage[1].enable() ) return;
	if ( calculatedimage[0].width != calculatedimage[1].width ||
		calculatedimage[0].height != calculatedimage[1].height ) return;
	//�v�Z
	ci.create( calculatedimage[0].width, calculatedimage[0].height );
	pixelLuminance<int32> pl32;
	checkMaximum<int32> limitpixel;
	for ( int y = 0; y < ci.height; ++y ) {
		for ( int x = 0; x < ci.width; ++x ) {
			int32 ci0 = calculatedimage[0].get( x, y ).y;
			int32 ci1 = calculatedimage[1].get( x, y ).y;
			// per 10000�i�������H�j
			if ( ci1 ) {
				pl32.y = int32( clamp<double>( -0x7FFFFFFF, ( double( ci0 ) - double( ci1 ) ) / double( ci1 ) * 10000, 0x7FFFFFFF ) );
			} else {
				pl32.y = int32( clamp<double>( -0x7FFFFFFF, 0, 0x7FFFFFFF ) );
			}
			limitpixel( pl32.y );
			ci.set( x, y, pl32 );
		}
	}
	SendMessage( progressbar, PBM_SETPOS, 1, 0 );
	vi.create( ci.width, ci.height );
	pixel p;
	p.a = 255;
	for ( int y = 0; y < ci.height; ++y ) {
		for ( int x = 0; x < ci.width; ++x ) {
			int32 i = ci.get( x, y ).y / 100;
			p.r = p.g = p.b = 0;
			if ( i < 0 ) {
				p.r = uint8( clamp<int32>( 0, -i, 255 ) );
			} else {
				p.b = uint8( clamp<int32>( 0, i, 255 ) );
			}
			vi.set( x, y, p );
		}
	}
	SendMessage( progressbar, PBM_SETPOS, 0, 0 );
	EnableWindow( GetDlgItem( hDlg, IDC_SAVE ), TRUE );
	EnableWindow( GetDlgItem( hDlg, IDC_VIEW ), TRUE );
}
void updateImage( double rate )
{
	image & vi = viewimage[2];
	imageInterface< pixelLuminance<int32> > & ci = calculatedimage[2];
	if ( ci.empty() ) return;
	//�v�Z
	pixelLuminance<int32> pl32;
	checkMaximum<int32> limitpixel;
	for ( int y = 0; y < ci.height; ++y ) {
		for ( int x = 0; x < ci.width; ++x ) {
			limitpixel( abs( ci.get( x, y ).y ) );
		}
	}
	vi.finalize();
	vi.create( ci.width, ci.height );
	pixel p;
	p.a = 255;
	for ( int y = 0; y < ci.height; ++y ) {
		for ( int x = 0; x < ci.width; ++x ) {
			int32 i = int32( ci.get( x, y ).y * rate / 100 );
			p.r = p.g = p.b = 0;
			if ( i < 0 ) {
				p.r = uint8( clamp<int32>( 0, -i, 255 ) );
			} else {
				p.b = uint8( clamp<int32>( 0, i, 255 ) );
			}
			vi.set( x, y, p );
		}
	}
}

void updateList( HWND hDlg )
{
	HWND listwnd[2] = { GetDlgItem( hDlg, IDC_LIST1 ), GetDlgItem( hDlg, IDC_LIST2 ) };
	for ( int i = 0; i < 2; ++i ) {
		//���X�g�̃A�b�v�f�[�g
		HWND listwindow = listwnd[i];
		SendMessage( listwindow, LB_RESETCONTENT, 0, 0 );//����
		for ( list<string>::iterator it( fileslists[i] ); it; ++it ) {
			char wk[1024];
			char drive[MAX_PATH], dir[MAX_PATH], name[MAX_PATH], ext[MAX_PATH];
			_splitpath( it().chars(), drive, dir, name, ext );
			sprintf( wk, "%s : %s", name, it().chars() );
			SendMessage( listwindow, LB_ADDSTRING, 0, LPARAM( wk ) );
		}
	}
}

void dicomdirListup( const MSXML2::IXMLDOMElementPtr & root, int targetOffset, list<string> & fileslist, char * path )
{
	MSXML2::IXMLDOMElementPtr element;
	//�����i�f�B���N�g���̍��ځj
	for ( element = root->firstChild; element != NULL; element = element->nextSibling ) {
		MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "offset" ) );
		if ( attr ) {
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( wk.toi() == targetOffset ) break;
		}
	}
	if ( element ) {} else return;
	//�񋓁i���ړ��̃G�������g�ɂ��āj
	int nextSibling = 0, firstChild = 0;
	string itemName;
	for ( MSXML2::IXMLDOMElementPtr item = element->firstChild; item != NULL; item = item->nextSibling ) {
		MSXML2::IXMLDOMAttributePtr attr = item->getAttributeNode( _bstr_t( "name" ) );
		if ( attr ) {} else continue;
		string name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		if ( name == "���̃f�B���N�g�����R�[�h�̃I�t�Z�b�g" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			nextSibling = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "�Q�Ɖ��ʃf�B���N�g���G���e�B�e�B�̃I�t�Z�b�g" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			firstChild = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "�f�B���N�g�����R�[�h�^�C�v" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			itemName = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		}
	}
	if ( itemName.length() >= 5 && memcmp( itemName.chars(), "IMAGE", 5 ) == 0 ) {
		for ( MSXML2::IXMLDOMElementPtr item = element->firstChild; item != NULL; item = item->nextSibling ) {
			MSXML2::IXMLDOMAttributePtr attr = item->getAttributeNode( _bstr_t( "name" ) );
			string name;
			if ( attr ) {} else continue;
			name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( name != "�Q�ƃt�@�C���h�c" ) continue;
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			char drive[MAX_PATH], dir[MAX_PATH], filename[MAX_PATH], ext[MAX_PATH];
			_splitpath( path, drive, dir, filename, ext );
			char dicomfilename[MAX_PATH];
			_makepath( dicomfilename, drive, dir, wk.chars(), "" );
			fileslist.push_back( dicomfilename );
		}
	}
	if ( firstChild ) dicomdirListup( root, firstChild,fileslist, path );
	if ( nextSibling ) dicomdirListup( root, nextSibling,fileslist, path );
}
HANDLE event;
BOOL CALLBACK ViewDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK DialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_INITDIALOG:
		{
			SetWindowText( GetDlgItem( hDlg, IDC_MEMO ), 
				"�t�@�C���̓o�^�F���X�g�Ƀh���b�O�A���h�h���b�v\r\n"
				"�t�@�C���̖����F���X�g���A�N�e�B�u��ԂɂāA�I������DELETE�L�["
				"���X�g�̉E���̋�`���N���b�N�őI������Ă���摜�i��I���Ȃ�S�āj���畽�ω摜���쐬\r\n"
				"([100%]-[0%])/[0%]�Ő��͐C���͐ԂɂȂ�܂�\r\n"
				"�ڍו\�����͍��N���b�N�ł��̓_�́����A�E�h���b�O�ŗ̈���w�肵���̕��ς́�����ɕ\�����܂�\r\n"
				);
			EnableWindow( GetDlgItem( hDlg, IDC_SAVE ), FALSE );
			EnableWindow( GetDlgItem( hDlg, IDC_VIEW ), FALSE );
		}
		return 1;
	case WM_DROPFILES:
		{
			HDROP hdrop = HDROP( wParam );
			//�ǂ��Ƀh���b�v���ꂽ�����ׂ�
			POINT droppoint;
			bool found = false;
			bool firstlist = true;
			if ( DragQueryPoint( hdrop, & droppoint ) ) {
				HWND dropwnd = ChildWindowFromPoint( hDlg, droppoint );
				if ( dropwnd == GetDlgItem( hDlg, IDC_LIST1 ) ) {
					found = true;
					firstlist = true;
				} else if ( dropwnd == GetDlgItem( hDlg, IDC_LIST2 ) ) {
					found = true;
					firstlist = false;
				}
			}
			if ( ! found ) {
				DragFinish( hdrop );
				break;
			}
			list<string> & fileslist = fileslists[firstlist ? 0 : 1];
			//�h���b�v���ꂽ�t�@�C���ꗗ
			char path[MAX_PATH];
			int number = DragQueryFile( hdrop, 0xFFFFFFFF, path, MAX_PATH );
			for ( int i = 0; i < number; ++i ) {
				DragQueryFile( hdrop, i, path, MAX_PATH );
				bool found = false;
				for ( list<string>::iterator it( fileslist ); it; ++it ) {
					if ( _stricmp( it().chars(), path ) == 0 ) {
						found = true;
						break;
					}
				}
				if ( ! found ) {
					char drive[MAX_PATH], dir[MAX_PATH], name[MAX_PATH], ext[MAX_PATH];
					_splitpath( path, drive, dir, name, ext );
					if ( _stricmp( name, "DICOMDIR" ) == 0 ) {
						//DICOMDIR�̏ꍇ
						retainer<file> f = file::initialize( path, false, true );
						MSXML2::IXMLDOMDocumentPtr dom = IXMLDOMDocumentPtr( "MSXML.DOMDocument" );
						if ( dcm.convert( f(), dom ) ) {
							int firstChildOffset = 0;
							MSXML2::IXMLDOMElementPtr element = dom->documentElement;
							for ( element = element ? element->firstChild : NULL; element != NULL; element = element->nextSibling ) {
								string name = static_cast<const char*>( static_cast<_bstr_t>( element->nodeName ) );
								MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "name" ) );
								if ( attr ) {} else continue;
								name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
								if ( name == "���[�g�f�B���N�g���G���e�B�e�B�̍ŏ��̃f�B���N�g�����R�[�h�̃I�t�Z�b�g" ) {
									attr = element->getAttributeNode( _bstr_t( "value" ) );
									if ( attr ) {} else continue;
									firstChildOffset = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
								} else if ( name == "�f�B���N�g�����R�[�h�V�[�P���X" ) {
									if ( firstChildOffset && element->hasChildNodes() ) {
										dicomdirListup( element, firstChildOffset, fileslist, path );
									}
								}
							}
						}
					} else {
						DWORD attr = GetFileAttributes( path );
						if ( ! ( FILE_ATTRIBUTE_DIRECTORY & attr ) ) {
							fileslist.push_back( path );
						}
					}
				}
			}
			DragFinish( hdrop );
			//���X�g�̃A�b�v�f�[�g
			updateList( hDlg );
		}
		break;
	case WM_VKEYTOITEM:
		{
			int vkey = LOWORD(wParam);
			int nCaretPos = HIWORD(wParam);
			HWND hwndLB = HWND( lParam );
			if ( vkey == VK_DELETE ) {
				bool firstlist = true;
				if ( hwndLB == GetDlgItem( hDlg, IDC_LIST1 ) ) {
					firstlist = true;
				} else if ( hwndLB == GetDlgItem( hDlg, IDC_LIST2 ) ) {
					firstlist = false;
				} else {
					break;
				}
				int selcount = SendMessage( hwndLB, LB_GETSELCOUNT, 0, 0 );
				if ( ! selcount ) break;
				list<string> & fileslist = fileslists[firstlist ? 0 : 1];
				list<string> newlist;
				int i = 0;
				for ( list<string>::iterator it( fileslist ); it; ++it, ++i ) {
					if ( 0 < SendMessage( hwndLB, LB_GETSEL, i, 0 ) ) {
					} else {
						newlist.push_back( it() );
					}
				}
				fileslist = newlist;
				updateList( hDlg );
			}
			return -1;//�f�t�H���g�̓����������
		}
		break;
	case WM_DRAWITEM:
		{
			UINT idCtl = (UINT) wParam;             // control identifier 
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; // item-drawing information
			switch ( idCtl ) {
			case IDC_IMAGE1:
				return renderImage( viewimage[0], *lpdis );
			case IDC_IMAGE2:
				return renderImage( viewimage[1], *lpdis );
			case IDC_IMAGE3:
				return renderImage( viewimage[2], *lpdis );
			}
		}
		break;
	case WM_COMMAND:
		switch( LOWORD( wParam ) ) {
		case IDC_IMAGE1:
			makeImage( viewimage[0], calculatedimage[0], fileslists[0], GetDlgItem( hDlg, IDC_LIST1 ), hDlg );
			makeImage( hDlg );
			InvalidateRect( GetDlgItem( hDlg, IDC_IMAGE1 ), NULL, FALSE );
			InvalidateRect( GetDlgItem( hDlg, IDC_IMAGE3 ), NULL, FALSE );
			break;
		case IDC_IMAGE2:
			makeImage( viewimage[1], calculatedimage[1], fileslists[1], GetDlgItem( hDlg, IDC_LIST2 ), hDlg );
			makeImage( hDlg );
			InvalidateRect( GetDlgItem( hDlg, IDC_IMAGE3 ), NULL, FALSE );
			InvalidateRect( GetDlgItem( hDlg, IDC_IMAGE2 ), NULL, FALSE );
			break;
		case IDC_SAVE:
			{
				imageInterface<pixelRGB> result;
				result.reformat( viewimage[2] );
				char savefilename[MAX_PATH];
				if ( windows::fileSave( savefilename, "output.bmp", "BMP�t�@�C��\0*.bmp\0", "�t�@�C���ۑ�(BMP)", hDlg ) ) {
					retainer<file> savefile = file::initialize( savefilename, true, true );
					bitmap<pixelRGB>::write( savefile(), result );
				}
			}
			break;
		case IDC_VIEW:
			{
				event = CreateEvent( NULL, FALSE, FALSE, "waitview" );
				CreateWindow( "differ", "�ڍו\��", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, fc::window::instance, NULL );
				mainDialog->enable( false );
				MSG msg;
				int r = WAIT_TIMEOUT;
				while( r == WAIT_TIMEOUT ) {
					r = WaitForSingleObject( event, 0 );
					if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
						TranslateMessage( &msg );
						DispatchMessage( &msg );
					}
					Sleep( 0 );
				}
				mainDialog->enable( true );
			}
			break;
		case IDC_QUIT:
//		case IDCANCEL:
			if ( MessageBox( hDlg, "�I���Ȃ�u�͂��v�������Ă�������", "�I�����܂����H", MB_YESNO | MB_DEFBUTTON2 ) == IDYES ) {
				EndDialog( hDlg, LOWORD( wParam ) );
			}
			break;
		}
		break;
	case WM_DESTROY:
		break;
	case WM_QUIT:
		break;
	default:
		return 0;
	}
	return 1;
}
