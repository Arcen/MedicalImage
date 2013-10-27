//メイン処理

#include <agl/kernel/agl.h>
using namespace agl;
#include <fc/fc.h>

enum {
	//control ID
	statusBarID = 31000,
	sizeSpinID,
	levelSpinID,
	sliceSpinID,
	extractUpLimitSpinID,
	extractDownLimitSpinID,
	modifyClosingSpinID,
	modifyOpeningSpinID,
	brushSpinID,
	calibrationAirSpinID,
	calibrationWaterSpinID,
	gaussCorrelationParamStartSpinID,
	gaussCorrelationParamEndSpinID,
	gaussCorrelationParamStepSpinID,
	//popup
	rclickAppendRegionID,
	rclickDeleteID,
	rclickDefineCenterID,
	rclickDeleteRegionID,
	rclickContinouseDeleteID,
	rclickSamplingRegionID,
	rclickAverageRegionID,
	rclickDeleteMinorRegionID,
	rclickMergeMinorRegionID,

	//タブシートインデックス
	viewSheetIndex = 0,
	regionSheetIndex,
	attributeSheetIndex,

	//領域インデックス
	regionHuman = 0,
	regionLung,
	regionNormal,

	//表示モードインデックス
	viewModeMix = 0,
	viewModeOriginal,
	viewModeLabel,

	viewSideBoth = 0,
	viewSideLeft,
	viewSideRight,

	regionOperationModeAppend = 0,
	regionOperationModeDelete,
	regionOperationModeChange,

	lungTop = 0,
	lungMiddle,
	lungBottom,
	numberLungTopBottom,
	lungFront = 0,
	//lungMiddle = 1,//２重定義
	lungRear = 2,
	numberLungFrontRear,
};

typedef region<int16>	ctRegion;

