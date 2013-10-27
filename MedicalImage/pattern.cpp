////////////////////////////////////////////////////////////////////////////////
//パターン
#include "main.h"

//初期化
bool miPattern::initialize( const char * drive, const char * path, const char * name, const char * postfix )
{
	regionChange = true;//変更フラグ

	//画像初期化
	ctimg.create( 512, 512 );
	ctimg.fill( pixelLuminance<int16>( 0 ) );

	//教師情報のXMLのパス
	char ct_filename[MAX_PATH], tea_filename[MAX_PATH];
	_makepath( tea_filename, drive, path, name, NULL );

	//画像のパス
	char _drive[MAX_PATH], _path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( tea_filename, _drive, _path, filename, extension );
	_makepath( ct_filename, drive, path, filename, ".2di" );

	//ある場合にのみ読み込み
	if ( ! file::exist( ct_filename ) || ! file::exist( tea_filename ) ) 
		return false;

	imageInterface< pixelLuminance<int16> > wk;
	wk.topdown = true;
	if ( ! ctSlice::load( wk, ct_filename ) ) return false;
	ctimg = wk;

	if ( ! regionIO::load( *this, tea_filename ) ) return false;
	if ( postfix && strlen( postfix ) ) this->postfix = postfix;
	//パラメータの初期化
	initializeParameter();
	return true;
}
//終了処理
bool miPattern::finalize( const char * drive, const char * path, const char * name )
{
	//空の場合には保存しない
	if ( shape.empty() || ctimg.empty() ) return false;

	//保存用パス作成
	char ct_filename[MAX_PATH], tea_filename[MAX_PATH];
	_makepath( ct_filename, drive, path, name, ".2di" );
	_makepath( tea_filename, drive, path, name, ".tea" );

	//保存
	imageInterface< pixelLuminance<int16> > wk;
    wk.create( ctimg.width, ctimg.height );
	wk.fill( pixelLuminance<int16>( 0 ) );
	for ( ctImage::iterator it( ctimg ); it; ++it ) {
		wk.setInternal( it.x(), it.y(), ctimg.getInternal( it() ) );
	}
	ctSlice::save( wk, ct_filename );
    regionIO::save( *this, tea_filename );
	return true;
}
bool miPattern::remove( const char * drive, const char * path, const char * name )
{
	//保存用パス作成
	char ct_filename[MAX_PATH], tea_filename[MAX_PATH], rgn_filename[MAX_PATH];
	_makepath( ct_filename, drive, path, name, ".2di" );
	_makepath( tea_filename, drive, path, name, ".tea" );
	_makepath( rgn_filename, drive, path, name, ".rgn" );
	if ( file::exist( rgn_filename ) ||
		file::exist( ct_filename ) || file::exist( tea_filename ) ) {
		DeleteFile( rgn_filename );//古い形式があれば削除
		DeleteFile( ct_filename );
		DeleteFile( tea_filename );
		return true;
	} else {
		return false;
	}
}

//指定した領域での初期化
bool miPattern::initialize( ctSlice & slice, const ctRegion & rgn )
{
	if ( rgn.empty() ) return false;
	regionChange = true;
	slice.getImage( ctimg );
	shape = rgn;
	initializeParameter();
	return true;
}
static int compairInt16( const void * v1, const void * v2 )
{
	return *( reinterpret_cast< const int16* >( v1 ) ) - 
		*( reinterpret_cast< const int16* >( v2 ) );
}

