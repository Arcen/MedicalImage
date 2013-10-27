////////////////////////////////////////////////////////////////////////////////
//ガウシアン推定

class gaussianEstimate
{
public:
	enum {
		minimumValue = -4096,//最小値
		maximumValue = 4096,//最大値
		sizeValue = maximumValue - minimumValue + 1,//取り得る範囲の数
	};
	//テーブル
	array<decimal> table;//２つのガウス関数の相関
	decimal sd;
	//テーブル初期化
	void initialize( double var )
	{
        sd = ::sqrt( var );
		//以下の値を計算する
		// 1 / sqrt( 4 pi var ) * exp( - subtract ^ 2 / ( 4 var ) )
		table.allocate( sizeValue );
        const decimal coefficient = 1 / ::sqrt( 4 * pi * var );
		for ( int i = 0; i < sizeValue; ++i ) {
			table[i] = coefficient * exp( - decimal( i ) * decimal( i ) / ( 4 * var ) );
		}
	}
private:
	//２つのガウス関数の相関
	decimal gaussCorrelation( int subtract )
	{
		return table[ subtract ];
	}
public:
	//パターンとガウス関数よりパターンの事前計算テーブルを計算
	void buildup( const array<int16> & pettern, array<decimal> & table )
	{
		table.allocate( sizeValue );
		//相関計算を行う
		for ( int16 v = minimumValue; v <= maximumValue; ++v ) {
			decimal & value = table[v-minimumValue];
			value = 0;
			for ( array<int16>::iterator its( pettern ); its; ++its ) {
				value += gaussCorrelation( abs( v - its() ) );
			}
		}
		//平均にする
		table /= pettern.size;
	}
	//テーブルを片方の自己相関の２乗根で割る
	void normalized( const array<int16> & pettern, array<decimal> & table )
	{
		//自己相関の２乗根で割る
        table /= ::sqrt( self( pettern, table ) );
	}
	//サンプルから自己相関を計算する
	decimal self( const array<int16> & pettern )
	{
		//以下をコメントアウトすると同時確率となる．
		//相関計算を行う
		decimal self = 0;
		for ( array<int16>::iterator it1( pettern ); it1; ++it1 ) {
			const int16 & i1 = it1();
			array<int16>::iterator it2 = it1;
			++it2;
			for ( ; it2; ++it2 ) {
				self += gaussCorrelation( abs( i1 - it2() ) );
			}
		}
		self = self * 2 + gaussCorrelation( 0 ) * pettern.size;
		//平均にする
		return ( self / pettern.size ) / pettern.size;
	}
	//テーブルを使い自己相関を計算する
	decimal self( const array<int16> & pettern, const array<decimal> & table )
	{
		decimal self = 0;
		for ( array<int16>::iterator its( pettern ); its; ++its ) {
			self += table[its()-minimumValue];
		}
		return self / pettern.size;
	}
	//検査する標本値と事前計算テーブルより、ガウシアン推定を計算
	decimal estimate( const int16 & sample, const array<decimal> & table )
	{
		return table[sample-minimumValue];
	}
	//検査する標本と事前計算テーブルより、ガウシアン推定を計算
	decimal estimate( const array<int16> & sample, const array<decimal> & table )
	{
		decimal c = 0;
		for ( array<int16>::iterator it( sample ); it; ++it ) 
#ifdef NDEBUG
			c += table.data[it()-minimumValue];//高速に
#else
			c += estimate( it(), table );
#endif
		return c / sample.size;
	}
};

extern gaussianEstimate ge;

