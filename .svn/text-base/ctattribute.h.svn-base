//コメント変更ダイアログ
class ctAttribute : public fc::dialog
{
public:
	fc::editbox editTopMiddle, editBottomMiddle;
	fc::button checkBottom, checkProne;

	int numberSlice;
	int numberTop, numberBottom;
	bool bottomFirst;//trueの場合、尾側が最初の画像になる.
	bool prone;//trueの場合うつ伏せ
	
	ctAttribute( HWND _parent ) : fc::dialog( _parent, IDD_CT_ATTRIBUTE, dlgProc )
	{
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static ctAttribute * _this = NULL;
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<ctAttribute*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->editTopMiddle.initialize( hDlg, IDC_EDIT_TOP_MIDDLE );
					_this->editTopMiddle.set( _this->numberTop );
					_this->editBottomMiddle.initialize( hDlg, IDC_EDIT_BOTTOM_MIDDLE );
					_this->editBottomMiddle.set( _this->numberBottom );
					_this->checkBottom.initialize( hDlg, IDC_BOTTOM );
					_this->checkBottom.set( _this->bottomFirst ? fc::button::checked : fc::button::unchecked );
					_this->checkProne.initialize( hDlg, IDC_PRONE );
					_this->checkProne.set( _this->prone ? fc::button::checked : fc::button::unchecked );
				}
				break;
			case WM_COMMAND:
				{
					int numSlices = miDocument::get()->slicesize;
 					switch(LOWORD(wParam))
					{
					case IDOK:
						_this->numberTop = clamp( 0, atoi( _this->editTopMiddle.get() ), numSlices - 1 );
						_this->numberBottom = clamp( 0, atoi( _this->editBottomMiddle.get() ), numSlices - 1 );
						_this->prone = _this->checkProne.get() & 1 ? true : false;
						_this->bottomFirst = _this->checkBottom.get() & 1 ? true : false;
						if ( ( _this->bottomFirst && _this->numberTop < _this->numberBottom ) || 
							( ! _this->bottomFirst && _this->numberBottom < _this->numberTop ) ) {
							swap( _this->numberBottom, _this->numberTop );
						}
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
	bool call( int numberTop, int numberBottom, bool bottomFirst, bool prone )
	{
		this->numberTop = numberTop;
		this->numberBottom = numberBottom;
		this->bottomFirst = bottomFirst;
		this->prone = prone;
		return this->dialog::call();
	}
};


