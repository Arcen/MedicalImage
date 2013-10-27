//Windowsヘッダファイル
#include <windows.h>	//Windows基本ヘッダファイル

//リソースヘッダファイル
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
	// debug DICOM情報書き出し
	char drive[MAX_PATH], path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( dicomfilename, drive, path, filename, extension );
	char output[MAX_PATH];
	_makepath( output, drive, path, filename, ".xml" );
	retainer<file> f = file::initialize( dicomfilename, false, true );
	if ( ! f ) return;
	static dicom dcm;
    dcm.export_( f(), output );
}

//グローバル変数
HINSTANCE hInstance;//インスタンスハンドル（アプリケーションファイルの情報）
char szWindowClass[1024] = "DicomViewer";//ウィンドウクラス名（アプリケーション固有の名称）
fc::window * imageWindow = NULL;
fc::dialog * elementViewerWindow = NULL;
MSXML2::IXMLDOMDocumentPtr domDocument;
bool validDocument = false;
imageInterface< pixelLuminance<int16> > dicomImage;
image displayImage;
int32 displayImageCenter = 0, displayImageWidth = 2000;

//Open用ファイル選択ダイアログ
//戻り値がfalseの時はキャンセル、trueは成功
//filename:選択したファイルの名称を保存するバッファ 最低MAX_PATH分のメモリが必要
//defaultName:最初に表示されるデフォルト名称
//mask:選択ウィンドウでマスク処理を行うための文字列。表示名\0マスク\0　と続き、\0が２連続するところまでリストになる。マスクは*.bmpなどワイルドカードが使用できる。
//title:タイトルバーの文字列
//parent:親ウィンドウのハンドル
bool fileOpen( char * filename, char * defaultName, char * mask, char * title, HWND parent )
{
	//初期化
	filename[0] = '\0';
	if ( defaultName ) strcpy( filename, defaultName );

	OPENFILENAME openfilename = { sizeof( OPENFILENAME ) };
	openfilename.hwndOwner = parent;
	openfilename.hInstance = hInstance;
	openfilename.lpstrFilter = mask;
	openfilename.lpstrFile = filename;
	openfilename.nMaxFile = MAX_PATH;
	openfilename.lpstrTitle = title;
	//フラグの設定
	openfilename.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if ( ! GetOpenFileName( & openfilename ) ) return false;
	return true;
}

#include "dom2treeview.inl"
#include "dicomdir2treeview.inl"
#include "elementsProc.inl"
#include "contrastProc.inl"
#include "windowProc.inl"

//Windows32APIプログラムのmain関数にあたる物。実際はライブラリ内にmain関数がありそこから呼び出される
//hInstance:アプリケーションファイル情報
//hPrevInstance:利用しない
//lpCmdLine:コマンドラインで起動したときの情報
//nCmdShow:ファイルプロパティで設定した起動時のウィンドウの表示の種類
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	::hInstance = hInstance;
	bool CoInitialized = ( S_OK == CoInitialize( NULL ) );
	fc::window::instance = hInstance;
	fc::window::initialize();
	//ウィンドウ作成
	imageWindow = new fc::window();
	imageWindow->registerClass( WndProc, szWindowClass, IDR_MENU1, IDI_ICON1, 0, COLOR_WINDOW+1 );

	imageWindow->createRoot( szWindowClass, szWindowClass, WS_OVERLAPPEDWINDOW );
	if ( ! imageWindow->hwnd ) return 0;

	elementViewerWindow = new fc::dialog( imageWindow->hwnd, IDD_ELEMENTS, DialogProc );
	elementViewerWindow->create();
	if ( ! elementViewerWindow->hwnd ) return 0;

	imageWindow->show( true );
	elementViewerWindow->show( false );

	UpdateWindow( imageWindow->hwnd );//ウィンドウの再描画を促す

	// メイン メッセージ ループ
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
