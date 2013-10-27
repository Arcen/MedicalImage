//コメント変更ダイアログ
class changeComment : public fc::dialog
{
	static colorPicker * _this;
public:
	fc::editbox editname;
	fc::button red, green, blue, hue, saturation, value, alpha;
	fc::button original, current, picker, control;
	fc::slider slider;

	string name;
	pixel original_color;
	vector3 rgb;
	vector3 hsv;
	decimal a;
	pixel current_color;
	
	colorPicker( HWND _parent ) : fc::dialog( _parent, IDD_COLOR_PICKER, dlgProc )
	{
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<colorPicker*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->editname.initialize( hDlg, IDC_EDIT_NAME );
					_this->editname.set( _this->name );
					_this->red.initialize( hDlg, IDC_RADIO_RED );
					_this->green.initialize( hDlg, IDC_RADIO_GREEN );
					_this->blue.initialize( hDlg, IDC_RADIO_BLUE );
					_this->hue.initialize( hDlg, IDC_RADIO_HUE );
					_this->saturation.initialize( hDlg, IDC_RADIO_SATURATION );
					_this->value.initialize( hDlg, IDC_RADIO_VALUE );
					_this->alpha.initialize( hDlg, IDC_RADIO_ALPHA );
					_this->red.set( 1 );
					_this->original.initialize( hDlg, IDC_ORIGINAL_COLOR );
					_this->current.initialize( hDlg, IDC_CURRENT_COLOR );
					_this->picker.initialize( hDlg, IDC_COLOR_PICKER );
					_this->control.initialize( hDlg, IDC_COLOR_CONTROL );
					_this->original.paint = painter;
					_this->current.paint = painter;
					_this->slider.initialize( hDlg, IDC_SLIDER_CONTROL );
					_this->slider.set( 0, 255 );
					_this->slider.set( _this->current_color.r );
					_this->slider.changed = event;
					_this->red.clicked = event;
					_this->green.clicked = event;
					_this->blue.clicked = event;
					_this->hue.clicked = event;
					_this->saturation.clicked = event;
					_this->value.clicked = event;
					_this->alpha.clicked = event;
				}
				break;
			case WM_COMMAND:
				{
 					switch(LOWORD(wParam))
					{
					case IDOK:
						_this = NULL;
						EndDialog( hDlg, TRUE );
						break;
					case IDCANCEL:
						_this = NULL;
						EndDialog( hDlg, FALSE );
						break;
					}
				}
				break;
			}
		}
		return 0;
	}
	static void painter( fc::window * w )
	{
		if ( w == ( fc::window * ) & _this->original ) {
			RECT cr;
			GetClientRect( w->hwnd, & cr );
			HDC hdc = GetDC( w->hwnd );
			HBRUSH brush = CreateSolidBrush( RGB( _this->original_color.r, _this->original_color.g, _this->original_color.b ) );
			FillRect( hdc, & cr, brush );
			DeleteObject( brush );
			ReleaseDC( w->hwnd, hdc );
		} else if ( w == ( fc::window * ) & _this->current ) {
			RECT cr;
			GetClientRect( w->hwnd, & cr );
			HDC hdc = GetDC( w->hwnd );
			HBRUSH brush = CreateSolidBrush( RGB( _this->current_color.r, _this->current_color.g, _this->current_color.b ) );
			FillRect( hdc, & cr, brush );
			DeleteObject( brush );
			ReleaseDC( w->hwnd, hdc );
		}
	}
	static void event( fc::window * w )
	{
		if ( w == ( fc::window * ) & _this->slider ) {
			if ( _this->red.get() ) { _this->rgb.x = _this->slider.index / 255.0; _this->changeRGBA(); }
			else if ( _this->green.get() ) { _this->rgb.y = _this->slider.index / 255.0; _this->changeRGBA(); }
			else if ( _this->blue.get() ) { _this->rgb.z = _this->slider.index / 255.0; _this->changeRGBA(); }
			else if ( _this->alpha.get() ) { _this->a = _this->slider.index / 255.0; _this->changeRGBA(); }
			else if ( _this->hue.get() ) { _this->hsv.x = _this->slider.index / 255.0; _this->changeHSV(); }
			else if ( _this->saturation.get() ) { _this->hsv.y = _this->slider.index / 255.0; _this->changeHSV(); }
			else if ( _this->value.get() ) { _this->hsv.z = _this->slider.index / 255.0; _this->changeHSV(); }
			_this->current.invalidate();
		} else {
			if ( _this->red.get() ) { _this->slider.index = _this->rgb.x * 255.0; }
			else if ( _this->green.get() ) { _this->slider.index = _this->rgb.y * 255.0; }
			else if ( _this->blue.get() ) { _this->slider.index = _this->rgb.z * 255.0; }
			else if ( _this->alpha.get() ) { _this->slider.index = _this->a * 255.0; }
			else if ( _this->hue.get() ) { _this->slider.index = _this->hsv.x * 255.0; }
			else if ( _this->saturation.get() ) { _this->slider.index = _this->hsv.y * 255.0; }
			else if ( _this->value.get() ) { _this->slider.index = _this->hsv.z * 255.0; }
			_this->slider.set( _this->slider.index );
		}
	}
	pixel call( pixel original_color )
	{
		this->current_color = original_color;
		this->original_color = original_color;
		set( current_color );
		return this->dialog::call() ? current_color : original_color;
	}
	void set( const pixel & c )
	{
		rgb.x = clamp<decimal>( 0, c.r / 255.0, 1 );
		rgb.y = clamp<decimal>( 0, c.g / 255.0, 1 );
		rgb.z = clamp<decimal>( 0, c.b / 255.0, 1 );
		a = clamp<decimal>( 0, c.a / 255.0, 1 );
		hsv = rgb2hsv( rgb );
	}
	void changeRGBA()
	{
		hsv = rgb2hsv( rgb );
		current_color = pixel( rgb.x * 255, rgb.y * 255, rgb.z * 255, a * 255 );
	}
	void changeHSV()
	{
		rgb = hsv2rgb( hsv );
		current_color = pixel( rgb.x * 255, rgb.y * 255, rgb.z * 255, a * 255 );
	}
	// rgb([0,1],[0,1],[0,1])->hsv([0,1),[0,1],[0,1])
	vector3 rgb2hsv( vector3 rgb )
	{
		decimal & r = rgb.x;
		decimal & g = rgb.y;
		decimal & b = rgb.z;
		//ＲＧＢ値からＨＳＶ値を出す。
		//最大値、最小値、その差を出す
		decimal minimumcolor = minimum( r, minimum( g, b ) );
		decimal maximumcolor = maximum( r, maximum( g, b ) );
		decimal difference = maximumcolor - minimumcolor;
		//明度を出す　一番高い数値の色が明度になる。
		decimal v = maximumcolor;

		//彩度を出す　彩度は最大値と最低値の差を最大値で割る。
		decimal s = ( 0 < difference ) ? difference / maximumcolor : 0;

		//色相は　　　０度(1,0,0)
		//　　　→　６０度(1,1,0)
		//　　　→１２０度(0,1,0)
		//　　　→１８０度(0,1,1)
		//　　　→２４０度(0,0,1)
		//　　　→３００度(1,0,1)
		//　　　→３６０度(1,0,0)
		decimal h = 0;
		if ( 0 < s ) {
			if ( g == maximumcolor ) {
				h = ( 2 + ( b - r ) / difference ) / 6.0;
			} else if ( b == maximumcolor ) {
				h = ( 4 + ( r - g ) / difference ) / 6.0;
			} else if ( g < b ) {
				h = ( 6 + ( g - b ) / difference ) / 6.0;
			} else {
				h = ( ( g - b ) / difference ) / 6.0;
			}
		}
		return vector3( h, s, v );
	}
	// hsv([0,1),[0,1],[0,1])->rgb([0,1],[0,1],[0,1])
	vector3 hsv2rgb( vector3 hsv )
	{
		decimal & h = hsv.x;
		decimal & s = hsv.y;
		decimal & v = hsv.z;
		//ＨＳＶ値からＲＧＢ値を出す
		if ( ! s ) {//彩度が０だと白黒。明度に合わせる。
			return vector3( v, v, v );
		} else {
			int ih = clamp<int>( 0, int( h * 6 ), 5 );
			decimal dh = clamp<decimal>( 0, h * 6 - ih, 1 );
			decimal p = v * ( 1 - s );
			decimal q = v * ( 1 - s * dh );
			decimal t = v * ( 1 - s * ( 1 - dh ) );
			switch ( ih ) {
			case 0: return vector3( v, t, p );
			case 1: return vector3( q, v, p );
			case 2: return vector3( p, v, t );
			case 3: return vector3( p, q, v );
			case 4: return vector3( t, p, v );
			case 5: return vector3( v, p, q );
			}
		}
		return vector3::black;
	}
};


