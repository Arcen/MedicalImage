#include "main.h"
#include "gcparam.h"
#include "calibration.h"
#include "operation.h"
//#include <omp.h>
#include "parallel_for.h"
extern int thread_count;

namespace operationCommand
{
	void calibration()
	{
		//アンドゥは不可能
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		calibrationParam dlg( doc->slices.air, doc->slices.water, doc->slices.airValid, doc->slices.waterValid, mainWindow->hwnd );
		if ( ! dlg.call() ) return;

		doc->slices.calibration( dlg.air, dlg.water, dlg.airValid, dlg.waterValid );
		doc->change();
	}
	class miCommandDivideRegionThreshhold : public miCommand
	{
	public:
		bool allSlices;
		miCommandDivideRegionThreshhold( bool allSlices )
		{
			this->allSlices = allSlices;
			m.initialize();
			m.change( allSlices );
		}
		virtual bool execute()
		{
			bool done = false;
			miDocument * doc = miDocument::get();
			if ( doc && doc->patternsInitialized() ) {
				done = doc->divideRegionThreshhold( allSlices );
			}
			return done;
		}
	};
	void divideRegionThreshhold( bool allSlices )
	{
		miCommands::get().regist( new miCommandDivideRegionThreshhold( allSlices ) );
	}
	class miCommandDivideRegionSmoothedDifference : public miCommand
	{
	public:
		bool allSlices;
		int radius;
		miCommandDivideRegionSmoothedDifference( bool allSlices, int radius )
		{
			this->allSlices = allSlices;
			this->radius = radius;
			m.initialize();
			m.change( allSlices );
		}
		virtual bool execute()
		{
			bool done = false;
			miDocument * doc = miDocument::get();
			if ( doc && doc->patternsInitialized() ) {
				done = doc->divideRegionBySmoothedDifference( allSlices, radius );
			}
			return done;
		}
	};
	void divideRegionBySmoothedDifference( bool allSlices, int radius )
	{
		miCommands::get().regist( new miCommandDivideRegionSmoothedDifference( allSlices, radius ) );
	}
	class miCommandDivideRegionKolmogorovSmirnov : public miCommand
	{
	public:
		bool allSlices;
		int radius;
		miCommandDivideRegionKolmogorovSmirnov( bool allSlices, int radius )
		{
			this->allSlices = allSlices;
			this->radius = radius;
			m.initialize();
			m.change( allSlices );
		}
		virtual bool execute()
		{
			bool done = false;
			miDocument * doc = miDocument::get();
			if ( doc && doc->patternsInitialized() ) {
				done = doc->divideRegionByKolmogorovSmirnov( allSlices, radius );
			}
			return done;
		}
	};
	void divideRegionByKolmogorovSmirnov( bool allSlices, int radius )
	{
		miCommands::get().regist( new miCommandDivideRegionKolmogorovSmirnov( allSlices, radius ) );
	}
	class miCommandDivideRegionGaussianHistogramCorrelation : public miCommand
	{
	public:
		bool allSlices;
		int start, end, step;
		bool normalized, edge;
		miCommandDivideRegionGaussianHistogramCorrelation( bool allSlices, const int start, const int end, const int step, bool normalized, bool edge )
		{
			this->allSlices = allSlices;
			this->start = start;
			this->end = end;
			this->step = step;
			this->normalized = normalized;
			this->edge = edge;
			m.initialize();
			m.change( allSlices );
		}
		virtual bool execute()
		{
			bool done = false;
			miDocument * doc = miDocument::get();
			if ( doc && doc->patternsInitialized() ) {
				array<stepParameter> radiuses( doc->numberRegion() );
				if ( step == 0 ) {
					for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
						radiuses[i].set( 4 );
					}
				} else {
					for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
						radiuses[i].set( start, end, step );
					}
				}
				done = doc->divideRegionByGaussianHistogramCorrelation( allSlices, radiuses, normalized, edge );
				doc->exportPatterns();
			}
			return done;
		}
	};
	void divideRegionByGaussianHistogramCorrelation( bool allSlices, bool dlg, bool normalized, bool edge )
	{
		if ( ! miDocument::get() ) return;
		int start = 3, end = 3, step = 1;
		if ( dlg ) {
			start = 1;
			end = 32;
			step = 3;
			gaussCorrelationParam gcp( start, end, step, mainWindow->hwnd );
			if ( ! gcp.call() ) return;
			start = gcp.start;
			end = gcp.end;
			step = gcp.step;
		} else {
			step = 0;
		}
		miCommands::get().regist( new miCommandDivideRegionGaussianHistogramCorrelation( allSlices, start, end, step, normalized, edge ) );
	}
	void divideRegionByCorrelationMultiParam( bool allSlices )
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//正解のrgnファイルを指定する
		char answer[MAX_PATH];
		if ( ! windows::fileOpen( answer, "regions.rgn", "Regions Data(*.rgn)\0*.rgn\0", "Open Regions File", mainWindow->hwnd ) ) return;
		for ( int i = 1; i <= 7; ++i ) {
			char wk[1024];
			sprintf( wk, "多パラメータテスト中%d", i );
			statusBar->set( wk );
			array<stepParameter> radiuses( doc->numberRegion() );
			for ( int j = regionNormal; j < doc->numberRegion(); ++j ) radiuses[j].set( i );
			if ( ! doc->divideRegionByGaussianHistogramCorrelation( allSlices, radiuses, true, false ) ) return;
			char filename[MAX_PATH];
			sprintf( filename, "SGC%d.rgn", i );
			doc->saveAsRgn( filename );
			sprintf( filename, "SGC%d.csv", i );
			doc->analysis( answer, filename );
			miCommands::get().clear();
		}
	}
	void divideRegionBySmoothedDifferenceMultiParam( bool allSlices )
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return;
		//正解のrgnファイルを指定する
		char answer[MAX_PATH];
		if ( ! windows::fileOpen( answer, "regions.rgn", "Regions Data(*.rgn)\0*.rgn\0", "Open Regions File", mainWindow->hwnd ) ) return;
		for ( int i = 1; i <= 7; ++i ) {
			char wk[1024];
			sprintf( wk, "多パラメータテスト中%d", i );
			statusBar->set( wk );
			if ( ! doc->divideRegionBySmoothedDifference( allSlices, i ) ) return;
			char filename[MAX_PATH];
			sprintf( filename, "SD%d.rgn", i );
			doc->saveAsRgn( filename );
			sprintf( filename, "SD%d.csv", i );
			doc->analysis( answer, filename );
			miCommands::get().clear();
		}
	}
	class miCommandRemoveSmallLungRegion : public miCommand
	{
	public:
		miCommandRemoveSmallLungRegion()
		{
			m.initialize();
			m.change( true );
		}
		virtual bool execute()
		{
			bool done = false;
			miDocument * doc = miDocument::get();
			if ( doc ) {
				doc->removeSmallLungRegion();
				done = true;
			}
			return done;
		}
	};
	void removeSmallLungRegion()
	{
		miCommands::get().regist( new miCommandRemoveSmallLungRegion() );
	}
	class miCommandAnalyzeLungRegion : public miCommand
	{
	public:
		miCommandAnalyzeLungRegion()
		{
			m.initialize();
			m.change( true );
		}
		virtual bool execute()
		{
			bool done = false;
			miDocument * doc = miDocument::get();
			if ( doc ) {
				doc->slices.searchLung();
				done = true;
			}
			return done;
		}
	};
	void analyzeLungRegion()
	{
		miCommands::get().regist( new miCommandAnalyzeLungRegion() );
	}
};
//小さい肺野領域除去
void miDocument::removeSmallLungRegion()
{
	statusBar->reset();
    int first = sliceindex;//0が底
    int last = slicesize;
    if ( ! slices.bottomFirst ) {//0が頭.
        first = 0;
        last = sliceindex + 1;
    }
    int count = last - first;
    if ( count < 1 ) count = 1;
	statusBar->expand( count );
    for ( int i = first; i < last; ++i ) {
		ctSlice & slice = slices.slices[i];
        double left = slice.imageSize.x / 512.0 * 200;
        double right = slice.imageSize.x / 512.0 * 312;
		const ctRegion & lung = slice.regions[regionLung];
        std::list<ctRegion> parts;
		lung.divideMultiRegion( parts );
		ctRegion removed;
		for ( std::list<ctRegion>::iterator itp = parts.begin(); itp != parts.end(); ++itp ) {
			ctRegion & part = *itp;
			ctRegion::point p = part.center();
			if ( part.area() < 1000 && left < p.x && p.x < right ) {
				removed |= part;
			}
            part.release();
		}
		if ( ! removed.empty() ) {
			slice.removeRegion( regionLung, removed );
		}
		statusBar->progress();
	}
	statusBar->reset();
	change();
	imageWindow->invalidate();
}

