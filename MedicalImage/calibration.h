//キャリブレーションパラメータ設定ダイアログインタフェース
class calibrationParam : public fc::dialog
{
public:
	editboxSpin _air, _water;
	fc::button _airValid, _waterValid;
	int16 air, water;
	bool airValid, waterValid;

	calibrationParam( int16 __air, int16 __water, bool __airValid, bool __waterValid, HWND _parent ) 
		: air( __air ), water( __water ), airValid( __airValid ), waterValid( __waterValid ), 
		fc::dialog( _parent, IDD_CALIBRATION_PARAM, dlgProc )
	{
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static calibrationParam * _this = NULL;
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<calibrationParam*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->_air.initialize( hDlg, IDC_EDIT_AIR, calibrationAirSpinID, -2047, 2047 );
					_this->_air( _this->air );
					_this->_water.initialize( hDlg, IDC_EDIT_WATER, calibrationWaterSpinID, -2047, 2047 );
					_this->_water( _this->water );
					_this->_airValid.initialize( hDlg, IDC_CHECK_AIR );
					_this->_waterValid.initialize( hDlg, IDC_CHECK_WATER );
					_this->_airValid.set( _this->airValid ? 1 : 0 );
					_this->_waterValid.set( _this->waterValid ? 1 : 0 );
				}
				break;
			case WM_COMMAND:
				{
 					switch(LOWORD(wParam))
					{
					case IDOK:
						_this->air = _this->_air();
						_this->water = _this->_water();
						_this->airValid = _this->_airValid.get() ? true : false;
						_this->waterValid = _this->_waterValid.get() ? true : false;
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
			default:
				if ( _this ) {
					_this->_air.process();
					_this->_water.process();
				}
				break;
			}
		}
		return 0;
	}
};


