//ƒRƒƒ“ƒg•ÏXƒ_ƒCƒAƒƒO
class patternAttribute : public fc::dialog
{
public:
	fc::editbox editname, editpostfix, editcomment;
	fc::combobox lungTopBottom, lungFrontRear;

	string name, postfix, comment;
	int topbottom, frontrear;
	
	patternAttribute( HWND _parent ) : fc::dialog( _parent, IDD_PATTERN_ATTRIBUTE, dlgProc )
	{
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static patternAttribute * _this = NULL;
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<patternAttribute*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->editname.initialize( hDlg, IDC_EDIT_NAME );
					_this->editname.set( _this->name );
					_this->editpostfix.initialize( hDlg, IDC_EDIT_POSTFIX );
					_this->editpostfix.set( _this->postfix );
					_this->editcomment.initialize( hDlg, IDC_EDIT_COMMENT );
					_this->editcomment.set( _this->comment );
					_this->lungTopBottom.initialize( hDlg, IDC_COMBO_LUNG_TOP_BOTTOM );
					_this->lungTopBottom.push_back( "”xë•”" );
					_this->lungTopBottom.push_back( "”x’†•”" );
					_this->lungTopBottom.push_back( "”x’ê•”" );
					_this->lungTopBottom.select( _this->topbottom );
					_this->lungFrontRear.initialize( hDlg, IDC_COMBO_LUNG_FRONT_REAR );
					_this->lungFrontRear.push_back( "• ‘¤" );
					_this->lungFrontRear.push_back( "’†ŠÔ" );
					_this->lungFrontRear.push_back( "”w‘¤" );
					_this->lungFrontRear.select( _this->frontrear );
				}
				break;
			case WM_COMMAND:
				{
 					switch(LOWORD(wParam))
					{
					case IDOK:
						_this->postfix = _this->editpostfix.get();
						_this->comment = _this->editcomment.get();
						_this->topbottom = clamp<int>( lungTop, _this->lungTopBottom.select(), lungBottom );
						_this->frontrear = clamp<int>( lungFront, _this->lungFrontRear.select(), lungRear );
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
	bool call( const string & name, const string & postfix, const string & comment, int topbottom, int frontrear )
	{
		this->name = name;
		this->postfix = postfix;
		this->comment = comment;
		this->topbottom = topbottom;
		this->frontrear = frontrear;
		return this->dialog::call();
	}
};