void miPattern::initializeParameter()
{
	//面積の計算
	area = shape.area();
	average = variance = 0;
	//メモリの確保と初期化
	sample.reserve( int( area ) );
	for ( int i = 0; i < 3; ++i ) {
		moment[i].allocate( maximumValue - minimumValue + 1 );
		memset( moment[i].data, 0, moment[i].size * sizeof( decimal ) );
		cumulativeMoment[i].allocate( maximumValue - minimumValue + 1 );
		memset( cumulativeMoment[i].data, 0, cumulativeMoment[i].size * sizeof( decimal ) );
	}

	//内部の最大最小の初期化
	minimumCT = maximumValue;
	maximumCT = minimumValue;
	int16 ct;
	for ( ctRegion::point_enumrator it( shape ); it; ++it ) {
		ct = ctimg.getInternal( it->x, it->y ).y;
		if ( between<int32>( minimumValue, int32( ct ), maximumValue ) ) {
			sample.push_back( ct );
			average += ct;
			//getHistogram( ct ) += d;
			minimumCT = minimum( minimumCT, ct );
			maximumCT = maximum( maximumCT, ct );
		}
	}
    if ( sample.size )
    {
        area = sample.size;
        const decimal d = 1 / area;
	    qsort( sample.data, sample.size, sizeof( int16 ), compairInt16 );
	    average /= area;
	    for ( array<int16>::iterator its( sample ); its; ++its ) {
		    ct = its();
		    getHistogram( ct ) += d;
		    variance += ( ct - average ) * ( ct - average );
	    }
	    variance /= area;
    }
	getCumulativeHistogram( minimumValue ) = getHistogram( minimumValue );
	getAverages( minimumValue ) = getCumulativeAverages( minimumValue ) = getHistogram( minimumValue ) * decimal( minimumValue );
	get2Moment( minimumValue ) = getCumulative2Moment( minimumValue ) = getHistogram( minimumValue ) * square( decimal( minimumValue ) );
	for ( int16 i = minimumValue + 1; i <= maximumValue; ++i ) {
		getAverages(i) = getHistogram(i) * decimal( i );
		get2Moment(i) = getHistogram(i) * square( decimal( i ) );
		getCumulativeHistogram(i) = getCumulativeHistogram(i-1) + getHistogram(i);
		getCumulativeAverages(i) = getCumulativeAverages(i-1) + getAverages(i);
		getCumulative2Moment(i) = getCumulative2Moment(i-1) + get2Moment(i);
	}
    standardDeviation = ::sqrt( variance );
	ge->buildup( sample, histogram_table );
	histogram_table_normalized = histogram_table;
	ge->normalized( sample, histogram_table_normalized );
	info.release();//再変更時，後ほど更新必須
	regionChange = true;
}

//判別分析法の閾値を算出
decimal miPattern::discriminantAnalysis()
{
	checkMaximum<decimal,int16> threshold;//閾値以下と閾値超でわける
	for ( int16 t = minimumCT + 1; t < maximumCT - 1; ++t ) {
		const decimal percentageLower = getCumulativeHistogram( t );
		const decimal percentageHigher = getCumulativeHistogram( maximumValue ) - percentageLower;
		if ( percentageLower == 0 || percentageHigher == 0 ) continue;
		const decimal averageLower = getCumulativeAverages( t ) / percentageLower;
		const decimal averageHigher = ( getCumulativeAverages( maximumValue ) - getCumulativeAverages( t ) ) / percentageHigher;
		const decimal momentLower = getCumulative2Moment( t ) / percentageLower;
		const decimal momentHigher = ( getCumulative2Moment( maximumValue ) - getCumulative2Moment( t ) ) / percentageHigher;
		const decimal varianceLower = momentLower - square( averageLower );
		const decimal varianceHigher = momentHigher - square( averageHigher );

		//クラス内分散
		const decimal varianceInner = percentageLower * varianceLower + percentageHigher * varianceHigher;
		//クラス間分散
		const decimal varianceOuter = percentageLower * square( averageLower - average ) + percentageHigher * square( averageHigher - average );
		if ( varianceInner == 0 ) continue;
		threshold( varianceOuter / varianceInner, t );
	}
	//エラー
	if ( ! threshold ) 
		return ( maximumCT + minimumCT ) / 2;
	return threshold.sub;
}
static int compairDecimal( const void * v1, const void * v2 )
{
	return *( reinterpret_cast< const decimal* >( v1 ) ) - 
		*( reinterpret_cast< const decimal* >( v2 ) );
}
decimal miPattern::regionAverage( const ctRegion & rgn )
{
	decimal area = rgn.area();
	decimal average = 0;
	for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
		average += static_cast<decimal>( ctimg.get( it->x, it->y ).y ) / area;
	}
	return average;
}
decimal miPattern::regionMedian( const ctRegion & rgn )
{
	decimal area = rgn.area();
	//輪郭のCT値をリストアップ
	array<decimal> cts; cts.reserve( area );
	for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
		cts.push_back( static_cast<decimal>( ctimg.get( it->x, it->y ).y ) );
	}
	//中央値を求める
	qsort( cts.data, cts.size, sizeof( decimal ), compairDecimal );
	return cts[cts.size/2];
}
decimal miPattern::regionVariance( const ctRegion & rgn, decimal average )
{
	decimal area = rgn.area();
	decimal variance = 0;
	for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
		variance += square( ctimg.get( it->x, it->y ).y - average ) / area;
	}
	return variance;
}

