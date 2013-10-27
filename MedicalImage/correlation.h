////////////////////////////////////////////////////////////////////////////////
//ガウシアンヒストグラム正規化相関計算

class GaussianHistogramNormalizedCorrelation
{
public:
	enum {
		minimumValue = -2048,//最小値
		maximumValue = 2048,//最大値
		sizeValue = maximumValue - minimumValue + 1,//取り得る範囲の数
	};
	//テーブル
	static array<decimal> gaussCorrelationTable;//２つのガウス関数の相関
	static array<decimal> gaussCorrelation2Table;//その２倍
	//テーブル初期化
	static void initialize( double var )
	{
		//以下の値を計算する
		// 1 / sqrt( 4 pi var ) * exp( - subtract ^ 2 / ( 4 var ) )
		gaussCorrelationTable.allocate( sizeValue );
		gaussCorrelation2Table.allocate( sizeValue );
		const decimal coefficient = 1 / sqrt( 4 * pi * var );
		for ( int i = 0; i < sizeValue; ++i ) {
			const decimal v = coefficient * exp( - decimal( i ) * decimal( i ) / ( 4 * var ) );
			gaussCorrelationTable[i] = v;
			gaussCorrelation2Table[i] = v * 2;
		}
	}
private:
	//２つのガウス関数の相関
	static decimal gaussCorrelation( int subtract )
	{
		return gaussCorrelationTable[ subtract ];
	}
	static decimal gaussCorrelation2( int subtract )
	{
		return gaussCorrelation2Table[ subtract ];
	}
public:
	//パターンとガウス関数よりパターンの事前計算テーブルを計算
	//事前計算テーブルはあるパターンのガウシアンヒストグラムと全ての場合の要素とのガウシアンヒストグラム相関値を自己ガウシアンヒストグラム相関の二乗根で割った値
	static void preCalcTable( const array<int16> & pettern, array<decimal> & pattern_pct )
	{
		pattern_pct.allocate( sizeValue );
		//相関計算を行う
		for ( int16 v = minimumValue; v <= maximumValue; ++v ) {
			decimal & f = pattern_pct[v-minimumValue];
			f = 0;
			for ( array<int16>::iterator its( pettern ); its; ++its ) {
				f += gaussCorrelation( abs( v - its() ) );
			}
		}
		//自己ガウシアンヒストグラム相関の二乗根で割る
		const decimal coefficient = 1.0 / sqrt( selfGaussHistogramCorrelation( pettern ) );
		for ( array<decimal>::iterator itg( pattern_pct ); itg; ++itg ) itg() *= coefficient;
	}
	//標本より、自己ガウシアンヒストグラム相関を計算
	static decimal selfGaussHistogramCorrelation( const array<int16> & sample )
	{
		//同じ標本同士での相関は要素数×０での相関
		decimal result = sample.size * gaussCorrelation( 0 );
		//異なる標本同士での相関は、逆に選んだ時にも同じであるので、繰り返さないように２倍で計算する
		for ( array<int16>::iterator it1( sample ); it1; ++it1 ) {
			array<int16>::iterator it2( it1 );
			for ( ++it2; it2; ++it2 ) {
				result += gaussCorrelation2( abs( it1() - it2() ) );
			}
		}
		return result;
	}
	//検査する標本値と事前計算テーブルより、自己ガウシアンヒストグラム相関の２乗根倍のガウシアンヒストグラム正規化相関を計算
	//semiとは正規化の場合より自己相関の二乗根倍であるという意味。
	static decimal seminormalizedGaussHistogramCorrelation( const int16 & sample, const array<decimal> & pattern_pct )
	{
		return pattern_pct[clamp<int>(0,sample-minimumValue,sizeValue)];
	}
	//検査する標本と事前計算テーブルより、自己ガウシアンヒストグラム相関の２乗根倍のガウシアンヒストグラム正規化相関を計算
	static decimal seminormalizedGaussHistogramCorrelation( const array<int16> & sample, const array<decimal> & pattern_pct )
	{
		decimal c = 0;
		for ( array<int16>::iterator it( sample ); it; ++it ) 
			c += seminormalizedGaussHistogramCorrelation( it(), pattern_pct );
		return c;
	}
	//検査する標本と事前計算テーブルより、ガウシアンヒストグラム正規化相関を計算
	static decimal normalizedGaussHistogramCorrelation( const array<int16> & sample, const array<decimal> & pattern_pct )
	{
		return seminormalizedGaussHistogramCorrelation( sample, pattern_pct ) / sqrt( selfGaussHistogramCorrelation( sample ) );
	}
};
