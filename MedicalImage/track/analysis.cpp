#include "main.h"
#include "document.h"
#include "bmptool.inl"

const double multiScale = 20;//三次元ハフ変換時の量子化スケール

//線上の点列を作成
bool linepoints( const point2<int> & p1, const point2<int> & p2, list< point2<int> > & points )
{
	int x1 = p1.x;
	int y1 = p1.y;
	int x2 = p2.x;
	int y2 = p2.y;
	const int ax = sign( x2 - x1 ), ay = sign( y2 - y1 );//進行方向の符号の計算
	const int dx = abs( x2 - x1 ), dy = abs( y2 - y1 );//方向の計算
	const int w = dx + 1, h = dy + 1;//方向の計算
	points.release();
	if ( dx + dy == 0 ) {//点しかない．
		return false;
	}
	const int ex = dx << 1, ey = dy << 1;//真の値に近いように誤差を0.5の上下で判定するために誤差の計算用の数値を計算する
	int e = 0, x = x1, y = y1;//誤差と初期位置を設定
	int count = 0;
	if ( dx >= dy ) {
		for ( int t = 0; t <= dx; ++t ) {
			points.push_back( point2<int>( x, y ) );
			x += ax; e += ey;
			if ( e >= dx ) {
				y += ay; e -= ex;
			}
		}
	} else {
		for ( int t = 0; t <= dy; ++t ) {
			points.push_back( point2<int>( x, y ) );
			y += ay; e += ex;
			if ( e >= dy ) {
				x += ax; e -= ey;
			}
		}
	}
	return true;
}
//点列上の画像作成
void trackDocument::lineimage( const list< point2<int> > & points, array2<int16> & img )
{
	img.allocate( originalImages.size, points.size );
	for ( int t = 0; t < points.size; ++t ) {
		for ( int i = 0; i < originalImages.size; ++i ) {
			img( i, t ) = originalImages[i].get( points[t].x, points[t].y ).y;
		}
	}
}
//エッジ画像作成
inline void edgeimage( array2<int16> & img, array2<int16> & edge, int distance )
{
	edge.allocate( img.w, img.h );
	memset( edge.data, 0, sizeof( int16 ) * edge.size );
	for ( int x = 0; x < img.w; ++x ) {
		for ( int y = 1; y < img.h - 1; ++y ) {
			double edgeminus = 0, edgeplus = 0;
			int countminus = 0, countplus = 0;
			for ( int t = 0; t < abs( distance ); ++t ) {
				if ( between( 0, y - t - 1, img.h - 1 ) ) {
					edgeminus += img( x, y - t - 1 );
					++countminus;
				}
				if ( between( 0, y + t, img.h - 1 ) ) {
					edgeplus += img( x, y + t );
					++countplus;
				}
			}
			if ( distance > 0 ) {
				edge( x, y ) = zerodivide<double>( edgeplus, countplus ) - zerodivide<double>( edgeminus, countminus );
			} else {
				edge( x, y ) = - zerodivide<double>( edgeplus, countplus ) + zerodivide<double>( edgeminus, countminus );
			}
		}
	}
}
//エッジ画像からハフ空間への投影
void trackDocument::project( array2<int16> & slice, array2<int16> & edge, array2<statisticsValue> & hough )
{
	for ( int l = 0; l < slice.h; ++l ) {
		for ( int t = 0; t < slice.w; ++t ) {
			const int16 e = edge( t, l );
			const int16 v = slice( t, l );
			//pos : line( line parameter = coefficient * base wave(t) + bias )
			//slice:( time(t), line parameter(l) )
			//hough:( coefficient(c), bias(b) )
			//b = - base wave(t) * c + l
			//coefficient:[-houghRange,houghRange]の範囲を取るので
			for ( int c = 0; c < hough.w; ++c ) {
				const double coefficient = c / houghScale - houghRange;
				const int b = round<int,double>( - wave[t] * coefficient + l );
				if ( between( 0, b, hough.h - 1 ) ) {
					hough( c, b ).append( v, e );
				}
			}
		}
	}
}
void trackDocument::project( array2<int16> & slice, array2<double> & hough )
{
	for ( int l = 0; l < slice.h; ++l ) {
		for ( int t = 0; t < slice.w; ++t ) {
			int16 value = slice( t, l );
			if ( value == 0 ) continue;
			//pos : line( line parameter = coefficient * base wave(t) + bias )
			//slice:( time(t), line parameter(l) )
			//hough:( coefficient(c), bias(b) )
			//b = - base wave(t) * c + l
			//coefficient:[-houghRange,houghRange]の範囲を取るので
			for ( int c = 0; c < hough.w; ++c ) {
				double coefficient = c / houghScale - houghRange;
				int b = round<int,double>( - wave[t] * coefficient + l );
				if ( between( 0, b, hough.h - 1 ) ) {
					hough( c, b ) += value;
				}
			}
		}
	}
}

void trackDocument::project( array2<int16> & slice, array3<double> & hough )
{
	const double addition = 1.0 / slice.w;
	for ( int l = 0; l < slice.h; ++l ) {
		for ( int t = 0; t < slice.w; ++t ) {
			int16 value = slice( t, l ) / multiScale;
			if ( value == 0 ) continue;
			//pos : line( line parameter = coefficient * base wave(t) + bias )
			//slice:( time(t), line parameter(l) )
			//hough:( coefficient(c), bias(b) )
			//b = - base wave(t) * c + l
			//coefficient:[-houghRange,houghRange]の範囲を取るので
			for ( int c = 0; c < hough.w; ++c ) {
				double coefficient = c / houghScale - houghRange;
				int b = round<int,double>( - wave[t] * coefficient + l );
				if ( between( 0, b, hough.h - 1 ) ) {
					hough( c, b, clamp<int>( 0, value, hough.d - 1 ) ) += addition;
				}
			}
		}
	}
}
//ハフ空間での余分な個所の削除
void trackDocument::clear( double coefficient, double bias, double mn, double mx, array2<statisticsValue> & hough )
{
	coefficient = coefficient / houghScale - houghRange;
	//pos : line( line parameter = coefficient * base wave(t) + bias )
	for ( int x = 0; x < hough.w; ++x ) {
		double c = x / houghScale - houghRange;
		// b = - c m + c0 m + b0の直線に囲まれた空間を削除
		double b1 = - c * mn + coefficient * mn + bias;
		double b2 = - c * mx + coefficient * mx + bias;
		if ( b1 > b2 ) swap( b1, b2 );
		const int room = 0;
		int s = maximum<double>( 0, floor( b1 - room ) );
		int e = minimum<double>( hough.h-1, ceil( b2 + room ) );
		for ( int y = s; y <= e; ++y ) {
			hough( x, y ).enable = false;
		}
	}
}
void trackDocument::clear( double coefficient, double bias, double mn, double mx, array2<double> & hough )
{
	coefficient = coefficient / houghScale - houghRange;
	//pos : line( line parameter = coefficient * base wave(t) + bias )
	for ( int x = 0; x < hough.w; ++x ) {
		double c = x / houghScale - houghRange;
		// b = - c m + c0 m + b0の直線に囲まれた空間を削除
		double b1 = - c * mn + coefficient * mn + bias;
		double b2 = - c * mx + coefficient * mx + bias;
		if ( b1 > b2 ) swap( b1, b2 );
		//int s = maximum<double>( 0, floor( b1 - 1 ) );
		//int e = minimum<double>( hough.h-1, ceil( b2 + 1 ) );
		int s = maximum<double>( 0, floor( b1 ) );
		int e = minimum<double>( hough.h-1, ceil( b2 ) );
		for ( int y = s; y <= e; ++y ) {
			hough( x, y ) = 0;
		}
	}
}
void trackDocument::clear( double coefficient, double bias, double mn, double mx, array3<double> & hough )
{
	coefficient = coefficient / houghScale - houghRange;
	//pos : line( line parameter = coefficient * base wave(t) + bias )
	for ( int x = 0; x < hough.w; ++x ) {
		double c = x / houghScale - houghRange;
		// b = - c m + c0 m + b0の直線に囲まれた空間を削除
		double b1 = - c * mn + coefficient * mn + bias;
		double b2 = - c * mx + coefficient * mx + bias;
		if ( b1 > b2 ) swap( b1, b2 );
		//int s = maximum<double>( 0, floor( b1 - 1 ) );
		//int e = minimum<double>( hough.h-1, ceil( b2 + 1 ) );
		int s = maximum<double>( 0, floor( b1 ) );
		int e = minimum<double>( hough.h-1, ceil( b2 ) );
		for ( int y = s; y <= e; ++y ) {
			for ( int z = 0; z < hough.d; ++z ) {
				hough( x, y, z ) = 0;
			}
		}
	}
}
//静止状態の波を削除
void trackDocument::clearStatic( array2<statisticsValue> & hough )
{
	const int x = ( 0 + houghRange ) * houghScale;
	for ( int y = 0; y < hough.h; ++y ) {
		hough( x, y ).enable = false;
	}
}
void trackDocument::clearStatic( array2<double> & hough )
{
	const int x = ( 0 + houghRange ) * houghScale;
	for ( int y = 0; y < hough.h; ++y ) {
		hough( x, y ) = 0;
	}
}
void trackDocument::clearStatic( array3<double> & hough )
{
	const int x = ( 0 + houghRange ) * houghScale;
	for ( int y = 0; y < hough.h; ++y ) {
		for ( int z = 0; z < hough.d; ++z ) {
			hough( x, y, z ) = 0;
		}
	}
}
double trackDocument::constancy( link & lnk )
{
	double average = 0;
	double average2 = 0;
	int count = 0;
	const int timelength = sizeTime();
	for ( int t = 0; t < timelength; ++t ) {
		int x = lnk.cx * wave[t] + lnk.bx;
		int y = lnk.cy * wave[t] + lnk.by;
		if ( ! between( 0, x, width - 1 ) || 
				! between( 0, y, height - 1 ) ) continue;
		int16 value = originalImages[t].getInternal( x, y ).y;
		average += value;
		average2 += value * value;
		++count;
	}
	average /= count;
	average2 /= count;
	double variance = average2 - average * average;
	double standardDeviation = ::sqrt( variance );
	return clamp<double>( 0, 1 - standardDeviation / average, 1 );
}
//ハフ空間での検索
bool trackDocument::search( point2<int> & pt, double & value, array2<statisticsValue> & hough )
{
	checkMaximum< double, point2<int> > mp;
	for ( int y = 0; y < hough.h; ++y ) {
		for ( int x = 0; x < hough.w; ++x ) {
			statisticsValue & sv = hough( x, y );
			if ( ! sv.enable ) continue;
			double v = sv.constancy() * sv.edgeMoment1;
			if ( v == 0 ) continue;
			mp( v, point2<int>( x, y ) );
		}
	}
	if ( ! mp ) return false;
	pt = mp.sub;
	value = mp();
	return true;
}
void trackDocument::search( point2<int> & pt, double & value, array2<double> & hough )
{
	checkMaximum< double, point2<int> > mp;
	for ( int y = 0; y < hough.h; ++y ) {
		for ( int x = 0; x < hough.w; ++x ) {
			mp( absolute( hough( x, y ) ), point2<int>( x, y ) );
		}
	}
	pt = mp.sub;
	value = mp();
}