class averageClusterIndex
{
public:
	int16 average;
	int index;
	miPattern * pattern;
};

//averageClusterIndexのaverageに対するqsort用関数
static int compairAverageClusterIndex( const void * v1, const void * v2 )
{
	return ( reinterpret_cast< const averageClusterIndex* >( v1 ) )->average - 
		( reinterpret_cast< const averageClusterIndex* >( v2 ) )->average;
}
//閾値による領域分割
bool miDocument::divideRegionThreshhold( bool allSlices )
{
	statusBar->reset();
	miDocument * doc = miDocument::get();
	const int numClusters = doc->patternsInitialized();
	array< averageClusterIndex > average( numClusters );
	int i = regionNormal, j = 0;//jは連番
	for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		for ( array< retainer<miPattern> >::iterator it( rit->patterns ); it; ++it ) {
			average[j].average = it()->average;
			average[j].index = i;
			average[j].pattern = & it()();
			++j;
		}
		++i;
	}
	qsort( average, numClusters, sizeof( averageClusterIndex ), compairAverageClusterIndex );
	//平均順にクラスタを並べたあと閾値を決定していく
	array<int16> threshhold;
	threshhold.reserve( numClusters+1 );
	threshhold.push_back( -2048 );
	array< averageClusterIndex > clusterIndex;
	clusterIndex.reserve( numClusters );
	for ( int i = 0; i < numClusters - 1; ++i ) {
		const int firstClusterIndex = average[i].index;
		const int nextClusterIndex = average[i+1].index;
		miPattern * firstPattern = average[i].pattern;
		miPattern * nextPattern = average[i+1].pattern;
		if ( firstClusterIndex != nextClusterIndex ) {
			checkMaximum<decimal,int16> maximumThreshhold;
			for ( int j = average[i].average; j < average[i+1].average; ++j ) {
				maximumThreshhold( firstPattern->getPercent( -2048, j ) + 
					nextPattern->getPercent( j + 1, 2048 ), j );
			}
			threshhold.push_back( maximumThreshhold ? maximumThreshhold.sub : average[i].average );
			clusterIndex.push_back( average[i] );
		}
	}
	if ( 0 < numClusters ) {
		clusterIndex.push_back( average.last() );
	}
	threshhold.push_back( 2048 );
	string wk;
	for ( int i = 0; i < clusterIndex.size; ++i ) {
		if ( i ) wk += string( " <= " );
		wk += doc->regionName( clusterIndex[i].index );
		if ( i != clusterIndex.size - 1 ) {
			wk += string( " < " );
			wk += string( threshhold[i+1] );
		}
	}
	int ret = MessageBox( mainWindow->hwnd, wk, "以下の閾値で領域分割を行いますか？", MB_OKCANCEL );
	if ( ret != IDOK ) return false;
	statusBar->expand( clusterIndex.size * ( allSlices ? slicesize : 1 ) );
	array<ctRegion> rgn( doc->numberRegion() );
	for ( ctSlices::iterator its( slices.createIterator( allSlices ) ); its; ++its ) {
		ctSlice & slice = its();
		for ( int i = 0; i < doc->numberRegion(); ++i ) {
			rgn[i].initialize();
		}
		for ( int i = 0; i < clusterIndex.size; ++i ) {
			rgn[clusterIndex[i].index] |= slice.ct2region( threshhold[i] + ( i == 0 ? 0 : 1 ), threshhold[i+1] );
			statusBar->progress();
		}
		for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
			if ( ! doc->patterns(i).size ) continue;
			slice.setRegion( i, rgn[i] );
		}
		slice.adjustRegion();
	}
	statusBar->reset();
	change();
	imageWindow->invalidate();
	return true;
}

