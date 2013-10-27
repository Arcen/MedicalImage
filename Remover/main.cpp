//Windows�w�b�_�t�@�C��
#include <windows.h>	//Windows��{�w�b�_�t�@�C��

//���\�[�X�w�b�_�t�@�C��
#include "resource.h"

#include <agl/kernel/agl.h>
using namespace agl;

#include "../dicom.h"

#include <fc/fc.h>
#pragma comment( lib, "gdi32.lib" )
#pragma comment( lib, "advapi32.lib" )
#pragma comment( lib, "comdlg32.lib" )

void dicomExport( const char * dicomfilename )
{
	// debug DICOM��񏑂��o��
	char drive[MAX_PATH], path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( dicomfilename, drive, path, filename, extension );
	char output[MAX_PATH];
	_makepath( output, drive, path, filename, ".xml" );
	retainer<file> f = file::initialize( dicomfilename, false, true );
	if ( ! f ) return;
	static dicom dcm;
	dcm.export_( f(), output );
}

//�O���[�o���ϐ�
HINSTANCE hInstance;//�C���X�^���X�n���h���i�A�v���P�[�V�����t�@�C���̏��j
char szWindowClass[1024] = "DicomViewer";//�E�B���h�E�N���X���i�A�v���P�[�V�����ŗL�̖��́j
fc::window * imageWindow = NULL;
fc::dialog * elementViewerWindow = NULL;
MSXML2::IXMLDOMDocumentPtr domDocument;
imageInterface< pixelLuminance<int16> > dicomImage;
image displayImage;

//Open�p�t�@�C���I���_�C�A���O
//�߂�l��false�̎��̓L�����Z���Atrue�͐���
//filename:�I�������t�@�C���̖��̂�ۑ�����o�b�t�@ �Œ�MAX_PATH���̃��������K�v
//defaultName:�ŏ��ɕ\�������f�t�H���g����
//mask:�I���E�B���h�E�Ń}�X�N�������s�����߂̕�����B�\����\0�}�X�N\0�@�Ƒ����A\0���Q�A������Ƃ���܂Ń��X�g�ɂȂ�B�}�X�N��*.bmp�Ȃǃ��C���h�J�[�h���g�p�ł���B
//title:�^�C�g���o�[�̕�����
//parent:�e�E�B���h�E�̃n���h��
bool fileOpen( char * filename, char * defaultName, char * mask, char * title, HWND parent )
{
	//������
	filename[0] = '\0';
	if ( defaultName ) strcpy( filename, defaultName );

	OPENFILENAME openfilename = { sizeof( OPENFILENAME ) };
	openfilename.hwndOwner = parent;
	openfilename.hInstance = hInstance;
	openfilename.lpstrFilter = mask;
	openfilename.lpstrFile = filename;
	openfilename.nMaxFile = MAX_PATH;
	openfilename.lpstrTitle = title;
	//�t���O�̐ݒ�
	openfilename.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if ( ! GetOpenFileName( & openfilename ) ) return false;
	return true;
}
//Save�p�t�@�C���I���_�C�A���O
//�߂�l��false�̎��̓L�����Z���Atrue�͐���
//filename:�I�������t�@�C���̖��̂�ۑ�����o�b�t�@ �Œ�MAX_PATH���̃��������K�v
//defaultName:�ŏ��ɕ\�������f�t�H���g����
//mask:�I���E�B���h�E�Ń}�X�N�������s�����߂̕�����B�\����\0�}�X�N\0�@�Ƒ����A\0���Q�A������Ƃ���܂Ń��X�g�ɂȂ�B�}�X�N��*.bmp�Ȃǃ��C���h�J�[�h���g�p�ł���B
//title:�^�C�g���o�[�̕�����
//parent:�e�E�B���h�E�̃n���h��
//defext:�f�t�H���g�g���q
bool fileSave( char * filename, char * defaultName, char * mask, char * title, HWND parent, char * defext = NULL )
{
	strcpy( filename, "" );
	if ( defaultName ) strcpy( filename, defaultName );

	OPENFILENAME openfilename; memset( & openfilename, 0, sizeof( openfilename ) );
	openfilename.lStructSize = sizeof( OPENFILENAME );
	openfilename.hwndOwner = parent;
	openfilename.hInstance = hInstance;
	openfilename.lpstrFilter = mask;
	openfilename.lpstrFile = filename;
	openfilename.nMaxFile = MAX_PATH;
	openfilename.lpstrTitle = title;
	openfilename.lpstrDefExt = defext;
	openfilename.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT;

	if ( ! GetSaveFileName( & openfilename ) ) return false;
	return true;
}
void SHFreeIDList( ITEMIDLIST * iidl )
{
	if( ! iidl ) return;
	LPMALLOC pMalloc;
	if( ::SHGetMalloc( &pMalloc ) != NOERROR ) return;
	pMalloc->Free( iidl );
	pMalloc->Release();
}
int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	if ( uMsg == BFFM_INITIALIZED ){
		SendMessage( hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData );
	}
    return 0;
}
bool selectDirectory( char * filename, char * defaultName, char * title, HWND parent )
{
	//�p�X�I��
	BROWSEINFO bi = { NULL };
	bi.hwndOwner = parent;
	bi.pszDisplayName = filename;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
	//�f�t�H���g�̃f�B���N�g��
	if ( defaultName ) {
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM) defaultName;
	}
	ITEMIDLIST * iidl = SHBrowseForFolder(&bi);
	if ( ! iidl ) return false;
	SHGetPathFromIDList( iidl, filename );
	return true;
}

#include "windowProc.inl"

//Windows32API�v���O������main�֐��ɂ����镨�B���ۂ̓��C�u��������main�֐������肻������Ăяo�����
//hInstance:�A�v���P�[�V�����t�@�C�����
//hPrevInstance:���p���Ȃ�
//lpCmdLine:�R�}���h���C���ŋN�������Ƃ��̏��
//nCmdShow:�t�@�C���v���p�e�B�Őݒ肵���N�����̃E�B���h�E�̕\���̎��
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	::hInstance = hInstance;
	bool CoInitialized = ( S_OK == CoInitialize( NULL ) );
	fc::window::instance = hInstance;
	fc::window::initialize();
	//�E�B���h�E�쐬
	imageWindow = new fc::window();
	imageWindow->registerClass( WndProc, szWindowClass, IDR_MENU1, IDI_ICON1, 0, COLOR_WINDOW+1 );

	imageWindow->createRoot( szWindowClass, szWindowClass, WS_OVERLAPPEDWINDOW );
	if ( ! imageWindow->hwnd ) return 0;

	imageWindow->show( true );

	UpdateWindow( imageWindow->hwnd );//�E�B���h�E�̍ĕ`��𑣂�

	// ���C�� ���b�Z�[�W ���[�v
	MSG msg;
	HACCEL accelerator = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );
	while( GetMessage(&msg, NULL, 0, 0) ) {
		if ( accelerator && TranslateAccelerator( imageWindow->hwnd, accelerator, & msg ) ) {
		} else {
			TranslateMessage( &msg );
		    DispatchMessage( &msg );
		}
	}
	if ( domDocument ) {
		domDocument.Release();
	}

	if ( CoInitialized ) CoUninitialize();
	return msg.wParam;
}
