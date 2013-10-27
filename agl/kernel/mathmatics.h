////////////////////////////////////////////////////////////////////////////////
// Mathematics

typedef float decimal;

////////////////////////////////////////////////////////////////////////////////
// Constant
const decimal pi = static_cast<decimal>( 3.1415926535897932384626433832795 );
const decimal pi_2 = pi / 2;
const decimal pi_4 = pi / 4;
const decimal pi_8 = pi / 8;
const decimal pi2 = pi * 2;
const decimal inversePi = 1 / pi;
const decimal rootPi = static_cast<decimal>( 1.7724538509055160272981674833411 );
const decimal log2 = static_cast<decimal>( 0.6931471805599453094172321214581765680755 );
const decimal log10 = static_cast<decimal>( 2.3025850929940456840179914546844 );
const decimal root2 = static_cast<decimal>( 1.4142135623730950488016887242097 );
const decimal infinity = FLT_MAX;//DBL_MAX
const decimal infinitesimal = FLT_MIN;//DBL_MIN
const decimal epsilon = FLT_EPSILON;//DBL_EPSILON
const decimal infinityInt = static_cast<decimal>( INT_MAX );
const decimal infinitesimalInt = static_cast<decimal>( INT_MIN );

const decimal roughEpsilon = static_cast<decimal>( 1e-6 ); // 誤差

////////////////////////////////////////////////////////////////////////////////
// API

//絶対値
template<class T> const T absolute( const T & v ){ return v < 0 ? -v : v; }
//2乗
template<class T> const T square( const T & v ){ return v * v; }
//3乗
template<class T> const T cubic( const T & v ){ return v * v * v; }
//radianからdegree
template<class T> const T rad2deg( const T & rad ){ return rad * 360 / pi2; }
//degreeからradian
template<class T> const T deg2rad( const T & deg ){ return deg * pi2 / 360; }
//0での除算を0とする割り算
template<class T> const T zerodivide( const T & left, const T & right ){ if ( left == 0 || right == 0 ) return 0; return left / right; }
//符号（但し０で１）
template<class T> const T plus( const T & v ){ return v < T( 0 ) ? T( -1 ) : T( 1 ); }

