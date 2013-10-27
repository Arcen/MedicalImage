//Windows�w�b�_�t�@�C��
#include <windows.h>	//Windows��{�w�b�_�t�@�C��

//���\�[�X�w�b�_�t�@�C��
#include "resource.h"

#include <agl/kernel/agl.h>
using namespace agl;

#include "../dicom.h"

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
const char szWindowClass[1024] = "DICOMEXPORT";//�E�B���h�E�N���X���i�A�v���P�[�V�����ŗL�̖��́j

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

//�E�B���h�E�v���V�[�W���i���낢��ȃ��b�Z�[�W���������邽�߂̃R�[���o�b�N�֐��j
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message ) {
	case WM_COMMAND:// ���j���[�I���C�x���g
		switch( LOWORD(wParam) ) {
		case IDM_OPEN://�J�����j���[�I��
			{
				char filename[MAX_PATH];//�I�������t�@�C�����������o�b�t�@
				//�t�@�C���I���_�C�A���O�Ńt�@�C����I��
				if ( fileOpen( filename, "", "DICOM�t�@�C��(*)\0*\0", "DICOM�t�@�C�����J��", hWnd ) ) {
					dicomExport( filename );
				}
			}
			break;
		case IDM_EXIT://�I�����j���[�I��
			DestroyWindow( hWnd );//�E�B���h�E��j��
			break;
		default:
			//���̃��j���[��I�񂾂Ƃ��ɂ̓f�t�H���g�̓����������
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
		/*
	case WM_PAINT://�`��C�x���g
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint (hWnd, &ps);//�X�V�̈�݂̂̃f�o�C�X�R���e�L�X�g���擾
			RECT cr;//��`���
			GetClientRect( hWnd, & cr );//�E�B���h�E�̕`��ł���傫�����擾
			EndPaint( hWnd, &ps );
		}
		break;
		*/
	case WM_DESTROY://�E�B���h�E�̔j��C�x���g�������ꍇ�Ƀ��b�Z�[�W���[�v�𔲂���悤�ɃC�x���g�𓊂���
		PostQuitMessage( 0 );
		break;
	default:
		//���̂ق��̃C�x���g�̎��ɂ̓f�t�H���g�̓�����s��
		return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}
namespace MersenneTwister
{
	// http://www.math.keio.ac.jp/~matumoto/mt.html

	
	// buffer : unsigned int[625];

	// Period parameters
	const int N = 624;
	const int M = 397;
	
	inline unsigned int matrix( unsigned int y )
	{ return ( y & 1 ) ? 0x9908b0df : 0; }
	inline unsigned int connect( unsigned int u, unsigned int l )
	{ return (u&0x80000000)|(l&0x7fffffff); }
	inline unsigned int random(unsigned int * mt)
	{
		unsigned int & mti = mt[N];
		unsigned int y;
		
		if (mti >= N) { // generate N words at one time
			unsigned int * p = mt;
			unsigned int * sentinel = p + N-M;
			while ( p != sentinel ) {
				y = connect( *p, p[1] );
				*p = p[M] ^ (y >> 1) ^ matrix( y );
				p++;
			}
			sentinel = mt + N - 1;
			while ( p != sentinel ) {
				y = connect( *p, p[1] );
				*p = p[M-N] ^ (y >> 1) ^ matrix( y );
				p++;
			}
			y = connect( *p, mt[0] );
			mt[N-1] = mt[M-1] ^ (y >> 1) ^ matrix( y );
			
			mti = 0;
		}
		
		y = mt[mti++];
		y ^= (y>>11);
		y ^= (y<<7) & 0x9d2c5680;
		y ^= (y<<15) & 0xefc60000;
		y ^= (y>>18);
		return y & 0xffffffff;
	}
	// buffer mt[N+1]
	// Initializing the array with a seed
	inline void initialize( unsigned int seed, unsigned int * mt )
	{
		unsigned int * p = mt;
		unsigned int * sentinel = p + N;
		while ( p != sentinel ) {
			*p = seed & 0xffff0000;
			seed = 69069 * seed + 1;
			*p |= (seed & 0xffff0000) >> 16;
			seed = 69069 * seed + 1;
			p++;
		}
		*p = N;
		// warm up
		for ( int i = 0; i < 200; i++ ) random( mt );
	}
};
// thread safe�œ������Ƃ��ɂ�externalBuffer��int[626]�̂O�N���A�f�[�^��n��
// [0]�������ς݃t���O
// [��]�o�b�t�@
inline double random( bool initialize = false, unsigned int seed = 4357, unsigned int * externalBuffer = 0 )
{
	// Knuth's random number generator
	static unsigned int ib[626] = { 0 };
	unsigned int * ip = ( externalBuffer ? externalBuffer : ib );
	if ( initialize || ( ! *ip ) ) {
		*ip = 1;
		MersenneTwister::initialize( seed, ip + 1 );
	}
	return double( MersenneTwister::random( ip + 1 ) ) / double( 4294967296.0 );
}

//Windows32API�v���O������main�֐��ɂ����镨�B���ۂ̓��C�u��������main�֐������肻������Ăяo�����
//hInstance:�A�v���P�[�V�����t�@�C�����
//hPrevInstance:���p���Ȃ�
//lpCmdLine:�R�}���h���C���ŋN�������Ƃ��̏��
//nCmdShow:�t�@�C���v���p�e�B�Őݒ肵���N�����̃E�B���h�E�̕\���̎��
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	const int N = 4;
	matrixT<double> wk( N, N ), H0L, H0R, v, d, u;
	for ( int i = 0; i < N; ++i ) {
		for ( int j = 0; j < N; ++j ) {
			wk[i][j] = random() * 100 - 50;
		}
	}
	matrixT<double>::debug( wk, "init" );
	wk.singularValueDecomposition( v, d, u );
	matrixT<double>::debug( wk, "svd" );
	matrixT<double>::debug( v, "v" );
	matrixT<double>::debug( d, "d" );
	matrixT<double>::debug( u, "u" );

	return 0;
	::hInstance = hInstance;

	bool CoInitialized = ( S_OK == CoInitialize( NULL ) );

	//�E�B���h�E�N���X�o�^
	WNDCLASSEX wcex = { sizeof(wcex) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;//�����E�����̃T�C�Y���ς�����Ƃ��ɂ͍ĕ`�悷��t���O��ݒ�
	wcex.lpfnWndProc = (WNDPROC)WndProc;//�f�t�H���g�̃E�B���h�E�v���V�[�W����ݒ�
	wcex.hInstance = hInstance;//�����̃C���X�^���X�n���h����ݒ�
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);//�E�B���h�E��ɂ���Ƃ��ɕ\������}�E�X�J�[�\����ݒ�
	wcex.lpszMenuName = (LPCSTR)IDR_MENU1;//���j���[���\�[�XID��ݒ肷��
	wcex.lpszClassName = szWindowClass;//�E�B���h�E�N���X����ݒ�
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);//�w�i�h��Ԃ��p�̃u���V��ݒ�
	RegisterClassEx( &wcex );//�E�B���h�E�N���X�o�^

	//�E�B���h�E�쐬
	HWND hWnd = CreateWindow( szWindowClass, //�E�B���h�E�N���X��
		szWindowClass, //�^�C�g��������
		WS_OVERLAPPEDWINDOW, //�E�B���h�E�X�^�C��
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, //�����E�B���h�E�ʒu�@CW_USEDEFAULT�͓K���ȑ傫���ɐݒ肷��
		NULL, //�e�E�B���h�E�ւ̃n���h��
		NULL, //���j���[�ւ̃n���h��
		hInstance, //�C���X�^���X�n���h��
		NULL );//WM_CREATE�C�x���g����lParam�Ƃ��ēn�����CREATESTRUCT�\���̂ւ̃|�C���^
	if( ! hWnd ) return 0;

	ShowWindow( hWnd, nCmdShow );//�E�B���h�E�������l��Ԃŕ\����Ԃ�ݒ�
	UpdateWindow( hWnd );//�E�B���h�E�̍ĕ`��𑣂�

	// ���C�� ���b�Z�[�W ���[�v
	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0 ) ) {//���݂̃��b�Z�[�W�����邩�`�F�b�N���Ď��o��
		TranslateMessage( &msg );//���b�Z�[�W��|��
		DispatchMessage( &msg );//���b�Z�[�W����E�B���h�E�v���V�[�W�����Ăяo��
	}

	if ( CoInitialized ) CoUninitialize();
	return msg.wParam;
}
