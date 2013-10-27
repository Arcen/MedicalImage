#include "main.h"
#include "operation.h"
#include "popup.h"
#include "mouse.h"
#include "gsd.h"

HINSTANCE hInstance = NULL;
LPSTR lpCmdLine = 0;
semaphore singleThread;
semaphore onTrackMenu;


fc::window * mainWindow = NULL;
fc::window * imageWindow = NULL;
controlInterface * controlWindow = NULL;
fc::statusbar * statusBar = NULL;
freehand freehandinterface;
brush brushinterface;

regionMenuInterface * regionMenu = NULL;
clickMenuInterface * clickMenu = NULL;
miCommands miCommands::instance;
gaussianEstimate ge;

void clipboardCopy()
{
	//imageWindowの内容をメモリDCにコピーする．
	windowSurfaceBitmap wsb;
	RECT r = imageWindow->rect();
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

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool destroyed = false;
	fc::window::processAll( hWnd, message, wParam, lParam );
	static int mouseOldX = -1, mouseOldY = -1;
	miDocument * doc = miDocument::get();
	switch( message ) {
    case WM_CREATE:
        {
            if ( lpCmdLine && strlen( lpCmdLine ) > 0 )
            {
                miDocument::open( lpCmdLine );
            }
        }
        break;
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
			case IDM_SAVE_PATTERN_SETTING: miDocument::savePatternSetting(); break;
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
			case IDM_VIEW_TIMES1: miView::changeTimes( 1 ); break;
			case IDM_VIEW_TIMES2: miView::changeTimes( 2 ); break;
			case IDM_VIEW_TIMES4: miView::changeTimes( 4 ); break;
			case IDM_VIEW_FIT: 
				{
					miView::changeTimes( 1 );
					RECT iwr = imageWindow->rect();
					int iww = iwr.right - iwr.left;
					int iwh = iwr.bottom - iwr.top;
					if ( iww == 512 && iwh == 512 ) break;
					RECT r = mainWindow->rect();
					int rw = r.right - r.left;
					int rh = r.bottom - r.top;
					mainWindow->resize( -1, -1, rw + 512 - iww, rh + 512 - iwh );
				}
				break;
#ifdef VIEW3D
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
#endif
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
			case IDM_OPERATION_DIVIDEREGION_MULTI_NORMALIZED_CORRELATION: operationCommand::divideRegionByGaussianHistogramCorrelation( true, true, true, false ); break;
			case IDM_OPERATION_DIVIDEREGION_MULTI_NORMALIZED_CORRELATION_SINGLE_SLICE: operationCommand::divideRegionByGaussianHistogramCorrelation( false, true, true, false ); break;

			case IDM_OPERATION_REMOVE_SMALL_LUNG_REGION: operationCommand::removeSmallLungRegion(); break;
			case IDM_ANALYSIS_RECOGNITION_NETLIKE: if ( doc ) doc->recognitionNetlike(); break;
			case IDM_ANALYSIS_VARIANCE_AVERAGE: if ( doc ) doc->displayAverageVariance(); break;
			case IDM_ANALYSIS_SAVE_HISTOGRAM: if ( doc ) doc->saveHistogram(); break;
			case IDM_ANALYSIS_SAVE_DISTORIBUTION: if ( doc ) doc->saveDistoribution(); break;
			case IDM_ANALYSIS_LUNG: operationCommand::analyzeLungRegion(); break;
				
			//case IDM_LOAD_LABEL_AS: if ( doc ) doc->loadLabel(); break;
			case IDM_ANALYSIS_SAVE_INFORMATION: if ( doc ) doc->saveInformation(); break;
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
			if ( mainWindow && statusBar && imageWindow && controlWindow &&
				mainWindow->hwnd && statusBar->hwnd && imageWindow->hwnd && controlWindow->hwnd ) {
				if ( mainWindow->hwnd == hWnd ) {
					RECT cr = mainWindow->client();
					int crw = cr.right - cr.left;
					int crh = cr.bottom - cr.top;
					RECT cwr = controlWindow->rect();
					int cww = cwr.right - cwr.left;
					int cwh = cwr.bottom - cwr.top;
					RECT iwr = imageWindow->rect();
					int iww = iwr.right - iwr.left;
					int iwh = iwr.bottom - iwr.top;
					//画像の幅を調整
					imageWindow->resize( cww, 0, crw - cww, crh - statusBar->height );
					controlWindow->resize( 0, 0, cww, crh - statusBar->height );
					imageWindow->updateScrollRange();
				}
				/*
				RECT iwr = imageWindow->rect();
				int iww = iwr.right - iwr.left;
				int iwh = iwr.bottom - iwr.top;
				if ( r.right != cww + iww || r.bottom != maximum( iwh, cwh ) + statusBar->height )
					mainWindow->client( cww + iww, maximum( iwh, cwh ) + statusBar->height );
				*/
			}
		}
		break;
	case WM_SIZING:
		{
			if ( mainWindow && statusBar && imageWindow && controlWindow &&
				mainWindow->hwnd && statusBar->hwnd && imageWindow->hwnd && controlWindow->hwnd ) {
				if ( hWnd == mainWindow->hwnd ) {
					RECT cr = mainWindow->client();
					int crw = cr.right - cr.left;
					int crh = cr.bottom - cr.top;
					RECT wr = mainWindow->rect();
					int wrw = wr.right - wr.left;
					int wrh = wr.bottom - wr.top;
					RECT cwr = controlWindow->rect();
					int cww = cwr.right - cwr.left;
					int cwh = cwr.bottom - cwr.top;
					int w = cww + wrw - crw;//最低横幅
					int h = statusBar->height + wrh - crh;//最低縦幅
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
			}
			return TRUE;
		}
	case WM_QUIT:
        if ( ! destroyed )
        {
            destroyed = true;
		    miDocument::close();
        }
		break;
	case WM_DESTROY:
        if ( ! destroyed )
        {
            destroyed = true;
		    miDocument::close();
        }
		PostQuitMessage( 0 );
		break;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return FALSE;
}
bool CoInitialized = false;

#if defined(_DEBUG)
//メモリーリークテスト
//#include <crtdbg.h>
#endif
//#include <omp.h>
int thread_count = 1;

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
#if defined(_DEBUG)
    //メモリーリークテスト
    //_CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG  ) | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	//SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
	singleThread.signal();
	onTrackMenu.signal();
	::hInstance = hInstance;
    ::lpCmdLine = lpCmdLine;
	CoInitialized = ( S_OK == CoInitialize( NULL ) );

    //OpenMPのスレッド数指定.
    //omp_set_num_threads(omp_get_num_procs());
    {
        DWORD mask = 0, system_mask = 0;
        int cores = 0;
        ::GetProcessAffinityMask( ::GetCurrentProcess(), & mask, & system_mask );
        for ( int i = 0; i < 32; ++i )
        {
            if ( mask & ( 1 << i ) ) ++cores;
        }
        if ( ! cores ) cores = 1;
        thread_count = cores;
    }

	ge.initialize( square( 23.35 ) );
//	ge.initialize( square( 30.0 ) );
	::brushinterface.initialize();
	::freehandinterface.initialize();

	fc::window::registerClass( wndProc, "LCTIPS", IDR_MENU1, IDI_ICON1, IDI_ICON1, COLOR_MENU );

	mainWindow = new fc::window( NULL, 0 );
	mainWindow->hwnd = CreateWindow( "LCTIPS", "肺野CT画像処理システム", 
//		( WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX ) | 
		WS_OVERLAPPEDWINDOW | 
		WS_CLIPCHILDREN | WS_VISIBLE, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 
		LoadMenu( hInstance, MAKEINTRESOURCE( IDR_MENU1 ) ), hInstance, NULL );

#ifndef VIEW3D
	HMENU menu = GetMenu( mainWindow->hwnd );
	if ( menu ) {
		EnableMenuItem( menu, IDM_SAVE_GRAPH, MF_GRAYED );
		EnableMenuItem( menu, IDM_VIEW_CAMERA_INITIALIZE, MF_GRAYED );
		EnableMenuItem( menu, IDM_VIEW_CAMERA_ROTATION, MF_GRAYED );
	}
#endif

	windows * window = new windows( hInstance, "LCTIPS", wndProc ); 
	window->setWindow( NULL, mainWindow->hwnd, LoadAccelerators( hInstance, MAKEINTRESOURCE( IDR_ACCELERATOR1 ) ) );

	statusBar = new fc::statusbar( mainWindow->hwnd, statusBarID );
	statusBar->setStyle( CCS_NOMOVEY );
	//statusBar->setStyle( SBARS_SIZEGRIP, false );
	statusBar->create();

	controlWindow = new controlInterface( mainWindow->hwnd );
	imageWindow = new fc::window( mainWindow->hwnd, 0 );
	RECT cwr = controlWindow->rect();
	int cwWidth = cwr.right - cwr.left;
	RECT r = { cwWidth, 0, cwWidth + 512, 512 };
	DWORD imageWindowStyle = WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN | WS_CHILD | WS_VISIBLE;
	AdjustWindowRect( &r, imageWindowStyle, FALSE );
	imageWindow->create( "LCTIPS", "", imageWindowStyle, r.left, r.top, r.right - r.left, r.bottom - r.top );

    regionMenu = new regionMenuInterface();
	regionMenu->initialize();
    clickMenu = new clickMenuInterface();
	clickMenu->initialize();
	
	imageWindow->scrollDocumentSize.x = 512;
	imageWindow->scrollDocumentSize.y = 512;
	imageWindow->updateScrollRange();
	mainWindow->resize( -1, -1, 800, 600 );
	mainWindow->invalidate();

	sgGraph::initialize();

	miFrame * frame = new miFrame();
	miFrame::set( frame );
	frame->initialize();

	window->loop();

	miDocument::close();

    delete regionMenu;
    regionMenu = NULL;
    delete clickMenu;
    clickMenu = NULL;

	frame->finalize();
	miFrame::set( NULL );
	delete frame;
	
	sgGraph::finalize();

	delete window;

	::brushinterface.finalize();
	::freehandinterface.finalize();
	if ( CoInitialized ) CoUninitialize();
	//SetPriorityClass( GetCurrentProcess(), NORMAL_PRIORITY_CLASS );
	return 0;
}
