enum {
	WM_CONVERT_IMAGE = WM_USER,
};

//ウィンドウプロシージャ（いろいろなメッセージを処理するためのコールバック関数）
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message ) {
	case WM_COMMAND:// メニュー選択イベント
		switch( LOWORD(wParam) ) {
		case IDM_OPEN://開くメニュー選択
			{
				char filename[MAX_PATH];//選択したファイル名をいれるバッファ
				//ファイル選択ダイアログでファイルを選ぶ
				if ( fileOpen( filename, "", "DICOMファイル(*)\0*\0", "DICOMファイルを開く", hWnd ) ) {
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
								if ( dcm.read_value( domDocument, "ウィンドウ中心", wk ) ) {
									displayImageCenter = wk;
								}
								if ( dcm.read_value( domDocument, "ウィンドウ幅", wk ) ) {
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
		case IDM_QUIT://終了メニュー選択
			DestroyWindow( hWnd );//ウィンドウを破壊
			break;
		case IDM_CONTRAST:
			if ( DialogBox( hInstance, MAKEINTRESOURCE( IDD_CONTRAST ), hWnd, ContrastDialogProc ) ) {
				SendMessage( hWnd, WM_CONVERT_IMAGE, 0, 0 );
			}
			break;
		case IDM_FIT:
			if ( dicomImage.width && dicomImage.height ) {
				RECT cr, wr;//矩形情報
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
			//他のメニューを選んだときにはデフォルトの動作をさせる
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
	case WM_PAINT://描画イベント
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint (hWnd, &ps);//更新領域のみのデバイスコンテキストを取得
			RECT cr;//矩形情報
			GetClientRect( hWnd, & cr );//ウィンドウの描画できる大きさを取得
			if ( dicomImage.width && dicomImage.height ) {
				BITMAPINFO bmpsrcinfo;
				memset( & bmpsrcinfo, 0, sizeof( bmpsrcinfo ) );
				bmpsrcinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
				bmpsrcinfo.bmiHeader.biPlanes = 1;
				bmpsrcinfo.bmiHeader.biCompression = BI_RGB;
				bmpsrcinfo.bmiHeader.biWidth = displayImage.width;
				bmpsrcinfo.bmiHeader.biHeight = displayImage.height;
				bmpsrcinfo.bmiHeader.biBitCount = 32;//４バイト長で画像を作るのでパディングがいらなくなる
				SetStretchBltMode( hDC, HALFTONE );
				StretchDIBits( hDC, 0, 0, cr.right, cr.bottom, 0, 0, displayImage.width, displayImage.height, displayImage.data, & bmpsrcinfo, DIB_RGB_COLORS, SRCCOPY );
			} else {
				BitBlt( hDC, 0, 0, cr.right, cr.bottom, hDC, 0, 0, WHITENESS );
			}
			EndPaint( hWnd, &ps );
		}
		break;
	case WM_DESTROY://ウィンドウの破壊イベントが来た場合にメッセージループを抜けるようにイベントを投げる
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
		//そのほかのイベントの時にはデフォルトの動作を行う
		return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}