void trackDocument::search( point2<int> & pt, bool & findplus, double & value, array3<double> & minus, array3<double> & plus, int z )
{
	checkMaximum< double, point2<int> > mpplus, mpminus;
	for ( int y = 0; y < plus.h; ++y ) {
		for ( int x = 0; x < plus.w; ++x ) {
			mpplus( plus( x, y, z ), point2<int>( x, y ) );
			mpminus( minus( x, y, z ), point2<int>( x, y ) );
		}
	}
	findplus = ( mpplus() > mpminus() );
	pt = findplus ? mpplus.sub : mpminus.sub;
	value = findplus ? mpplus() : mpminus();
}
//見つけたときの状態を記録
/*
static void foundVertical( link & lnk, bool findplus, point2<int> & pt, int baseposition )
{
	lnk.type = findplus ? link::typeDown : link::typeUp;
	lnk.bx = baseposition;
	lnk.by = pt.y;
	lnk.cy = pt.x / houghScale - houghRange;
	lnk.cx = 0;
}
static void foundHorizontal( link & lnk, bool findplus, point2<int> & pt, int baseposition )
{
	lnk.type = findplus ? link::typeRight : link::typeLeft;
	lnk.bx = pt.y;
	lnk.by = baseposition;
	lnk.cx = pt.x / houghScale - houghRange;
	lnk.cy = 0;
}
static void foundDescent( link & lnk, bool findplus, point2<int> & pt, int baseposition )
{
	//右下下がりなので，(0,h-1)->(0,0)->(w-1,0)の方向にスキャンしている．
	const int height = trackDocument::get()->height;
	lnk.type = findplus ? link::typeDownRight : link::typeUpLeft;
	if ( baseposition < height ) {
		lnk.bx = 0;
		lnk.by = ( height - 1 ) - baseposition;
	} else {
		lnk.bx = baseposition - ( height - 1 );
		lnk.by = 0;
	}
	lnk.bx += pt.y;
	lnk.by += pt.y;
	lnk.cx = pt.x / houghScale - houghRange;
	lnk.cy = lnk.cx;
}
static void foundAscent( link & lnk, bool findplus, point2<int> & pt, int baseposition )
{
	//右上上がりなので，(0,0)->(0,h-1)->(w-1,h-1)の方向にスキャンしている．
	const int height = trackDocument::get()->height;
	lnk.type = findplus ? link::typeUpRight : link::typeDownLeft;
	if ( baseposition < height ) {
		lnk.bx = 0;
		lnk.by = baseposition;
	} else {
		lnk.bx = baseposition - ( height - 1 );
		lnk.by = height - 1;
	}
	lnk.bx += pt.y;
	lnk.by -= pt.y;
	lnk.cx = pt.x / houghScale - houghRange;
	lnk.cy = -lnk.cx;
}
*/
//波の検索
void trackDocument::findwaveS( list<link> & links, list<point2<int> > & points, int baseposition, int type )
{
	static array2<int16> slice, edge;
	static array2<statisticsValue> hough;
	//時間方向スライス画像を作成
	lineimage( points, slice );
	edgeimage( slice, edge, 3 );
	hough.allocate( houghWidth, slice.h );
	memset( hough.data, 0, sizeof( statisticsValue ) * hough.size );
	project( slice, edge, hough );
	clearStatic( hough );
	point2<int> pt;
	double mp;
	checkMaximum< double > maxEdge, maxConstancy;
	for ( int y = 0; y < hough.h; ++y ) {
		for ( int x = 0; x < hough.w; ++x ) {
			statisticsValue & sv = hough( x, y );
			sv.finish();
			maxEdge( sv.edgeMoment1 );
			maxConstancy( sv.constancy() );
		}
	}
	while ( search( pt, mp, hough ) ) {
		statisticsValue & sv = hough( pt.x, pt.y );
		link lnk;
		lnk.intensity = sv.edgeMoment1 / maxEdge();
		lnk.reliability = sv.constancy() / maxConstancy();
		lnk.set( type, pt, baseposition, height );
		clear( pt.x, pt.y, mnwave, mxwave, hough );
		links.push_back( lnk );
	};
}
void trackDocument::findwave( list<link> & links, list<point2<int> > & points, int baseposition, int type )
{
	static array2<int16> slice, edge;
	static array2<double> hough;
	//時間方向スライス画像を作成
	lineimage( points, slice );
	edgeimage( slice, edge, 1 );
	hough.allocate( houghWidth, slice.h );
	memset( hough.data, 0, sizeof( double ) * hough.size );
	project( edge, hough );
	//論文用画像生成
	if ( false ) {
		if ( type == link::typeVertical && baseposition == 87 ) {
			array2<double> exp;
			exp = hough;
			for ( int x = 0; x < hough.w; ++x ) {
				for ( int y = 0; y < hough.h; ++y ) {
					exp( x, y ) = absolute( exp( x, y ) );
				}
			}
			saveBmp( exp, string( "D:\\temp\\" ) + "hough_v" + string( baseposition ) + ".bmp" );
		} else if ( type == link::typeHorizontal && baseposition == 133 ) {
			array2<double> exp;
			exp = hough;
			for ( int x = 0; x < hough.w; ++x ) {
				for ( int y = 0; y < hough.h; ++y ) {
					exp( x, y ) = absolute( exp( x, y ) );
				}
			}
			saveBmp( exp, string( "D:\\temp\\" ) + "hough_h" + string( baseposition ) + ".bmp" );
		}
	}
	point2<int> pt;
	double mp;
	bool first = true;
	double maxMp;
	while ( 1 ) {
		search( pt, mp, hough );
		//if ( mp == 0 || mp < maxMp * 0.1 ) break;
		if ( mp == 0 ) break;
		if ( first ) {
			maxMp = mp;
			first = false;
		}
		link lnk;
		lnk.intensity = mp;
		lnk.reliability = mp / maxMp;
		lnk.set( type, pt, baseposition, height );

		clear( pt.x, pt.y, mnwave, mxwave, hough );
		links.push_back( lnk );
	};
}
//累加させる
inline void cumulate( array3<double> & a3 )
{
	for ( int y = 0; y < a3.h; ++y ) {
		for ( int x = 0; x < a3.w; ++x ) {
			for ( int z = a3.d - 2; 0 <= z; --z ) {
				a3( x, y, z ) += a3( x, y, z + 1 );
			}
		}
	}
}
//近くにだけ累加させる
inline void cumulateNear( array3<double> & a3, int affection )
{
	for ( int y = 0; y < a3.h; ++y ) {
		for ( int x = 0; x < a3.w; ++x ) {
			for ( int z = 0; z < a3.d; ++z ) {
				for ( int t = 1; t < affection && z - t >= 0; ++t ) {
					a3( x, y, z - t ) += a3( x, y, z ) * ( t / double( affection ) );
				}
			}
		}
	}
}
//波の検索
void trackDocument::findwave3( list<link> & links, list<point2<int> > & points, int baseposition, int type )
{
	static array2<int16> slice, edgeplus, edgeminus;
	static array3<double> houghplus, houghminus;
	//時間方向スライス画像を作成
	lineimage( points, slice );
	edgeimage( slice, edgeplus, 3 );
	edgeimage( slice, edgeminus, -3 );
	int maxEdge = 256 / multiScale;
	houghplus.allocate( houghWidth, slice.h, maxEdge );
	houghminus.allocate( houghWidth, slice.h, maxEdge );
	memset( houghplus.data, 0, sizeof( double ) * houghplus.size );
	memset( houghminus.data, 0, sizeof( double ) * houghminus.size );
	project( edgeplus, houghplus );
	project( edgeminus, houghminus );
	clearStatic( houghplus );
	clearStatic( houghminus );
	cumulateNear( houghplus, 5 );
	cumulateNear( houghminus, 5 );
	point2<int> pt;
	bool findplus;
	double mp;
	for ( double clearyLine = 1.0; clearyLine >= 0.0; clearyLine -= 0.1 ) {
		int z = maxEdge - 1;
		while ( 1 ) {
			search( pt, findplus, mp, houghminus, houghplus, z );
			if ( mp == 0 || mp < clearyLine ) {
				if ( 0 < z ) {
					--z;
					continue;
				}
				break;
			}
			link lnk;
			lnk.reliability = mp;
			lnk.intensity = z / double( maxEdge - 1 );
			lnk.set( type, pt, baseposition, height );
			double c = constancy( lnk );
			if ( c < 0.75 ) {
				houghplus( pt.x, pt.y, z ) = 0;
				houghminus( pt.x, pt.y, z ) = 0;
				continue;
			}
			clear( pt.x, pt.y, mnwave, mxwave, houghplus );
			clear( pt.x, pt.y, mnwave, mxwave, houghminus );
			links.push_back( lnk );
		};
	}
}
/*
void convert( imageInterface< pixelLuminance<int16> > & dst, const array2<int16> & src )
{
	dst.create( src.w, src.h );
	pixelLuminance<int16> p;
	for ( int y = 0; y < src.h; ++y ) {
		for ( int x = 0; x < src.w; ++x ) {
			p.y = src( x, y );
			dst.setInternal( x, y, p );
		}
	}
}
*/
//基本波検出
bool trackDocument::updateBaseWaveImage()
{
	baseWave.release();
	if ( firstwave == lastwave ) return false;

	//線上の点列を作成
	list<point2<int> > baseWavePoints;
	if ( ! linepoints( firstwave, lastwave, baseWavePoints ) ) return false;

	//断層の画像を作成
	lineimage( baseWavePoints, baseWave );
	return true;
}
void trackDocument::setBaseWave()
{
	if ( ! updateBaseWaveImage() ) return;

	//エッジ画像を評価
	array2<int16> edge;
	edgeimage( baseWave, edge, 3 );
	//最大エッジから，基本波を検出
	wave.allocate( baseWave.w );
	for ( int x = 0; x < baseWave.w; ++x ) {
		checkMaximum<double> maxPoint;
		for ( int y = 0; y < baseWave.h; ++y ) {
			if ( x ) {
				//int dif = absolute( baseWave( 0, wave[0] ) - baseWave( x, y ) );
				int dif = absolute( wave[x-1] - y );
				maxPoint( edge( x, y ) * clamp<double>( 0, ( 1 - dif / 50.0 ), 1 ), y );
			} else {
				maxPoint( edge( x, y ), y );
			}
		}
		wave[x] = 0;
		if ( maxPoint ) {
			wave[x] = maxPoint.sub;
		}
	}
	waveBias = wave[baseIndex];
	for ( int x = 0; x < wave.size; ++x ) {
		wave[x] = wave[x] - waveBias;
	}
	waveParameterUpdate( true );
	volatile int bi = inspirationFrame;
	changeBaseIndex( bi );
}

