
class trackDocument
{
	static retainer<trackDocument> hidden;
public:
	array< list<link> > verticalLinks, horizontalLinks, ascentLinks, descentLinks;
	array2<flicker> flickers[4];
	array<int> wave;
	int mxwave, mnwave;
	int width, height;
	bool sourceDicom;
	point2<int> firstwave, lastwave;
	list< boundaryArea > areas;
	int waveBias;//baseIndexの位置での基本波形画像内での波のオフセット値
	string documentPath;
	array<string> imageFilenames;
	array< imageInterface< pixelLuminance<int16> > > originalImages;//原画像
	array< imageInterface< pixelLuminance<int16> > > horizontalEdgeImages, verticalEdgeImages;
	array2<int16> baseWave;//基本波形
	array<int> maxValues;//原画像内の最大値リスト
	int maxValue;//原画像内の最大値
	int minValue;//原画像内の最小値
	int currentViewImageIndex;//画像表示番号
	int baseIndex;//輪郭を書いたり，検出の原点となる位置
	int inspirationFrame, expirationFrame;
	bool show_curve;
	double pixelSize;//[mm]
	static trackDocument * get() { if ( ! hidden ) return NULL; return & hidden(); }
	static void set( trackDocument * v ) { hidden = v; }
	// pt(c,l):ハフ空間での位置
	trackDocument()
	{
		minValue = 0;
		documentPath = "";
		currentViewImageIndex = 0;//画像表示番号

		verticalLinks.release();
		horizontalLinks.release();
		ascentLinks.release();
		descentLinks.release();

		wave.release();
		mxwave = mnwave = 0;
		waveBias = 0;
		baseIndex = 0;
		inspirationFrame = 0;
		expirationFrame = 0;
		baseWave.release();//基本波形

		firstwave = point2<int>( 0, 0 );
		lastwave = point2<int>( 0, 0 );
		//curves.release();
		areas.release();
		width = height = 0;
		pixelSize = 1;
		sourceDicom = true;
		originalImages.release();//原画像
		maxValues.release();//原画像内の最大値リスト
		show_curve = true;
	}
	void operator=( const trackDocument & doc )
	{
		verticalLinks.allocate( doc.verticalLinks.size );
		horizontalLinks.allocate( doc.horizontalLinks.size );
		ascentLinks.allocate( doc.ascentLinks.size );
		descentLinks.allocate( doc.descentLinks.size );
		for( int i = 0; i < 4; ++i ) flickers[i].allocate( doc.flickers[i].w, doc.flickers[i].h );
		wave = doc.wave;
		mxwave = doc.mxwave;
		mnwave = doc.mnwave;
		width = doc.width;
		height = doc.height;
		sourceDicom = doc.sourceDicom;
		firstwave = doc.firstwave;
		lastwave = doc.lastwave;
		waveBias = doc.waveBias;
		documentPath = doc.documentPath;
		originalImages = doc.originalImages;
		horizontalEdgeImages = doc.horizontalEdgeImages;
		verticalEdgeImages = doc.verticalEdgeImages;
		baseWave = doc.baseWave;
		maxValues = doc.maxValues;
		maxValue = doc.maxValue;
		currentViewImageIndex = doc.currentViewImageIndex;
		baseIndex = doc.baseIndex;
		pixelSize = doc.pixelSize;
		baseIndex = doc.baseIndex;
		inspirationFrame = doc.inspirationFrame;
		expirationFrame = doc.expirationFrame;
		imageFilenames = doc.imageFilenames;
	}
	int sizeTime()
	{
		return originalImages.size;
	}
	void changeBaseIndex( int newIndex )
	{
		if ( baseIndex == newIndex ) return;
		if ( wave.size ) {
			int newWaveBias = wave[newIndex];
			for ( int i = 0; i < wave.size; ++i ) {
				wave[i] -= newWaveBias;
			}
			waveBias += newWaveBias;
		}
		baseIndex = newIndex;
		waveParameterUpdate( false );
	}
	void clearLinks()
	{
		for ( array< list<link> >::iterator it( verticalLinks ); it; ++it ) {
			it->release();
		}
		for ( array< list<link> >::iterator it( horizontalLinks ); it; ++it ) {
			it->release();
		}
		for ( array< list<link> >::iterator it( ascentLinks ); it; ++it ) {
			it->release();
		}
		for ( array< list<link> >::iterator it( descentLinks ); it; ++it ) {
			it->release();
		}
	}
	void clearWave()
	{
		mxwave = mnwave = 0;
		waveBias = 0;
		baseIndex = 0;
		firstwave = point2<int>( 0, 0 );
		lastwave = point2<int>( 0, 0 );
		baseWave.release();//基本波形
		wave.release();
		inspirationFrame = 0;
		expirationFrame = 0;
	}
	void clearAreas()
	{
		areas.release();
	}
	void waveParameterUpdate( bool updateInsExFrame );
	bool openRaw( const string & documentPath );
	bool openDicom( const string & documentPath );
	void initializeAfterOpen();
	bool updateBaseWaveImage();
	void saveXML( const string & filename );
	void loadXML( const string & filename, bool replace );
	void exportInformation( const string & filename );
	void project( array2<int16> & slice, array2<int16> & edge, array2<statisticsValue> & hough );
	void project( array2<int16> & slice, array2<double> & hough );
	void project( array2<int16> & slice, array3<double> & hough );
	void clear( double coefficient, double bias, double mn, double mx, array2<statisticsValue> & hough );
	void clear( double coefficient, double bias, double mn, double mx, array2<double> & hough );
	void clear( double coefficient, double bias, double mn, double mx, array3<double> & hough );
	void clearStatic( array2<statisticsValue> & hough );
	void clearStatic( array2<double> & hough );
	void clearStatic( array3<double> & hough );
	bool search( point2<int> & pt, double & value, array2<statisticsValue> & plus );
	void search( point2<int> & pt, double & value, array2<double> & houhg );
	void search( point2<int> & pt, bool & findplus, double & value, array3<double> & minus, array3<double> & plus, int z );
	void findwaveS( list<link> & links, list<point2<int> > & points, int baseposition, int type );
	void findwave( list<link> & links, list<point2<int> > & points, int baseposition, int type );
	void findwave3( list<link> & links, list<point2<int> > & points, int baseposition, int type );
	void setBaseWave();
	void analysisVertical( int type );
	void analysisHorizontal( int type );
	void analysisDescent( int type );
	void analysisAscent( int type );
	point2<int> interpolate( int t, const point2<int> & p );
	point2<int> interpolateGotoh( int t, const point2<int> & p );
	point2<int> interpolateNear( int t, const point2<int> & p );
	point2<int> interpolateNearest( int t, const point2<int> & p );
	point2<int> interpolateVerticalEdge( int t, const point2<int> & now, const point2<int> & prev, const point2<int> & next );
	point2<double> coefficientVerticalEdge( const point2<int> & now, const point2<int> & prev, const point2<int> & next );
	double constancy( link & lnk );
	void linksort( list<link> & links );
	void flickerSetup( array2<flicker> & fs, int type, array< list<link> > & arraylink );
	void analysisFlicker( int direction );
	point2<double> analysis( const point2<int> & p );
	point2<double> analysisBearings( const point2<int> & p );
	point2<double> analysisNear( const point2<int> & p );
	void againAnalysis();
	void analysisDoC( const char * correct, const char * output );
	double analysisDoC( trackDocument & cdoc, retainer<file> & f );

	void lineimage( const list< point2<int> > & points, array2<int16> & img );
};
