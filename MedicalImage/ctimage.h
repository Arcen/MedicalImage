#include "activate.h"
////////////////////////////////////////////////////////////////////////////////
//CT画像

typedef regionImageInterface< pixelLuminance<int16>, int16 > ctImage;
//スライス情報
class ctSlice
{
	array<ctRegion> _regions;//左右両方の領域
	array<ctRegion> _left;//左側の領域
	array<ctRegion> _right;//右側の領域
public:
	enum {
		bodyBoneBound = 200,//体と骨の境界
		bodyLungBound = -200,//体と肺の境界
		imageSourceNone = 0,
		imageSourceSliceData,
		imageSourceDicom,
	};
	double position;//撮影位置
    critical_section cs;//スライス画像用のクリティカルセクション.
private:
	ctImage rimg;//スライス画像(メモリが足りなければ開放される)
public:
	bool active;//メモリマネージャ用フラグ.
	int imageSource;//読み込み位置情報
	string imageSourcePath;
	int imageSourceOffset;
	point2<int16> imageSize;
	bool airCalibrate, waterCalibrate;
	int16 air, water;
	bool regionChange;//変更確認フラグ
	//公開用
	const ctRegion * const & regions;//左右両方の領域
	const ctRegion * const & left;//左側の領域
	const ctRegion * const & right;//右側の領域
	//面積情報
	array<decimal> clusterArea;
	array<decimal> clusterAreaLeft;
	array<decimal> clusterAreaRight;