void trackDocument::waveParameterUpdate( bool updateInsExFrame )
{
	//基礎の画像を原点に持ってくる
	checkMaximum<int> maximumWave;
	checkMinimum<int> minimumWave;
	for ( int x = 0; x < wave.size; ++x ) {
		maximumWave( wave[x], x );
		minimumWave( wave[x], x );
	}
	mxwave = maximumWave();
	mnwave = minimumWave();
	if ( updateInsExFrame ) {
		inspirationFrame = maximumWave.sub;
		expirationFrame = minimumWave.sub;
	}
}
void trackDocument::linksort( list<link> & links )
{
	list<link> wk;
	wk = links;
	links.release();
	while ( ! wk.empty() ) {
		list<link>::iterator it( wk );
		list<link>::iterator top = it; ++it;
		for ( ; it; ++it ) {
			if ( it->offset( height ) < top->offset( height ) ) {
				top = it;
			}
		}
		links.push_back( top() );
		wk.pop( top );
	}
}
void trackDocument::flickerSetup( array2<flicker> & fs, int type, array< list<link> > & arraylink )
{
	fs.allocate( width, height );
	link temp;
	temp.type = type;
	for ( int x = 0; x < width; ++x ) {
		for ( int y = 0; y < height; ++y ) {
			temp.bx = x; temp.by = y;
			const int base = temp.base( height );
			const int offset = temp.offset( height );
			list<link> & links = arraylink[base];
			list<link>::iterator next( links );
			list<link>::iterator prev;
			for ( ; next && next->offset( height ) < offset; ++next ) {
				prev = next;
			}
			flicker & f = fs( x, y );
			f.initialize();
			if ( links.empty() ) continue;
			if ( ! prev && ! next ) continue;
			if ( ! prev ) {
				//リストの前の状態のため，次と同じ動きをする
				f.lnk.set( temp.type, point2<int>( next->efficientIndex(), offset ), base, height );
			} else if ( ! next ) {
				//リストの後ろの状態のため，前と同じ動きをする
				f.lnk.set( temp.type, point2<int>( prev->efficientIndex(), offset ), base, height );
			} else if ( next->offset( height ) == offset ) {
				//nextと同じ位置
				f.lnk.set( temp.type, point2<int>( next->efficientIndex(), offset ), base, height );
			} else {
				//補間
				double prevoffset = prev->offset( height );
				double nextoffset = prev->offset( height );
				double alpha = clamp<double>( 0, zerodivide<double>( offset - prevoffset, nextoffset - prevoffset ), 1 );
				f.lnk.set( temp.type, point2<int>( next->efficientIndex(), offset ), base, height );
				f.lnk.cx = alpha * next->cx + ( 1 - alpha ) * prev->cx;
				f.lnk.cy = alpha * next->cy + ( 1 - alpha ) * prev->cy;
			}
		}
	}
}
void trackDocument::analysisVertical( int type )
{
	//いらない部分を消す
	for ( array< list<link> >::iterator it( verticalLinks ); it; ++it ) {
		it->release();
	}

	//縦方向でハフ変換をおこなう．
	list<point2<int> > points;
	for ( int w = 0; w < width; ++w ) {
		if ( ! linepoints( point2<int>( w, 0 ), point2<int>( w, height - 1 ), points ) ) continue;
		list<link> & links = verticalLinks[w];
		switch ( type ) {
		case 0: findwave( links, points, w, link::typeVertical ); break;
		case 1: findwaveS( links, points, w, link::typeVertical ); break;
		case 2: findwave3( links, points, w, link::typeVertical ); break;
		}
		linksort( links );
		imageWindow->invalidate();
		imageWindow->update();
	}
	flickerSetup( flickers[0], link::typeVertical, verticalLinks );
}
void trackDocument::analysisHorizontal( int type )
{
	//いらない部分を消す
	for ( array< list<link> >::iterator it( horizontalLinks ); it; ++it ) {
		it->release();
	}
	//横方向でハフ変換をおこなう．
	list<point2<int> > points;
	for ( int h = 0; h < height; ++h ) {
		if ( ! linepoints( point2<int>( 0, h ), point2<int>( width - 1, h ), points ) ) continue;
		list<link> & links = horizontalLinks[h];
		linksort( links );
		switch ( type ) {
		case 0: findwave( links, points, h, link::typeHorizontal ); break;
		case 1: findwaveS( links, points, h, link::typeHorizontal ); break;
		case 2: findwave3( links, points, h, link::typeHorizontal ); break;
		}
		linksort( horizontalLinks[h] );
		imageWindow->invalidate();
		imageWindow->update();
	}
	flickerSetup( flickers[1], link::typeHorizontal, horizontalLinks );
}
void trackDocument::analysisAscent( int type )
{
	//いらない部分を消す
	for ( array< list<link> >::iterator it( ascentLinks ); it; ++it ) {
		it->release();
	}

	//右上上がり方向でハフ変換をおこなう．
	//右上上がりなので，(0,0)->(0,h-1)->(w-1,h-1)の方向にスキャンしている．
	list<point2<int> > points;
	for ( int h = 0; h < height; ++h ) {
		point2<int> s( 0, h );
		point2<int> e( h, 0 );
		if ( width - 1 < e.x ) {
			e.y += e.x - ( width - 1 );
			e.x = width - 1;
		}
		if ( ! linepoints( s, e, points ) ) continue;
		list<link> & links = ascentLinks[h];
		switch ( type ) {
		case 0: findwave( links, points, h, link::typeAscent ); break;
		case 1: findwaveS( links, points, h, link::typeAscent ); break;
		case 2: findwave3( links, points, h, link::typeAscent ); break;
		}
		linksort( links );
		imageWindow->invalidate();
		imageWindow->update();
	}
	for ( int w = 1; w < width; ++w ) {
		point2<int> s( w, height - 1 );
		point2<int> e( width - 1, ( height - 1 ) - ( ( width - 1 ) - w ) );
		if ( e.y < 0 ) {
			e.x = width - 1 + e.y;
			e.y = 0;
		}
		if ( ! linepoints( s, e, points ) ) continue;
		list<link> & links = ascentLinks[w + height - 1];
		switch ( type ) {
		case 0: findwave( links, points, w + height - 1, link::typeAscent ); break;
		case 1: findwaveS( links, points, w + height - 1, link::typeAscent ); break;
		case 2: findwave3( links, points, w + height - 1, link::typeAscent ); break;
		}
		linksort( links );
		imageWindow->invalidate();
		imageWindow->update();
	}
	flickerSetup( flickers[2], link::typeAscent, ascentLinks );
}
void trackDocument::analysisDescent( int type )
{
	//いらない部分を消す
	for ( array< list<link> >::iterator it( descentLinks ); it; ++it ) {
		it->release();
	}
	//右下下がり方向でハフ変換をおこなう．
	//右下下がりなので，(0,h-1)->(0,0)->(w-1,0)の方向にスキャンしている．
	list<point2<int> > points;
	for ( int h = 0; h < height; ++h ) {
		point2<int> s( 0, height - 1 - h );
		point2<int> e( h, height - 1 );
		if ( width - 1 < e.x ) {
			e.y -= e.x - ( width - 1 );
			e.x = width - 1;
		}
		if ( ! linepoints( s, e, points ) ) continue;
		list<link> & links = descentLinks[h];
		switch ( type ) {
		case 0: findwave( links, points, h, link::typeDescent ); break;
		case 1: findwaveS( links, points, h, link::typeDescent ); break;
		case 2: findwave3( links, points, h, link::typeDescent ); break;
		}
		linksort( links );
		imageWindow->invalidate();
		imageWindow->update();
	}
	for ( int w = 1; w < width; ++w ) {
		point2<int> s( w, 0 );
		point2<int> e( width - 1, ( width - 1 ) - w );
		if ( height - 1 < e.y ) {
			e.x -= e.y - ( height - 1 );
			e.y = height - 1;
		}
		if ( ! linepoints( s, e, points ) ) continue;
		list<link> & links = descentLinks[w + height - 1];
		switch ( type ) {
		case 0: findwave( links, points, w + height - 1, link::typeDescent ); break;
		case 1: findwaveS( links, points, w + height - 1, link::typeDescent ); break;
		case 2: findwave3( links, points, w + height - 1, link::typeDescent ); break;
		}
		linksort( links );
		imageWindow->invalidate();
		imageWindow->update();
	}
	flickerSetup( flickers[3], link::typeDescent, descentLinks );
}

