#include "main.h"
#include "document.h"
#include "bmptool.inl"

inline double directionFromHough( const int d, double resolutionDirection )
{
	return d / resolutionDirection * pi;
}
inline double amplitudeFromHough( const int a, double halfResolutionAmplitude, double unitAmplitude )
{
	return ( a - halfResolutionAmplitude ) * unitAmplitude;
}

point2<double> trackDocument::analysisBearings( const point2<int> & p )
{
	//論文用画像生成
	const bool outputBMP = false;
	//const_cast<int&>( p.x ) = 86;
	//const_cast<int&>( p.y ) = 173;
	const int resolutionDirection = 100;
	const int _resolutionAmplitude = 100;

	const double rangeAmplitude = 2.0;
	const double unitAmplitude = rangeAmplitude / ( _resolutionAmplitude / 2 );
	const int halfResolutionAmplitude = round<int>( rangeAmplitude / unitAmplitude );
	const int resolutionAmplitude = 2 * halfResolutionAmplitude + 1;
	
	static array2<statisticsValue> hough;
	hough.allocate( resolutionAmplitude, resolutionDirection );

	//点周りの統計量計算
	for ( int d = 0; d < resolutionDirection; ++d ) {
		const double angle = directionFromHough( d, resolutionDirection );
		const double cos_angle = ::cos( angle );
		const double sin_angle = ::sin( angle );
		for ( int a = 0; a < resolutionAmplitude; ++a ) {
			const double amp = amplitudeFromHough( a, halfResolutionAmplitude, unitAmplitude );
			statisticsValue & sv = hough( a, d );
			sv.initialize();
			for ( int t = 0; t < wave.size; ++t ) {
				const int & wt = wave[t];
				int x = round<int>( amp * cos_angle * wt + p.x );
				int y = round<int>( amp * sin_angle * wt + p.y );
				if ( ! between<int>( 0, x, width - 1 ) || 
					! between<int>( 0, y, height - 1 ) ) continue;
				imageInterface< pixelLuminance<int16> > & img = originalImages[t];
				int16 v = img.getInternal( x, y ).y;
				int16 xe = horizontalEdgeImages[t].getInternal( x, y ).y;
				int16 ye = verticalEdgeImages[t].getInternal( x, y ).y;
				double e = absolute( xe * cos_angle + ye * sin_angle );
				if ( t == baseIndex ) {
					sv.baseEdge = e;
				}
				sv.append( v, e );
			}
			sv.finish();
		}
	}
	string outputfilehead = string( "D:\\temp\\T(" ) + string( p.x ) + "-" + string( p.y ) + ")";

	static array2<double> output;
	output.allocate( hough.w, hough.h );
	if ( outputBMP ) {
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.valueAverage();
			}
		}
		saveBmp( output, outputfilehead + "value_average.bmp" );
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.valueStandardDeviation();
			}
		}
		saveBmp( output, outputfilehead + "value_SD.bmp" );
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.edgeAverage();
			}
		}
		saveBmp( output, outputfilehead + "edge_average.bmp" );
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.edgeStandardDeviation();
			}
		}
		saveBmp( output, outputfilehead + "edge_SD.bmp" );
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.valueConstancy();
			}
		}
		saveBmp( output, outputfilehead + "value_constancy.bmp" );
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.edgeConstancy();
			}
		}
		saveBmp( output, outputfilehead + "edge_constancy.bmp" );
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.constancy();
			}
		}
		saveBmp( output, outputfilehead + "constancy.bmp" );
		for ( int d = 0; d < resolutionDirection; ++d ) {
			for ( int a = 0; a < resolutionAmplitude; ++a ) {
				statisticsValue & sv = hough( a, d );
				output( a, d ) = sv.likelyhood();
			}
		}
		saveBmp( output, outputfilehead + "likelyhood.bmp" );
	}
	//動きを抽出
	checkMaximum<double> found;
	array< checkMaximum<double> > maxConstancyEachDirection;
	maxConstancyEachDirection.allocate( resolutionDirection );
	checkMaximum<double> maxConstancyAllDirection;
	for ( int d = 0; d < resolutionDirection; ++d ) {
		checkMaximum<double> & mc = maxConstancyEachDirection[d];
		for ( int a = 0; a < resolutionAmplitude; ++a ) {
			statisticsValue & sv = hough( a, d );
			//double v = sv.likelyhood();
			const double amplitude = amplitudeFromHough( a, halfResolutionAmplitude, unitAmplitude );
			double v = sv.likelyhood() * pow( 0.9, absolute( amplitude ) );
			if ( outputBMP ) output( a, d ) = v;
			mc( v, a );
			maxConstancyAllDirection( v, d );
		}
	}
	double th = maxConstancyAllDirection() * 0.99;
	checkMinimum<double> nearDirection;
	for ( int d = 0; d < resolutionDirection; ++d ) {
		checkMaximum<double> & mc = maxConstancyEachDirection[d];
		if ( mc() < th ) continue;
		const double amplitude = amplitudeFromHough( mc.sub, halfResolutionAmplitude, unitAmplitude );
		nearDirection( absolute( amplitude ), d );
	}
	if ( ! nearDirection ) return point2<double>( 0, 0 );

	int directionIndex = maxConstancyAllDirection.sub;
	int amplitudeIndex = maxConstancyEachDirection[directionIndex].sub;
	/*
	int directionIndex = nearDirection.sub;
	int amplitudeIndex = maxConstancyEachDirection[nearDirection.sub].sub;
	*/
	if ( outputBMP ) saveBmp( output, outputfilehead + "result D" + string( directionIndex ) + " A" + string( amplitudeIndex ) + ".bmp" );


	//値に直す
	const double direction = directionFromHough( directionIndex, resolutionDirection );
	const double amplitude = amplitudeFromHough( amplitudeIndex, halfResolutionAmplitude, unitAmplitude );

	return point2<double>( amplitude * ::cos( direction ), amplitude * ::sin( direction ) );
}
