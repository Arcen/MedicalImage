//Windowsヘッダファイル
#include <windows.h>	//Windows基本ヘッダファイル

//リソースヘッダファイル
#include "resource.h"

#include <agl/kernel/agl.h>
using namespace agl;

#include "../dicom.h"

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
const char szWindowClass[1024] = "DICOMEXPORT";//ウィンドウクラス名（アプリケーション固有の名称）

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

//ウィンドウプロシージャ（いろいろなメッセージを処理するためのコールバック関数）
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message ) {
	case WM_COMMAND:// メニュー選択イベント
		switch( LOWORD(wParam) ) {
		case IDM_OPEN://開くメニュー選択
			{
				char filename[MAX_PATH];//選択したファイル名をいれるバッファ
				//ファイル選択ダイアログでファイルを選ぶ
				if ( fileOpen( filename, "", "DICOMファイル(*)\0*\0", "DICOMファイルを開く", hWnd ) ) {
					dicomExport( filename );
				}
			}
			break;
		case IDM_EXIT://終了メニュー選択
			DestroyWindow( hWnd );//ウィンドウを破壊
			break;
		default:
			//他のメニューを選んだときにはデフォルトの動作をさせる
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
		/*
	case WM_PAINT://描画イベント
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint (hWnd, &ps);//更新領域のみのデバイスコンテキストを取得
			RECT cr;//矩形情報
			GetClientRect( hWnd, & cr );//ウィンドウの描画できる大きさを取得
			EndPaint( hWnd, &ps );
		}
		break;
		*/
	case WM_DESTROY://ウィンドウの破壊イベントが来た場合にメッセージループを抜けるようにイベントを投げる
		PostQuitMessage( 0 );
		break;
	default:
		//そのほかのイベントの時にはデフォルトの動作を行う
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
// thread safeで動かすときにはexternalBufferにint[626]の０クリアデータを渡す
// [0]初期化済みフラグ
// [他]バッファ
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

//Windows32APIプログラムのmain関数にあたる物。実際はライブラリ内にmain関数がありそこから呼び出される
//hInstance:アプリケーションファイル情報
//hPrevInstance:利用しない
//lpCmdLine:コマンドラインで起動したときの情報
//nCmdShow:ファイルプロパティで設定した起動時のウィンドウの表示の種類
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

	//ウィンドウクラス登録
	WNDCLASSEX wcex = { sizeof(wcex) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;//水平・垂直のサイズが変わったときには再描画するフラグを設定
	wcex.lpfnWndProc = (WNDPROC)WndProc;//デフォルトのウィンドウプロシージャを設定
	wcex.hInstance = hInstance;//自分のインスタンスハンドルを設定
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);//ウィンドウ上にあるときに表示するマウスカーソルを設定
	wcex.lpszMenuName = (LPCSTR)IDR_MENU1;//メニューリソースIDを設定する
	wcex.lpszClassName = szWindowClass;//ウィンドウクラス名を設定
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);//背景塗りつぶし用のブラシを設定
	RegisterClassEx( &wcex );//ウィンドウクラス登録

	//ウィンドウ作成
	HWND hWnd = CreateWindow( szWindowClass, //ウィンドウクラス名
		szWindowClass, //タイトル文字列
		WS_OVERLAPPEDWINDOW, //ウィンドウスタイル
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, //初期ウィンドウ位置　CW_USEDEFAULTは適当な大きさに設定する
		NULL, //親ウィンドウへのハンドル
		NULL, //メニューへのハンドル
		hInstance, //インスタンスハンドル
		NULL );//WM_CREATEイベント時にlParamとして渡されるCREATESTRUCT構造体へのポインタ
	if( ! hWnd ) return 0;

	ShowWindow( hWnd, nCmdShow );//ウィンドウを初期値状態で表示状態を設定
	UpdateWindow( hWnd );//ウィンドウの再描画を促す

	// メイン メッセージ ループ
	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0 ) ) {//現在のメッセージがあるかチェックして取り出す
		TranslateMessage( &msg );//メッセージを翻訳
		DispatchMessage( &msg );//メッセージからウィンドウプロシージャを呼び出す
	}

	if ( CoInitialized ) CoUninitialize();
	return msg.wParam;
}
