#include "main.h"
#include "adom.h"
#include "bmptool.inl"
#include "document.h"
#include "view.h"
#include "resource.h"
#include "state.h"
#include "reg.h"
//#include "dicom.h"

//ウィンドウ関連
HINSTANCE hInstance = NULL;
fc::window * mainWindow = NULL;
fc::window * imageWindow = NULL;
fc::statusbar * statusBar = NULL;
fc::toolbar * toolBar = NULL;
fc::rebar * reBar = NULL;
fc::slider * sliderBar = NULL;
fc::menu * mainMenu = NULL;
fc::window * windowControlWindow = NULL;
fc::slider * windowSizeBar = NULL;
fc::slider * windowLevelBar = NULL;
fc::editbox * windowSizeEditbox = NULL;
fc::editbox * windowLevelEditbox = NULL;
fc::tooltip * tooltip = NULL;
const int windowTextWidth = 40;

point2<int> editingPoint;
int windowLevel = 32, windowSize = 64;

void clipboardCopy()
{
	//imageWindowの内容をメモリDCにコピーする．
	windowSurfaceBitmap wsb;
	RECT r = imageWindow->client();
	int w = r.right - r.left, h = r.bottom - r.top;
	wsb.initialize( w, h );
	HDC hdc = imageWindow->getDC();
    BitBlt( wsb.hdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY );
	imageWindow->release( hdc );
	//クリップボードの処理の開始をオーナーウィンドウを指定して知らせる
    if ( OpenClipboard( mainWindow->hwnd ) ) {
        //現在のデータを消去
		EmptyClipboard();
		//HBITMAPを複製し，クリップボードに設定
        SetClipboardData( CF_BITMAP, ::CopyImage( wsb.object, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG ) );
		//処理の終了を知らせる
        CloseClipboard();
    }
}