extern HINSTANCE hInstance;
extern fc::window * mainWindow;
extern fc::window * imageWindow;
extern fc::statusbar * statusBar;
/*
inline bool regGet( const char * name, string & value )
{
	return windows::get( "SOFTWARE\\Akira", "MedicalImage", name, value );
}
inline bool regSet( const char * name, const string & value )
{
	return windows::set( "SOFTWARE\\Akira", "MedicalImage", name, value );
}
inline bool regsKill( const char * directory, const list< pair<string> > & value )
{
	for ( list< pair<string> >::iterator it( value ); it; ++it ) {
		windows::kill( "SOFTWARE\\Akira\\MedicalImage", directory, it().left );
	}
	return true;
}
inline bool regsSet( const char * directory, const list< pair<string> > & value )
{
	string path = string( "SOFTWARE\\Akira\\MedicalImage" );
	if ( directory ) {
		path = path + string( "\\" ) + string( directory );
	}
	for ( list< pair<string> >::iterator it( value ); it; ++it ) {
		windows::set( "SOFTWARE\\Akira\\MedicalImage", directory, it().left, it().right );
	}
	return true;
}
inline bool regsGet( const char * directory, list< pair<string> > & value )
{
	string path = string( "SOFTWARE\\Akira\\MedicalImage" );
	if ( directory ) {
		path = path + string( "\\" ) + string( directory );
	}
	HKEY reg = NULL;
	if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, path, 0, KEY_ALL_ACCESS, & reg ) ) return false;
	for ( int index = 0; ; index++ ) {
		char name[1024];
		DWORD nameLength = 1024;
		DWORD type = 0;
		BYTE data[1024];
		DWORD dataSize = 1024;
		if ( RegEnumValue( reg, index, name, & nameLength, NULL, & type, data, & dataSize ) ) break;
		switch ( type ) {
		case REG_SZ: value.push_back( pair<string>( name, string( ( const char * ) data ) ) ); break;
		}
	}
	RegCloseKey( reg );
	return ! value.empty();
}
inline decimal gauss( decimal x, decimal u, decimal s )
{
	static const decimal coefficient = 1 / sqrt( 2 * pi );
	x -= u;
	x /= s;
	return coefficient / sqrt( s ) * exp( - x * x / 2 );
}
struct pixelInfo
{
//	double gx, gy, gnorm, gangle;
	float gnorm;
};
inline void analyzePixelInformation( const imageInterface< pixelLuminance<int16> > & ctimg, array2<pixelInfo> & info )
{
	info.allocate( ctimg.width, ctimg.height );
	for ( int y = 0; y < info.h; ++y ) {
		for ( int x = 0; x < info.w; ++x ) {
			pixelInfo & pi = info( x, y );
			int16 ct = ctimg.getInternal( x, y ).y;
			//∇F=dF/di･i
			//粒状影では各方向の勾配が平均して分布している
			//回転に依存しないように角度情報自体には重みを置けない。
			//ある程度の勾配の大きさのある角度の分布度は重要だと思われる。
			//pi.gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );
			//pi.gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );
			double gx = ( ( x == 0 ) ? 0 : ct - ctimg.getInternal( x - 1, y ).y );
			double gy = ( ( y == 0 ) ? 0 : ct - ctimg.getInternal( x, y - 1 ).y );
			//勾配の大きさ=||∇F||=√(∇F･∇F)
			//勾配の大きさ自体は濃度情報に多くが含まれているので重点をおかなくてもいい
			//pi.gnorm = sqrt( pi.gx * pi.gx + pi.gy * pi.gy );
			pi.gnorm = sqrt( gx * gx + gy * gy );
			//pi.gnorm = clamp<double>( 0, pi.gnorm, 2048 );
			pi.gnorm = clamp<float>( 0, pi.gnorm, 2048 );
			//角度
			//pi.gangle = ( pi.gnorm == 0 ? 0 : atan2( pi.gx, pi.gy ) );

			//線状影ならば角度依存性が多少出てくる。
			//網状影と粒状影なら密度の差が出ると思われる。
			//CT値の方は１次のデータになるが、こちらは２次のデータになる。
			//角度、大きさそれぞれに周辺化し２つの１次データとして相関を取るのが良いか？
			//線状影に関しては角度方向で回転し相関を取らないとあわないと思われる。
			//大きさのみでサンプリング化してみる方が良いかもしれない
		}
	}
}

class stepParameter
{
public:
	int start, end, step;
	stepParameter() : start( 0 ), end( 0 ), step( 1 )
	{
	}
	int count()
	{
		if ( step < 1 ) return 1;
		int c = 0;
		for ( int i = start; i <= end; i += step ) ++c;
		return c;
	}
	void set( int _start, int _end, int _step = 1 )
	{
		start = _start;
		end = _end;
		step = _step;
	}
	void set( int _start )
	{
		set( _start, _start );
	}
	class iterator
	{
		const stepParameter * sp;
		int i;
	public:
		iterator() : sp( NULL ), i( 0 ){}
		iterator( const stepParameter & src ) : sp( & src ), i( sp->start ){}
		iterator( const iterator & it ) : sp( it.sp ), i( sp->start ){}
		iterator & operator=( const iterator & it ){ sp = it.sp; i = it.i; return *this; }
		////////////////////////////////////////////////////////////////////////////////
		//条件判定用型変換
		operator bool () const {
			if ( ! sp ) return false;
			if ( sp->step < 1 ) return sp->start == i;
			return ( i <= sp->end );
		}
		////////////////////////////////////////////////////////////////////////////////
		//型変換
		int operator()() const { return i; }
		////////////////////////////////////////////////////////////////////////////////
		//次へ
		iterator & operator++()
		{
			if ( sp->step < 1 ) {
				i = sp->end + 1;
			} else {
				i += sp->step;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//前へ
		iterator & operator--()
		{
			if ( sp->step < 1 ) {
				i = sp->end + 1;
			} else {
				i -= sp->step;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//その他のインタフェース
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
	};
};
*/
#include "resource.h"
#include "semaphore.h"
/*
#include "regionfile.h"
#include "freehand.h"
#include "brush.h"

extern semaphore singleThread;
extern semaphore onTrackMenu;
extern freehand freehandinterface;
extern brush brushinterface;

#include "interface.h"
#include "control.h"
#include "estimate.h"
#include "ctimage.h"
#include "pattern.h"
#include "document.h"
#include "command.h"
#include "frame.h"
#include "view.h"
*/