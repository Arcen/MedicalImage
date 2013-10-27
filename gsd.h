//ガウシアン相関用パラメータ設定ダイアログインタフェース

class gaussSDParam : public fc::dialog
{
public:
	fc::editbox _sd;
	decimal sd;
	gaussSDParam( decimal __sd, HWND _parent ) 
		: sd( __sd ), 
		fc::dialog( _parent, IDD_GSD, dlgProc )
	{
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static gaussSDParam * _this = NULL;
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<gaussSDParam*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->_sd.initialize( hDlg, IDC_EDIT_SD );
					_this->_sd.set( _this->sd );
				}
				break;
			case WM_COMMAND:
				{
 					switch(LOWORD(wParam))
					{
					case IDOK:
						_this->sd = atof( _this->_sd() );
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
};