int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	if ( uMsg == BFFM_INITIALIZED ){
		SendMessage( hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData );
	}
    return 0;
}
void SHFreeIDList( ITEMIDLIST * iidl )
{
	if( ! iidl ) return;
	LPMALLOC pMalloc;
	if( ::SHGetMalloc( &pMalloc ) != NOERROR ) return;
	pMalloc->Free( iidl );
	pMalloc->Release();
}
inline void viewModeChange()
{
	mainMenu->check( IDM_VM_ORIGINAL, false );
	mainMenu->check( IDM_VM_VERTEDGE, false );
	mainMenu->check( IDM_VM_HORZEDGE, false );
	mainMenu->check( IDM_VM_NONE, false );
	mainMenu->check( IDM_VM_V, false );
	mainMenu->check( IDM_VM_H, false );
	mainMenu->check( IDM_VM_A, false );
	mainMenu->check( IDM_VM_D, false );
	HMENU menu = GetMenu( mainWindow->hwnd );
	switch ( trackView::get()->mode & trackView::baseImageMask ) {
	case trackView::original: mainMenu->check( IDM_VM_ORIGINAL, true ); break;
	case trackView::vertEdge: mainMenu->check( IDM_VM_VERTEDGE, true ); break;
	case trackView::horzEdge: mainMenu->check( IDM_VM_HORZEDGE, true ); break;
	case trackView::none: mainMenu->check( IDM_VM_NONE, true ); break;
	}
	if ( trackView::get()->mode & trackView::horizontal ) mainMenu->check( IDM_VM_H, true );
	if ( trackView::get()->mode & trackView::vertical ) mainMenu->check( IDM_VM_V, true );
	if ( trackView::get()->mode & trackView::descent ) mainMenu->check( IDM_VM_D, true );
	if ( trackView::get()->mode & trackView::ascent ) mainMenu->check( IDM_VM_A, true );
	imageWindow->invalidate();
	imageWindow->update();
}
void windowSliderMoved( fc::window * value )
{
	if ( imageWindow ) {
		imageWindow->invalidate();
		if ( windowSizeBar ) windowSize = windowSizeBar->get();
		if ( windowLevelBar ) windowLevel = windowLevelBar->get();
	}
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	fc::window::processAll( hWnd, message, wParam, lParam );
	static int mouseOldX = -1, mouseOldY = -1;
	MENUITEMINFO mii = { sizeof( mii ) };
	switch( message ) {
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			switch ( pnmh->code ) {
			case TTN_NEEDTEXT:
				{
					LPTOOLTIPTEXT ttt = (LPTOOLTIPTEXT)pnmh;
					int index = 0;//SendMessage( TB_COMMANDTOINDEX, pnmh->idFrom );
					/*
					TBBUTTON tb;
					if ( send( TB_GETBUTTON, index, LPARAM( & tb ) ) ) {
						ttt->lpszText = ( char * ) tb.dwData;
						ttt->uFlags |= TTF_DI_SETITEM;
					}
					*/
				}
				break;
			}
		}
		break;
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam), wmEvent = HIWORD(wParam);
			if ( wmId == IDM_COPY ) {
				clipboardCopy();
				break;
			} else if ( wmId == IDM_FIT ) {
				for ( int i = 0; i < 3; ++i ) {
					const RECT r = mainWindow->rect();
					const RECT icr = imageWindow->client();
					const int icw = icr.right - icr.left;
					const int ich = icr.bottom - icr.top;
					mainWindow->resize( 
						r.left, 
						r.top, 
						r.right - r.left + ( imageWindow->scrollDocumentSize.x - icw ), 
						r.bottom - r.top + ( imageWindow->scrollDocumentSize.y - ich ) );
				}
				break;
			}
			trackDocument * doc = trackDocument::get();
			if ( doc ) {
				if ( trackState::get()->now != stateDocument::get() ) {
					trackState::get()->change( stateDocument::get() );
				}
			} else {
				if ( trackState::get()->now != stateNoDocument::get() ) {
					trackState::get()->change( stateNoDocument::get() );
				}
			}
			switch( wmId ) {
			case IDM_QUIT: DestroyWindow( hWnd ); break;
			case IDM_OPEN:
				{
					//初期化処理
					{
						trackDocument * doc = trackDocument::get();
						if ( doc ) trackDocument::set( NULL );
					}
					trackState::get()->change( stateNoDocument::get() );
					//パス選択
					char folder[MAX_PATH];
					BROWSEINFO bi = { NULL };
					bi.hwndOwner = mainWindow->hwnd;
					bi.pszDisplayName = folder;
					bi.lpszTitle = "パターンを読み込むフォルダを指定してください";
					bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
					//デフォルトのディレクトリ
					static string defaultDirectory;
					if ( regGet( "defaultDirectory", defaultDirectory ) ) {
						bi.lpfn = BrowseCallbackProc;
						bi.lParam = (LPARAM) defaultDirectory.chars();
					}
					ITEMIDLIST * iidl = SHBrowseForFolder(&bi);
					if ( ! iidl ) break;
					SHGetPathFromIDList( iidl, folder );
					defaultDirectory = folder;
					SHFreeIDList( iidl );
					string documentPath = string( folder ) + "\\";
					//ファイル検索
					char search_path[MAX_PATH];
					strcpy( search_path, documentPath );
					strcat( search_path, "*" );
					WIN32_FIND_DATA wfd;
					HANDLE fff = FindFirstFile( search_path, & wfd );
					bool dicom = false;
					bool raw = false;
					if ( fff != INVALID_HANDLE_VALUE ) {
						do {
							if ( stricmp( "DICOMDIR", wfd.cFileName ) == 0 ) {
								dicom = true;
							}
							if ( stricmp( "1", wfd.cFileName ) == 0 ) {
								raw = true;
							}
						} while( FindNextFile( fff, & wfd ) );
					}
					FindClose( fff );
					retainer<trackDocument> doc;
					if ( dicom && ! doc ) {
						doc = new trackDocument();
						if ( ! doc->openDicom( documentPath ) ) {
							doc = NULL;
						}
					}
					if ( raw && ! doc ) {
						doc = new trackDocument();
						if ( ! doc->openRaw( documentPath ) ) {
							doc = NULL;
						}
					}
					if ( ! doc ) {
						doc = new trackDocument();
						if ( ! doc->openDicom( documentPath ) ) {
							doc = NULL;
						}
					}
					if ( doc ) {
						trackDocument::set( doc.loot() );
						trackView::get()->updateImageScale();
						trackState::get()->change( stateDocument::get() );
						regSet( "defaultDirectory", defaultDirectory );
					}
				}
				break;
			case IDM_LOAD_AS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					char filename[MAX_PATH];
					if ( ! fc::window::fileOpen( filename, doc->documentPath + "attribute.xml", "属性ファイル(*.xml)\0*.xml\0", "属性ファイルを開く", mainWindow->hwnd ) ) break;
					doc->loadXML( filename, true );
					trackState::get()->change( stateDocument::get() );
					imageWindow->invalidate();
				}
				break;
			case IDM_IMPORT_FROM:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					char filename[MAX_PATH];
					if ( ! fc::window::fileOpen( filename, doc->documentPath + "attribute.xml", "属性ファイル(*.xml)\0*.xml\0", "属性ファイルを開く", mainWindow->hwnd ) ) break;
					doc->loadXML( filename, false );
					trackState::get()->change( stateDocument::get() );
					imageWindow->invalidate();
				}
				break;
			case IDM_SAVE_AS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					char filename[MAX_PATH];
					if ( ! fc::window::fileSave( filename, doc->documentPath + "attribute.xml", "属性ファイル(*.xml)\0*.xml\0", "属性ファイルを保存", mainWindow->hwnd, ".xml" ) ) break;
					doc->saveXML( filename );
					imageWindow->invalidate();
				}
				break;
			case IDM_LOAD:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->loadXML( doc->documentPath + "attribute.xml", true );
					trackState::get()->change( stateDocument::get() );
					imageWindow->invalidate();
				}
				break;
			case IDM_SAVE:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->saveXML( doc->documentPath + "attribute.xml" );
					imageWindow->invalidate();
				}
				break;
			case IDM_EXPORT_INFO:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					char filename[MAX_PATH];
					if ( ! fc::window::fileSave( filename, doc->documentPath + "information.csv", "情報ファイル(*.csv)\0*.csv\0", "情報ファイルを保存", mainWindow->hwnd, ".csv" ) ) break;
					doc->exportInformation( filename );
					imageWindow->invalidate();
				}
				break;
			case IDM_EXPORT_SELECTED_INFO:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					char filename[MAX_PATH];
					if ( ! fc::window::fileSave( filename, doc->documentPath + "information.csv", "情報ファイル(*.csv)\0*.csv\0", "選択した情報ファイルを保存", mainWindow->hwnd, ".csv" ) ) break;
					stateSelectPointForExport * s = stateSelectPointForExport::get();
					s->filename = filename;
					trackState::get()->change( s );
				}
				break;
			case IDM_ANALYSIS_DOC:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					char correct[MAX_PATH];
					char output[MAX_PATH];
					if ( ! fc::window::fileOpen( correct, doc->documentPath + "attribute.xml", "属性ファイル(*.xml)\0*.xml\0", "正解の属性ファイルを開く", mainWindow->hwnd ) ) break;
					if ( ! fc::window::fileSave( output, doc->documentPath + "degree_of_coincidence.csv", "情報ファイル(*.csv)\0*.csv\0", "解析結果を保存", mainWindow->hwnd, ".csv" ) ) break;
					doc->analysisDoC( correct, output );
				}
				break;
			case IDM_PLAY:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					{
						for ( doc->currentViewImageIndex = 0; doc->currentViewImageIndex < doc->sizeTime(); ++( doc->currentViewImageIndex ) ) {
							sliderBar->set( doc->currentViewImageIndex );
							imageWindow->invalidate();
							imageWindow->update();
							Sleep( 50 );
						}
						doc->currentViewImageIndex = 0;
						sliderBar->set( doc->currentViewImageIndex );
						imageWindow->invalidate();
					}
				}
				break;
			case IDM_PREV:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					{
						doc->currentViewImageIndex = clamp( 0, doc->currentViewImageIndex - 1, doc->sizeTime() - 1 );
						sliderBar->set( doc->currentViewImageIndex );
						imageWindow->invalidate();
					}
				}
				break;
			case IDM_NEXT:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					{
						doc->currentViewImageIndex = clamp( 0, doc->currentViewImageIndex + 1, doc->sizeTime() - 1 );
						sliderBar->set( doc->currentViewImageIndex );
						imageWindow->invalidate();
					}
				}
				break;
			case IDM_SETBASETIME:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->changeBaseIndex( doc->currentViewImageIndex );
					imageWindow->invalidate();
				}
				break;
			case IDM_GETBASETIME:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->currentViewImageIndex = doc->baseIndex;
					sliderBar->set( doc->currentViewImageIndex );
					imageWindow->invalidate();
				}
				break;
			case IDM_SETINSPIRATIONFRAME:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->inspirationFrame = doc->currentViewImageIndex;
				}
				break;
			case IDM_GETINSPIRATIONFRAME:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->currentViewImageIndex = doc->inspirationFrame;
					sliderBar->set( doc->currentViewImageIndex );
					imageWindow->invalidate();
				}
				break;
			case IDM_SETEXPIRATIONFRAME:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->expirationFrame = doc->currentViewImageIndex;
				}
				break;
			case IDM_GETEXPIRATIONFRAME:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->currentViewImageIndex = doc->expirationFrame;
					sliderBar->set( doc->currentViewImageIndex );
					imageWindow->invalidate();
				}
				break;
			case IDM_VM_ORIGINAL:
				trackView::get()->mode &= ~ trackView::baseImageMask;
				trackView::get()->mode |= trackView::original;
				viewModeChange();
				break;
			case IDM_VM_VERTEDGE:
				trackView::get()->mode &= ~ trackView::baseImageMask;
				trackView::get()->mode |= trackView::vertEdge;
				viewModeChange();
				break;
			case IDM_VM_HORZEDGE:
				trackView::get()->mode &= ~ trackView::baseImageMask;
				trackView::get()->mode |= trackView::horzEdge;
				viewModeChange();
				break;
			case IDM_VM_NONE:
				trackView::get()->mode &= ~ trackView::baseImageMask;
				trackView::get()->mode |= trackView::none;
				viewModeChange();
				break;
			case IDM_VM_V:
				trackView::get()->mode ^= trackView::vertical;
				viewModeChange();
				break;
			case IDM_VM_H:
				trackView::get()->mode ^= trackView::horizontal;
				viewModeChange();
				break;
			case IDM_VM_D:
				trackView::get()->mode ^= trackView::descent;
				viewModeChange();
				break;
			case IDM_VM_A:
				trackView::get()->mode ^= trackView::ascent;
				viewModeChange();
				break;
			case IDM_VS_1:
				trackView::get()->changeImageScale( 1 );
				break;
			case IDM_VS_2:
				trackView::get()->changeImageScale( 2 );
				break;
			case IDM_VS_4:
				trackView::get()->changeImageScale( 4 );
				break;
			case IDM_CLEAR:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->clearLinks();
					imageWindow->invalidate();
				}
				break;
			case IDM_SETBASEWAVE:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					trackState::get()->change( stateIndex1stPointOfBaseWave::get() );
				}
				break;
			case IDM_EDITBASEWAVE:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					trackState::get()->change( stateBaseWave::get() );
				}
				break;
			case IDM_NORMALMODE:
				break;
			case IDM_WRITECURVE:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					trackState::get()->change( stateWriteCurve::get() );
				}
				break;
			case IDM_EDITCURVE:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					if ( ! doc->areas.size ) break;
					trackState::get()->change( stateEditCurve::get() );
				}
				break;
			case IDM_DIVIDECURVE:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					if ( ! doc->areas.size ) break;
					trackState::get()->change( stateDivideCurve::get() );
				}
				break;
			case IDM_CLEARCURVES:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					//doc->curves.release();
					doc->areas.release();
					trackState::get()->change( stateDocument::get() );
					imageWindow->invalidate();
				}
				break;
			case IDM_AGAIN_ANALYSIS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->againAnalysis();
				}
				break;
			case IDM_A_ALL:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisVertical( 0 );
					doc->analysisHorizontal( 0 );
					doc->analysisDescent( 0 );
					doc->analysisAscent( 0 );
				}
				break;
			case IDM_A_ALLS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisVertical( 1 );
					doc->analysisHorizontal( 1 );
					doc->analysisDescent( 1 );
					doc->analysisAscent( 1 );
				}
				break;
			case IDM_A_V:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisVertical( 0 );
				}
				break;
			case IDM_A_H:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisHorizontal( 0 );
				}
				break;
			case IDM_A_D:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisDescent( 0 );
				}
				break;
			case IDM_A_A:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisAscent( 0 );
				}
				break;
			case IDM_A_VS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisVertical( 1 );
				}
				break;
			case IDM_A_HS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisHorizontal( 1 );
				}
				break;
			case IDM_A_DS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisDescent( 1 );
				}
				break;
			case IDM_A_AS:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					doc->analysisAscent( 1 );
				}
				break;
			case IDM_A_BLOOD_ALL:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					if ( ! doc->wave.size ) break;
					for ( int i = 0; i < 4; ++i ) {
						doc->analysisFlicker( i );
					}
				}
				break;
			case ID_SHOW_CURVE:
				{
					trackDocument * doc = trackDocument::get();
					if ( ! doc ) break;
					doc->show_curve = ! doc->show_curve;
					CheckMenuItem( GetMenu( hWnd ), ID_SHOW_CURVE, doc->show_curve ? MF_CHECKED : MF_UNCHECKED );
					imageWindow->invalidate();
				}
				break;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			trackState::get()->mouseLeftPush( lParam );
		}
		break;
	case WM_LBUTTONUP:
		{
			trackState::get()->mouseLeftPop( lParam );
		}
		break;
	case WM_RBUTTONDOWN:
		{
			trackState::get()->mouseRightPush( lParam );
		}
		break;
	case WM_RBUTTONUP:
		{
			trackState::get()->mouseRightPop( lParam );
		}
		break;
	case WM_MOUSEMOVE:
		{
			trackState::get()->mouseMove( lParam );
			if ( wParam & MK_LBUTTON ) {
				trackState::get()->mouseLeftMove( lParam );
			} else if ( wParam & MK_RBUTTON ) {
				trackState::get()->mouseRightMove( lParam );
			}
		}
		break;
	case WM_DISPLAYCHANGE://解像度変更時に３Ｄ表示を再初期化する．
		break;
	case WM_PAINT:
		{
			trackState::get()->paint();
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
	case WM_SIZE:
		{
			if ( mainWindow && statusBar && mainWindow->hwnd && imageWindow && imageWindow->hwnd ) {
				if ( mainWindow->hwnd == hWnd ) {
					RECT cr = mainWindow->client();
					int crw = cr.right - cr.left;
					int crh = cr.bottom - cr.top;
					RECT iwr = imageWindow->rect();
					int iww = iwr.right - iwr.left;
					int iwh = iwr.bottom - iwr.top;
					//画像の幅を調整
					imageWindow->resize( 0, reBar->height, crw, crh - statusBar->height - reBar->height );
					imageWindow->updateScrollRange();
				} else if ( windowControlWindow->hwnd == hWnd ) {
					RECT cr = windowControlWindow->client();
					int wcwrw = cr.right - cr.left;
					int wcwrh = cr.bottom - cr.top;
					windowSizeBar->resize( 0, 0, wcwrw / 2 - windowTextWidth, wcwrh );
					windowSizeEditbox->resize( wcwrw / 2 - windowTextWidth, 0, windowTextWidth, wcwrh );
					windowLevelBar->resize( wcwrw / 2, 0, wcwrw - wcwrw / 2 - windowTextWidth, wcwrh );
					windowLevelEditbox->resize( wcwrw - windowTextWidth, 0, windowTextWidth, wcwrh );
				}
			}
		}
		break;
	case WM_SIZING:
		{
			if ( mainWindow && statusBar && mainWindow->hwnd && mainWindow->hwnd == hWnd &&
				imageWindow && imageWindow->hwnd ) {
				RECT cr = mainWindow->client();
				int crw = cr.right - cr.left;
				int crh = cr.bottom - cr.top;
				RECT wr = mainWindow->rect();
				int wrw = wr.right - wr.left;
				int wrh = wr.bottom - wr.top;
				int w = wrw - crw;//最低横幅
				int h = statusBar->height + reBar->height + wrh - crh;//最低縦幅
				RECT * r = ( RECT * ) lParam;
				if ( wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT ) {
					if ( r->right - r->left < w ) r->left = r->right - w;
				} else if ( wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT ) {
					if ( r->right - r->left < w ) r->right = r->left + w;
				}
				if ( wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT ) {
					if ( r->bottom - r->top < h ) r->top = r->bottom - h;
				} else if ( wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT ) {
					if ( r->bottom - r->top < h ) r->bottom = r->top + h;
				}
			}
			return TRUE;
		}
		break;
	case WM_QUIT:
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return FALSE;
}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	//グローバル変数にインスタンス情報を設定
	::hInstance = hInstance;
	//プロセス優先度を設定
	SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
	//COMの初期化
	bool CoInitialized = ( S_OK == CoInitialize( NULL ) );
	//ウィンドウの作成
	fc::window::instance = hInstance;
	fc::window::initialize();
	fc::window::registerClass( wndProc, "TRACK", IDR_MENU1, IDI_ICON1, IDI_ICON1 );
	RECT r = { 0, 0, 512, 512 };
	DWORD mainWindowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE;
	AdjustWindowRect( &r, mainWindowStyle, TRUE );
	mainWindow = new fc::window( NULL, 0 );
	mainWindow->createRoot( "TRACK", "TRACK", mainWindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, 
			r.right - r.left, r.bottom - r.top, IDR_MENU1 );

	mainMenu = new fc::menu( mainWindow->hwnd );

	SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );

	reBar = new fc::rebar( mainWindow->hwnd, rebarID );
	reBar->create();
	
	static char * tooltipOpen = "Open Image File";
	static char * tooltipNormal = "Normal Mode";
	static char * tooltipPlay = "Play";
	static char * tooltipSet = "Set Base Wave";
	static char * tooltipEdit = "Edit Base Wave";
	static char * tooltipWrite = "Write Curve";
	static char * tooltipEditCurve = "Edit Curve";
	static char * tooltipDivide = "Divide Curve";
	int ttOpen = ( int ) tooltipOpen;
	int ttNormal = ( int ) tooltipNormal;
	int ttPlay = ( int ) tooltipPlay;
	int ttSet = ( int ) tooltipSet;
	int ttEdit = ( int ) tooltipEdit;
	int ttWrite = ( int ) tooltipWrite;
	int ttEditCurve = ( int ) tooltipEditCurve;
	int ttDivide = ( int ) tooltipDivide;
	static TBBUTTON toolBarButton[] = {
//		{fc::toolbar::sysIcon( STD_FILEOPEN, IDB_STD_SMALL_COLOR ), IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttOpen },
//		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
		{0, IDM_NORMALMODE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttNormal },
		{2, IDM_PLAY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttPlay },
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
		{3, IDM_SETBASEWAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttSet },
		{4, IDM_EDITBASEWAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttEdit },
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP},
		{5, IDM_WRITECURVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttWrite },
		{7, IDM_EDITCURVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttEditCurve },
		{6, IDM_DIVIDECURVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, ttDivide },
	};

	toolBar = new fc::toolbar( reBar->hwnd, toolBarID );
	toolBar->style = CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;
	toolBar->create( IDR_TOOLBAR1, 12, sizeof( toolBarButton ) / sizeof( toolBarButton[0] ), toolBarButton );

	reBar->add( toolBar, rebar4toolbarID, NULL, 180 );

	sliderBar = new fc::slider( reBar->hwnd, sliderID );
	sliderBar->style = CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER;
	sliderBar->create();
	sliderBar->set( 0, 0 );
	sliderBar->set( 0 );
	reBar->add( sliderBar, rebar4sliderID, NULL, 200 );

	statusBar = new fc::statusbar( mainWindow->hwnd, statusBarID );
	statusBar->setStyle( CCS_NOMOVEY );
	//statusBar->setStyle( SBARS_SIZEGRIP, false );
	statusBar->create();

	windowControlWindow = new fc::window( reBar->hwnd, windowControlID );
	windowControlWindow->create( "TRACK", "", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | CCS_NOPARENTALIGN | CCS_NORESIZE | CCS_NODIVIDER );
	reBar->add( windowControlWindow, rebar4windowID, NULL, 140 );
	RECT wcwr = windowControlWindow->client();
	int wcwrw = wcwr.right - wcwr.left;
	int wcwrh = wcwr.bottom - wcwr.top;
	windowSizeEditbox = new fc::editbox( windowControlWindow->hwnd, windowSizeEditID );
	windowSizeEditbox->style = ES_CENTER | WS_BORDER | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	windowSizeEditbox->create();
	windowLevelEditbox = new fc::editbox( windowControlWindow->hwnd, windowLevelEditID );
	windowLevelEditbox->style = ES_CENTER | WS_BORDER | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
	windowLevelEditbox->create();
	windowSizeBar = new fc::slider( windowControlWindow->hwnd, windowSizeID );
	windowSizeBar->style = CCS_NOPARENTALIGN | CCS_NODIVIDER;
	windowSizeBar->changed = windowSliderMoved;
	windowSizeBar->edit = windowSizeEditbox;
	windowSizeBar->create();
	windowSizeBar->set( 1, 4096 );
	windowLevelBar = new fc::slider( windowControlWindow->hwnd, windowLevelID );
	windowLevelBar->style = CCS_NOPARENTALIGN | CCS_NODIVIDER;
	windowLevelBar->changed = windowSliderMoved;
	windowLevelBar->edit = windowLevelEditbox;
	windowLevelBar->create();
	windowLevelBar->set( 1, 4096 );
	string ws, wl;
	if ( regGet( "windowSize", ws ) ) {
		windowSize = ws.toi();
	}
	if ( regGet( "windowLevel", wl ) ) {
		windowLevel = wl.toi();
	}
	windowSizeBar->set( windowSize );
	windowLevelBar->set( windowLevel );

	tooltip = new fc::tooltip();
	tooltip->create();
	tooltip->add( windowLevelBar, "Window Level" );
	tooltip->add( windowSizeBar, "Window Size" );
	tooltip->add( windowLevelEditbox, "Window Level" );
	tooltip->add( windowSizeEditbox, "Window Size" );
	tooltip->add( sliderBar, "Frame" );

	RECT ir = { 0, 0, 512, 512 };
	DWORD imageWindowStyle = WS_VSCROLL | WS_HSCROLL | WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE;
	AdjustWindowRect( &ir, imageWindowStyle, TRUE );
	imageWindow = new fc::window( mainWindow->hwnd, 0 );
	imageWindow->create( "TRACK", "TRACK", imageWindowStyle, 0, toolBar->height, ir.right - ir.left, ir.bottom - ir.top - statusBar->height - toolBar->height );

	imageWindow->scrollDocumentSize.x = 0;
	imageWindow->scrollDocumentSize.y = 0;
	imageWindow->flatScroll();
	imageWindow->updateScrollRange();

	viewModeChange();
	mainWindow->send( WM_SIZE );
	windowControlWindow->send( WM_SIZE );

	trackState::get()->change( stateNoDocument::get() );
	imageWindow->invalidate();
	
	mainWindow->loop( NULL, IDR_ACCELERATOR1 );
	regSet( "windowSize", string( windowSize ) );
	regSet( "windowLevel", string( windowLevel ) );

	//COMの終了化
	if ( CoInitialized ) CoUninitialize();
	return 0;
}
