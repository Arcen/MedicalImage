//ガウスの消去法：m*x=bからxを解く
template<typename T>
bool gaussianElimination( const matrixT<T> & m, const vectorT<T> & b, vectorT<T> & x )
{
	matrixT<T> lu;//計算用
	vectorT<int> exchange;//ピボット選択のためのテーブル　行を入れ替える
	if ( ! m.luFactorization( lu, exchange ) ) return false;
	if ( ! lu.luSolve( exchange, b, x ) ) return false;
	return true;
}

//マルカート法(Ｊt Ｊ+λＤ)δ=-Ｊt Ｅ
template<typename T>//N:パラメータ数,E:誤差の数
class levenbergMarquardt
{
public:
	T tiny;//微分を行うときの微小量
	T final_error;//誤差の終了条件
	matrixT<T> scaleParameter;//スケーリング対角行列
	typedef T ( * functionError )( vectorT<T> & error, const vectorT<T> & parameter, void * ext );
	void initialize( const int N, T final_error = T( 10e-8 ), T tiny = 1 )
	{
		this->tiny = tiny;
		this->final_error = final_error;
		scaleParameter = matrixT<T>::identity( N );
	}
	//ヤコビアン
	void jacobian( matrixT<T> & jcbn, const vectorT<T> & parameter, const vectorT<T> & error, functionError functor, void * ext )
	{
		const int N = parameter.dimension();
		const int E = error.dimension();
		jcbn = matrixT<T>::zero( E, N );
		//行数：誤差の要素数，列数：パラメータ数
		//ヤコビアン：誤差のベクトル関数の各要素を各パラメータで微分した値を要素として持つ行列
		vectorT<T> pi( N );
		vectorT<T> ei( E );
		for ( int i = 0; i < N; ++i ) {
			pi = parameter;
			pi[i] += tiny;
			functor( ei, pi, ext );
			for ( int j = 0; j < E; ++j ) {
				jcbn[j][i] = ( ei[j] - error[j] ) / tiny;
			}
		}
	}
	//marquardt  戻り値がtrueならば，終了
	bool operator()( vectorT<T> & parameter, functionError functor, void * ext, T lamda = T( 0.1 ) )
	{
		const int N = parameter.dimension();

		T scale = T( 10e-10 );//ラムダの最大ゆれ量．
		//lamda //マルカート法のλの値．
		matrixT<T> jcbn, jcbnT, left, leftInverse, hessian;
		vectorT<T> right, delta, error;
		for ( int t = 0; t < 1; ++t ) {
			//現在のエラーを計算し，終了条件を確認する
			T sum_error = functor( error, parameter, ext );
			const int E = error.dimension();
			if ( sum_error < final_error ) {
				return true;
			}
			//ヤコビアン計算
			jacobian( jcbn, parameter, error, functor, ext );

			jcbnT = matrixT<T>::transpose( jcbn );//転置計算

			//ヘッシアン計算
			// 積計算だが，バグのため%にて，正方行列以外の行列の積計算をおこなっている．
			hessian = jcbnT * jcbn;//誤差を含む 
			//行列式を(次元）乗根計算してλに掛ける
			T det = matrixT<T>::determinant( hessian );//正方行列なので，行列式が計算可能
			T sgn = sign( det );//負になる場合に符号を保存しておく
			if ( det == 0 || ! _isnan( det ) ) det = 1;//行列式が不正の場合には，影響を取り除く
			det = absolute( det );
			T powereddet = sgn * pow( det, 1.0 / N );//スケールパラメータへの係数を計算
			if ( ! _isnan( powereddet ) ) powereddet = 1;//行列式が不正の場合には，影響を取り除く

			//マルカート法(Ｊt Ｊ+λＤ)δ=-Ｊt Ｅ
			//Ｊ：ヤコビアン,Ｊt：転置ヤコビアン
			//λ：lamda,Ｄ：スケールパラメータ
			//δ：降下方向
			//Ｅ：エラー
			int count = 0;//逆行列計算の繰り返し数
			bool ok = false;//逆行列が求まるか？
			do {//逆行列をラムダを揺らしながらもとまる物を選択する
				//逆行列計算が失敗した時などはλに手を入れたりして何とかする。
				//ラムダの値を揺らす
				//lamda = maximum<T>( 0, lamda + scale * ( rand() % 201 - 100 ) / 100.0 );
				lamda += scale * ( ( rand() % 201 - 100 ) / 100.0 );
				//左辺の行列を計算
				left = hessian + scaleParameter * ( lamda * powereddet );
				//逆行列を計算
				if ( left.inverse( leftInverse ) ) ok = true;//逆行列が存在するかチェック
				count++;
				if ( 100 < count ) return true;//失敗
			} while ( ! ok );//逆行列計算の失敗
			//右辺の行列を計算
			right = ( jcbnT * error ) * -1;
			//δを計算
			delta = leftInverse * right;
			//数値が有効かチェック
			if ( ! delta.valid() ) return true;

			//方向の大きさに手を入れて見る

			//方向が逆，若しくは大きすぎて動けないときには小さくする．
			T old_error = sum_error, trial_error;//パラメータを動かすときに，誤差が大きくなる方向に動かないように誤差を保存しておく
			vectorT<T> trialparameter( N );//テスト用のパラメータ
			count = 0;
			do {
				if ( count ) delta *= T( 0.1 );//２回目以降は移動量を少なくする
				trialparameter = parameter + delta;
				trial_error = functor( error, trialparameter, ext );
				//エラーが増える場合反転を試して見る
				if ( old_error <= trial_error ) {
					trialparameter = parameter - delta;
					trial_error = functor( error, trialparameter, ext );
					if ( trial_error < old_error ) delta *= -1;//減ったら，反転
				}
				count++;
			} while ( old_error < trial_error && count < 10 );
			//適当な大きさになった方向へ，最急降下法の様に降下する．
			count = 0;
			while ( trial_error < old_error && count < 10000 ) {
				parameter = trialparameter;//パラメータの更新
				old_error = trial_error;
				//効果の程度が小さいようなら，速度をあげる
				if ( count % 10 == 0 ) delta *= 2.0;
				trialparameter += delta;
				trial_error = functor( error, trialparameter, ext );
				count++;
			};
		}
		return false;
	}
};