class inputBox
{
	string title;
	string msg;
	string editbox;
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static inputBox * _this = NULL;
		static HWND okbottun = NULL;
		static HWND cancelbottun = NULL;
		if ( uMsg != WM_INITDIALOG && _this == NULL ) return 0;
		switch( uMsg ) {
		case WM_INITDIALOG:
			{
				fc::dialog * dlg = reinterpret_cast<fc::dialog*>( lParam );
				_this = reinterpret_cast<inputBox*>( dlg->externalData );
				dlg->hwnd = hDlg;
				RECT wr;
				int width = 320, height = 200;
				GetWindowRect( dlg->parent, & wr );
				//非表示，センタリング，一番手前，サイズ設定
				SetWindowPos( hDlg, HWND_TOPMOST, 
					( wr.right - wr.left - width ) / 2, 
					( wr.bottom - wr.top - height ) / 2, 
					width, height,
					SWP_HIDEWINDOW );
				//クライアント位置取得
				RECT cr;
				GetClientRect( hDlg, & cr );
				//ボタン作成
				int w = 80, h = 20;
				okbottun = CreateWindow( "BUTTON", "OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER, 
					20, 100, w, h, hDlg, reinterpret_cast<HMENU>( IDOK ), dlg->instance, NULL );
				cancelbottun = CreateWindow( "BUTTON", "OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER, 
					120, 100, w, h, hDlg, reinterpret_cast<HMENU>( IDCANCEL ), dlg->instance, NULL );
				//表示
				ShowWindow( hDlg, SW_SHOW );
			}
			return 1;
		case WM_COMMAND:
			{
 				switch(LOWORD(wParam))
				{
				case IDOK:
					EndDialog( hDlg, IDOK );
					break;
				case IDCANCEL:
					EndDialog( hDlg, IDCANCEL );
					break;
				}
			}
			break;
		default:
			return 0;
		}
		return 1;
	}
public:
	static const string call( HWND parent, const char * _title = "input box", const char * _msg = "info" )
	{
		inputBox ib;
		ib.title = _title;
		ib.msg = _msg;
		fc::dialog dlg( parent, 0, dlgProc );
		dlg.externalData = & ib;
		list< array<char> > tmpls;
		dlg.setTemplate( tmpls.push_back( array<char>() ), 0, 0, 0, 320, 240, _title );
		bool r = dlg.call( tmpls );
		if ( ! r ) return "";
		return ib.editbox;
	}
};
//パラメータ算出
void miPattern::parameterAnalysis()
{
	//判別分析法で閾値抽出 明確に区別できなかった
	decimal th = 750;//discriminantAnalysis();

	inputBox::call( mainWindow->hwnd );

	//閾値により形状をわける
	ctRegion lower;
	int area = shape.area();
	lower.reserve( area );
	for ( ctRegion::point_enumrator it( shape ); it; ++it ) {
		if ( ctimg.getInternal( it->x, it->y ).y <= th ) {
			lower.push_back( it->x, it->y );
		}
	}
	lower.connect();
	//それでも，くっついている個所をopening処理で離す
	lower.opening( lower, 1 );
	//孔ごとにわける
    std::list<ctRegion> lowers;
	lower.divideMultiRegion( lowers );

	ctRegion outline;
	checkCT.initialize();
	checkArea.initialize();
	checkCircular.initialize();
	checkMargin.initialize();
	checkOutlineWidth.initialize();
	char wk[1024];
	sprintf( wk, "lowers.size(%d)", lowers.size() );
	MessageBox( mainWindow->hwnd, wk, "analysis", MB_OK );
    for ( std::list<ctRegion>::iterator it = lowers.begin(); it != lowers.end(); ++it ) {
		ctRegion & hole = *it;
		//標準偏差内の輪郭成分は内部に含まれる様に拡張する
		bool expanced = false;
		decimal average, sd;
		int area = 0;
		do {
			expanced = false;
			area = hole.area();
			average = regionAverage( hole );
			decimal variance = regionVariance( hole, average );
            sd = ::sqrt( variance );
			hole.dilation( outline, 1 );
			outline -= hole;
			ctRegion add;
			add.reserve( outline.area() );
			for ( ctRegion::point_enumrator ito( outline ); ito; ++ito ) {
				decimal ct = ctimg.get( ito->x, ito->y ).y;
				if ( absolute( ct - average ) < sd  ) {
					add.push_back( ito->x, ito->y );
				}
			}
			add.connect();
			if ( ! add.empty() ) {
				expanced = true;
				hole = hole | add;
			}
		} while ( expanced );
		//外側の壁が平均が小さくなるまで拡張する
		int width = 1;
		/*
		decimal prevOutlineAverage = 0;
		for ( width = 1; width < 1; ++width ) {
			hole.dilation( outline, width );
			outline -= hole;
			decimal outlineAverage = regionAverage( outline );
			if ( width == 1 ) {
				prevOutlineAverage = outlineAverage;
				continue;
			}
			if ( prevOutlineAverage > outlineAverage ) {
				--width; break;
			}
			prevOutlineAverage = outlineAverage;
		}
		*/
		hole.dilation( outline, width );
		outline -= hole;
		int outlinearea = outline.area();
        decimal circular = ::sqrt( area / pi ) / ( outlinearea / pi / 2.0 / width );
		if ( circular < 0.3 ) continue;
		//中央値を求める
		decimal outlinemedian = regionMedian( outline );
		decimal margin = outlinemedian - average;

		//記録
		for ( ctRegion::point_enumrator ito( hole ); ito; ++ito ) {
			decimal ct = ctimg.get( ito->x, ito->y ).y;
			checkCT( ct );
		}
		checkArea( area );
		checkCircular( circular );
		checkOutlineWidth( width );
		checkMargin( margin );
	}
	if ( checkArea ) {
		sprintf( wk, "ct(%d,%d), area(%d,%d), circular(%f,%f), outline width(%f,%f), margin(%f,%f)", 
			checkCT.minimumValue, checkCT.maximumValue, 
			checkArea.minimumValue, checkArea.maximumValue, 
			checkCircular.minimumValue, checkCircular.maximumValue, 
			checkOutlineWidth.minimumValue, checkOutlineWidth.maximumValue, 
			checkMargin.minimumValue, checkMargin.maximumValue );
		MessageBox( mainWindow->hwnd, wk, "analysis", MB_OK );
	}
}

