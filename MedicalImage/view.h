//��p�摜�\���f�[�^�N���X

class miView
{
public:
	static miView * instance;
	static void set( miView * _instance ){ instance = _instance; }
	static miView * get(){ return instance; }
	array<ctSliceView> slices;
	teacherPatternView patterns;
	int16 windowSize, windowLevel;
	int viewmode, viewside;
	int16 currentCTValue;
	int times;
#ifdef VIEW3D
	//�R�����\���p�f�[�^
	sgGraph * graph;
	sgCamera * camera;
	sgOrtho * ortho;
	sgPerspective * perspective;
	list<sgNode*> lights;
#endif	
	miView() : windowSize( 1024 ), windowLevel( -400 ), viewmode( viewModeOriginal ), viewside( viewSideBoth ), currentCTValue( -2048 ),
		times( 1 )
#ifdef VIEW3D
		, graph( NULL )
#endif	
	{
		string s;
		if ( ::regGet( "windowSize", s ) ) windowSize = s.toi();
		if ( ::regGet( "windowLevel", s ) ) windowLevel = s.toi();
	}
	~miView()
	{
		::regSet( "windowSize", string( int( windowSize ) ) );
		::regSet( "windowLevel", string( int( windowLevel ) ) );
	}
	void mode( int16 windowSize, int16 windowLevel, int viewmode, int viewside );
	void notify();//���[�h�̒ʒm
	void update();
	void paint();
	void initialize( miDocument * doc );
	void finalize( miDocument * doc );
	static void statusUpdate();
	void patternUpdate();
	static void changeTimes( int t );
};
