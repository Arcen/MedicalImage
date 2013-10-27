#include <windows.h>
#include <agl/kernel/agl.h>
#include <fc/fc.h>
#include "resource.h"
using namespace agl;
#include "../dicom.h"

list<string> fileslists[2];
dicom dcm;
#include "filelist.h"
#include "view.h"

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	bool CoInitialized = ( S_OK == CoInitialize( NULL ) );
	fc::window::instance = hInstance;
	fc::window::initialize();
	fc::window::registerClass( wndProc, "differ", 0, 0, 0, COLOR_MENU );
	::freehandinterface.initialize();
	//ウィンドウ作成
	mainDialog = new fc::dialog( NULL, IDD_DIALOG, DialogProc );
	mainDialog->call();
	freehandinterface.finalize();
	if ( CoInitialized ) CoUninitialize();
	return 0;
}
