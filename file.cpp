#include "main.h"

ctSlices * slices = NULL;

void openDocument()
{
	closeDocument();

	slices = new ctSlices();

	char slice_data_filename[MAX_PATH];
	if ( ! windows::fileOpen( slice_data_filename, "slice.data", "Slice Data(slice.data)\0slice.data\0", "Open CT File", NULL ) ) return;
	char drive[MAX_PATH], path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( slice_data_filename, drive, path, filename, extension );
	char slice_header_filename[MAX_PATH];
	_makepath( slice_header_filename, drive, path, filename, ".hdr" );
	if ( ! file::exist( slice_header_filename ) ) {
		if ( ! windows::fileOpen( slice_header_filename, "slice.hdr", "Slice Header(slice.hdr)\0slice.hdr\0", "Open CT Header", NULL ) ) return;
	}
	slices->initialize( slice_data_filename, slice_header_filename );
	imageWindow->invalidate();
}

void closeDocument()
{
	if ( slices ) {
		slices->finalize();
		delete slices;
		slices = NULL;
		imageWindow->invalidate();
	}
}

void paintDocument()
{
	HWND hwnd = imageWindow->hwnd;
	if ( ! slices || ! slices->slices.size ) {
		HDC hdc = GetDC( hwnd );
		RECT r;
		GetClientRect( hwnd, & r );
		BitBlt( hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, NULL, 0, 0, WHITENESS );
		ReleaseDC( hwnd, hdc );
		imageWindow->validate();
		return;
	}
	HDC hdc = GetDC( hwnd );
	RECT r;
	GetClientRect( hwnd, & r );
	ctSlice & slice = slices->slices[slices->index];
	StretchBlt( hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, 
		slice.surface.hdc, 0, 0, slice.img.width, slice.img.height, SRCCOPY );
	ReleaseDC( hwnd, hdc );
	imageWindow->validate();
	return;
}

class ctWindowInterface : public fc::dialog
{
public:
	int size, level;
	fc::editbox * sizeEdit, * levelEdit;
	fc::spin * sizeSpin, * levelSpin;
	fc::slider * sizeSlider, * levelSlider;
	fc::button * updateButton;
	
	ctWindowInterface( HWND _parent )
		: fc::dialog( _parent, IDD_WINDOW, dlgProc ), 
		sizeEdit( NULL ), levelEdit( NULL ),
		sizeSpin( NULL ), levelSpin( NULL ),
		sizeSlider( NULL ), levelSlider( NULL ),
		updateButton( NULL )
	{
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static int _size, _level;
		static ctWindowInterface * _this = NULL;
		if ( _this ) fc::base::processAll( hDlg, uMsg, wParam, lParam );
		switch( uMsg ) {
		case WM_INITDIALOG:
			{
				_this = reinterpret_cast<ctWindowInterface*>( lParam );
				_this->hwnd = hDlg;

				_size = _this->size;
				_level = _this->level;

				_this->sizeEdit = new fc::editbox( hDlg, IDC_EDIT_SIZE );
				_this->levelEdit = new fc::editbox( hDlg, IDC_EDIT_LEVEL );

				_this->sizeSpin = new fc::spin( hDlg, sizeSpinID );
				_this->levelSpin = new fc::spin( hDlg, levelSpinID );
				_this->sizeSpin->edit = _this->sizeEdit;
				_this->levelSpin->edit = _this->levelEdit;
				_this->sizeSpin->create();
				_this->levelSpin->create();
				_this->sizeSpin->set( 1, 4096 );
				_this->levelSpin->set( -2048, 2048 );

				_this->sizeSlider = new fc::slider( hDlg, IDC_SLIDER_SIZE );
				_this->levelSlider = new fc::slider( hDlg, IDC_SLIDER_LEVEL );
				_this->sizeSlider->edit = _this->sizeEdit;
				_this->levelSlider->edit = _this->levelEdit;

				_this->sizeSlider->set( 1, 4096 );
				_this->levelSlider->set( -2048, 2048 );
				_this->sizeSlider->set( _this->size );
				_this->levelSlider->set( _this->level );

				_this->updateButton = new fc::button( hDlg, IDC_UPDATE );

				ShowWindow( hDlg, SW_SHOW );
				BringWindowToTop( hDlg );
			}
			return 1;
		case WM_COMMAND:
			{
 				switch(LOWORD(wParam))
				{
				case IDOK:
					_this->size = _this->sizeSlider->index;
					_this->level = _this->levelSlider->index;
				case IDCANCEL:
					if ( _this ) {
						delete _this->sizeSpin; _this->sizeSpin = NULL;
						delete _this->levelSpin; _this->levelSpin = NULL;
						delete _this->sizeEdit; _this->sizeEdit = NULL;
						delete _this->levelEdit; _this->levelEdit = NULL;
						delete _this->sizeSlider; _this->sizeSlider = NULL;
						delete _this->levelSlider; _this->levelSlider = NULL;
						delete _this->updateButton; _this->updateButton = NULL;
						_this = NULL;
					}
					EndDialog( hDlg, LOWORD(wParam) == IDOK ? 1 : 0 );
					break;
				}
			}
			break;
		}
		if ( _this && _this->updateButton && _this->updateButton->get() ) {
			if ( _size != _this->sizeSlider->index || _level != _this->levelSlider->index ) {
				_size = _this->sizeSlider->index;
				_level = _this->levelSlider->index;
				ctSlice & slice = slices->slices[slices->index];
				slice.update( _size, _level );
				imageWindow->invalidate();
			}
		}
		return 0;
	}
};

void windowDocument()
{
	if ( ! slices ) return;
	ctWindowInterface dialog( mainWindow->hwnd );
	dialog.size = slices->windowSize;
	dialog.level = slices->windowLevel;
	if ( dialog.call() ) {
		slices->windowSize = dialog.size;
		slices->windowLevel = dialog.level;
		slices->update();
	} else {
		ctSlice & slice = slices->slices[slices->index];
		slice.update( slices->windowSize, slices->windowLevel );
	}
	imageWindow->invalidate();
}
