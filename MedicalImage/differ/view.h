typedef region<int16> thisRegion;

//同期チェック用セマフォクラス

class semaphore
{
	HANDLE smphr;
public:
	semaphore() : smphr( NULL )
	{
		smphr = CreateSemaphore( NULL, 0, 1, NULL );
	}
	~semaphore()
	{
		if ( smphr ) {
			CloseHandle( smphr );
			smphr = NULL;
		}
	}
	bool wait( DWORD wait_ms = INFINITE )
	{
		if ( ! smphr ) return false;
		if ( check() ) return true;
		if ( wait_ms != INFINITE ) return ( WaitForSingleObject( smphr, wait_ms ) == WAIT_OBJECT_0 );

		if ( WaitForSingleObject( smphr, 30000 ) == WAIT_OBJECT_0 ) return true;
		int r = MessageBox( NULL, "30 sec past. wait for forever", "WaitForSingleObject", MB_YESNO );
		if ( r == IDYES ) WaitForSingleObject( smphr, INFINITE );
		return true;
	}
	void signal()
	{
		if ( ! smphr ) return;
		ReleaseSemaphore( smphr, 1, NULL );
	}
	bool check()
	{
		if ( ! smphr ) return false;
		return ( WaitForSingleObject( smphr, 0 ) == WAIT_OBJECT_0 );
	}
};

//宣言により有効スコープ内でのセマフォによる同期チェック用クラス
class semaphoreCheck
{
public:
	bool semachk;
	semaphore & s;
	semaphoreCheck( semaphore & _s, bool _semachk = true ) : s( _s ), semachk( _semachk )
	{
		if ( semachk ) {
			if ( s.check() ) return;
			s.wait();
		}
	}
	~semaphoreCheck()
	{
		if ( semachk ) s.signal();
	}
};

//自由曲線により範囲を指定するインタフェース

class freehand
{
	semaphore s;
public:
	bool clean;
	thisRegion::rectangle rect;
	array2<bool> flag;
	HWND hwnd;
	point2<int> first, final;
	thisRegion rgn;
	HDC hdc;
	HPEN pen;
	HGDIOBJ old;
	SIZE ws;// window client size
	SIZE rs;// reagion size
	freehand() : clean( true ), hdc( NULL ), pen( NULL ), old( NULL )
	{
		s.signal();
	}
	void initialize()
	{
		semaphoreCheck sc( s );
		pen = CreatePen( PS_SOLID, 0, 0xFFFFFF );
	}
	void finalize()
	{
		semaphoreCheck sc( s );
		DeleteObject( pen );
	}
	void start( HWND _hwnd, point2<int> p, SIZE rs )
	{
		if ( started() ) return;
		semaphoreCheck sc( s );
		hwnd = _hwnd;
		SetCapture( hwnd );
		RECT wr;
		GetClientRect( hwnd, & wr );
		this->rs = rs;
		ws.cx = wr.right - wr.left;
		ws.cy = wr.bottom - wr.top;
		rect = thisRegion::rectangle( 0, 0, rs.cx, rs.cy );
		flag.allocate( rect.right, rect.bottom );
		for ( array2<bool>::iterator it( flag ); it; ++it ) it() = false;
		rgn.initialize();
		clean = true;
		hdc = GetDC( hwnd );
		old = SelectObject( hdc, pen );
		track( p, p, false );
	}
	bool started()
	{
		semaphoreCheck sc( s );
		return flag.size ? true : false;
	}
	//領域処理終了
	void done()
	{
		semaphoreCheck sc( s );
		flag.release();
		ReleaseCapture();
	}
	void track( point2<int> start, point2<int> end, bool semachk = true )
	{
		semaphoreCheck sc( s, semachk );
		if ( ! hdc ) return;
		start = point2<int>( clamp( 0, start.x, rect.right - 1 ), clamp( 0, start.y, rect.bottom - 1 ) );
		end = point2<int>( clamp( 0, end.x, rect.right - 1 ), clamp( 0, end.y, rect.bottom - 1 ) );

		if ( clean ) {
			first = start;
			clean = false;
		}
		final = end;

		thisRegion line;
		line.line( start.x, start.y, end.x, end.y );
		for ( thisRegion::iterator it( line ); it; ++it ) {
			if ( between<int16>( 0, it->x, rect.right - 1 ) && 
				between<int16>( 0, it->y, rect.bottom - 1 ) ) {
				flag( it->x, it->y ) = true;
			}
		}
		MoveToEx( hdc, int( double( start.x ) / rs.cx * ws.cx ), int( double( start.y ) / rs.cy * ws.cy ), NULL );
		LineTo( hdc, int( double( end.x ) / rs.cx * ws.cx ), int( double( end.y ) / rs.cy * ws.cy ) );
	}
	//track終了
	bool finish()
	{
		semaphoreCheck sc( s );
		if ( ! clean ) track( final, first, false );
		if ( ! hdc || ! old ) return false;
		if ( hdc ) {
			SelectObject( hdc, old );
			ReleaseDC( hwnd, hdc );
			hdc = NULL;
			old = NULL;
		}
		if ( clean ) return false;
		thisRegion closedLine, outBox, outRgn;
		closedLine.set( flag, 0, 0 );
		thisRegion::rectangle outer = rect;
		outer.expand( 1 );
		outBox.push_back( outer );
		( outBox - closedLine ).select( outRgn, -1, -1 );
		rgn = outBox - outRgn;
		if ( rgn.empty() ) return false;
		return true;
	}
};
freehand freehandinterface;