static int compairInt16( const void * v1, const void * v2 )
{
	return *reinterpret_cast< const int16* >( v1 ) - 
		*reinterpret_cast< const int16* >( v2 );
}

//smoothed difference法による領域分割
bool miDocument::divideRegionBySmoothedDifference( bool allSlices, int nearRegionRadius )
{
	const int numClusters = patternsInitialized();
	ctRegion nearRegion;
	( ctRegion( ctRegion::rectangle::pixel( 0, 0 ) ) ).dilation( nearRegion, nearRegionRadius );
	statusBar->reset();
	if ( allSlices ) {
		for ( array<ctSlice>::iterator it( slices.slices ); it; ++it ) {
			statusBar->expand( ( it->regions[regionLung].area() + progressStepForPixel - 1 ) / progressStepForPixel );
		}
        SD_status status;//並列実行用に、パラメータに渡されるデータを保存する.
        status.doc = this;
        status.nearRegion = & nearRegion;
        parallel_for pf( mainWindow, thread_count, 0, slices.slices.size, SD_process, & status );
        pf.begin();
        pf.join();
	} else {
		statusBar->expand( ( slice().regions[regionLung].area() + progressStepForPixel - 1 ) / progressStepForPixel );
        divideRegionBySmoothedDifference( slices.slices[slices.index], nearRegion );
	}
	statusBar->reset();
	change();
	imageWindow->invalidate();
	return true;
}
//smoothed difference法によるシングルスライスの領域分割
bool miDocument::divideRegionBySmoothedDifference( ctSlice & slice, const ctRegion & nearRegion )
{
    check_critical_section ccs( slice.cs );//スライスロック.
	const ctRegion & lung = slice.regions[regionLung];
	ctImage & ctimg = slice.getImage();
	int w = ctimg.width, h = ctimg.height;
    std::vector<char> lungRegion;
	lung.get( lungRegion, w, h, 0, 0 );
	int lungarea = lung.area();
    const int numRegion = numberRegion();//領域数.
	array<ctRegion> regions( numRegion );//結果保存領域
	for ( int i = regionNormal; i < numRegion; ++i ) regions[i].reserve( lungarea );
	array<int16> samples;
	samples.reserve( nearRegion.area() );
	int count = 0;
	for ( ctRegion::point_enumrator it( lung ); it; ++it ) {
		//近傍のサンプリング
		samples.allocate( 0 );
		for ( ctRegion::point_enumrator itn( nearRegion ); itn; ++itn ) {
			const int x = it->x + itn->x, y = it->y + itn->y;

			if ( ! between( 0, x, w - 1 ) ||
				! between( 0, y, h - 1 ) ) continue;
			if ( ! lungRegion[x+w*y] ) continue;
			int address = ctimg.address( x, y );
			if ( address < 0 ) continue;
			samples.push_back( clamp<int16>( -2048, ctimg.getInternal( address ).y, 2048 ) );
		}
		//ソート
		qsort( samples.data, samples.size, sizeof( int16 ), compairInt16 );

		checkMinimum<decimal,int> minimum;
		for ( int i = regionNormal; i < numRegion; ++i ) {
			for ( array< retainer<miPattern> >::iterator it( patterns( i ) ); it; ++it ) {
				minimum( miPattern::calcSmoothDiffereneSample( samples, it()->sample, -2048, 2048 ), i );
			}
		}
		regions[minimum.sub].push_back( ctRegion::rectangle::pixel( it->x, it->y ) );
		if ( count % progressStepForPixel == 0 ) this->progress();
		++count;
	}
	for ( int i = regionNormal; i < numRegion; ++i ) {
		if ( ! patterns(i).size ) continue;
		regions[i].connect();
		slice.changeRegion( i, regions[i] );
	}
    return true;
}
//Kolmogorov Smirnov法による領域分割
bool miDocument::divideRegionByKolmogorovSmirnov( bool allSlices, int nearRegionRadius )
{
	const int numClusters = patternsInitialized();
	ctRegion nearRegion;//近傍領域.
	( ctRegion( ctRegion::rectangle::pixel( 0, 0 ) ) ).dilation( nearRegion, nearRegionRadius );
	statusBar->reset();
	if ( allSlices ) {
		for ( array<ctSlice>::iterator it( slices.slices ); it; ++it ) {
			statusBar->expand( ( it->regions[regionLung].area() + progressStepForPixel - 1 ) / progressStepForPixel );
		}
        KS_status status;//並列実行用に、パラメータに渡されるデータを保存する.
        status.doc = this;
        status.nearRegion = & nearRegion;
        parallel_for pf( mainWindow, thread_count, 0, slices.slices.size, KS_process, & status );
        pf.begin();
        pf.join();
	} else {
		statusBar->expand( ( slice().regions[regionLung].area() + progressStepForPixel - 1 ) / progressStepForPixel );
        divideRegionByKolmogorovSmirnov( slices.slices[slices.index], nearRegion );
	}
	statusBar->reset();
	change();
	imageWindow->invalidate();
	return true;
}
//Kolmogorov Smirnov法によるシングルスライスの領域分割.
bool miDocument::divideRegionByKolmogorovSmirnov( ctSlice & slice, const ctRegion & nearRegion )
{
	const int numClusters = patternsInitialized();
    check_critical_section ccs( slice.cs );//スライスロック.
	ctImage & ctimg = slice.getImage();
	const ctRegion & lung = slice.regions[regionLung];
	int w = ctimg.width, h = ctimg.height;
    std::vector<char> lungRegion;
	lung.get( lungRegion, w, h, 0, 0 );
	int lungarea = lung.area();
    const int numRegion = numberRegion();//領域数.
	array<ctRegion> regions( numRegion );//結果保存領域
	for ( int i = regionNormal; i < numRegion; ++i ) regions[i].reserve( lungarea );
	array<int16> samples;
	samples.reserve( nearRegion.area() );
	int count = 0;
	for ( ctRegion::point_enumrator it( lung ); it; ++it ) {
		//近傍のサンプリング
		samples.allocate( 0 );
		for ( ctRegion::point_enumrator itn( nearRegion ); itn; ++itn ) {
			const int x = it->x + itn->x, y = it->y + itn->y;
			if ( ! between( 0, x, w - 1 ) ||
				! between( 0, y, h - 1 ) ) continue;
			if ( ! lungRegion[x+w*y] ) continue;
			int address = ctimg.address( x, y );
			if ( address < 0 ) continue;
			samples.push_back( clamp<int16>( -2048, ctimg.getInternal( address ).y, 2048 ) );
		}
		//ソート
		qsort( samples.data, samples.size, sizeof( int16 ), compairInt16 );
		//最小値を探す.
		checkMinimum<decimal,int> minimum;
		for ( int i = regionNormal; i < numRegion; ++i ) {
			for ( array< retainer<miPattern> >::iterator it( patterns(i) ); it; ++it ) {
				minimum( miPattern::calcKolmogorovSmirnovSample( samples, it()->sample, -2048, 2048 ), i );
			}
		}
		regions[minimum.sub].push_back( ctRegion::rectangle::pixel( it->x, it->y ) );
        if ( count % progressStepForPixel == 0 ) this->progress();//並列化しながらカウントが可能.
		++count;
	}
	for ( int i = regionNormal; i < numRegion; ++i ) {
		if ( ! patterns(i).size ) continue;
		regions[i].connect();
		slice.changeRegion( i, regions[i] );
	}
	return true;
}
enum {
	topdownApexOfLung,//肺尖
	topdownUpperLobe,//上葉
	topdownMiddleLobe,//中葉
	topdownLowerLobe,//下葉
	topdownBaseOfLung,//肺底
	frontrearAnteriorSegment,//前
	frontrearMedialSegment,//内側
	frontrearPosteriorSegment,//後
};
//pair<int,int>のrightに対するqsort用関数
static int compairRadiusIndex( const void * v1, const void * v2 )
{
	int r = ( reinterpret_cast< const pair<int16,int>* >( v1 ) )->right - ( reinterpret_cast< const pair<int16,int>* >( v2 ) )->right;
	if ( r == 0 ) r = ( reinterpret_cast< const pair<int16,int>* >( v1 ) )->left - ( reinterpret_cast< const pair<int16,int>* >( v2 ) )->left;
	return r;
}
#define OPERATION_FLAG_USE_ALL_PATTERN //教師情報がまともにセットされていなくても計算されるように全ての教師画像を使用する．

