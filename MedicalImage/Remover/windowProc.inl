//�E�B���h�E�v���V�[�W���i���낢��ȃ��b�Z�[�W���������邽�߂̃R�[���o�b�N�֐��j
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static dicom dcm;
	switch( message ) {
	case WM_COMMAND:// ���j���[�I���C�x���g
		switch( LOWORD(wParam) ) {
		case IDM_OPEN://�J�����j���[�I��
			{
				char filename[MAX_PATH];//�I�������t�@�C�����������o�b�t�@
				//�t�@�C���I���_�C�A���O�Ńt�@�C����I��
				if ( fileOpen( filename, "", "DICOM�t�@�C��(*)\0*\0", "DICOM�t�@�C�����J��", hWnd ) ) {
					retainer<file> f = file::initialize( filename, false, true );
					if ( ! f ) break;
					dicomImage.finalize();
					
                    domDocument = MSXML2::IXMLDOMDocumentPtr( "MSXML.DOMDocument" );
					if ( dcm.convert( f(), domDocument ) ) {
						f->seek( 0, SEEK_SET );
						if ( dcm.read( f(), dicomImage ) ) {
							char drive[MAX_PATH], path[MAX_PATH], fn[MAX_PATH], extension[MAX_PATH];
							_splitpath( filename, drive, path, fn, extension );
							char rawpath[MAX_PATH];
							_makepath( rawpath, drive, path, fn, ".raw" );
							retainer<file> raw = file::initialize( rawpath, true, true );
							if ( raw ) {
								raw->write( dicomImage.data, dicomImage.size * 2 );
								raw->write( dicomImage.width );
								raw->write( dicomImage.height );
								int band = 2;
								raw->write( band );
							}
						}
					} else {
						domDocument = IXMLDOMDocumentPtr( "MSXML.DOMDocument" );
					}
				}
			}
			break;
		case IDM_REMOVE_FILE://�t�@�C���P�ʂ̌l���폜
			{
				char filename[MAX_PATH];//�I�������t�@�C�����������o�b�t�@
				//�t�@�C���I���_�C�A���O�Ńt�@�C����I��
				if ( fileOpen( filename, "", "DICOM�t�@�C��(*)\0*\0", "DICOM�t�@�C�����J��", hWnd ) ) {
					retainer<file> f = file::initialize( filename, false, true );
					if ( ! f ) break;
					char drive[MAX_PATH], path[MAX_PATH], fn[MAX_PATH], extension[MAX_PATH];
					_splitpath( filename, drive, path, fn, extension );
					char outdirectory[MAX_PATH];
					_makepath( outdirectory, drive, path, "", "" );
					if ( selectDirectory( outdirectory, outdirectory, "�o�͂���f�B���N�g���I��", hWnd ) ) {
						char outfilename[MAX_PATH];
						_makepath( outfilename, "", outdirectory, fn, extension );
						if ( stricmp( filename, outfilename ) == 0 ) {
							strcat( outfilename, ".out" );
						}
						retainer<file> out = file::initialize( outfilename, true, true );
						if ( ! out ) break;
						
						if ( dcm.removeInformation( f(), out() ) ) {
							//����
						}
					}
				}
			}
			break;
			
		case IDM_QUIT://�I�����j���[�I��
			DestroyWindow( hWnd );//�E�B���h�E��j��
			break;
		default:
			//���̃��j���[��I�񂾂Ƃ��ɂ̓f�t�H���g�̓����������
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
		case WM_PAINT://�`��C�x���g
			{
				PAINTSTRUCT ps;
				HDC hDC = BeginPaint (hWnd, &ps);//�X�V�̈�݂̂̃f�o�C�X�R���e�L�X�g���擾
				RECT cr;//��`���
				GetClientRect( hWnd, & cr );//�E�B���h�E�̕`��ł���傫�����擾
				BitBlt( hDC, 0, 0, cr.right, cr.bottom, hDC, 0, 0, WHITENESS );
				EndPaint( hWnd, &ps );
			}
			break;
		case WM_DESTROY://�E�B���h�E�̔j��C�x���g�������ꍇ�Ƀ��b�Z�[�W���[�v�𔲂���悤�ɃC�x���g�𓊂���
			PostQuitMessage( 0 );
			break;
		default:
			//���̂ق��̃C�x���g�̎��ɂ̓f�t�H���g�̓�����s��
			return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}
