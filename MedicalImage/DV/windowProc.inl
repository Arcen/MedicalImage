enum {
	WM_CONVERT_IMAGE = WM_USER,
};

//�E�B���h�E�v���V�[�W���i���낢��ȃ��b�Z�[�W���������邽�߂̃R�[���o�b�N�֐��j
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message ) {
	case WM_COMMAND:// ���j���[�I���C�x���g
		switch( LOWORD(wParam) ) {
		case IDM_OPEN://�J�����j���[�I��
			{
				char filename[MAX_PATH];//�I�������t�@�C�����������o�b�t�@
				//�t�@�C���I���_�C�A���O�Ńt�@�C����I��
				if ( fileOpen( filename, "", "DICOM�t�@�C��(*)\0*\0", "DICOM�t�@�C�����J��", hWnd ) ) {
					retainer<file> f = file::initialize( filename, false, true );
					if ( ! f ) break;
					validDocument = false;
					elementsTreeWindow->release();
					elementViewerWindow->show( false );
					dicomImage.finalize();
					InvalidateRect( hWnd, NULL, FALSE );

					domDocument = MSXML2::IXMLDOMDocumentPtr( "MSXML.DOMDocument" );
					static dicom dcm;
					if ( dcm.convert( f(), domDocument ) ) {
						validDocument = true;
						dom2treeview( domDocument, *elementsTreeWindow );
						char drive[MAX_PATH], path[MAX_PATH], fn[MAX_PATH], extension[MAX_PATH];
						_splitpath( filename, drive, path, fn, extension );
						if ( stricmp( fn, "DICOMDIR" ) == 0 ) {
							dicomdir2treeview( domDocument, *elementsTreeWindow );
						} else {
							f->seek( 0, SEEK_SET );
							if ( dcm.read( f(), dicomImage ) ) {
								int wk;
								if ( dcm.read_value( domDocument, "�E�B���h�E���S", wk ) ) {
									displayImageCenter = wk;
								}
								if ( dcm.read_value( domDocument, "�E�B���h�E��", wk ) ) {
									displayImageWidth = wk;
								}
								SendMessage( hWnd, WM_CONVERT_IMAGE, 0, 0 );
								SendMessage( hWnd, WM_COMMAND, IDM_FIT, 0 );
							} else {
								dicomImage.finalize();
							}
						}
						elementViewerWindow->show( true );
					} else {
						domDocument = IXMLDOMDocumentPtr( "MSXML.DOMDocument" );
					}
				}
			}
			break;
		case IDM_QUIT://�I�����j���[�I��
			DestroyWindow( hWnd );//�E�B���h�E��j��
			break;
		case IDM_CONTRAST:
			if ( DialogBox( hInstance, MAKEINTRESOURCE( IDD_CONTRAST ), hWnd, ContrastDialogProc ) ) {
				SendMessage( hWnd, WM_CONVERT_IMAGE, 0, 0 );
			}
			break;
		case IDM_FIT:
			if ( dicomImage.width && dicomImage.height ) {
				RECT cr, wr;//��`���
				GetClientRect( hWnd, & cr );
				GetWindowRect( hWnd, & wr );
				imageWindow->resize( -1, -1, 
					dicomImage.width + ( wr.right - wr.left ) - ( cr.right - cr.left ), 
					dicomImage.height + ( wr.bottom - wr.top ) - ( cr.bottom - cr.top ) );
			}
			break;
		case IDM_ATTRIBUTE:
			elementViewerWindow->show( validDocument && ! elementViewerWindow->show() );
			break;
		default:
			//���̃��j���[��I�񂾂Ƃ��ɂ̓f�t�H���g�̓����������
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
	case WM_PAINT://�`��C�x���g
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint (hWnd, &ps);//�X�V�̈�݂̂̃f�o�C�X�R���e�L�X�g���擾
			RECT cr;//��`���
			GetClientRect( hWnd, & cr );//�E�B���h�E�̕`��ł���傫�����擾
			if ( dicomImage.width && dicomImage.height ) {
				BITMAPINFO bmpsrcinfo;
				memset( & bmpsrcinfo, 0, sizeof( bmpsrcinfo ) );
				bmpsrcinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
				bmpsrcinfo.bmiHeader.biPlanes = 1;
				bmpsrcinfo.bmiHeader.biCompression = BI_RGB;
				bmpsrcinfo.bmiHeader.biWidth = displayImage.width;
				bmpsrcinfo.bmiHeader.biHeight = displayImage.height;
				bmpsrcinfo.bmiHeader.biBitCount = 32;//�S�o�C�g���ŉ摜�����̂Ńp�f�B���O������Ȃ��Ȃ�
				SetStretchBltMode( hDC, HALFTONE );
				StretchDIBits( hDC, 0, 0, cr.right, cr.bottom, 0, 0, displayImage.width, displayImage.height, displayImage.data, & bmpsrcinfo, DIB_RGB_COLORS, SRCCOPY );
			} else {
				BitBlt( hDC, 0, 0, cr.right, cr.bottom, hDC, 0, 0, WHITENESS );
			}
			EndPaint( hWnd, &ps );
		}
		break;
	case WM_DESTROY://�E�B���h�E�̔j��C�x���g�������ꍇ�Ƀ��b�Z�[�W���[�v�𔲂���悤�ɃC�x���g�𓊂���
		PostQuitMessage( 0 );
		break;
	case WM_CONVERT_IMAGE:
		{
			if ( dicomImage.width == 0 || dicomImage.height == 0 ) break;
			displayImage.create( dicomImage.width, dicomImage.height );
			pixel dp;
			dp.a = 255;
			for ( int y = 0; y < dicomImage.height; ++y ) {
				for ( int x = 0; x < dicomImage.width; ++x ) {
					const pixelLuminance<int16> & sp = dicomImage.getInternal( x, y );
					dp.r = dp.g = dp.b = static_cast<unsigned char>( static_cast<int>( clamp<double>( 0, static_cast<double>( static_cast<int32>( sp.y ) - ( displayImageCenter - displayImageWidth / 2 ) ) / displayImageWidth * 255.0, 255 ) ) );
					displayImage.setInternal( x, y, dp );
				}
			}
			InvalidateRect( hWnd, NULL, FALSE );
		}
		break;
	default:
		//���̂ق��̃C�x���g�̎��ɂ̓f�t�H���g�̓�����s��
		return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}
