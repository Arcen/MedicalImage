#include "../kernel/agl.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	CoInitialize( NULL );
	agl::express e( hInstance );
	return 0;
}
