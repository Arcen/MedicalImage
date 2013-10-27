//É}ÉEÉXÇ≈ÇÃèàóù
class mouseInterface
{
public:
	static void leftPush( HWND hWnd, bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
		int mouseX, int mouseY, int mouseOldX, int mouseOldY );
	static void leftPop( HWND hWnd, bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
		int mouseX, int mouseY, int mouseOldX, int mouseOldY );
	static void rightPush( HWND hWnd, bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
		int mouseX, int mouseY, int mouseOldX, int mouseOldY );
	static void rightPop( HWND hWnd, bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
		int mouseX, int mouseY, int mouseOldX, int mouseOldY );
	static void move( HWND hWnd, bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
		int mouseX, int mouseY, int mouseOldX, int mouseOldY );
};
