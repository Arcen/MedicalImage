//�L�����u���[�V�����p�����[�^�ݒ�_�C�A���O�C���^�t�F�[�X
class setPartAttributeDialog : public fc::dialog
{
public:
	fc::editbox index;
	fc::combobox _type;
	int areaIndex, partIndex;
	int type;

	setPartAttributeDialog( int __areaIndex, int __partIndex, int __type, HWND _parent ) 
		: areaIndex( __areaIndex ), partIndex( __partIndex ), type( __type ),
		fc::dialog( _parent, IDD_DIALOG_SET_ATTRIBUTE, dlgProc )
	{
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static setPartAttributeDialog * _this = NULL;
		if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
		if ( ! _this || hDlg == _this->hwnd ) {
			switch( uMsg ) {
			case WM_INITDIALOG:
				{
					if ( _this ) break;
					_this = static_cast<setPartAttributeDialog*>( reinterpret_cast<fc::dialog*>( lParam ) );
					_this->hwnd = hDlg;
					_this->_type.initialize( hDlg, IDC_COMBO_TYPE );
					_this->_type.push_back( "�E��t" );
					_this->_type.push_back( "�E���t" );
					_this->_type.push_back( "�E���t" );
					_this->_type.push_back( "����t" );
					_this->_type.push_back( "�����t" );
					_this->_type.select( _this->type );
					_this->index.initialize( hDlg, IDC_EDIT_INDEX );
					static string wk;
					wk.print( "�x��[%d]�ʒu[%d]", _this->areaIndex, _this->partIndex );
					_this->index.set( wk );
					SetFocus( GetDlgItem( hDlg, IDOK ) );
					return 0;
				}
				break;
			case WM_COMMAND:
				{
 					switch(LOWORD(wParam))
					{
					case IDOK:
						_this->type = clamp<int>( 0, _this->_type.select(), 4 );
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
				return 0;
			}
		}
		return 0;
	}
};