//インデックスの上下を計算する.
//@param[out] topdownIndex 肺尖部、気管分岐部より上、気管分岐部、気管分岐部より下、肺底部に分類
//@param[out] topdownAlpha 気管分岐部より上、気管分岐部より下の場合、(0,1)で大きいほうが端に設定される.
//@param[out] useLungTopPattern 肺尖部教師画像を使用するかどうか（だが、未設定の場合があるので、すべて利用される）.
//@param[out] useLungCenterPattern 気管分岐部教師画像を使用するかどうか（だが、未設定の場合があるので、すべて利用される）.
//@param[out] useLungBottomPattern 肺底部教師画像を使用するかどうか（だが、未設定の場合があるので、すべて利用される）.
//@param[in] sindex 処理するスライス番号.
//@param[in] bottomFirst 若い番号が肺底部の場合true.
//@param[in] numberTop 肺尖部の境界.
//@param[in] numberCenter 気管分岐部.
//@param[in] numberBottom 肺底部の境界.
static void setIndexTopBottom( int & topdownIndex, decimal & topdownAlpha, 
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
                       bool & useLungTopPattern, bool & useLungCenterPattern, bool & useLungBottomPattern, 
#endif
					   int sindex, bool bottomFirst, int numberTop, int numberCenter, int numberBottom )
{
	topdownAlpha = 0;
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
	useLungTopPattern = useLungCenterPattern = useLungBottomPattern = false;
#endif
	if ( ( bottomFirst && numberTop <= sindex ) ||//0が尾側なら、大きいほうが頭側.
		( ! bottomFirst && sindex <= numberTop ) ) {//肺尖部
		topdownIndex = topdownApexOfLung;
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		useLungTopPattern = true;
#endif
	} else if ( ( bottomFirst && numberCenter < sindex ) ||
		( ! bottomFirst && sindex < numberCenter ) ) {//気管分岐部より上
		topdownIndex = topdownUpperLobe;
		topdownAlpha = double( sindex - numberCenter ) / ( numberTop - numberCenter );//頭に近いほうが１になる.
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		useLungTopPattern = useLungCenterPattern = true;
#endif
	} else if ( ( bottomFirst && numberCenter == sindex ) ||
		( ! bottomFirst && sindex == numberCenter ) ) {//気管分岐部
		topdownIndex = topdownMiddleLobe;
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		useLungCenterPattern = true;
#endif
	} else if ( ( bottomFirst && numberBottom < sindex ) ||
		( ! bottomFirst && sindex < numberBottom ) ) {//気管分岐部より下
		topdownIndex = topdownLowerLobe;
		topdownAlpha = double( sindex - numberCenter ) / ( numberBottom - numberCenter );//尾に近いほうが1になる.
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		useLungBottomPattern = useLungCenterPattern = true;
#endif
	} else {//肺底部
		topdownIndex = topdownBaseOfLung;
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		useLungBottomPattern = true;
#endif
	}
}
void setIndexFrontRear( int & forntrearIndex, decimal & frontrearAlpha,
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
                       bool & useLungFrontPattern, bool & useLungRearPattern, 
#endif
					   int y, int center, int top, int bottom, bool prone )
{
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
	useLungFrontPattern = useLungRearPattern = false;
#endif
	double pos = 0;
	if ( center <= y ) {//下側.
		pos = - double( y - center ) / ( bottom - 1 - center );//下側がマイナス
	} else {
		pos = double( y - center ) / ( top - center );//上側がプラス.
	}
	const double limit = 0.4;//中心０，外側１．中心からのところから外側の影響が始まる．　＊５で　２０％で完全に外側のみになる
	frontrearAlpha = clamp<double>( 0, ( abs( pos ) - limit ) * 5.0, 1 );
	if ( ( prone && pos < -limit ) ||//うつぶせで下側は腹側.
		( ! prone && limit < pos ) ) {//腹側
		forntrearIndex = frontrearAnteriorSegment;
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		useLungFrontPattern = true;
#endif
	} else if ( ( prone && pos > limit ) ||
		( ! prone && -limit > pos ) ) {//背側
		forntrearIndex = frontrearPosteriorSegment;
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		useLungRearPattern = true;
#endif
	} else {//中心 abs(pos)<limit.
		forntrearIndex = frontrearMedialSegment;
        //下側が0になるような補間.
		frontrearAlpha = clamp<double>( 0, ( pos + limit ) / 0.8, 1 );//[-.4,.4]->[0,.8]->[0,1]
		if ( ! prone ) frontrearAlpha = 1 - frontrearAlpha;//仰向けの場合、(上部)腹側が０にする
	}
}
//位置に応じて補間する.
decimal interpolate( array2< checkMaximum<decimal,int> > & st, int topdownIndex, int forntrearIndex, decimal topdownAlpha, decimal frontrearAlpha )
{
	frontrearAlpha = clamp<decimal>( 0, frontrearAlpha, 1 );//前後のアルファ値.
	topdownAlpha = clamp<decimal>( 0, topdownAlpha, 1 );//頭尾のアルファ値（端のほうが1、中心が0）.
	decimal fr[numberLungTopBottom] = { 0 };
	for ( int tb = 0; tb < numberLungTopBottom; ++tb ) {
		switch ( forntrearIndex ) {
		case frontrearAnteriorSegment:
			if ( st( tb, lungFront ) && st( tb, lungMiddle ) ) {
				fr[tb] = st( tb, lungFront )() * frontrearAlpha + 
					st( tb, lungMiddle )() * ( 1 - frontrearAlpha );
			} else if ( st( tb, lungFront ) ) {
				fr[tb] = st( tb, lungFront )();
			} else if ( st( tb, lungMiddle ) ) {
				fr[tb] = st( tb, lungMiddle )();
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
			} else if ( st( tb, lungRear ) ) {
				fr[tb] = st( tb, lungRear )();
#endif
			}
			break;
		case frontrearMedialSegment:
			if ( st( tb, lungMiddle ) ) {//中央があれば.
				fr[tb] = st( tb, lungMiddle )();
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
			} else if ( st( tb, lungFront ) && st( tb, lungRear ) ) {
				//fr[tb] = st( tb, lungFront )() * 0.5 + st( tb, lungRear )() * 0.5;
				fr[tb] = st( tb, lungFront )() * ( 1 - frontrearAlpha ) + st( tb, lungRear )() * frontrearAlpha;
			} else if ( st( tb, lungFront ) ) {
				fr[tb] = st( tb, lungFront )();
			} else if ( st( tb, lungRear ) ) {
				fr[tb] = st( tb, lungRear )();
#endif
			}
			break;
		case frontrearPosteriorSegment:
			if ( st( tb, lungRear ) && st( tb, lungMiddle ) ) {
				fr[tb] = st( tb, lungRear )() * frontrearAlpha + 
					st( tb, lungMiddle )() * ( 1 - frontrearAlpha );
			} else if ( st( tb, lungRear ) ) {
				fr[tb] = st( tb, lungRear )();
			} else if ( st( tb, lungMiddle ) ) {
				fr[tb] = st( tb, lungMiddle )();
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
			} else if ( st( tb, lungFront ) ) {
				fr[tb] = st( tb, lungFront )();
#endif
			}
			break;
		}
	}
	switch ( topdownIndex ) {
	case topdownApexOfLung:
		if ( fr[lungTop] ) return fr[lungTop];
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
		if ( fr[lungMiddle] ) {
			return fr[lungMiddle];
		} else if ( fr[lungBottom] ) {
			return fr[lungBottom];
		}
#endif
		break;
	case topdownMiddleLobe:
		if ( fr[lungMiddle] ) return fr[lungMiddle];
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
		if ( fr[lungTop] && fr[lungBottom] ) {
			return ( fr[lungTop] + fr[lungBottom] ) / 2.0;
		} else if ( fr[lungTop] ) {
			return fr[lungTop];
		} else if ( fr[lungBottom] ) {
			return fr[lungBottom];
		}
#endif
		break;
	case topdownBaseOfLung:
		if ( fr[lungBottom] ) return fr[lungBottom];
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
		if ( fr[lungMiddle] ) {
			return fr[lungMiddle];
		} else if ( fr[lungTop] ) {
			return fr[lungTop];
		}
#endif
		break;
	case topdownUpperLobe:
		if ( fr[lungTop] && fr[lungMiddle] ) {
			return fr[lungTop] * topdownAlpha + fr[lungMiddle] * ( 1 - topdownAlpha );
		} else if ( fr[lungTop] ) {
			return fr[lungTop];
		} else if ( fr[lungMiddle] ) {
			return fr[lungMiddle];
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
		} else if ( fr[lungBottom] ) {
			return fr[lungBottom];
#endif
		}
		break;
	case topdownLowerLobe:
		if ( fr[lungBottom] && fr[lungMiddle] ) {
			return fr[lungBottom] * topdownAlpha + fr[lungMiddle] * ( 1 - topdownAlpha );
		} else if ( fr[lungBottom] ) {
			return fr[lungBottom];
		} else if ( fr[lungMiddle] ) {
			return fr[lungMiddle];
#if defined(OPERATION_FLAG_USE_ALL_PATTERN)
		} else if ( fr[lungTop] ) {
			return fr[lungTop];
#endif
		}
		break;
	}
	return 0;
}
//ガウシアンヒストグラム相関による領域分割（多近傍領域形状での最尤をとる）
bool miDocument::divideRegionByGaussianHistogramCorrelation( bool allSlices, const array<stepParameter> & sp, bool normalized, bool edge )
{
	// SHIFTキーが押されているときは勾配を考慮する．
	bool considerGradient = edge;
	bool considerPatternPosition = true;

	//パターンの情報を計算（後ほど前処理に追加する）
	if ( considerGradient ) {
		for ( regionIterator rit( *this, true ); rit; ++rit ) {
			for ( array< retainer<miPattern> >::iterator it( rit->patterns ); it; ++it ) {
				it()->calcGradientInfo();
			}
		}
	}
	//使用する近傍領域半径と領域インデックスの組をリストアップ
	array< pair<int,int> > radiusIndexList;//左が領域番号、右が近傍距離.
	radiusIndexList.reserve( 1000 );
	{
		int i = regionNormal;
		for ( array<stepParameter>::iterator sit( sp, regionNormal ); sit; ++sit, ++i ) {
			for ( stepParameter::iterator it( sit() ); it; ++it ) {
				radiusIndexList.push_back( pair<int,int>( i, it() ) );
			}
		}
	}
	//近傍領域半径順にソート
	qsort( radiusIndexList.data, radiusIndexList.size, sizeof( pair<int,int> ), compairRadiusIndex );
	//近傍領域半径をリストアップ（ソート済み）
	array<int> radiusList;
	radiusList.reserve( radiusIndexList.size );
	for ( array< pair<int,int> >::iterator itp( radiusIndexList ); itp; ++itp ) {
		if ( ! radiusList.size || radiusList.last() < itp().right ) {
			radiusList.push_back( itp().right );
		}
	}
	//利用する半径分のみ用意　近傍領域形状とひとつ小さい形状からの差分領域
	array<ctRegion> nearRegion( radiusList.size ), subtractNearRegion( radiusList.size );
	array<ctRegion>::iterator itnr( nearRegion ), itsnr( subtractNearRegion );
	//近傍領域と前ステップからの差分領域作成
	const ctRegion * prevRegion = NULL;
	for ( array<int>::iterator itr( radiusList ); itr; ++itr, ++itnr, ++itsnr ) {
		( ctRegion( ctRegion::rectangle::pixel( 0, 0 ) ) ).dilation( itnr(), itr() );
		if ( ! prevRegion ) {
			itsnr() = itnr();
		} else {
			itsnr() = itnr() - *prevRegion;
		}
		prevRegion = & itnr();
	}
	const bool multiRadius = ( radiusList.size != 1 ? true : false );
	const int maxAreaNearRegion = nearRegion.last().area();
	array<int16> sample( maxAreaNearRegion ), sampleD( maxAreaNearRegion );//サンプリング用データに最大面積分用意
	//各場所での最大値を保存しておく
	array< array2< checkMaximum<decimal,int> > > similarityTable;//最大の相関とそのパターン
	similarityTable.allocate( numberRegion() );
	for ( int i = 0; i < similarityTable.size; ++i ) {
		similarityTable[i].allocate( numberLungTopBottom, numberLungFrontRear );
	}
	//ステータスバー用意
	statusBar->reset();
    if ( allSlices )
    {
		for ( array<ctSlice>::iterator it( slices.slices ); it; ++it ) {
			statusBar->expand( ( it->regions[regionLung].area() + progressStepForPixel - 1 ) / progressStepForPixel );
		}
        GHNC_status status;//並列実行用に、パラメータに渡されるデータを保存する.
        status.doc = this;
        status.normalized = normalized;
        status.considerGradient = considerGradient;
        status.radiusIndexList = & radiusIndexList;
        status.radiusList = & radiusList;
        status.nearRegion = & nearRegion;
        status.subtractNearRegion = & subtractNearRegion;
        parallel_for pf( mainWindow, thread_count, 0, slices.slices.size, GHNC_process, & status );
        pf.begin();
        pf.join();
/* OpenMPでの実装(メインスレッドを使用してしまう).
//#include <omp.h>
#pragma omp parallel
        {
#pragma omp for schedule(static,1)
            for ( i = 0; i < n; ++i )
                divideRegionByGaussianHistogramCorrelation( normalized, considerGradient, slices.slices[i], radiusIndexList, radiusList, nearRegion, subtractNearRegion );
        }
*/
    } else {
		statusBar->expand( ( slice().regions[regionLung].area() + progressStepForPixel - 1 ) / progressStepForPixel );
        divideRegionByGaussianHistogramCorrelation( normalized, considerGradient, slices.slices[slices.index], radiusIndexList, radiusList, nearRegion, subtractNearRegion );
    }
	patternsInitialize();
	statusBar->reset();
	change();
	imageWindow->invalidate();
	return true;
}

