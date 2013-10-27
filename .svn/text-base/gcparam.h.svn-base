//ガウシアン相関用パラメータ設定ダイアログインタフェース

class gaussCorrelationParam : public fc::dialog
{
public:
	editboxSpin _start, _end, _step;
	fc::editbox _count;
	int start, end, step, count;
	gaussCorrelationParam( int __start, int __end, int __step, HWND _parent ) 
		: start( __start ), end( __end ), step( __step ), count( 0 ), 
		fc::dialog( _parent, IDD_GC, dlgProc )
	{
		update();
	}
	void update()
	{
		end = maximum( start, end );
		step = maximum( 1, minimum( step, end - start ) );
		count = 0;
		for ( int i = start; i <= end; i += step ) ++count;
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static gaussCorrelationParam * _this = NULL;
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<gaussCorrelationParam*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->_start.initialize( hDlg, IDC_EDIT_START, gaussCorrelationParamStartSpinID, 1, _this->end );
					_this->_start( _this->start );
					_this->_end.initialize( hDlg, IDC_EDIT_END, gaussCorrelationParamEndSpinID, _this->start, 64 );
					_this->_end( _this->end );
					_this->_step.initialize( hDlg, IDC_EDIT_STEP, gaussCorrelationParamStepSpinID, 1, _this->end - _this->start );
					_this->_step( _this->step );
					_this->_count.initialize( hDlg, IDC_EDIT_COUNT );
					_this->_count.set( _this->count );
				}
				break;
			case WM_COMMAND:
				{
 					switch(LOWORD(wParam))
					{
					case IDOK:
						_this->start = _this->_start();
						_this->end = _this->_end();
						_this->step = _this->_step();
						_this->count = atoi( _this->_count() );
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
					bool strt = _this->_start.process();
					bool end = _this->_end.process();
					bool stp = _this->_step.process();
					if ( strt || end || stp ) {
						_this->start = _this->_start();
						_this->end = _this->_end();
						_this->step = _this->_step();
						_this->update();
						_this->_start.initialize( 1, _this->end );
						_this->_end.initialize( _this->start, 64 );
						_this->_step.initialize( 1, _this->end - _this->start );
						_this->_start( _this->start );
						_this->_end( _this->end );
						_this->_step( _this->step );
						_this->_count( _this->count );
					}
				}
				break;
			}
		}
		return 0;
	}
};


