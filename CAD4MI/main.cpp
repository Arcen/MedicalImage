#include "main.h"
/*
#include "operation.h"
#include "popup.h"
#include "mouse.h"
#include "gsd.h"
*/
HINSTANCE hInstance = NULL;

semaphore singleThread;

fc::window * mainWindow = NULL;

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	fc::window::processAll( hWnd, message, wParam, lParam );
	static int mouseOldX = -1, mouseOldY = -1;
//	miDocument * doc = miDocument::get();
	switch( message ) {
		/*
	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam), wmEvent = HIWORD(wParam);
			switch( wmId ) {
			case IDM_OPEN: miDocument::open(); break;
			case IDM_CLOSE: miDocument::close(); break;
			case IDM_LOAD_AS: miDocument::loadAsRgn(); break;
			case IDM_SAVE_AS: miDocument::saveAsRgn(); break;
			case IDM_LOAD_PATTERN: miDocument::loadPattern( "", "", true, true ); break;
			case IDM_SAVE_PATTERN: miDocument::savePattern( "", "", true ); break;
			case IDM_ADD_PATTERN: miDocument::loadPattern( "", "", false, true ); break;
			case IDM_SAVE: miDocument::save(); break;
			case IDM_SAVE_HISTOGRAM: miDocument::saveSampleHistogram(); break;
			case IDM_QUIT: DestroyWindow( hWnd ); break;
			case IDM_UNDO: if ( ! controlWindow->extracting && ! controlWindow->modifing ) miCommands::get().undo(); break;
			case IDM_REDO: if ( ! controlWindow->extracting && ! controlWindow->modifing ) miCommands::get().redo(); break;
			case IDM_CLEAR_UNDO: if ( ! controlWindow->extracting && ! controlWindow->modifing ) miCommands::get().clear(); break;
			case IDM_COPY: clipboardCopy(); break;
			case IDM_VIEW_NEXT_SLICE: controlWindow->nextSlice(); break;
			case IDM_VIEW_PREV_SLICE: controlWindow->prevSlice(); break;
			case IDM_VIEW_HUMAN: controlWindow->changeRegion( regionHuman ); break;
			case IDM_VIEW_LUNG: controlWindow->changeRegion( regionLung ); break;
			case IDM_VIEW_NORMAL: controlWindow->changeRegion( regionNormal ); break;
			case IDM_VIEW_MIX: controlWindow->changeViewMode( viewModeMix ); break;
			case IDM_VIEW_ORIGINAL: controlWindow->changeViewMode( viewModeOriginal ); break;
			case IDM_VIEW_LABEL: controlWindow->changeViewMode( viewModeLabel ); break;
			case IDM_VIEW_BOTH: controlWindow->changeViewSide( viewSideBoth ); break;
			case IDM_VIEW_LEFT: controlWindow->changeViewSide( viewSideLeft ); break;
			case IDM_VIEW_RIGHT: controlWindow->changeViewSide( viewSideRight ); break;
			case IDM_VIEW_CAMERA_INITIALIZE:
				{
					miView * view = miView::get();
					if ( view && view->graph && view->camera ) {
						view->camera->initializeAttribute();
						view->camera->position( vector3( 0, 300, 0 ) );
						view->camera->upper( vector3( 0, 0, 1 ) );
						view->camera->update();
						imageWindow->invalidate();
					}
				}
				break;
			case IDM_VIEW_CAMERA_ROTATION:
				{
					miView * view = miView::get();
					if ( view && view->graph && view->camera && controlWindow->viewing3d ) {
						view->camera->follow( 1.0 / 64.0, 0, 0 );
						//上方向の補正
						vector3 upper = vector3::normalize3( view->camera->upper() );
						if ( upper.z ) {
							vector3 recomandupper( 0, 0, upper.z > 0 ? 1 : -1 );
							vector3 direction = vector3::normalize3( view->camera->interest() - view->camera->position() );
							vector3 other = vector3::normalize3( vector3::cross( recomandupper, direction ) );
							view->camera->upper( vector3::normalize3( vector3::cross( direction, other ) ) );
						}
						view->camera->update();
						imageWindow->invalidate();
					}
				}
				break;
			case IDM_OPERATION_CALIBRATION: operationCommand::calibration(); break;
			case IDM_OPERATION_DIVIDEREGION_THRESHHOLD: operationCommand::divideRegionThreshhold( true ); break;
			case IDM_OPERATION_DIVIDEREGION_SMOOTHEDDIFFERENCE: operationCommand::divideRegionBySmoothedDifference( true ); break;
			case IDM_OPERATION_DIVIDEREGION_KOLMOGOROVSMIRNOV: operationCommand::divideRegionByKolmogorovSmirnov( true ); break;
			case IDM_OPERATION_DIVIDEREGION_CORRELATION: operationCommand::divideRegionByGaussianHistogramCorrelation( true, false ); break;
			case IDM_OPERATION_DIVIDEREGION_MULTI_CORRELATION: operationCommand::divideRegionByGaussianHistogramCorrelation( true, true ); break;
			case IDM_OPERATION_DIVIDEREGION_THRESHHOLD_SINGLE_SLICE: operationCommand::divideRegionThreshhold( false ); break;
			case IDM_OPERATION_DIVIDEREGION_SMOOTHEDDIFFERENCE_SINGLE_SLICE: operationCommand::divideRegionBySmoothedDifference( false ); break;
			case IDM_OPERATION_DIVIDEREGION_KOLMOGOROVSMIRNOV_SINGLE_SLICE: operationCommand::divideRegionByKolmogorovSmirnov( false ); break;
			case IDM_OPERATION_DIVIDEREGION_CORRELATION_SINGLE_SLICE: operationCommand::divideRegionByGaussianHistogramCorrelation( false, false ); break;
			case IDM_OPERATION_DIVIDEREGION_MULTI_CORRELATION_SINGLE_SLICE: operationCommand::divideRegionByGaussianHistogramCorrelation( false, true ); break;
			case IDM_OPERATION_DIVIDEREGION_CORRELATION_MULTI_PARAM: operationCommand::divideRegionByCorrelationMultiParam( true ); break;
			case IDM_OPERATION_DIVIDEREGION_SMOOTHEDDIFFERENCE_MULTI_PARAM: operationCommand::divideRegionBySmoothedDifferenceMultiParam( true ); break;

			case IDM_OPERATION_DIVIDEREGION_NORMALIZED_CORRELATION: operationCommand::divideRegionByGaussianHistogramCorrelation( true, false, true, false ); break;
			case IDM_OPERATION_DIVIDEREGION_NORMALIZED_CORRELATION_SINGLE_SLICE: operationCommand::divideRegionByGaussianHistogramCorrelation( false, false, true, false ); break;
			case IDM_OPERATION_DIVIDEREGION_NORMALIZED_CORRELATION_WITH_EDGE: operationCommand::divideRegionByGaussianHistogramCorrelation( true, false, true, true ); break;
			case IDM_OPERATION_DIVIDEREGION_NORMALIZED_CORRELATION_WITH_EDGE_SINGLE_SLICE: operationCommand::divideRegionByGaussianHistogramCorrelation( false, false, true, true ); break;

			case IDM_OPERATION_REMOVE_SMALL_LUNG_REGION: operationCommand::removeSmallLungRegion(); break;
			case IDM_ANALYSIS_RECOGNITION_NETLIKE: if ( doc ) doc->recognitionNetlike(); break;
			case IDM_ANALYSIS_VARIANCE_AVERAGE: if ( doc ) doc->displayAverageVariance(); break;
			case IDM_ANALYSIS_SAVE_HISTOGRAM: if ( doc ) doc->saveHistogram(); break;
			case IDM_ANALYSIS_SAVE_DISTORIBUTION: if ( doc ) doc->saveDistoribution(); break;
			case IDM_LOAD_LABEL_AS: if ( doc ) doc->loadLabel(); break;
			case IDM_ANALYSIS: if ( doc ) doc->analysis(); break;
			case IDM_SAVE_GRAPH: if ( doc ) doc->saveAsGraph(); break;
			case IDM_OPERATION_GAUSSIAN_SD:
				{
					gaussSDParam gsd( ge.sd, mainWindow->hwnd );
					if ( gsd.call() ) {
						ge.initialize( square( gsd.sd ) );
						if ( doc ) {
							doc->patternsInitialize();
						}
					}
				}
				break;
			}
		}
		break;
	case WM_KEYDOWN:
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		if ( imageWindow && imageWindow->hwnd == hWnd ) {
			if ( onTrackMenu.check() ) {
				onTrackMenu.signal();
				semaphoreCheck sc( singleThread );
				switch( message ) {
				case WM_LBUTTONDOWN: mouseInterface::leftPush( hWnd, ( wParam & MK_LBUTTON ) ? true : false, ( wParam & MK_MBUTTON ) ? true : false, ( wParam & MK_RBUTTON ) ? true : false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), mouseOldX, mouseOldY ); break;
				case WM_RBUTTONDOWN: mouseInterface::rightPush( hWnd, ( wParam & MK_LBUTTON ) ? true : false, ( wParam & MK_MBUTTON ) ? true : false, ( wParam & MK_RBUTTON ) ? true : false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), mouseOldX, mouseOldY ); break;
				case WM_LBUTTONUP: mouseInterface::leftPop( hWnd, ( wParam & MK_LBUTTON ) ? true : false, ( wParam & MK_MBUTTON ) ? true : false, ( wParam & MK_RBUTTON ) ? true : false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), mouseOldX, mouseOldY ); break;
				case WM_RBUTTONUP: mouseInterface::rightPop( hWnd, ( wParam & MK_LBUTTON ) ? true : false, ( wParam & MK_MBUTTON ) ? true : false, ( wParam & MK_RBUTTON ) ? true : false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), mouseOldX, mouseOldY ); break;
				case WM_MOUSEMOVE: mouseInterface::move( hWnd, ( wParam & MK_LBUTTON ) ? true : false, ( wParam & MK_MBUTTON ) ? true : false, ( wParam & MK_RBUTTON ) ? true : false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), mouseOldX, mouseOldY ); break;
				}
				mouseOldX = GET_X_LPARAM(lParam); mouseOldY = GET_Y_LPARAM(lParam);
			}
		}
		break;
	case WM_DISPLAYCHANGE://解像度変更時に３Ｄ表示を再初期化する．
		{
			semaphoreCheck sc( singleThread );//描画を止めるため
			miFrame * frame = miFrame::get();
			if ( controlWindow && controlWindow->viewing3d ) {
				controlWindow->viewing3d = false;
				controlWindow->validateControl();
			}
			if ( frame ) {
				miDocument * doc = miDocument::get();
				miView * view = miView::get();
				if ( doc && view ) view->finalize( doc );
				frame->finalize();
				frame->initialize();
				if ( doc && view ) view->initialize( doc );
				if ( doc ) doc->change();
			};
		}
		break;
	case WM_PAINT:
		{
			if ( imageWindow && imageWindow->hwnd == hWnd ) {
				miFrame::get()->paint();
			} else {
				return DefWindowProc( hWnd, message, wParam, lParam );
			}
		}
		break;
	case WM_SIZE:
		{
			if ( mainWindow && statusBar ) {
				RECT r = mainWindow->client();
				RECT cwr = controlWindow->rect();
				int cww = cwr.right - cwr.left;
				if ( r.right != cww + 512 || r.bottom != 512 + statusBar->height )
					mainWindow->client( cww + 512, 512 + statusBar->height );
			}
		}
		break;
	case WM_SIZING:
		{
			/*
			RECT wr, cr;
			GetWindowRect( hWnd, & wr );
			GetClientRect( hWnd, & cr );
			const int fw = ( wr.right - wr.left ) - ( cr.right - cr.left );//フレームの大きさ
			const int fh = ( wr.bottom - wr.top ) - ( cr.bottom - cr.top ) + statusBar->height;
			const int w = 512 + 256 + fw;//ウィンドウの最小サイズ
			const int h = 512 + fh;
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
			* /
			return TRUE;
		}
			*/
	case WM_QUIT:
		//miDocument::close();
		break;
	case WM_DESTROY:
		//miDocument::close();
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
	//シングルスレッドとして動作するように同期オブジェクトをシグナルする
	singleThread.signal();
	//COMの初期化
	bool CoInitialized = ( S_OK == CoInitialize( NULL ) );
	//ウィンドウの作成
	fc::window::registerClass( wndProc, "CAD4MI", IDR_MENU1, IDI_ICON1, IDI_ICON1 );
	RECT r = { 0, 0, 512, 512 };
	AdjustWindowRect( &r, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE, TRUE );
	mainWindow = new fc::window( NULL, 0 );
	mainWindow->hwnd = CreateWindow( "CAD4MI", "肺野CT画像処理システム", 
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE, 
		CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, NULL, 
		LoadMenu( hInstance, MAKEINTRESOURCE( IDR_MENU1 ) ), hInstance, NULL );
/*
	windows * window = new windows( hInstance, "LCTIPS", wndProc ); 
	window->setWindow( NULL, mainWindow->hwnd, LoadAccelerators( hInstance, MAKEINTRESOURCE( IDR_ACCELERATOR1 ) ) );

	statusBar = new fc::statusbar( mainWindow->hwnd, statusBarID );
	statusBar->create();

	controlWindow = new controlInterface( mainWindow->hwnd );
	imageWindow = new fc::window( mainWindow->hwnd, 0 );
	RECT cwr = controlWindow->rect();
	int right = cwr.right - cwr.left;
	RECT r = { right, 0, right + 512, 512 };
	AdjustWindowRect( &r, WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE, FALSE );
	imageWindow->create( "LCTIPS", "", 0, right, 0, r.right - r.left, r.bottom - r.top );
	regionMenu.initialize();
	clickMenu.initialize();
	
	mainWindow->invalidate();

	sgGraph::initialize();

	mainWindow->client( right + 512, 512 + statusBar->height );
	imageWindow->client( 512, 512 );

	miFrame * frame = new miFrame();
	miFrame::set( frame );
	frame->initialize();
*/
	mainWindow->loop();
/*
	miDocument::close();

	frame->finalize();
	miFrame::set( NULL );
	delete frame;
	
	sgGraph::finalize();

	delete window;

	::brushinterface.finalize();
	::freehandinterface.finalize();
	*/
	//COMの終了化
	if ( CoInitialized ) CoUninitialize();
	return 0;
}
