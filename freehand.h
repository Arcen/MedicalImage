//自由曲線により範囲を指定するインタフェース

class freehand
{
	semaphore s;
public:
	bool clean;
	array2<bool> flag;
	HWND hwnd;
	point2<int> first, final;
	ctRegion rgn;
	HDC hdc;
	HPEN pen;
	HGDIOBJ old;
	point2<int> windowPos;
	point2<int> documentSize;
	int documentTimes;
	freehand() : clean( true ), hdc( NULL ), pen( NULL ), old( NULL )
	{
		s.signal();
		windowPos = point2<int>( 0, 0 );
		documentSize = point2<int>( 512, 512 );
		documentTimes = 1;
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
	void start( HWND _hwnd, point2<int> p )
	{
		if ( started() ) return;
		semaphoreCheck sc( s );
		hwnd = _hwnd;
		SetCapture( hwnd );
		flag.allocate( documentSize.x, documentSize.y );
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
		point2<int> startAtWindow = start;
		point2<int> endAtWindow = end;
		start += windowPos;
		end += windowPos;
		start /= documentTimes;
		end /= documentTimes;
		start = point2<int>( clamp( 0, start.x, documentSize.x - 1 ), clamp( 0, start.y, documentSize.y - 1 ) );
		end = point2<int>( clamp( 0, end.x, documentSize.x - 1 ), clamp( 0, end.y, documentSize.y - 1 ) );
		
		if ( clean ) {
			first = startAtWindow;
			clean = false;
		}
		final = endAtWindow;
		
		ctRegion line;
		line.line( start.x, start.y, end.x, end.y );
		for ( ctRegion::point_enumrator it( line ); it; ++it ) {
			if ( between<int16>( 0, it->x, documentSize.x - 1 ) && 
				between<int16>( 0, it->y, documentSize.y - 1 ) ) {
				flag( it->x, it->y ) = true;
			}
		}
		MoveToEx( hdc, startAtWindow.x, startAtWindow.y, NULL );
		LineTo( hdc, endAtWindow.x, endAtWindow.y );
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
		ctRegion bbox( ctRegion::rectangle::size( documentSize.x, documentSize.y ) );
		rgn &= bbox;
		ctRegion closedLine, outRgn;
        std::vector<char> v_flag( flag.w * flag.h, 0 );
        std::vector<char>::iterator itd = v_flag.begin();
        for ( array2<bool>::iterator its( flag ); its; ++its, ++itd ) {
            if ( *its ) *itd = 1;
        }
		closedLine.set( v_flag, flag.w, flag.h, 0, 0 );
		ctRegion outBox( ctRegion::rectangle( -1, -1, documentSize.x + 1, documentSize.y + 1 ) );
		( outBox - closedLine ).select( outRgn, -1, -1 );
		rgn = outBox - outRgn;
		if ( rgn.empty() ) return false;
		return true;
	}
};
