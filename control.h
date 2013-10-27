//�펞�\���p�R���g���[���p�l���C���^�t�F�[�X

class controlInterface : public fc::dialog
{
public:
	fc::tab tab;
	//�\���V�[�g
	fc::sheet view;
	fc::combobox viewmode, viewside;
	fc::button viewupdate, view3d;
	editboxSpinSlider viewslice, viewsize, viewlevel;
	fc::editbox editctattribute;
	fc::button changectattribute;
	bool viewing3d;
	//�̈�V�[�g
	fc::sheet regions;
	fc::combobox target;
	int targeted;
	fc::button regionPreview, regionAll;
	fc::combobox regionOperationMode;
	//臒l�ɂ��̈撊�o
	fc::button extract, extractStop;
	editboxSpinSliderLimit extractLimit;
	bool extracting;
	int16 lastExtractDown, lastExtractUp;
	//�g��k���ɂ��̈�C��
	fc::button modify, modifyStop;
	editboxSpin modifyClosing, modifyOpening;
	bool modifing;
	int lastModifyClosing, lastModifyOpening;
	//�u���V�ɂ��C��
	editboxSpinSlider brushes;
	fc::button brushEnable;
	//�����V�[�g
	fc::sheet attribute;
	fc::treeview cluster;
	array<fc::treeview::node *> clusters;
	fc::button clusterColor, clusterAlpha;
	fc::editbox clusterArea, clusterVolume, clusterAverage, clusterStandardDeviation;
	fc::button patternDelete, patternChange, patternAllDelete;
	fc::editbox patternComment;

	controlInterface( HWND _parent )
		: fc::dialog( _parent, IDD_CONTROL, dlgProc ), view( tab ), regions( tab ), attribute( tab ), 
		extracting( false ), modifing( false ), viewing3d( false )
	{
		create();
	}
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static void clicked( fc::window * );
	static void changeMode( fc::window * );
	static void sheetChange( fc::window * );
	static void painter( fc::window * );
	void validateControl();
	void doExtract( bool execute );
	void doModify( bool execute );
	void prevSlice();
	void nextSlice();
	void changeRegion( int index );
	void changeViewMode( int src );
	void changeViewSide( int src );
	void releaseClusters( bool changeClusterSet );
	void updateClusters( bool changeClusterSet = false );
};
extern controlInterface * controlWindow;
