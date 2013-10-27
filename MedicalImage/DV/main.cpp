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
bool validDocument = false;
imageInterface< pixelLuminance<int16> > dicomImage;
image displayImage;
int32 displayImageCenter = 0, displayImageWidth = 2000;

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

#include "dom2treeview.inl"
#include "dicomdir2treeview.inl"
#include "elementsProc.inl"
#include "contrastProc.inl"
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

	elementViewerWindow = new fc::dialog( imageWindow->hwnd, IDD_ELEMENTS, DialogProc );
	elementViewerWindow->create();
	if ( ! elementViewerWindow->hwnd ) return 0;

	imageWindow->show( true );
	elementViewerWindow->show( false );

	UpdateWindow( imageWindow->hwnd );//�E�B���h�E�̍ĕ`��𑣂�

	// ���C�� ���b�Z�[�W ���[�v
	MSG msg;
	HACCEL accelerator = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );
	while( GetMessage(&msg, NULL, 0, 0) ) {
		if ( accelerator && TranslateAccelerator( imageWindow->hwnd, accelerator, & msg ) ) {
		} else {
			TranslateMessage( &msg );
		}
		DispatchMessage( &msg );
	}
	if ( domDocument ) {
		domDocument.Release();
	}

	if ( CoInitialized ) CoUninitialize();
	return msg.wParam;
}
