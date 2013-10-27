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
imageInterface< pixelLuminance<int16> > dicomImage;
image displayImage;

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
//Save用ファイル選択ダイアログ
//戻り値がfalseの時はキャンセル、trueは成功
//filename:選択したファイルの名称を保存するバッファ 最低MAX_PATH分のメモリが必要
//defaultName:最初に表示されるデフォルト名称
//mask:選択ウィンドウでマスク処理を行うための文字列。表示名\0マスク\0　と続き、\0が２連続するところまでリストになる。マスクは*.bmpなどワイルドカードが使用できる。
//title:タイトルバーの文字列
//parent:親ウィンドウのハンドル
//defext:デフォルト拡張子
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
	//パス選択
	BROWSEINFO bi = { NULL };
	bi.hwndOwner = parent;
	bi.pszDisplayName = filename;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
	//デフォルトのディレクトリ
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

	imageWindow->show( true );

	UpdateWindow( imageWindow->hwnd );//ウィンドウの再描画を促す

	// メイン メッセージ ループ
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