	ctSlice() : regionChange( true ), position( 0 ), 
		regions( _regions.data ), left( _left.data ), right( _right.data ), 
		imageSource( imageSourceNone ), active( false ),
		airCalibrate( false ), waterCalibrate( false ), air( -1000 ), water( 0 )
	{
	}
	~ctSlice()
	{
		finalize();
	}
	//代入・コピーコンストラクタの禁止（実行時）
	ctSlice & operator=( const ctSlice & value )
	{
		assertInstance( "ctSlice::operator=", __FILE__, __LINE__ );
		return *this;
	}
	//初期化
	bool initialize();
	bool initialize( const char * filename );//DICOM
	bool initialize( const char * filename, int offset, int width, int height, double position );//SLICE.DATA
private:
	bool reload();
public:
	//終了化
	bool finalize();
	ctImage & getImage()
	{
		if ( ! active ) reload();
		return rimg;
	}
	void getImage( imageInterface< pixelLuminance<int16> > & img )
	{
		ctImage & ctimg = getImage();
		convert( ctimg, img );
	}
	void getImage( ctImage & img )
	{
		ctImage & ctimg = getImage();
		img.create( ctimg.width, ctimg.height );
		img.fill( pixelLuminance<int16>( 0 ) );
		for ( ctImage::iterator it( ctimg ); it; ++it ) {
			img.setInternal( it.x(), it.y(), ctimg.getInternal( it() ) );
		}
	}
	static void inactivate();
	static void convert( const ctImage & rimg, imageInterface< pixelLuminance<int16> > & img )
	{
		img.create( rimg.width, rimg.height );
		img.fill( pixelLuminance<int16>( -2048 ) );
		for ( ctImage::iterator it( rimg ); it; ++it ) {
			img.setInternal( it.x(), it.y(), rimg.getInternal( it() ) );
		}
	}
	//２次微分画像設定
	static void set2differentiation( imageInterface< pixelLuminance<int16> > & laplacian, const imageInterface< pixelLuminance<int16> > & ctimg )
	{
		pixelLuminance<int16> p;
		p.y = 0;
		laplacian.create( ctimg.width, ctimg.height );
		for ( int y = 0; y < ctimg.height; ++y ) {
			for ( int x = 0; x < ctimg.width; x += ctimg.width - 1 ) {
				laplacian.setInternal( x, y, p );
			}
		}
		for ( int y = 0; y < ctimg.height; y += ctimg.height - 1 ) {
			for ( int x = 1; x < ctimg.width - 1; ++x ) {
				laplacian.setInternal( x, y, p );
			}
		}
		for ( int y = 1; y < ctimg.height - 1; ++y ) {
			for ( int x = 1; x < ctimg.width - 1; ++x ) {
				p.y = clamp<int16>( -4096, ctimg.getInternal( x, y ).y - ( 
					ctimg.getInternal( x - 1, y ).y + ctimg.getInternal( x + 1, y ).y + 
					ctimg.getInternal( x, y - 1 ).y + ctimg.getInternal( x, y + 1 ).y + 
					ctimg.getInternal( x - 1, y - 1 ).y + ctimg.getInternal( x + 1, y - 1 ).y + 
					ctimg.getInternal( x - 1, y + 1 ).y + ctimg.getInternal( x + 1, y + 1 ).y ) / 8, 4096 );
				laplacian.setInternal( x, y, p );
			}
		}
	}
	//上下の閾値から領域を獲得
	const ctRegion ct2region( int minimum, int maximum )
	{
		ctImage & ctimg = getImage();
		ctRegion wk;
		wk.reserve( ctimg.shape.area() );
		for ( ctRegion::point_enumrator it( ctimg.shape ); it; ++it ) {
			int16 x = it->x, y = it->y;
			if ( between<int16>( minimum, ctimg.getInternal( x, y ).y, maximum ) ) {
				wk.push_back( ctRegion::rectangle::pixel( x, y ) );
			}
		}
		wk.connect();
		return wk;
	}
	//医学的知識に基づく人体領域抽出
	//・含気領域以外の高吸収領域で人体領域が閉じている
	//・隅には人体領域はない
	void searchHuman();
	//閾値により変更
	void search( int index, int minimum, int maximum, int mode )
	{
		if ( index == regionHuman ) return;//人体は無効
		// down ～ upの領域を見つける
		ctRegion rgn = ct2region( minimum, maximum );
		// 肉体領域内にあるもののみにする
		if ( index == regionLung ) {
			//中の穴は埋める
			( regions[regionHuman] & rgn ).fill( rgn );
			changeRegion( regionLung, rgn );
		} else {
			switch ( mode ) {
			case regionOperationModeAppend: appendRegion( index, rgn ); break;
			case regionOperationModeDelete: removeRegion( index, rgn ); break;
			case regionOperationModeChange: changeRegion( index, rgn ); break;
			}
		}
	}
	//マスマティカルモルフォロジ処理で変更
	void modify( int index, int closing, int opening )
	{
		if ( index == regionHuman ) return;
		ctRegion wk, rgn;
		regions[index].closing( wk, closing );
		wk.opening( rgn, opening );
		changeRegion( index, rgn );
	}
	//指定領域内のヒストグラムを取得
	void getHistogram( const ctRegion & rgn, array<decimal> & histogram )
	{
		histogram.allocate( 4097 );
		memset( histogram.data, 0, histogram.size * sizeof( decimal ) );
		decimal d = 1 / decimal( rgn.area() );
		ctImage & ctimg = getImage();
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			int address = ctimg.address( it->x, it->y );
			if ( address < 0 ) continue;
			histogram[ctimg.getInternal( address ).y+2048] += d;
		}
	}
	//指定領域内の画素群を取得
	void getSample( const ctRegion & rgn, array<int16> & sample )
	{
		ctImage & ctimg = getImage();
		getSample( ctimg, rgn, sample );
	}
	//指定画像で指定領域内の画素群を取得
	static void getSample( const ctImage & ctimg, const ctRegion & rgn, array<int16> & sample )
	{
		int area = int( rgn.area() );
		sample.reserve( area );
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			int address = ctimg.address( it->x, it->y );
			if ( address < 0 ) continue;
			sample.push_back( ctimg.getInternal( address ).y );
		}
	}
private:
	//左右領域調整（デフォルトは全領域）
	void adjustSide( int index = -1 );