point2<int> trackDocument::interpolate( int t, const point2<int> & p )
{
	if ( t == baseIndex ) return p;
	checkMaximum<int,link*> upCheck, leftCheck, upLeftCheck, downLeftCheck;
	checkMinimum<int,link*> downCheck, rightCheck, downRightCheck, upRightCheck;
	for ( list<link>::iterator it( verticalLinks[p.x] ); it; ++it ) {
		if ( it->by <= p.y ) upCheck( it->by - p.y, & it() );
		if ( it->by > p.y ) downCheck( it->by - p.y, & it() );
	}
	for ( list<link>::iterator it( horizontalLinks[p.y] ); it; ++it ) {
		if ( it->bx <= p.x ) leftCheck( it->bx - p.x, & it() );
		if ( it->bx > p.x ) rightCheck( it->bx - p.x, & it() );
	}
	for ( list<link>::iterator it( ascentLinks[p.x+p.y] ); it; ++it ) {
		if ( it->bx - it->by <= p.x - p.y ) downLeftCheck( it->bx - it->by - ( p.x - p.y ), & it() );
		if ( it->bx - it->by > p.x - p.y ) upRightCheck( it->bx - it->by - ( p.x - p.y ), & it() );
	}
	for ( list<link>::iterator it( descentLinks[p.x-p.y+height-1] ); it; ++it ) {
		if ( it->by + it->bx <= p.y + p.x ) upLeftCheck( it->bx + it->by - ( p.y + p.x ), & it() );
		if ( it->by + it->bx > p.y + p.x ) downRightCheck( it->bx + it->by - ( p.y + p.x ), & it() );
	}
	point2<double> result( 0, 0 );
	double denominator = 0;
	// ∑λ reliability * ( wave[t] * c + b )+(1-λ)reliability * ( wave[t] * c + b ) / ∑λ reliability + (1-λ) reliability 
	if ( upCheck && downCheck ) {
		checkMaximum<int,link*> & lowerCheck = upCheck;
		checkMinimum<int,link*> & upperCheck = downCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		if ( whole == 0 ) {
			result.x += ( wave[t] * upper.cx + upper.bx ) * upper.reliability;
			result.y += ( wave[t] * upper.cy + upper.by ) * upper.reliability;
			denominator += upper.reliability;
		} else {
			double lamda = position / whole;
			result.x += ( ( wave[t] * upper.cx + upper.bx ) * upper.reliability * lamda + ( wave[t] * lower.cx + lower.bx ) * lower.reliability * ( 1 - lamda ) );
			result.y += ( ( wave[t] * upper.cy + upper.by ) * upper.reliability * lamda + ( wave[t] * lower.cy + lower.by ) * lower.reliability * ( 1 - lamda ) );
			denominator += ( upper.reliability * lamda + lower.reliability * ( 1 - lamda ) );
		}
	}
	if ( leftCheck && rightCheck ) {
		checkMaximum<int,link*> & lowerCheck = leftCheck;
		checkMinimum<int,link*> & upperCheck = rightCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		if ( whole == 0 ) {
			result.x += ( wave[t] * upper.cx + upper.bx ) * upper.reliability;
			result.y += ( wave[t] * upper.cy + upper.by ) * upper.reliability;
			denominator += upper.reliability;
		} else {
			double lamda = position / whole;
			result.x += ( ( wave[t] * upper.cx + upper.bx ) * upper.reliability * lamda + ( wave[t] * lower.cx + lower.bx ) * lower.reliability * ( 1 - lamda ) );
			result.y += ( ( wave[t] * upper.cy + upper.by ) * upper.reliability * lamda + ( wave[t] * lower.cy + lower.by ) * lower.reliability * ( 1 - lamda ) );
			denominator += ( upper.reliability * lamda + lower.reliability * ( 1 - lamda ) );
		}
	}
	if ( upLeftCheck && downRightCheck ) {
		checkMaximum<int,link*> & lowerCheck = upLeftCheck;
		checkMinimum<int,link*> & upperCheck = downRightCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		if ( whole == 0 ) {
			result.x += ( wave[t] * upper.cx + upper.bx ) * upper.reliability;
			result.y += ( wave[t] * upper.cy + upper.by ) * upper.reliability;
			denominator += upper.reliability;
		} else {
			double lamda = position / whole;
			result.x += ( ( wave[t] * upper.cx + upper.bx ) * upper.reliability * lamda + ( wave[t] * lower.cx + lower.bx ) * lower.reliability * ( 1 - lamda ) );
			result.y += ( ( wave[t] * upper.cy + upper.by ) * upper.reliability * lamda + ( wave[t] * lower.cy + lower.by ) * lower.reliability * ( 1 - lamda ) );
			denominator += ( upper.reliability * lamda + lower.reliability * ( 1 - lamda ) );
		}
	}
	if ( downLeftCheck && upRightCheck ) {
		checkMaximum<int,link*> & lowerCheck = downLeftCheck;
		checkMinimum<int,link*> & upperCheck = upRightCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		if ( whole == 0 ) {
			result.x += ( wave[t] * upper.cx + upper.bx ) * upper.reliability;
			result.y += ( wave[t] * upper.cy + upper.by ) * upper.reliability;
			denominator += upper.reliability;
		} else {
			double lamda = position / whole;
			result.x += ( ( wave[t] * upper.cx + upper.bx ) * upper.reliability * lamda + ( wave[t] * lower.cx + lower.bx ) * lower.reliability * ( 1 - lamda ) );
			result.y += ( ( wave[t] * upper.cy + upper.by ) * upper.reliability * lamda + ( wave[t] * lower.cy + lower.by ) * lower.reliability * ( 1 - lamda ) );
			denominator += ( upper.reliability * lamda + lower.reliability * ( 1 - lamda ) );
		}
	}
	if ( denominator == 0 ) return p;
	result.x /= denominator;
	result.y /= denominator;
	return point2<int>( result.x, result.y );
}
point2<int> trackDocument::interpolateGotoh( int t, const point2<int> & p )
{
	if ( t == baseIndex ) return p;
	point2<double> result( 0, 0 );
	/*
	checkMaximum<int,link*> vlCheck, hlCheck, alCheck, dlCheck;
	checkMinimum<int,link*> vuCheck, huCheck, auCheck, duCheck;
	for ( list<link>::iterator it( verticalLinks[p.x] ); it; ++it ) {
		if ( it->by <= p.y ) vlCheck( it->by - p.y, & it() );
		if ( it->by > p.y ) vuCheck( it->by - p.y, & it() );
	}
	for ( list<link>::iterator it( horizontalLinks[p.y] ); it; ++it ) {
		if ( it->bx <= p.x ) hlCheck( it->bx - p.x, & it() );
		if ( it->bx > p.x ) huCheck( it->bx - p.x, & it() );
	}
	for ( list<link>::iterator it( ascentLinks[p.x+p.y] ); it; ++it ) {
		if ( it->bx - it->by <= p.x - p.y ) alCheck( it->bx - it->by - ( p.x - p.y ), & it() );
		if ( it->bx - it->by > p.x - p.y ) auCheck( it->bx - it->by - ( p.x - p.y ), & it() );
	}
	int height = originalImages.first().height;
	for ( list<link>::iterator it( descentLinks[p.x-p.y+height-1] ); it; ++it ) {
		if ( it->by + it->bx <= p.y + p.x ) dlCheck( it->bx + it->by - ( p.y + p.x ), & it() );
		if ( it->by + it->bx > p.y + p.x ) duCheck( it->bx + it->by - ( p.y + p.x ), & it() );
	}
	double denominator = 0;
	link v, h, a, d;
	v.reliability = 0;
	h.reliability = 0;
	a.reliability = 0;
	d.reliability = 0;
	if ( vlCheck && vuCheck ) {
		checkMaximum<int,link*> & lowerCheck = vlCheck;
		checkMinimum<int,link*> & upperCheck = vuCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		double lamda = position / whole;
		v.bx = upper.bx * lamda + lower.bx * ( 1 - lamda );
		v.by = upper.by * lamda + lower.by * ( 1 - lamda );
		v.cx = upper.cx * lamda + lower.cx * ( 1 - lamda );
		v.cy = upper.cy * lamda + lower.cy * ( 1 - lamda );
		v.reliability = 1;
	}
	if ( hlCheck && huCheck ) {
		checkMaximum<int,link*> & lowerCheck = hlCheck;
		checkMinimum<int,link*> & upperCheck = huCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		double lamda = position / whole;
		h.bx = upper.bx * lamda + lower.bx * ( 1 - lamda );
		h.by = upper.by * lamda + lower.by * ( 1 - lamda );
		h.cx = upper.cx * lamda + lower.cx * ( 1 - lamda );
		h.cy = upper.cy * lamda + lower.cy * ( 1 - lamda );
		h.reliability = 1;
	}
	if ( dlCheck && duCheck ) {
		checkMaximum<int,link*> & lowerCheck = dlCheck;
		checkMinimum<int,link*> & upperCheck = duCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		double lamda = position / whole;
		d.bx = upper.bx * lamda + lower.bx * ( 1 - lamda );
		d.by = upper.by * lamda + lower.by * ( 1 - lamda );
		d.cx = upper.cx * lamda + lower.cx * ( 1 - lamda );
		d.cy = upper.cy * lamda + lower.cy * ( 1 - lamda );
		d.reliability = 1;
	}
	if ( alCheck && auCheck ) {
		checkMaximum<int,link*> & lowerCheck = alCheck;
		checkMinimum<int,link*> & upperCheck = auCheck;
		link & lower = * lowerCheck.sub;
		link & upper = * upperCheck.sub;
		double position = 0 - lowerCheck();
		double whole = upperCheck() - lowerCheck();
		double lamda = position / whole;
		a.bx = upper.bx * lamda + lower.bx * ( 1 - lamda );
		a.by = upper.by * lamda + lower.by * ( 1 - lamda );
		a.cx = upper.cx * lamda + lower.cx * ( 1 - lamda );
		a.cy = upper.cy * lamda + lower.cy * ( 1 - lamda );
		a.reliability = 1;
	}
	checkMinimum<double,int> first;
	if ( v.reliability ) first( maximum<double>( fabs( v.cx ), fabs( v.cy ) ), 0 );
	if ( h.reliability ) first( maximum<double>( fabs( h.cx ), fabs( h.cy ) ), 1 );
	if ( a.reliability ) first( maximum<double>( fabs( a.cx ), fabs( a.cy ) ), 2 );
	if ( d.reliability ) first( maximum<double>( fabs( d.cx ), fabs( d.cy ) ), 3 );
	if ( ! first ) return p;
	link f, n;
	switch ( first.sub ) {
	case 0: f = v; break;
	case 1: f = h; break;
	case 2: f = a; break;
	case 3: f = d; break;
	}
	checkMinimum<double,int> next;
	if ( first.sub != 0 && v.reliability ) next( maximum<double>( fabs( v.cx ), fabs( v.cy ) ), 0 );
	if ( first.sub != 1 && h.reliability ) next( maximum<double>( fabs( h.cx ), fabs( h.cy ) ), 1 );
	if ( first.sub != 2 && a.reliability ) next( maximum<double>( fabs( a.cx ), fabs( a.cy ) ), 2 );
	if ( first.sub != 3 && d.reliability ) next( maximum<double>( fabs( d.cx ), fabs( d.cy ) ), 3 );
	if ( ! next ) {
		result.x = ( wave[t] * f.cx + f.bx );
		result.y = ( wave[t] * f.cy + f.by );
		return point2<int>( result.x, result.y );
	}
	switch ( next.sub ) {
	case 0: n = v; break;
	case 1: n = h; break;
	case 2: n = a; break;
	case 3: n = d; break;
	}
	result.x = ( wave[t] * ( f.cx + n.cx ) + ( f.bx + n.bx ) ) / 2.0;
	result.y = ( wave[t] * ( f.cy + n.cy ) + ( f.by + n.by ) ) / 2.0;
	*/
	return point2<int>( result.x, result.y );
}
point2<int> trackDocument::interpolateNear( int t, const point2<int> & p )
{
//	if ( t == baseIndex ) return p;
	list<link*> nears;
	const int radius = 3;
	const int radius2 = radius * radius;
	for ( int i = p.x - radius; i < p.x + radius; ++i ) {
		if ( ! between( 0, i, verticalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( verticalLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	for ( int i = p.y - radius; i < p.y + radius; ++i ) {
		if ( ! between( 0, i, horizontalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( horizontalLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	for ( int i = p.x + p.y - radius; i < p.x + p.y + radius; ++i ) {
		if ( ! between( 0, i, ascentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( ascentLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	for ( int i = p.x-p.y+height-1 - radius; i < p.x-p.y+height-1 + radius; ++i ) {
		if ( ! between( 0, i, descentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( descentLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	point2<double> result( 0, 0 );
	double denominator = 0;
	checkMaximum<double> maxWeight;
	for ( list<link*>::iterator it( nears ); it; ++it ) {
		link & lnk = *it();
		double length = ::sqrt( (double) square<int>( lnk.bx - p.x ) + square<int>( lnk.by - p.y ) );
		double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
		maxWeight( weight );
		result.x += ( lnk.cx * wave[t] + p.x ) * weight;
		result.y += ( lnk.cy * wave[t] + p.y ) * weight;
		denominator += weight;
	}
	if ( denominator == 0 || ! maxWeight ) return p;
	double weight = clamp<double>( 0, 1.5 - maxWeight(), 1 ) * denominator * 0;
	denominator += weight;
	result.x += p.x * weight;
	result.y += p.y * weight;
	result.x /= denominator;
	result.y /= denominator;
	return point2<int>( result.x, result.y );
}

point2<int> trackDocument::interpolateNearest( int t, const point2<int> & p )
{
	if ( t == baseIndex ) return p;
	list<link*> nears;
	const int radius = 4;
	const int radius2 = radius * radius;
	double sumv = 0, sumh = 0, suma = 0, sumd = 0;
	point2<double> pv( 0, 0 ), ph( 0, 0 ), pa( 0, 0 ), pd( 0, 0 );
	for ( int i = p.x - radius; i < p.x + radius; ++i ) {
		if ( ! between( 0, i, verticalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( verticalLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			double weight = it->link::reliability + it->link::intensity;
			pv.x += ( it->cx * wave[t] + p.x ) * weight;
			pv.y += ( it->cy * wave[t] + p.y ) * weight;
			sumv += weight;
		}
	}
	for ( int i = p.y - radius; i < p.y + radius; ++i ) {
		if ( ! between( 0, i, horizontalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( horizontalLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			double weight = it->link::reliability + it->link::intensity;
			ph.x += ( it->cx * wave[t] + p.x ) * weight;
			ph.y += ( it->cy * wave[t] + p.y ) * weight;
			sumh += weight;
		}
	}
	for ( int i = p.x + p.y - radius; i < p.x + p.y + radius; ++i ) {
		if ( ! between( 0, i, ascentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( ascentLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			double weight = it->link::reliability + it->link::intensity;
			pa.x += ( it->cx * wave[t] + p.x ) * weight;
			pa.y += ( it->cy * wave[t] + p.y ) * weight;
			suma += weight;
		}
	}
	for ( int i = p.x-p.y+height-1 - radius; i < p.x-p.y+height-1 + radius; ++i ) {
		if ( ! between( 0, i, descentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( descentLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			double weight = it->link::reliability + it->link::intensity;
			pd.x += ( it->cx * wave[t] + p.x ) * weight;
			pd.y += ( it->cy * wave[t] + p.y ) * weight;
			sumd += weight;
		}
	}
	checkMaximum<double,point2<double>*> chk;
	chk( sumv, & pv );
	chk( sumh, & ph );
	chk( suma, & pa );
	chk( sumd, & pd );
	if ( chk() == 0 ) return p;
	return point2<int>( chk.sub->x / chk(), chk.sub->y / chk() );
}

point2<int> trackDocument::interpolateVerticalEdge( int t, const point2<int> & now, const point2<int> & prev, const point2<int> & next )
{
	const point2<int> & p = now;
//	if ( t == baseIndex ) return p;

	vector2 v1( prev.x - now.x, prev.y - now.y );
	vector2 v2( next.x - now.x, next.y - now.y );
	vector2 vert = vector2::normalize2( vector2::normalize2( v1 ) + vector2::normalize2( v2 ) );
	const double one_par_sqrt2 = ::sqrt( 0.5 );
	double wv = absolute<double>( vector2::dot2( vector2( 0, 1 ), vert ) );
	double wh = absolute<double>( vector2::dot2( vector2( 1, 0 ), vert ) );
	double wa = absolute<double>( vector2::dot2( vector2( one_par_sqrt2, - one_par_sqrt2 ), vert ) );
	double wd = absolute<double>( vector2::dot2( vector2( one_par_sqrt2, one_par_sqrt2 ), vert ) );
//	wv *= wv; wh *= wh; wa *= wa; wd *= wd;
	
	const int radius = 5;
	const int radius2 = radius * radius;
	point2<double> result( 0, 0 );

	checkMaximum<double,link*> weightedLink;
	for ( int i = p.x - radius; i < p.x + radius; ++i ) {
		if ( ! between( 0, i, verticalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( verticalLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weightedLink( weight * wv, & lnk );
		}
	}
	for ( int i = p.y - radius; i < p.y + radius; ++i ) {
		if ( ! between( 0, i, horizontalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( horizontalLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weightedLink( weight * wh, & lnk );
		}
	}
	for ( int i = p.x + p.y - radius; i < p.x + p.y + radius; ++i ) {
		if ( ! between( 0, i, ascentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( ascentLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weightedLink( weight * wa, & lnk );
		}
	}
	for ( int i = p.x-p.y+height-1 - radius; i < p.x-p.y+height-1 + radius; ++i ) {
		if ( ! between( 0, i, descentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( descentLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weightedLink( weight * wd, & lnk );
		}
	}
	if ( ! weightedLink ) return p;
	result.x = weightedLink.sub->cx * wave[t] + p.x;
	result.y = weightedLink.sub->cy * wave[t] + p.y;
	/*
	double denominator = 0;
	checkMaximum<double> maxWeight;
	for ( int i = p.x - radius; i < p.x + radius; ++i ) {
		if ( ! between( 0, i, verticalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( verticalLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wv;
			maxWeight( weight );
			result.x += ( lnk.cx * wave[t] + p.x ) * weight;
			result.y += ( lnk.cy * wave[t] + p.y ) * weight;
			denominator += weight;
		}
	}
	for ( int i = p.y - radius; i < p.y + radius; ++i ) {
		if ( ! between( 0, i, horizontalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( horizontalLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wh;
			maxWeight( weight );
			result.x += ( lnk.cx * wave[t] + p.x ) * weight;
			result.y += ( lnk.cy * wave[t] + p.y ) * weight;
			denominator += weight;
		}
	}
	for ( int i = p.x + p.y - radius; i < p.x + p.y + radius; ++i ) {
		if ( ! between( 0, i, ascentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( ascentLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wa;
			maxWeight( weight );
			result.x += ( lnk.cx * wave[t] + p.x ) * weight;
			result.y += ( lnk.cy * wave[t] + p.y ) * weight;
			denominator += weight;
		}
	}
	for ( int i = p.x-p.y+height-1 - radius; i < p.x-p.y+height-1 + radius; ++i ) {
		if ( ! between( 0, i, descentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( descentLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wd;
			maxWeight( weight );
			result.x += ( lnk.cx * wave[t] + p.x ) * weight;
			result.y += ( lnk.cy * wave[t] + p.y ) * weight;
			denominator += weight;
		}
	}
	if ( denominator == 0 || ! maxWeight ) return p;
	double weight = clamp<double>( 0, 1.5 - maxWeight(), 1 ) * denominator * 0;
	if ( weight ) {
		denominator += weight;
		result.x += p.x * weight;
		result.y += p.y * weight;
	}
	result.x /= denominator;
	result.y /= denominator;
	*/
	return point2<int>( result.x, result.y );
}

point2<double> trackDocument::coefficientVerticalEdge( const point2<int> & now, const point2<int> & prev, const point2<int> & next )
{
	const point2<int> & p = now;

	vector2 v1( prev.x - now.x, prev.y - now.y );
	vector2 v2( next.x - now.x, next.y - now.y );
	vector2 vert = vector2::normalize2( vector2::normalize2( v1 ) + vector2::normalize2( v2 ) );
	const double one_par_sqrt2 = ::sqrt( 0.5 );
	double wv = absolute<double>( vector2::dot2( vector2( 0, 1 ), vert ) );
	double wh = absolute<double>( vector2::dot2( vector2( 1, 0 ), vert ) );
	double wa = absolute<double>( vector2::dot2( vector2( one_par_sqrt2, - one_par_sqrt2 ), vert ) );
	double wd = absolute<double>( vector2::dot2( vector2( one_par_sqrt2, one_par_sqrt2 ), vert ) );
//	wv *= wv; wh *= wh; wa *= wa; wd *= wd;
	
	const int radius = 5;
	const int radius2 = radius * radius;

	double denominator = 0;
	point2<double> result( 0, 0 );
	checkMaximum<double> maxWeight;
	for ( int i = p.x - radius; i < p.x + radius; ++i ) {
		if ( ! between( 0, i, verticalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( verticalLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wv;
			maxWeight( weight );
			result.x += lnk.cx * weight;
			result.y += lnk.cy * weight;
			denominator += weight;
		}
	}
	for ( int i = p.y - radius; i < p.y + radius; ++i ) {
		if ( ! between( 0, i, horizontalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( horizontalLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wh;
			maxWeight( weight );
			result.x += lnk.cx * weight;
			result.y += lnk.cy * weight;
			denominator += weight;
		}
	}
	for ( int i = p.x + p.y - radius; i < p.x + p.y + radius; ++i ) {
		if ( ! between( 0, i, ascentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( ascentLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wa;
			maxWeight( weight );
			result.x += lnk.cx * weight;
			result.y += lnk.cy * weight;
			denominator += weight;
		}
	}
	for ( int i = p.x-p.y+height-1 - radius; i < p.x-p.y+height-1 + radius; ++i ) {
		if ( ! between( 0, i, descentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( descentLinks[i] ); it; ++it ) {
			double length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			link & lnk = it();
			length = ::sqrt( length );
			double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
			weight *= wd;
			maxWeight( weight );
			result.x += lnk.cx * weight;
			result.y += lnk.cy * weight;
			denominator += weight;
		}
	}
	if ( denominator == 0 || ! maxWeight ) {
		return analysisBearings( p );
		//return point2<double>( 0, 0 );
	}
	double weight = clamp<double>( 0, 1.5 - maxWeight(), 1 ) * denominator * 0;
	if ( weight ) {
		denominator += weight;
	}
	result.x /= denominator;
	result.y /= denominator;
	return result;
}
void trackDocument::analysisFlicker( int direction )
{
	array2<flicker> & fs = flickers[direction];
	if ( fs.empty() ) return;
	for ( int y = 0; y < fs.h; ++y ) {
		for ( int x = 0; x < fs.w; ++x ) {
			double moment1 = 0;
			double moment2 = 0;
			int count = 0;
			flicker & f = fs( x, y );
			for ( int t = 0; t < this->originalImages.size; ++t ) {
				point2<int> pos = f.lnk.position( t, wave );
				if ( ! between<int>( 0, pos.x, width - 1 ) || ! between<int>( 0, pos.y, height - 1 ) ) continue;
				double v = originalImages[t].get( pos.x, pos.y ).y / double( maxValues[t] );
				moment1 += v;
				moment2 += square( v );
				++count;
			}
			double average = moment1 / count;
			double variance = moment2 / count - square<double>( average );
			double standardDeviation = ::sqrt( variance );
			count = 0;
			bool upper = false;
			bool first = true;
			for ( int t = 0; t < originalImages.size; ++t ) {
				const point2<int> & pos = f.lnk.position( t, wave );
				if ( ! between<int>( 0, pos.x, width - 1 ) || ! between<int>( 0, pos.y, height - 1 ) ) continue;
				double v = originalImages[t].get( pos.x, pos.y ).y / double( maxValues[t] );
				if ( ! first ) {
					upper = v >= average;
					first = false;
				} else {
					if ( upper && v < average - standardDeviation * 2.0 / 3.0 ) {
						upper = false;
						++count;
					} else if ( ! upper && v >= average + standardDeviation * 2.0 / 3.0 ) {
						upper = true;
						++count;
					}
				}
			}
			//if ( average > 1.0 / 20.0 ) {
				f.value = clamp<double>( 0, zerodivide<double>( count, sizeTime() - 1 ), 1 ) *
					clamp<double>( 0, standardDeviation / average * 10, 1 ) * 
					clamp<double>( 0, average * 20, 1 );
			//} else {
			//	f.value = 0;
			//}
		}
	}
	imageWindow->invalidate();
	imageWindow->update();
}

point2<double> trackDocument::analysisNear( const point2<int> & p )
{
	list<link*> nears;
	const int radius = 3;
	const int radius2 = radius * radius;
	for ( int i = p.x - radius; i <= p.x + radius; ++i ) {
		if ( ! between( 0, i, verticalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( verticalLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	for ( int i = p.y - radius; i <= p.y + radius; ++i ) {
		if ( ! between( 0, i, horizontalLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( horizontalLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	for ( int i = p.x + p.y - radius; i <= p.x + p.y + radius; ++i ) {
		if ( ! between( 0, i, ascentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( ascentLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	for ( int i = p.x - p.y + height-1 - radius; i <= p.x - p.y + height-1 + radius; ++i ) {
		if ( ! between( 0, i, descentLinks.size - 1 ) ) continue;
		for ( list<link>::iterator it( descentLinks[i] ); it; ++it ) {
			int length = square( it->bx - p.x ) + square( it->by - p.y );
			if ( length > radius2 ) continue;
			nears.push_back( & it() );
		}
	}
	checkMaximum<double,link*> nearest;
	for ( list<link*>::iterator it( nears ); it; ++it ) {
		link & lnk = *it();
		//const double dist = ::sqrt( square<int>( lnk.bx - p.x ) + square<int>( lnk.by - p.y ) );
		nearest( lnk.intensity, & lnk );
		//nearest( lnk.intensity / exp( dist ), & lnk );
		//nearest( lnk.intensity * pow( 0.8, dist ), & lnk );
	}
	if ( ! nearest ) {
		return analysisBearings( p );
	}
	return point2<double>( nearest.sub->cx, nearest.sub->cy );

	/*
	point2<double> result( 0, 0 );
	double denominator = 0;
	checkMaximum<double> maxWeight;
	for ( list<link*>::iterator it( nears ); it; ++it ) {
		link & lnk = *it();
		double length = ::sqrt( square<int>( lnk.bx - p.x ) + square<int>( lnk.by - p.y ) );
		double weight = ( lnk.intensity + lnk.reliability ) / maximum<double>( 1, length );
		maxWeight( weight );
		result.x += lnk.cx * weight;
		result.y += lnk.cy * weight;
		denominator += weight;
	}
	if ( denominator == 0 || ! maxWeight ) {
		return analysisBearings( p );
	}
	double weight = clamp<double>( 0, 1.5 - maxWeight(), 1 ) * denominator;
	denominator += weight;
	result.x /= denominator;
	result.y /= denominator;
	return result;
	*/
}
static bool export_average = false;
double trackDocument::analysisDoC( trackDocument & cdoc, retainer<file> & f )
{
	//正解率を出力する
	double orMinusAndArea = 0;
	double csArea = 0;
	double orArea = 0;
	double andArea = 0;
	for ( list< boundaryArea >::iterator it( areas ); it; ++it ) {
		for ( list< boundaryPart >::iterator itp( it->parts ); itp; ++itp ) {
			boundaryPart & xp = itp();
			int type = xp.type;
			boundaryPart * p = NULL;
			for ( list< boundaryArea >::iterator itca( cdoc.areas ); itca && ! p; ++itca ) {
				for ( list< boundaryPart >::iterator itcp( itca->parts ); itcp && ! p; ++itcp ) {
					boundaryPart & cp = itcp();
					if ( xp.type == cp.type ) {
						p = & cp;
						break;
					}
				}
			}
			if ( ! p ) continue;
			boundaryPart & cp = *p;
			static char * name[] = { "右上葉", "右中葉", "右下葉", "左上葉", "左下葉" };
			if ( ! export_average ) {
				if ( f ) {
					f->printf( "%s\n", name[type] );
					f->printf( "time,correct area,answer area,degree of coincidence,shape difference\n" );
					for ( int t = 0; t < sizeTime(); ++t ) {
						region<int> & xs = xp.shapes[t];
						region<int> & cs = cp.shapes[t];
						region<int> and = xs & cs;
						region<int> or = xs | cs;
						f->printf( "%d,%f,%f,%f,%f\n", t, cs.area(), xs.area(), 
							zerodivide( and.area(), or.area() ) * 100.0,
							zerodivide( ( or - and ).area(), cs.area() ) * 100.0 );
					}
					f->printf( "\n" );
				}
			} else {
				for ( int t = 0; t < sizeTime(); ++t ) {
					region<int> & xs = xp.shapes[t];
					region<int> & cs = cp.shapes[t];
					region<int> and = xs & cs;
					region<int> or = xs | cs;
					orArea += or.area();
					andArea += and.area();
					orMinusAndArea += ( or - and ).area();
					csArea += cs.area();
				}
			}
		}
	}
	if ( export_average && f ) {
		f->printf( "DoC,%f,SD,%f\n", 
			zerodivide( andArea, orArea ) * 100.0,
			zerodivide( orMinusAndArea, csArea ) * 100.0 );
	}
	if ( f ) f->flush();
	return zerodivide( andArea, orArea ) * 100.0;
}



void trackDocument::analysisDoC( const char * correct, const char * output )
{
	{ for ( list< boundaryArea >::iterator it( areas ); it; ++it ) it->update( -1 ); }
	retainer<file> f = file::initialize( output, true, false );
	trackDocument cdoc;
	cdoc = *this;
	cdoc.loadXML( correct, true );
	analysisDoC( cdoc, f );
	//return;//下は論文用データ作成のため用いない
	//作業毎に変化していく誤差のグラフを出力する
	export_average = true;
	int now = currentViewImageIndex;

	for ( list< boundaryArea >::iterator itn( areas ); itn; ++itn ) {
		itn->enable = true;
		itn->updatePoint( false, true, true );
		itn->update( -1 );
	}
	imageWindow->update();
	analysisDoC( cdoc, f );
	while ( true ) {
		checkMaximum< double, const point2<int> * > maxdoc;
		list< boundaryArea >::iterator itn( areas );
		list< boundaryArea >::iterator itc( cdoc.areas );
		for ( ; itn && itc; ++itn, ++itc ) {
			list< boundaryPoint* >::iterator itnp( itn->controlPoints );
			list< boundaryPoint* >::iterator itcp( itc->controlPoints );
			for ( ; itnp && itcp; ++itnp, ++itcp ) {
				for ( int t = 0; t < itnp()->points.size; ++t ) {
					const point2<int> & cp = ( * ( itcp() ) )( t );
					const point2<int> & np = ( * ( itnp() ) )( t );
					if ( itcp()->type( t ) == boundaryPointFrame::typeEdited &&
						itnp()->type( t ) == boundaryPointFrame::typeInterpolated ) {
					} else if ( itcp()->type( t ) == boundaryPointFrame::typeInitial &&
						itnp()->type( t ) == boundaryPointFrame::typeInitial ) {
						if ( cp.x == np.x && cp.y == np.y ) continue;
					} else {
						continue;
					}
					int ci, pi;
					const point2<int> wk = np;
					if ( itn->find( ci, pi, itnp() ) ) {
						//ためしに動かす
						currentViewImageIndex = t;
						itn->movePoint( ci, pi, t, cp );
						itn->enable = true;
						itnp()->update( false, true, true );
						itn->touch( -1 );
						itn->update( -1 );
						imageWindow->update();
						maxdoc( analysisDoC( cdoc, retainer<file>() ), & cp );
						itn->undoPoint( ci, pi, t, wk );
						itn->enable = true;
						itnp()->update( false, false, true );
						itn->touch( -1 );
					}
				}
			}
		}
		{
			for ( list< boundaryArea >::iterator itn( areas ); itn; ++itn ) {
				itn->enable = true;
				itn->update( -1 );
			}
		}
		imageWindow->update();
		if ( ! maxdoc ) break;
		itn = list< boundaryArea >::iterator( areas );
		itc = list< boundaryArea >::iterator( cdoc.areas );
		for ( ; itn && itc; ++itn, ++itc ) {
			list< boundaryPoint* >::iterator itnp( itn->controlPoints );
			list< boundaryPoint* >::iterator itcp( itc->controlPoints );
			for ( ; itnp && itcp; ++itnp, ++itcp ) {
				for ( int t = 0; t < itnp()->points.size; ++t ) {
					const point2<int> & cp = ( * ( itcp() ) )( t );
					const point2<int> & np = ( * ( itnp() ) )( t );
					if ( itcp()->type( t ) == boundaryPointFrame::typeEdited &&
						itnp()->type( t ) == boundaryPointFrame::typeInterpolated ) {
					} else if ( itcp()->type( t ) == boundaryPointFrame::typeInitial &&
						itnp()->type( t ) == boundaryPointFrame::typeInitial ) {
						if ( cp.x == np.x && cp.y == np.y ) continue;
					} else {
						continue;
					}
					if ( maxdoc.sub == & cp ) {
						//最大に動いているポイントが見つかった．
						int ci, pi;
						if ( itn->find( ci, pi, itnp() ) ) {
							currentViewImageIndex = t;
							itn->movePoint( ci, pi, t, cp );
							itn->enable = true;
							itnp()->update( false, true, true );
							itn->touch( -1 );
							itn->update( -1 );
						}
						maxdoc.initialize();
						break;
					}
				}
				if ( ! maxdoc ) break;
			}
			if ( ! maxdoc ) break;
		}
		analysisDoC( cdoc, f );
	}
	currentViewImageIndex = now;
	export_average = false;
}