fc::editbox * status = NULL;
fc::slider * slider = NULL;
fc::editbox * slider_value = NULL;
fc::window * imageWindow = NULL;
HWND viewwind = NULL;
image miximage;
int rate = 1;

POINT fixPoint( HWND hWnd, POINT p )
{
	//マウス座標をスクリーン座標にする
	ClientToScreen( hWnd, & p );
	//
	HWND imgwnd = imageWindow->hwnd;
	RECT wr;
	GetWindowRect( imgwnd, & wr );
//	ClientToScreen( imgwnd, ( ( POINT * ) ( & wr ) ) + 0 );
//	ClientToScreen( imgwnd, ( ( POINT * ) ( & wr ) ) + 1 );
	p.x = double( p.x - wr.left ) / ( wr.right - wr.left ) * miximage.width;
	p.y = double( p.y - wr.top ) / ( wr.bottom - wr.top ) * miximage.height;
	return p;
}
void regionBlend( image & img, const thisRegion & rgn, const pixel & color )
{
	if ( color.a == 255 ) {
		for ( thisRegion::iterator it( rgn ); it; ++it ) {
			img.set( it->x, it->y, color );
		}
		return;
	}
	decimal da = ( 255 - color.a ) / 255.0;
	decimal sa = color.a / 255.0;
	pixel sc( color.r * sa, color.g * sa, color.b * sa );
	for ( thisRegion::iterator it( rgn ); it; ++it ) {
		pixel p = img.getInternal( it->x, it->y );
		p.r = ( uint8 ) clamp<int32>( 0, p.r * da + sc.r, 255 );
		p.g = ( uint8 ) clamp<int32>( 0, p.g * da + sc.g, 255 );
		p.b = ( uint8 ) clamp<int32>( 0, p.b * da + sc.b, 255 );
		img.setInternal( it->x, it->y, p );
	}
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	fc::window::processAll( hWnd, uMsg, wParam, lParam );
	static POINT oldpoint;
	switch( uMsg ) {
	//case WM_INITDIALOG:
	case WM_CREATE:
		{
			if ( ! imageWindow ) {
				rate = 0;
				miximage.reformat( viewimage[2] );
				viewwind = hWnd;
				ShowWindow( hWnd, SW_SHOW );
				status = new fc::editbox( hWnd, 20001 );
				status->style |= ES_READONLY | WS_BORDER;
				slider = new fc::slider( hWnd, 20002 );
				slider_value = new fc::editbox( hWnd, 20003 );
				slider->edit = slider_value;
				imageWindow = new fc::window( hWnd, 0 );
				RECT cr;
				GetClientRect( hWnd, & cr );
				WORD nWidth = cr.right - cr.left;  // width of client area 
				WORD nHeight = cr.bottom - cr.top; // height of client area 
				const int edge = 10;
				const int editheight = 20;
				status->create( edge, edge, nWidth - 2 * edge, editheight );
				slider->create( edge * 2 + 50, edge + editheight + edge, nWidth - 3 * edge - 50, editheight );
				slider_value->create( edge, edge + editheight + edge, 50, editheight );
				slider->set( 1, 100 );
				rate = 10;
				slider->set( 40 );
				int w = nWidth - 2 * edge, h = nHeight - ( edge * 4 + editheight * 2 );
				imageWindow->create( "differ", "", WS_CHILD, edge, edge * 3 + editheight * 2, w, h );
				status->show( true );
				status->enable( false );
				imageWindow->show( true );
				//最初は等倍表示する
				RECT wr;
				GetWindowRect( hWnd, & wr );
				SetWindowPos( hWnd, NULL, 0, 0, 
					( wr.right - wr.left ) - w + miximage.width, 
					( wr.bottom - wr.top ) - h + miximage.height, 
					SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOCOPYBITS );
			}
		}
		return 1;
	case WM_PAINT:
		{
			if ( viewwind == hWnd ) {
				PAINTSTRUCT ps;
				RECT cr;
				GetClientRect( hWnd, & cr );
				HDC hDC = BeginPaint( hWnd, & ps );
				EndPaint( hWnd, & ps );
			} else if ( hWnd == imageWindow->hwnd ) {
				image & vi = miximage;
				HDC hDC = imageWindow->getDC();
				RECT cr = imageWindow->client();
				if ( vi.width && vi.height ) {
					BITMAPINFO bmpsrcinfo;
					memset( & bmpsrcinfo, 0, sizeof( bmpsrcinfo ) );
					bmpsrcinfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
					bmpsrcinfo.bmiHeader.biPlanes = 1;
					bmpsrcinfo.bmiHeader.biCompression = BI_RGB;
					bmpsrcinfo.bmiHeader.biWidth = vi.width;
					bmpsrcinfo.bmiHeader.biHeight = vi.height;
					bmpsrcinfo.bmiHeader.biBitCount = 32;//４バイト長で画像を作るのでパディングがいらなくなる
					SetStretchBltMode( hDC, HALFTONE );
					StretchDIBits( hDC, 0, 0, cr.right, cr.bottom, 0, 0, vi.width, vi.height, vi.data, & bmpsrcinfo, DIB_RGB_COLORS, SRCCOPY );
				} else {
					BitBlt( hDC, 0, 0, cr.right, cr.bottom, hDC, 0, 0, WHITENESS );
				}
				imageWindow->release( hDC );
				imageWindow->validate();
			}
		}
		break;
	case WM_NOTIFY:
	case WM_COMMAND:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if ( rate ) {
				if ( 
					( uMsg == WM_NOTIFY && LOWORD( wParam ) == 20002 && pnmh->code == NM_RELEASEDCAPTURE ) ||
					( uMsg == WM_COMMAND && LOWORD( wParam ) == 20003 && HIWORD(wParam) == EN_CHANGE ) ) {
					if ( slider->get() != rate ) {
						updateImage( ( rate = slider->get() ) / 10.0 );
						miximage.reformat( viewimage[2] );
						int area = freehandinterface.rgn.area();
						if ( area > 1 ) {
							//領域ありの場合
							pixel color;
							color.r = 0; color.b = 0; color.g = 255;
							color.a = 64;
							regionBlend( miximage, freehandinterface.rgn, color );
						}
						imageWindow->invalidate();
					}
				}
			}
			break;
		}
		break;
	case WM_DESTROY:
	case WM_QUIT:
		delete imageWindow; imageWindow = NULL;
		delete status; status = NULL;
		delete slider; slider = NULL;
		delete slider_value; slider_value = NULL;
		SetEvent( event );
		DestroyWindow( hWnd );
		break;
	case WM_SIZE:
		{
			if ( hWnd == viewwind ) {
				DWORD fwSizeType = wParam;      // resizing flag 
				WORD nWidth = LOWORD(lParam);  // width of client area 
				WORD nHeight = HIWORD(lParam); // height of client area 
				if ( fwSizeType == SIZE_MINIMIZED ) break;
				//ステータス
				const int edge = 10;
				const int editheight = 20;
				if ( status ) status->resize( edge, edge, nWidth - 2 * edge, editheight );
				if ( slider_value ) slider->resize( edge, edge * 2 + editheight, 50, editheight );
				if ( slider ) slider->resize( edge * 2 + 50, edge * 2 + editheight, nWidth - 3 * edge - 50, editheight );
				if ( imageWindow ) imageWindow->resize( edge, edge * 3 + editheight * 2, nWidth - 2 * edge, nHeight - ( edge * 4 + editheight * 2 ) );
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			if ( freehandinterface.started() ) break;//すでに動作している場合は無視
			POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			p = fixPoint( hWnd, p );
			SIZE rs = { miximage.width, miximage.height };
			freehandinterface.start( imageWindow->hwnd, point2<int>( p.x, p.y ), rs );
			oldpoint = p;
		}
		break;
	case WM_RBUTTONUP:
		{
			bool existRegion = false;
			POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			p = fixPoint( hWnd, p );

			//すでに領域指定をしていた場合
			if ( freehandinterface.started() ) {
				char wk[1024] = "";
				if ( freehandinterface.finish() ) {//有効領域な場合
					int area = freehandinterface.rgn.area();
					if ( area > 1 ) {
						//領域ありの場合
						miximage.reformat( viewimage[2] );
						pixel color;
						color.r = 0; color.b = 0; color.g = 255;
						color.a = 64;
						regionBlend( miximage, freehandinterface.rgn, color );
						imageInterface< pixelLuminance<int32> > & ci = calculatedimage[2];
						double average = 0;
						for ( thisRegion::iterator it( freehandinterface.rgn ); it; ++it ) {
							average += ci.getInternal( it->x, it->y ).y;
						}
						average /= area;
						sprintf( wk, "%.2f％：average percentage of target region ( greeny )", average / 100.0 );
					} else {
						miximage.reformat( viewimage[2] );
					}
				} else {
					miximage.reformat( viewimage[2] );
				}
				status->set( wk );
				freehandinterface.done();
				imageWindow->invalidate();
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			bool existRegion = false;
			POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			p = fixPoint( hWnd, p );

			if ( ! freehandinterface.started() ) {
				if ( between<int>( 0, p.x, calculatedimage[2].width - 1 ) && between<int>( 0, p.y, calculatedimage[2].height - 1 ) ) {
					char wk[1024];
					int32 y = calculatedimage[2].get( p.x, p.y ).y;
					sprintf( wk, "%.2f％ : percentage of target pixel [%d,%d]", y / 100.0, p.x, p.y );
					status->set( wk );
					freehandinterface.rgn.initialize();
					miximage.reformat( viewimage[2] );
					imageWindow->invalidate();
				}
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			POINT p = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			p = fixPoint( hWnd, p );
			if ( freehandinterface.started() ) {
				freehandinterface.track( point2<int>( oldpoint.x, oldpoint.y ), point2<int>( p.x, p.y ) );
				oldpoint = p;
			} else {
			}
		}
		break;
	default:
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
	return FALSE;
}