#define OPERATION_FLAG_CONSIDER_PATTERN_POSITION //教師画像の位置を利用する場合.
//1スライスのガウシアンヒストグラム正規化相関処理(並列処理可能に)
//@param[in] normalized 正規化するかどうか
//@param[in] considerGradient 勾配を考慮するかどうか
//@param[in] slice 対象スライス
//@param[in] radiusIndexList 左が領域番号、右が近傍距離のソート済みリスト
//@param[in] radiusList 近傍距離のソート済みリスト
bool miDocument::divideRegionByGaussianHistogramCorrelation( bool normalized, bool considerGradient, ctSlice & slice, const array< pair<int,int> > & radiusIndexList, const array<int> & radiusList, const array<ctRegion> & nearRegion, const array<ctRegion> & subtractNearRegion )
{
	const int sindex = slices.indexSlice( slice );//スライス番号.
    check_critical_section ccs( slice.cs );//スライスロック.
    //char debug[128];
    //sprintf( debug, "start %d\n", sindex ); ::OutputDebugString( debug );
    const ctRegion & lung = slice.regions[regionLung];//肺野形状.
	ctImage & ctimg = slice.getImage();//@todo ここでロードするなら、他で開放しないようにロックする必要がある.
	const int w = ctimg.width, h = ctimg.height;//画像のサイズ.
	int topdownIndex = 0;//垂直方向の位置.
	decimal topdownAlpha = 0;//中央の補間される領域での端の割合.
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
	bool useLungTopPattern = false, useLungCenterPattern = false, useLungBottomPattern = false;//教師画像を利用するかどうか(現在は常に利用される).
#endif
#if defined(OPERATION_FLAG_CONSIDER_PATTERN_POSITION)
	setIndexTopBottom( topdownIndex, topdownAlpha, 
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
        useLungTopPattern, useLungCenterPattern, useLungBottomPattern, 
#endif
        sindex, slices.bottomFirst, slices.numberTop, slices.numberCenter, slices.numberBottom );
#endif
	array2<pixelInfo> info;//勾配画像の生成.
	analyzePixelInformation( ctimg, info );
	const int maxAreaNearRegion = nearRegion.last().area();
	array<int16> sample( maxAreaNearRegion ), sampleD( maxAreaNearRegion );//サンプリング用データに最大面積分用意

    std::vector<char> lungRegion;
	lung.get( lungRegion, w, h, 0, 0 );//肺野形状の画像化.
	int lungarea = lung.area();//肺野画素数
    const int numRegion = numberRegion();//領域数.
	array<ctRegion> regions( numRegion );//結果領域
	for ( int i = regionNormal; i < numRegion; ++i ) {
		regions[i].reserve( lungarea );//肺野画素数で予約.
	}
	int count = 0;//プログレスバー用のカウンタ.
	ctRegion::rectangle lungrect = lung.get();//肺野の包含矩形
	array< array2< checkMaximum<decimal,int> > > similarityTable;//最大の相関とそのパターン
	similarityTable.allocate( numRegion );
	for ( int i = 0; i < similarityTable.size; ++i ) {
		similarityTable[i].allocate( numberLungTopBottom, numberLungFrontRear );
	}
	miDocument * doc = miDocument::get();
	for ( ctRegion::point_enumrator it( lung ); it; ++it ) {//肺の画素毎.
		int forntrearIndex = 0;
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
		bool useLungFrontPattern = false, useLungRearPattern = false;
#endif
		decimal frontrearAlpha = 0;
#if defined(OPERATION_FLAG_CONSIDER_PATTERN_POSITION)
		setIndexFrontRear( forntrearIndex, frontrearAlpha, 
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
            useLungFrontPattern, useLungRearPattern, 
#endif
			it->y, slices.centerposition.y, lungrect.top, lungrect.bottom, slices.prone );
#endif
		array< pair<int,int> >::iterator itri( radiusIndexList ); //ソート済みの近傍サイズ
		sample.reserve( 0 );//近傍サンプル初期化
		sampleD.reserve( 0 );//近傍サンプル初期化
		array<ctRegion>::iterator itsnr( subtractNearRegion );
		checkMaximum<decimal,int> similarity;//最大の相関とそのパターン
		for ( int i = 0; i < similarityTable.size; ++i ) {
			for ( int j = 0; j < numberLungTopBottom; ++j ) {
				for ( int k = 0; k < numberLungFrontRear; ++k ) {
					similarityTable[i]( j, k ).initialize();
				}
			}
		}
		for ( array<int>::iterator itr( radiusList ); itr; ++itr, ++itsnr ) {
			//前ステップからの差分領域からのみ画素を追加してサンプルをとる
			for ( ctRegion::point_enumrator itn( itsnr() ); itn; ++itn ) {
				const int x = it->x + itn->x, y = it->y + itn->y;
				if ( between( 0, x, w - 1 ) && between( 0, y, h - 1 ) && lungRegion[x+w*y] ) {
					if ( ctimg.in( x, y ) ) {
						sample.push_back( ctimg.getInternal( x, y ).y );
						sampleD.push_back( int16( info( x, y ).gnorm ) );
					}
				}
			}
            decimal coefficent = ( normalized ? 1.0 / ::sqrt( ge.self( sample ) * ( considerGradient ? ge.self( sampleD ) : 1 ) ) : 1 );
			//現在の半径と同じ個所だけをそれぞれパターンと比較
			for ( ; itri && itri().right == itr(); ++itri ) {
				const int indexRegion = itri().left;//教師画像のインデックス
				array2< checkMaximum<decimal,int> > & st = similarityTable[indexRegion];
				for ( array< retainer<miPattern> >::iterator ittp( doc->patterns( indexRegion ) ); ittp; ++ittp ) {
					miPattern & mip = ittp()();
#if defined(OPERATION_FLAG_CONSIDER_PATTERN_POSITION)
#if ! defined(OPERATION_FLAG_USE_ALL_PATTERN)
					switch ( mip.topbottom ) {
					case lungTop: if ( ! useLungTopPattern ) continue; break;
					case lungMiddle: if ( ! useLungCenterPattern ) continue; break;
					case lungBottom: if ( ! useLungBottomPattern ) continue; break;
					}
					switch ( mip.frontrear ) {
					case lungFront: if ( ! useLungFrontPattern ) continue; break;
					case lungRear: if ( ! useLungRearPattern ) continue; break;
					}
#endif
#endif
                    //最大値を計算.
#if defined(OPERATION_FLAG_CONSIDER_PATTERN_POSITION)
					checkMaximum<decimal,int> & smlrty = st( mip.topbottom, mip.frontrear );//区域ごとに最大値を確認する.
#else
					checkMaximum<decimal,int> & smlrty = st( 0, 0 );//同じ区域
#endif
					if ( considerGradient ) {//勾配も確認する.
						smlrty( mip.estimate( sample, normalized ) * 
							mip.estimateGradient( sampleD, normalized ) * 
							coefficent );
					} else {
						smlrty( mip.estimate( sample, normalized ) * coefficent );
					}
				}
			}
		}
		//類似度テーブルを線形補間する
#if defined(OPERATION_FLAG_CONSIDER_PATTERN_POSITION)
		for ( int i = 0; i < similarityTable.size; ++i ) {
			decimal s = interpolate( similarityTable[i], topdownIndex, forntrearIndex, topdownAlpha, frontrearAlpha );
			if ( s ) similarity( s, i );
		}
#else
		//場所を気にしない場合には先頭にのみデータがあるのでそれの最大を計算する
		for ( int i = 0; i < similarityTable.size; ++i ) {
			array2< checkMaximum<decimal,int> > & st = similarityTable[i];
			if ( st( 0, 0 ) ) similarity( st( 0, 0 )(), i );
		}
#endif
        //最大類似度を持つ領域に対象画素を割り当てる.
		if ( similarity ) regions[similarity.sub].push_back( ctRegion::rectangle::pixel( it->x, it->y ) );
        if ( count % progressStepForPixel == 0 ) this->progress();//並列化しながらカウントが可能.
		++count;
	}
    //処理結果を適用
	for ( int i = regionNormal; i < numRegion; ++i ) {
		if ( ! doc->patterns(i).size ) continue;//教師画像が無ければ処理を適用しない.
		regions[i].connect();//データを連結処理する
		slice.changeRegion( i, regions[i] );//変更を適用する
	}
    //sprintf( debug, "end %d\n", sindex ); ::OutputDebugString( debug );
    return true;
}

