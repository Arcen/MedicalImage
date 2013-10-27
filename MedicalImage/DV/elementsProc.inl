

static fc::treeview * elementsTreeWindow = NULL;

BOOL CALLBACK DialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_INITDIALOG:
		{
			elementsTreeWindow = new fc::treeview( hDlg, IDC_ELEMENTS );
		}
		return 1;
	case WM_SIZE:
		{
			const int edge = 10;
			DWORD fwSizeType = wParam;      // resizing flag 
			WORD nWidth = LOWORD(lParam);  // width of client area 
			WORD nHeight = HIWORD(lParam); // height of client area 
			if ( fwSizeType == SIZE_MINIMIZED ) break;
			RECT cr;
			GetClientRect( hDlg, & cr );
			elementsTreeWindow->resize( edge, edge, maximum<int>( 0, cr.right - cr.left - 2 * edge ), maximum<int>( 0, cr.bottom - cr.top - 2 * edge ) );
		}
		break;
	default:
		return 0;
	}
	return 1;
}

