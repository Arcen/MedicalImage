class dicomSeriesSelect : public fc::dialog
{
public:
	fc::listbox seriesList;
	fc::editbox editStart, editMaximum, editSpace;

	int index;
	int start, maximumNumber, space;
	array<dicomSeriesInformation> * series;
	
	dicomSeriesSelect( HWND _parent ) : fc::dialog( _parent, IDD_DICOM_SERIES, dlgProc ), series( NULL )
	{
		start = 1;
		maximumNumber = 1000;
		space = 1;
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static dicomSeriesSelect * _this = NULL;
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<dicomSeriesSelect*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->editStart.initialize( hDlg, IDC_EDIT_START );
					_this->editStart.set( _this->start );
					_this->editMaximum.initialize( hDlg, IDC_EDIT_MAXIMUM );
					_this->editMaximum.set( _this->maximumNumber );
					_this->editSpace.initialize( hDlg, IDC_EDIT_SPACE );
					_this->editSpace.set( _this->space );
					_this->seriesList.initialize( hDlg, IDC_LIST_SERIES );
					if ( _this->series ) {
						int i = 0;
						for ( array<dicomSeriesInformation>::iterator it( *(_this->series) ); it; ++it, ++i ) {
							dicomSeriesInformation & dsi = it();
							string wk;
							wk.allocate( 1024 );
							wk.print( "スライス厚 %.2f [mm]：枚数 %d枚", dsi.thickness, dsi.files.size );
							_this->seriesList.push_back( wk );
						}
						_this->seriesList.update();
						_this->seriesList.select( _this->index );
					}
					SetFocus( GetDlgItem( hDlg, IDOK ) );
					return 0;
				}
				break;
			case WM_COMMAND:
				{
 					switch(LOWORD(wParam))
					{
					case IDOK:
						if ( 0 <= _this->seriesList.get() ) {
							_this->index = _this->seriesList.get();
							_this->start = maximum<int>( 0, atoi( _this->editStart.get() ) );
							_this->maximumNumber = maximum<int>( 1, atoi( _this->editMaximum.get() ) );
							_this->space = maximum<int>( 1, atoi( _this->editSpace.get() ) );
							_this = NULL;
							EndDialog( hDlg, TRUE );
						}
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