namespace operationCommand
{
	class miCommandDeleteMinorRegion : public miCommand
	{
	public:
		bool allSlices;
		bool merge;
		int maximumSize, regionIndex;
		miCommandDeleteMinorRegion( bool allSlices, int maximumSize, int regionIndex, bool merge = false )
		{
			this->allSlices = allSlices;
			this->regionIndex = regionIndex;
			this->maximumSize = maximumSize;
			this->merge = merge;
			m.initialize();
			m.change( allSlices );
		}
		virtual bool execute()
		{
			bool done = false;
			miDocument * doc = miDocument::get();
			if ( ! doc ) return false;
			statusBar->reset();
			statusBar->expand( allSlices ? doc->slicesize : 1 );
			for ( ctSlices::iterator its( doc->slices.createIterator( allSlices ) ); its; ++its ) {
				statusBar->progress();
				ctSlice & s = its();
				const ctRegion & normal = s.regions[regionNormal];
				array<ctRegion> targets( doc->numberRegion() );
				for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
					targets[i] = s.regions[i];
				}
				ctRegion & target = targets[regionIndex];
                std::list<ctRegion> parts;
				target.divideMultiRegion( parts );
				bool changed = false;
				for ( std::list<ctRegion>::iterator itp = parts.begin(); itp != parts.end(); ++itp ) {
					ctRegion & part = *itp;
					int area = part.area();
					if ( maximumSize < area ) continue;
					ctRegion frame;
					part.dilation( frame );
					frame = ( frame - part );
					if ( merge ) {
						checkMaximum<int,int> maximumArea;
						for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
							if ( targets[i].empty() ) continue;
							maximumArea( ( frame & targets[i] ).area(), i );
						}
						if ( maximumArea ) {
							target -= part;
							targets[maximumArea.sub] |= part;
							changed = true;
						}
					} else {
						if ( ( frame - normal ).empty() ) {//周りが全て正常である
							target -= part;
							changed = true;
						}
					}
				}
				if ( ! changed ) continue;
				if ( merge ) {
					for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
						s.setRegion( i, targets[i] );
					}
					s.adjustRegion();
				} else {
					s.changeRegion( regionIndex, target );
				}
				done = true;
			}
			statusBar->reset();
			doc->change();
			imageWindow->invalidate();
			return done;
		}
	};
};