public:
	//領域調整
	void adjustRegion( int priority = -1 );
	//削除する
	void removeRegion( int index, const ctRegion & rgn );
	//追加する
	void appendRegion( int index, const ctRegion & rgn );
	//変更する
	void changeRegion( int index, const ctRegion & rgn );
	//調整無しに設定する
	void setRegion( int index, const ctRegion & rgn );
	//キャリブレーションの取り消し
	void uncalibrate();
	//キャリブレーションの値設定
	void calibrate( int16 air, int16 water, bool airCalibrate, bool waterCalibrate );
	//キャリブレーションの実行
	void calibrate();
	static bool save( const imageInterface< pixelLuminance<int16> > & ctimg, const char * filename );
	static bool load( imageInterface< pixelLuminance<int16> > & ctimg, const char * filename );
	double getAverage( int index )
	{
		ctImage & img = getImage();
		const ctRegion & rgn = _regions[index];
		double area = rgn.area();
		if ( ! area ) return 0;
		double average = 0;
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			if ( ! img.in( it->x, it->y ) ) continue;
			const double ctv = double( img.get( it->x, it->y ).y );
			average += ctv;
		}
		return average / area;
	}
	double getAverageSquare( int index )
	{
		ctImage & img = getImage();
		const ctRegion & rgn = _regions[index];
		double area = rgn.area();
		if ( ! area ) return 0;
		double average2 = 0;
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			if ( ! img.in( it->x, it->y ) ) continue;
			const double ctv = double( img.get( it->x, it->y ).y );
			average2 += ctv * ctv;
		}
		return average2 / area;
	}
	double getStandardDeviation( int index )
	{
		ctImage & img = getImage();
		const ctRegion & rgn = _regions[index];
		double area = rgn.area();
		if ( ! area ) return 0;
		double average = getAverage(index);
		double average2 = getAverageSquare(index);
		double variance = fabs( average2 - average * average );
        return ::sqrt( variance );
	}
};

