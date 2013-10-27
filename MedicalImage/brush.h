//ブラシにより領域を指定するインタフェース

class brush
{
	semaphore s;
public:
	class node
	{
	public:
		int size;
		HCURSOR cursor;
		ctRegion rgn;
		void initialize( int _size, int id )
		{
			size = _size;
			cursor = LoadCursor( hInstance, MAKEINTRESOURCE( id ) );
			ctRegion wk = ctRegion( ctRegion::rectangle::pixel( 0, 0 ) );
			if ( size ) {
				wk.dilation( rgn, size );
			} else {
				rgn = wk;
			}
		}
        node()
        {
            size = -1;
            cursor = 0;
        }
        ~node()
        {
            if ( cursor ) DestroyCursor( cursor );
        }
	};
	node n[16];
	node * current;
	HPEN pen;
	HCURSOR prev;
	ctRegion rgn;//現在作業中のバッファ
	HDC hdc;
	HGDIOBJ old;
	HWND hwnd;
	point2<int> windowPos;
	point2<int> documentSize;
	int documentTimes;
	brush() : pen( NULL ), current( NULL ), hdc( NULL ), old( NULL ), hwnd( NULL )
	{
		s.signal();
		windowPos = point2<int>( 0, 0 );
		documentSize = point2<int>( 512, 512 );
		documentTimes = 1;
	}
	void initialize()
	{
		semaphoreCheck sc( s );
		int ids[] = { IDC_BRUSH_0, IDC_BRUSH_1, IDC_BRUSH_2, IDC_BRUSH_3, IDC_BRUSH_4,
			IDC_BRUSH_5, IDC_BRUSH_6, IDC_BRUSH_7, IDC_BRUSH_8, IDC_BRUSH_9, 
			IDC_BRUSH_10, IDC_BRUSH_11, IDC_BRUSH_12, IDC_BRUSH_13, IDC_BRUSH_14, IDC_BRUSH_15 };
		for ( int i = 0; i < 16; ++i ) {
			n[i].initialize( i, ids[i] );
		}
		pen = CreatePen( PS_SOLID, 0, 0xFFFFFF );
	}
	void finalize()
	{
		semaphoreCheck sc( s );
		DeleteObject( pen );
	}
	void track( point2<int> p, bool semachk = true )
	{
		semaphoreCheck sc( s, semachk );
		//修正形状を保存する
		ctRegion wk = current->rgn;
		//画像の座標系に移動
		p += windowPos;
		p /= documentTimes;
		wk.offset( p.x, p.y );
		rgn |= wk;
		//画面表示
		if ( ! hdc ) return;
        ctRegion::rectangle r;
        for ( ctRegion::rectangle_const_enumrator it = wk.range(); it; ++it ) {
			r = *it;
			//表示用の座標系に移動
			r.left *= documentTimes;
			r.right *= documentTimes;
			r.top *= documentTimes;
			r.bottom *= documentTimes;
			r.offset( - windowPos.x, - windowPos.y );
			if ( r.area() == 1 ) {
				SetPixel( hdc, r.left, r.top, 0xFFFFFF );
			} else {
				Rectangle( hdc, r.left, r.top, r.right, r.bottom );
			}
		}
	}
	void start( HWND _hwnd, point2<int> p, int size )
	{
		semaphoreCheck sc( s );
		if ( current ) return;
		hwnd = _hwnd;
		//ブラシにより領域を修正
		SetCapture( hwnd );
		rgn.initialize();//修正領域を初期化
		current = & n[ clamp<int>( 0, size, 15 ) ];
		prev = SetCursor( current->cursor );
		hdc = GetDC( hwnd );
		old = SelectObject( hdc, pen );
		track( p, false );
	}
	bool started()
	{
		semaphoreCheck sc( s );
		return current ? true : false;
	}
	bool finish()
	{
		ReleaseCapture();
		if ( ! started() ) return false;
		semaphoreCheck sc( s );
		if ( hdc ) {
			SelectObject( hdc, old );
			ReleaseDC( hwnd, hdc );
			hdc = NULL;
			old = NULL;
			hwnd = NULL;
		}
		SetCursor( prev );
		current = NULL;
		prev = NULL;
		//領域を修正するコマンドを発行
		ctRegion bbox( ctRegion::rectangle::size( documentSize.x, documentSize.y ) );
		rgn &= bbox;
		return true;
	}
};