void miDocument::deleteMinorRegion( ctRegion & rgn, bool merge )
{
	ctSlice & s = slice();
	ctRegion normal = s.regions[regionNormal] & rgn;
	for ( int i = regionNormal + 1; i < numberRegion(); ++i ) {
		ctRegion target = s.regions[i] & rgn;
		if ( target.empty() ) continue;
		checkMaximum<int> maximumSize;
        std::list<ctRegion> parts;
		target.divideMultiRegion( parts );
		for ( std::list<ctRegion>::iterator itp = parts.begin(); itp != parts.end(); ++itp ) {
			ctRegion & part = *itp;
			int area = part.area();
			if ( merge ) {//併合ならば無条件で
				maximumSize( area );
			} else {
				ctRegion frame;
				part.dilation( frame );
				frame = ( frame - part ) & rgn;
				if ( ( frame - normal ).empty() ) {//周りが全て正常である
					//対象となりえる
					maximumSize( area );
				}
			}
		}
		if ( ! maximumSize ) continue;//対象が存在しない
		char wk[1024];
		if ( ! merge ) {
			sprintf( wk, "全スライスで面積%d画素以下の正常肺野に囲まれている%s領域を削除しますか？", 
				int( maximumSize() * 1.25 ), regionName( i ).chars() );
		} else {
			sprintf( wk, "全スライスで面積%d画素以下の%s領域を周りの領域に併合しますか？", 
				int( maximumSize() * 1.25 ), regionName( i ).chars() );
		}
		int ret = MessageBox( mainWindow->hwnd, wk, "確認", MB_OKCANCEL );
		if ( ret == IDOK ) {
			miCommands::get().regist( new operationCommand::miCommandDeleteMinorRegion( true, maximumSize() * 1.25, i, merge ) );
		}
	}
}
