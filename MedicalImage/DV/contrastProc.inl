static HWND contrastCenter = NULL;
static HWND contrastWidth = NULL;

BOOL CALLBACK ContrastDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	case WM_INITDIALOG:
		{
			contrastCenter = GetDlgItem( hDlg, IDC_CENTER );
			contrastWidth = GetDlgItem( hDlg, IDC_WIDTH );
			char wk[1024];
			sprintf( wk, "%d", displayImageCenter );
			SetWindowText( contrastCenter, wk );
			sprintf( wk, "%d", displayImageWidth );
			SetWindowText( contrastWidth, wk );
		}
		return 1;
	case WM_COMMAND:
		switch( LOWORD(wParam) ) {
		case IDOK:
			{
				char wk[1024];
				GetWindowText( contrastCenter, wk, 1023 );
				displayImageCenter = clamp<int32>( -0x7fff, atoi( wk ), 0x8000 );
				GetWindowText( contrastWidth, wk, 1023 );
				displayImageWidth = clamp<int32>( 0, atoi( wk ), 0xffff );
			}
			EndDialog( hDlg, 1 );
			break;
		case IDCANCEL:
			EndDialog( hDlg, 0 );
			break;
		}
		break;
	default:
		return 0;
	}
	return 1;
}