//表示用クラス

void teacherPatternView::initialize( miPattern * pattern, pixel * color )
{
	this->pattern = pattern;
	this->color = color;
	valid = false;
}
bool teacherPatternView::finalize()
{
	pattern = NULL;
	color = NULL;
	valid = false;
	return true;
}
void teacherPatternView::mode( int16 windowSize, int16 windowLevel, int viewmode )
{
	if ( this->windowSize == windowSize && this->windowLevel == windowLevel && this->viewmode == viewmode ) return;
	this->windowSize = windowSize;
	this->windowLevel = windowLevel;
	this->viewmode = viewmode;
	valid = false;
}
image & teacherPatternView::getImage()
{
	if ( ( ! valid ) || ( pattern && pattern->regionChange ) ) update();
	return img;
}

void teacherPatternView::update()
{
	valid = true;
	if ( ! color ) {
		static pixel black( 0, 0, 0, 255 );
		color = & black;
	}
	if ( pattern ) {
		pattern->regionChange = false;
		//表示用画像作成
		img.create( 512, 512 );
		decimal rate = 255.0 / windowSize;
		decimal offset = windowLevel - windowSize / 2;
		int x, y;
		pixel p( 0, 0, 0, 255 );
		if ( ! pattern->shape.empty() && 
			pattern->ctimg.height == img.height && 
			pattern->ctimg.width == img.width ) {
			for ( y = 0; y < img.height; y++ ) for ( x = 0; x < img.width; x++ ) {
				p.r = p.g = p.b = static_cast<int8>( clamp<decimal>( 0, ( pattern->ctimg.getInternal( x, y ).y - offset ) * rate, 255.0 ) );
				img.set( x, y, p );
			}
		}
		//外側を消す
		ctRegion outer;
		switch ( viewmode ) {
		case viewModeOriginal:
			break;
		case viewModeMix:
			pattern->shape.dilation( outer, 2 );
			outer = ( ctRegion( ctRegion::rectangle::size( img.width, img.height ) ) & outer ) - pattern->shape;
			break;
		case viewModeLabel:
			outer = ctRegion( ctRegion::rectangle::size( img.width, img.height ) ) - pattern->shape;
			break;
		}
		ctSliceView::regionBlend( img, outer, pixel( color->r, color->g, color->b, 255 ) );
		ctSliceView::setAlpha( img, outer, 0 );
	} else {//パターンがない場合.
		//表示用画像作成
		if ( img.width == 512 && img.height == 512 ) {
			img.fill( pixel( color->r, color->g, color->b, 255 ) );
		} else {
			img.create( 512, 512, pixel( color->r, color->g, color->b, 255 ) );
		}
	}
}