//複数のスライス
class ctSlices
{
public:
	array<ctSlice> slices;//各スライスデータ
	double fieldOfViewMM;//画像の大きさが何mmか
	//各領域の体積
	array<decimal> clusterVolume;
	array<decimal> clusterVolumeLeft;
	array<decimal> clusterVolumeRight;
	//現在のスライス
	int index;
	//最後に設定したキャリブレーション情報
	bool airValid, waterValid;
	int16 air, water;
	int numberTop, numberBottom, numberCenter;
	point2<int> centerposition;
	bool prone;//trueの場合うつ伏せ
	bool bottomFirst;//trueの場合、尾側が最初の画像になる.
	ctSlices() : fieldOfViewMM( 0 ), index( 0 ), 
		airValid( false ), waterValid( false ), air( -1000 ), water( 0 ),
		numberTop( 0 ), numberBottom( 0 ), numberCenter( 0 ), prone( false ), bottomFirst( true ),
		centerposition( 256, 256 )
	{
	}
	~ctSlices()
	{
	}
	//代入・コピーコンストラクタの禁止（実行時）
	ctSlices & operator=( const ctSlices & value )
	{
		assertInstance( "ctSlices::operator=", __FILE__, __LINE__ );
		return *this;
	}
	bool initialize( int slicesize );
	bool initialize( const char * slice, const char * header );
	bool initialize_dicom_dir( const char * dicomdir );
	bool initialize_directory( const char * dicompath );
	bool finalize()
	{
		slices.release();
		return true;
	}
	void search( int index, int down, int up, int mode, bool progress = true );
	void modify( int index, int closing, int opening, bool progress = true );
	void searchHuman( bool progress = true );
	//各スライスの面積・体積計算
	void calcVolume();
	void continuousRegion( int x, int y, int zslice, int rtype, array<ctRegion> & contRgn, bool progress = true );
	void searchLung();
	//キャリブレーション
	void calibration( int16 _air, int16 _water, bool _airCalibrate, bool _waterCalibrate )
	{
		air = _air; water = _water;
		airValid = _airCalibrate; waterValid = _waterCalibrate;
		for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
			it->calibrate( air, water, airValid, waterValid );
		}
	}
	void uncalibrate()
	{
		air = -1000; water = 0;
		airValid = false; waterValid = false;
		for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
			it->uncalibrate();
		}
	}
	class iterator
	{
	public:
		array<ctSlice>::iterator it;
		bool all;
		////////////////////////////////////////////////////////////////////////////////
		//生成・消滅
		iterator( array<ctSlice> & src, int offset, bool _all = false ) 
			: it( array<ctSlice>::iterator( src, offset ) ), all( _all ){}
		iterator( const iterator & v ) 
			: it( v.it ), all( v.all ){}
		iterator & operator=( const iterator & src )
		{
            if ( this != & src )
            {
			    it = src.it;
			    all = src.all;
            }
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator ctSlice * () { return it; }
		operator const ctSlice * () const { return it; }
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		ctSlice & operator()()				{ return it(); }
		const ctSlice & operator()() const	{ return it(); }
		////////////////////////////////////////////////////////////////////////////////
		//前へ
		iterator & operator--()
		{
			if ( ! all ) {
				while ( it ) --it;
			} else {
				--it;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		iterator & operator++()
		{
			if ( ! all ) {
				while ( it ) ++it;
			} else {
				++it;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		ctSlice & operator*() { return ( * this )(); } const ctSlice & operator*() const { return ( * this )(); }
		ctSlice * operator->() { return & ( * this )(); } const ctSlice * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
	};
	iterator createIterator( bool all )
	{
		return iterator( slices, all ? 0 : index, all );
	}
	int indexIterator( const iterator & it ) const
	{
		if ( ! it ) return -1;
		return slices.index( it.it );
	}
    int indexSlice( const ctSlice & slice ) const
    {
		return slices.index( slice );
    }
};

//スライスの表示用データ
class ctSliceView
{
#ifdef VIEW3D
	friend class miView;
	sgTexture * texture;//シーングラフ用テクスチャ
#else
	image surface;//画面表示用画像
#endif
	rectangle2<int> imageRect;
public:
	//windowSurfaceBitmap surface;//GDC用ビットマップサーフェース
	int16 windowSize, windowLevel;//表示用CTウィンドウ値
	int viewmode, viewside;//表示モードと左右モード
	int regionindex;//αを変更するために，表示中の領域の種類
	ctSlice * slice;//現在のスライス
	bool active;
	bool valid;
	ctSliceView() : viewmode( viewModeOriginal ), viewside( viewSideBoth ), regionindex( -1 ), 
		windowSize( 0 ), windowLevel( 0 ), slice( NULL ), active( false ), valid( false )
#ifdef VIEW3D
		, texture( NULL )
#endif
	{
	}
	~ctSliceView()
	{
		finalize();
	}
	//代入・コピーコンストラクタの禁止（実行時）
	ctSliceView & operator=( const ctSliceView & value )
	{
		assertInstance( "ctSliceView::operator=", __FILE__, __LINE__ );
		return *this;
	}
	bool initialize( ctSlice * slice );
	bool finalize();
	void mode( int16 windowSize, int16 windowLevel, int viewmode, int viewside, int regionindex );
	void update();
	const image & getImage();
	//対象画像に指定領域の形状を指定色で塗る。αブレンディング有効
	static void regionBlend( image & img, const ctRegion & rgn, const pixel & color )
	{
		if ( color.a == 255 ) {
			for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
				img.set( it->x, it->y, color );
			}
			return;
		}
		decimal da = ( 255 - color.a ) / 255.0;
		decimal sa = color.a / 255.0;
		pixel sc( color.r * sa, color.g * sa, color.b * sa );
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			pixel p = img.getInternal( it->x, it->y );
			p.r = ( uint8 ) clamp<int32>( 0, p.r * da + sc.r, 255 );
			p.g = ( uint8 ) clamp<int32>( 0, p.g * da + sc.g, 255 );
			p.b = ( uint8 ) clamp<int32>( 0, p.b * da + sc.b, 255 );
			img.set( it->x, it->y, p );
		}
	}
	//対象画像に指定領域の形状を指定αに設定。
	static void setAlpha( image & img, const ctRegion & rgn, const uint8 alpha )
	{
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			pixel p = img.getInternal( it->x, it->y );
			p.a = alpha;
			img.setInternal( it->x, it->y, p );
		}
	}
	//黒をより透過させる
	static void setAlpha( image & img, const ctRegion & rgn )
	{
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			pixel p = img.getInternal( it->x, it->y );
			p.a = static_cast<uint8>( ( static_cast<uint16>( p.r ) + p.g + p.a ) / 3.0 );
			img.setInternal( it->x, it->y, p );
		}
	}
};