inline const decimal sqrt( const float & value ) { return static_cast<decimal>( ::sqrt( value ) ); }
//inline const decimal sqrt( const double & value ) { return static_cast<decimal>( ::sqrt( value ) ); }
//inline const decimal sqrt( const long double & value ) { return static_cast<decimal>( ::sqrt( ( double ) value ) ); }
inline const decimal sin( const float & value ) { return static_cast<decimal>( ::sin( value ) ); }
inline const decimal sin( const double & value ) { return static_cast<decimal>( ::sin( value ) ); }
inline const decimal sin( const long double & value ) { return static_cast<decimal>( ::sin( ( double ) value ) ); }
inline const decimal cos( const float & value ) { return static_cast<decimal>( ::cos( value ) ); }
inline const decimal cos( const double & value ) { return static_cast<decimal>( ::cos( value ) ); }
inline const decimal cos( const long double & value ) { return static_cast<decimal>( ::cos( ( double ) value ) ); }
inline const decimal tan( const float & value ) { return static_cast<decimal>( ::tan( value ) ); }
inline const decimal tan( const double & value ) { return static_cast<decimal>( ::tan( value ) ); }
inline const decimal tan( const long double & value ) { return static_cast<decimal>( ::tan( ( double ) value ) ); }
inline const decimal asin( const float & value ) { if ( value >= 1 ) return pi_2; if ( value <= -1 ) return pi_2*3; return static_cast<decimal>( ::asin( value ) ); }
inline const decimal asin( const double & value ) { if ( value >= 1 ) return pi_2; if ( value <= -1 ) return pi_2*3; return static_cast<decimal>( ::asin( value ) ); }
inline const decimal asin( const long double & value ) { if ( value >= 1 ) return pi_2; if ( value <= -1 ) return pi_2*3; return static_cast<decimal>( ::asin( ( double ) value ) ); }
inline const decimal acos( const float & value ) { if ( value >= 1 ) return 0; if ( value <= -1 ) return pi; return static_cast<decimal>( ::acos( value ) ); }
inline const decimal acos( const double & value ) { if ( value >= 1 ) return 0; if ( value <= -1 ) return pi; return static_cast<decimal>( ::acos( value ) ); }
inline const decimal acos( const long double & value ) { if ( value >= 1 ) return 0; if ( value <= -1 ) return pi; return static_cast<decimal>( ::acos( ( double ) value ) ); }
inline const decimal atan( const float & value ) { return static_cast<decimal>( ::atan( value ) ); }
inline const decimal atan( const double & value ) { return static_cast<decimal>( ::atan( value ) ); }
inline const decimal atan( const long double & value ) { return static_cast<decimal>( ::atan( ( double ) value ) ); }
//立方根 qube root
inline decimal curt( decimal x ) { return sign( x ) * pow( fabs( x ), static_cast<decimal>( 1.0 / 3.0 ) ); }
//1元方程式
class equation
{
	//ソート
	bool sort( array<decimal> & x )
	{
		//最大４個しか要素が無いので、適当なソート
		if ( x.size ) {
			array<decimal> wk = x;
			checkMaximum<decimal> mx;
			x.release();
			while ( wk.size ) {
				for ( int i = 0; i < wk.size; ++i ) {
					mx( wk[i] );
				}
				x.push_back( mx() );
				wk.pop( mx() );
			}
		}
		return x.size ? true : false;
	}
	//同じ物を取り除く
	bool fix( array<decimal> & x )
	{
		array<decimal> tmp = x;
		x.initialize();
		for ( array<decimal>::iterator it( tmp ); it; ++it ) {
			bool find = false;
			array<decimal>::iterator next = it;
			for ( ++next; next; ++next ) {
				if ( it() == next() ) {
					find = true;
					break;
				}
			}
			if ( ! find ) {
				x.push_back( it() );
			}
		}
		return sort( x );
	}
	//最大の要素数になるように差が小さいものを小さいほうから削除する
	bool fix( array<decimal> & x, int maximum )
	{
		if ( ! fix( x ) ) return false;
		if ( x.size <= maximum ) return true;
		while ( maximum < x.size ) {
			checkMinimum<decimal> minimum;
			for ( int i = 0; i + 1 < x.size; ++i ) {
				minimum( fabs( x[i] - x[i+1] ), i );
			}
			x.popByIndex( minimum.sub );
		}
		return true;
	}
public:
	// ai * x^i = 0を解く（アインシュタインの記法）
	// 解はxに追加される。戻り値は解が含まれるときにはtrue
	bool operator()( decimal a1, decimal a0, array<decimal> & x )
	{
		if ( a1 == 0 ) return false;
		x.push_back( - a0 / a1 );
		return fix( x );
	}
	bool operator()( decimal a2, decimal a1, decimal a0, array<decimal> & x )
	{
		if ( a2 == 0 ) return (*this)( a1, a0, x );
		decimal D = a1 * a1 - 4 * a2 * a0;//判別式
		if ( D < -0 ) {
			return false;
		} else if ( 0 < D ) {
			D = sqrt( D );
			x.push_back( ( -a1 + D ) / ( 2 * a2 ) );
			x.push_back( ( -a1 - D ) / ( 2 * a2 ) );
		} else {
			x.push_back( -a1 / ( 2 * a2 ) );
		}
		return fix( x );
	}
	bool operator()( decimal a3, decimal a2, decimal a1, decimal a0, array<decimal> & x )
	{
		if ( a3 == 0 ) return (*this)( a2, a1, a0, x );
		// x^3+a2x^2+a1x+a0=0の形にする
		if ( a3 != 1 ) {
			a2 /= a3;
			a1 /= a3;
			a0 /= a3;
		}
		// x' <= x + a2 / 3と置いてx'^3+a1x'+a0=0の形にする
		decimal X = a2 / 3;
		if ( a2 != 0 ) {
			const decimal t2 = a2, t1 = a1, t0 = a0;
			a2 = 0;
			a1 = t1 - t2 * t2 / 3;
			a0 = 2 / 27.0 * t2 * t2 * t2 - t2 * t1 / 3 + t0;
		}
		//カルダノの解法
		// x^3+a1x+a0
		// 分解方程式y^6-Ay^3+B A=9a3a2a1-2a2^3-27a0, B=a2^2-3a3a1ラグランジュの場合
		// y^6-Ay^3+B^3 A=-27a0, B=-3a1カルダノの場合
		//分解方程式の判別式D=A^2-4B^3   = 27(27a0^2+4a1^3)
		// y^3=1/2 * (A+sqrt(D))
		decimal D = 27 * 27. * a0 * a0 + 4 * 27 * a1 * a1 * a1;
		if ( D < -0 ) {//３つの実数根を持つ　途中に虚数解が入る
			decimal qurtr = sqrt( - a1 / 3. );
			decimal sita = acos( - a0 / 2.0 / qurtr / qurtr / qurtr );
			x.push_back( 2. * qurtr * cos( sita / 3 ) - X );
			x.push_back( 2. * qurtr * cos( ( sita + 2 * pi ) / 3 ) - X );
			x.push_back( 2. * qurtr * cos( ( sita + 4 * pi ) / 3 ) - X );
		} else if ( 0 < D ) {//通常はωがついた三つの解（２つは虚数根）であるが、実数根のみを取り出す。
			decimal u = curt( - a0 / 2 + sqrt( D ) / 2.0 ), v = curt( - a0 / 2 - sqrt( D ) / 2.0 );
			x.push_back( u + v - X );
		} else {
			// y = qurt(A/2)
			//解はy+y,y(ω+ω^2)の２種類にω=(-1+sqrt(3)i)/2,ω^2=(-1-sqrt(3)i)/2,ω+ω^2=-1
			decimal y = curt( - a0 / 2 );
			x.push_back( 2 * y - X );
			x.push_back( - y - X );
		}
		return fix( x );
	}
	bool operator()( decimal a4, decimal a3, decimal a2, decimal a1, decimal a0, array<decimal> & x )
	{
		if ( a4 == 0 ) return (*this)( a3, a2, a1, a0, x );
		// x^4+a3x^3+a2x^2+a1x+a0=0の形にする
		if ( a4 != 1. ) {
			a3 /= a4;
			a2 /= a4;
			a1 /= a4;
			a0 /= a4;
		}
		// x^4+a3x^3+a2x^2+a1x+a0=0
		// x' <= x + a3 / 4と置いてx'^4+a2x'^2+a1x'+a0=0の形にする
		decimal X = a3 / 4.;
		if ( a3 != 0. ) {
			decimal t2 = a2, t1 = a1, t0 = a0;
			a2 = t2 - 6. * X * X;
			a1 = t1 + 8. * X * X * X - 2. * t2 * X;
			a0 = t0 - 3. * X * X * X * X + t2 * X * X - t1 * X;
		}
		// x^4+a2x^2+a1x+a0=0
		// x^4=-a2x^2-a1x-a0の両辺にyx^2+y^2/4を加えて
		//左辺を完全平方な形にして、右辺を完全平方になるように
		//右辺の判別式が０になるようにyを決める。
		//右辺の判別式-y^3+a2y^2+4a0y^1+a1^2-4a2a0=0
		//もしy=a2の解があるならばa1=0である。(y-a2)(-y^2+4a0)=0
		if ( a1 == 0 ) {
			//そのときはy=+-2sqrt(a0)が別解である。y=a2として
			//x^4+a2x^2+a0の２次方程式になるので
			array<decimal> x2;
			if ( ! (*this)( 1, a2, a0, x2 ) ) return false;
			bool exist = false;
			for ( array<decimal>::iterator it( x2 ); it; ++it ) {
				if ( it() < -0 ) continue;//虚数解になるために省く
				if ( 0 < it() ) {
					exist = true;
					x.push_back( sqrt( it() ) - X );
					x.push_back( - sqrt( it() ) - X );
				} else {
					exist = true;
					x.push_back( - X );
				}
			}
			if ( ! exist ) return false;
			return fix( x );
		}
		//判別式を解く
		array<decimal> ys;
		if ( ! (*this)( 1, - a2, -4. * a0, 4 * a2 * a0 - a1 * a1, ys ) ) return false;
		bool result = false;
		// x^4+a2x^2+a1x+a0=0を変形しx^4+yx^2+y^2/4=-a2x^2-a1x-a0+yx^2+y^2/4整理する
		//(x^2+y/2)^2=(y-a2)(x-a1/(2(y-a2)))^2
		//最大の数をつかう。対称式なので１つで十分なはず
		const decimal y = ys.last();
		if ( y - a2 <= 0 ) return false;//負の時には虚数解のみになるので削除
		//x^2+y/2=±sqrt(y-a2)(x-a1/(2(y-a2)))
		//x^2-±sqrt(y-a2)x+y/2±a1/(2sqrt(y-a2))=0
		decimal wk = sqrt( y - a2 );
		array<decimal> xs;
		(*this)( 1, - wk, y / 2. + a1 / ( 2. * wk ), xs );
		(*this)( 1, + wk, y / 2. - a1 / ( 2. * wk ), xs );
		for ( array<decimal>::iterator itx( xs ); itx; ++itx ) {
			x.push_back( itx() - X );
		}
		return fix( x, 4 );//最大４つになるようにする
	}
};

#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "quaternion.h"
#include "matrix22.h"
#include "matrix33.h"
#include "matrix44.h"
#include "complex.h"
#include "vectorc.h"
#include "vectort.h"
#include "matrixc.h"
#include "matrixt.h"
#include "geometrical.h"
#include "analytical.h"
