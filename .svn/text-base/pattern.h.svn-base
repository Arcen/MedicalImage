//教師パターン

class miPattern
{
public:
	ctImage ctimg;
	ctRegion shape;
	bool regionChange;
	int16 minimumCT, maximumCT;
	decimal area;//これがあるかどうかで標本があるかどうかを判別する
	decimal average, variance, standardDeviation;
	array<int16> sample;//テンプレートの値
	array<decimal> moment[3], cumulativeMoment[3];//モーメント（ヒストグラム・平均値・２次モーメント）
	array<decimal> histogram_table;//各画素用の尤度テーブル
	array<decimal> histogram_table_normalized;//各画素用の尤度テーブル
	int16 maximumValue, minimumValue;
	gaussianEstimate * ge;
	checkMinimumMaximum<int> checkCT;
	checkMinimumMaximum<int> checkArea;
	checkMinimumMaximum<decimal> checkCircular;
	checkMinimumMaximum<decimal> checkMargin;
	checkMinimumMaximum<decimal> checkOutlineWidth;
	array2<pixelInfo> info;
	array<int16> gradient_sample;//テンプレートの値
	array<decimal> gradient_table, gradient_table_normalized;
	string comment, postfix;
	int topbottom, frontrear;
	bool enable;
	miPattern() 
		: regionChange( true ), area( 0 ), average( 0 ), variance( 0 ), 
		standardDeviation( 0 ), maximumValue( 2048 ), minimumValue( -2048 ),
		ge( & ::ge ), topbottom( lungMiddle ), frontrear( lungMiddle ), enable( true )
	{
	}
	~miPattern()
	{
		finalize();
	}
	miPattern & operator=( const miPattern & value )
	{
		assertInstance( "miPattern::operator=", __FILE__, __LINE__ );
		ctimg = value.ctimg;
		shape = value.shape;
		info.release();
		regionChange = true;
		return *this;
	}
	bool initialize( const char * targetDrive, const char * targetDirectory, const char * name, const char * postfix = NULL );
	bool initialize( ctSlice & slice, const ctRegion & rgn );
	bool finalize( const char * targetDrive, const char * targetDirectory, const char * name );
	static bool remove( const char * targetDrive, const char * targetDirectory, const char * name );
	bool finalize()
	{
		shape.initialize();
		ctimg.finalize();
		info.release();
		return true;
	}
	void update( int16 windowSize, int16 windowLevel );
	//検査する標本とガウス相関関数より、自己ガウス相関の２乗根倍の正規化ガウス相関を計算
	decimal estimate( const array<int16> & sample, bool normalized )
	{
		if ( normalized ) return ge->estimate( sample, histogram_table_normalized );
		return ge->estimate( sample, histogram_table );
	}
	decimal estimateGradient( const array<int16> & sample, bool normalized )
	{
		if ( normalized ) return ge->estimate( sample, gradient_table_normalized );
		return ge->estimate( sample, gradient_table );
	}
	decimal self( const array<int16> & sample )
	{
		return ge->self( sample );
	}
	void initializeParameter();
	static decimal calcSmoothDifferene( const array<decimal> & ch1, const array<decimal> & ch2, int16 minimumValue, int16 maximumValue )
	{
		decimal result = 0;
		for ( int16 i = minimumValue; i <= maximumValue; ++i ) {
			const decimal & v1 = ch1[i-minimumValue];
			const decimal & v2 = ch2[i-minimumValue];
			result += fabs( v1 - v2 );
		}
		return result;
	}
	static decimal calcKolmogorovSmirnov( const array<decimal> & ch1, const array<decimal> & ch2, int16 minimumValue, int16 maximumValue )
	{
		decimal result = 0;
		for ( int16 i = minimumValue; i <= maximumValue; ++i ) {
			const decimal & v1 = ch1[i-minimumValue];
			const decimal & v2 = ch2[i-minimumValue];
			result = maximum<decimal>( result, fabs( v1 - v2 ) );
		}
		return result;
	}
	//ソートされているサンプルから計算
	static decimal calcSmoothDiffereneSample( const array<int16> & ch1, const array<int16> & ch2, int16 minimumValue, int16 maximumValue )
	{
		decimal result = 0;
		decimal total1 = 0;
		decimal total2 = 0;
		array<int16>::iterator it1( ch1 );
		array<int16>::iterator it2( ch2 );
		for ( int16 i = minimumValue; i < maximumValue; ) {
			const int16 next = minimum<int16>( it1 ? it1() : maximumValue, it2 ? it2() : maximumValue );
			if ( i < next ) {
				result += fabs( total1 - total2 ) * ( next - i );
			}
			while ( it1 && it1() == next ) ++it1;
			while ( it2 && it2() == next ) ++it2;
			total1 = it1 ? ch1.index( it1 ) / decimal( ch1.size ) : 1;
			total2 = it2 ? ch2.index( it2 ) / decimal( ch2.size ) : 1;
			i = next;
		}
		return result;
	}
	static decimal calcKolmogorovSmirnovSample( const array<int16> & ch1, const array<int16> & ch2, int16 minimumValue, int16 maximumValue )
	{
		decimal result = 0;
		const decimal d1 = 1.0 / ch1.size;
		const decimal d2 = 1.0 / ch2.size;
		decimal total1 = 0;
		decimal total2 = 0;
		array<int16>::iterator it1( ch1 );
		array<int16>::iterator it2( ch2 );
		for ( int16 i = minimumValue; i < maximumValue; ) {
			const int16 next = minimum<int16>( it1 ? it1() : maximumValue, it2 ? it2() : maximumValue );
			if ( i < next ) {
				result = maximum<decimal>( result, fabs( total1 - total2 ) );
			}
			while ( it1 && it1() == next ) {
				total1 = minimum<decimal>( total1 + d1, 1 );
				++it1;
			}
			while ( it2 && it2() == next ) {
				total2 = minimum<decimal>( total2 + d2, 1 );
				++it2;
			}
			i = next;
		}
		return result;
	}
	decimal & getHistogram( int16 ct )
	{
		return moment[0][ct-minimumValue];
	}
	decimal & getCumulativeHistogram( int16 ct )
	{
		return cumulativeMoment[0][ct-minimumValue];
	}
	decimal & getAverages( int16 ct )
	{
		return moment[1][ct-minimumValue];
	}
	decimal & getCumulativeAverages( int16 ct )
	{
		return cumulativeMoment[1][ct-minimumValue];
	}
	decimal & get2Moment( int16 ct )
	{
		return moment[2][ct-minimumValue];
	}
	decimal & getCumulative2Moment( int16 ct )
	{
		return cumulativeMoment[2][ct-minimumValue];
	}
	decimal getPercent( int16 startCT, int16 endCT )
	{
		return getCumulativeHistogram( endCT ) - getCumulativeHistogram( startCT );
	}
	decimal regionVariance( const ctRegion & rgn, decimal average );
	decimal regionMedian( const ctRegion & rgn );
	decimal regionAverage( const ctRegion & rgn );
	decimal discriminantAnalysis();
	void parameterAnalysis();
	static void sampleGradient( const array2<pixelInfo> & info, const ctRegion & rgn, array<int16> & sample )
	{
		sample.reserve( rgn.area() );
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			sample.push_back( clamp<int16>( 0, info( it->x, it->y ).gnorm, 2048 ) );
		}
	}
	void calcGradientInfo()
	{
		if ( info.size ) return;
		analyzePixelInformation( ctimg, info );
		sampleGradient( info, shape, gradient_sample );
		ge->buildup( gradient_sample, gradient_table );
		gradient_table_normalized = gradient_table;
		ge->normalized( gradient_sample, gradient_table_normalized );
	}
};


class teacherPatternView
{
	image img;
public:
	int16 windowSize, windowLevel;
	int viewmode;//表示モード
	miPattern * pattern;
	pixel * color;
	bool valid;
	teacherPatternView() : windowSize( 0 ), windowLevel( 0 ), pattern( NULL ), color( NULL ), valid( false ), viewmode( -1 )
	{
	}
	~teacherPatternView()
	{
		finalize();
	}
	teacherPatternView & operator=( const teacherPatternView & value )
	{
		assertInstance( "teacherPatternView::operator=", __FILE__, __LINE__ );
		return *this;
	}
	void initialize( miPattern * pattern, pixel * color );
	bool finalize();
	void mode( int16 windowSize, int16 windowLevel, int viewmode );
	void update();
	image & getImage();
};

