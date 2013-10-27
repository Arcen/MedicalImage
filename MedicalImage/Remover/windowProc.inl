//ウィンドウプロシージャ（いろいろなメッセージを処理するためのコールバック関数）
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static dicom dcm;
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
					dicomImage.finalize();
					
                    domDocument = MSXML2::IXMLDOMDocumentPtr( "MSXML.DOMDocument" );
					if ( dcm.convert( f(), domDocument ) ) {
						f->seek( 0, SEEK_SET );
						if ( dcm.read( f(), dicomImage ) ) {
							char drive[MAX_PATH], path[MAX_PATH], fn[MAX_PATH], extension[MAX_PATH];
							_splitpath( filename, drive, path, fn, extension );
							char rawpath[MAX_PATH];
							_makepath( rawpath, drive, path, fn, ".raw" );
							retainer<file> raw = file::initialize( rawpath, true, true );
							if ( raw ) {
								raw->write( dicomImage.data, dicomImage.size * 2 );
								raw->write( dicomImage.width );
								raw->write( dicomImage.height );
								int band = 2;
								raw->write( band );
							}
						}
					} else {
						domDocument = IXMLDOMDocumentPtr( "MSXML.DOMDocument" );
					}
				}
			}
			break;
		case IDM_REMOVE_FILE://ファイル単位の個人情報削除
			{
				char filename[MAX_PATH];//選択したファイル名をいれるバッファ
				//ファイル選択ダイアログでファイルを選ぶ
				if ( fileOpen( filename, "", "DICOMファイル(*)\0*\0", "DICOMファイルを開く", hWnd ) ) {
					retainer<file> f = file::initialize( filename, false, true );
					if ( ! f ) break;
					char drive[MAX_PATH], path[MAX_PATH], fn[MAX_PATH], extension[MAX_PATH];
					_splitpath( filename, drive, path, fn, extension );
					char outdirectory[MAX_PATH];
					_makepath( outdirectory, drive, path, "", "" );
					if ( selectDirectory( outdirectory, outdirectory, "出力するディレクトリ選択", hWnd ) ) {
						char outfilename[MAX_PATH];
						_makepath( outfilename, "", outdirectory, fn, extension );
						if ( stricmp( filename, outfilename ) == 0 ) {
							strcat( outfilename, ".out" );
						}
						retainer<file> out = file::initialize( outfilename, true, true );
						if ( ! out ) break;
						
						if ( dcm.removeInformation( f(), out() ) ) {
							//成功
						}
					}
				}
			}
			break;
			
		case IDM_QUIT://終了メニュー選択
			DestroyWindow( hWnd );//ウィンドウを破壊
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
				BitBlt( hDC, 0, 0, cr.right, cr.bottom, hDC, 0, 0, WHITENESS );
				EndPaint( hWnd, &ps );
			}
			break;
		case WM_DESTROY://ウィンドウの破壊イベントが来た場合にメッセージループを抜けるようにイベントを投げる
			PostQuitMessage( 0 );
			break;
		default:
			//そのほかのイベントの時にはデフォルトの動作を行う
			return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}
