#pragma once

//メイン処理

#include <agl/kernel/agl.h>
using namespace agl;
#include <fc/fc.h>

enum {
	statusBarID = 31000,
	toolBarID,
	rebarID,
	sliderID,
	windowControlID,
	windowSizeID,
	windowLevelID,
	windowSizeEditID,
	windowLevelEditID,
	rebar4toolbarID,
	rebar4sliderID,
	rebar4windowID,
};

//ウィンドウ関連
extern HINSTANCE hInstance;
extern fc::window * mainWindow;
extern fc::window * imageWindow;
extern fc::statusbar * statusBar;
extern fc::toolbar * toolBar;
extern fc::rebar * reBar;
extern fc::slider * sliderBar;
extern fc::menu * mainMenu;
extern fc::window * windowControlWindow;
extern fc::slider * windowSizeBar;
extern fc::slider * windowLevelBar;

const double houghScale = 20.0;//ハフ空間の横の拡大率
const double houghRange = 1.5;//ハフ空間の横の片側の大きさ
const int houghWidth = round<int,double>( 2 * houghRange * houghScale ) + 1;//ハフ空間の横幅の大きさ

class link 
{
public:
	enum {
		typeNone,
		typeVertical,
		typeHorizontal,
		typeAscent,
		typeDescent,
		/*
		typeUp,
		typeDown,
		typeLeft,
		typeRight,
		typeDownRight,
		typeUpLeft,
		typeUpRight,
		typeDownLeft,
		*/
	};
	int type;
	int bx, by;
	double cx, cy;
	double reliability, intensity;
	link() : type( typeNone ), bx( 0 ), by( 0 ), cx( 0 ), cy( 0 ), 
		reliability( 1 ), intensity( 1 )
	{
	}
	void initialize()
	{
		type = typeNone;
		bx = 0;
		by = 0;
		cx = 0;
		cy = 0;
		reliability = 1;
		intensity = 1;
	}
	link( const link & l )
	{
		*this = l;
	}
	link & operator=( const link & v )
	{
		type = v.type;
		bx = v.bx;
		by = v.by;
		cx = v.cx;
		cy = v.cy;
		reliability = v.reliability;
		intensity = v.intensity;
		return *this;
	}
	bool operator==( const link & v ) const 
	{
		return type == v.type && bx == v.bx && by == v.by && 
			cx == v.cx && cy == v.cy &&
			reliability == v.reliability &&
			intensity == v.reliability;
	}
	bool operator!=( const link & v ) const 
	{
		return ! ( *this == v );
	}
	const point2<int> position( int t, const array<int> & wave ) const
	{
		return point2<int>( bx + wave[t] * cx, by + wave[t] * cy );
	}
	void set( int type, const point2<int> & pt, int baseposition, int height )
	{
		this->type = type;
		switch( type ) {
		/*case typeUp:
		case typeDown:*/
		case typeVertical:
			bx = baseposition;
			by = pt.y;
			cx = 0;
			cy = pt.x / houghScale - houghRange;
			break;
			/*
		case typeLeft:
		case typeRight:*/
		case typeHorizontal:
			bx = pt.y;
			by = baseposition;
			cx = pt.x / houghScale - houghRange;
			cy = 0;
			break;
		/*
		case typeUpRight:
		case typeDownLeft:
		*/
		case typeAscent:
			if ( baseposition < height ) {
				bx = 0;
				by = baseposition;
			} else {
				bx = baseposition - ( height - 1 );
				by = height - 1;
			}
			bx += pt.y;
			by -= pt.y;
			cx = pt.x / houghScale - houghRange;
			cy = -cx;
			break;
		/*
		case typeUpLeft:
		case typeDownRight:
*/
		case typeDescent:
			if ( baseposition < height ) {
				bx = 0;
				by = ( height - 1 ) - baseposition;
			} else {
				bx = baseposition - ( height - 1 );
				by = 0;
			}
			bx += pt.y;
			by += pt.y;
			cx = pt.x / houghScale - houghRange;
			cy = cx;
			break;
		}
	}
	int efficientIndex() const
	{
		if ( type == typeVertical ) 
			return clamp<int>( 0, ( cy + houghRange ) * houghScale, houghWidth - 1 );
		return clamp<int>( 0, ( cx + houghRange ) * houghScale, houghWidth - 1 );
	}
	int base( int height ) const
	{
		switch ( type ) {
		case link::typeVertical: return bx;
		case link::typeHorizontal: return by;
		case link::typeAscent: return bx + by;
		case link::typeDescent: return bx-by+height-1;
		}
		return 0;
	}
	int offset( int height ) const
	{
		switch ( type ) {
		case link::typeVertical: return by;
		case link::typeHorizontal: return bx;
		case link::typeAscent:
			{
				int b = base( height );
				if ( b < height ) {
					return bx;
				} else {
					return height - 1 - by;
				}
			}
		case link::typeDescent:
			{
				int b = base( height );
				if ( b < height ) {
					return bx;
				} else {
					return by;
				}
			}
		}
		return 0;
	}
};
#include "boundary.h"

class statisticsValue
{
public:
	double valueMoment1;
	double valueMoment2;
	double edgeMoment1;
	double edgeMoment2;
	double valueAve, valueSD;
	double edgeAve, edgeSD;
	double baseEdge;
	bool enable;
	int count;
	void initialize()
	{
		valueMoment1 = 0;
		valueMoment2 = 0;
		edgeMoment1 = 0;
		edgeMoment2 = 0;
		valueAve = valueSD = edgeAve = edgeSD = 0;
		baseEdge = 0;
		count = 0;
		enable = false;
	}
	void append( double value, double edge )
	{
		enable = true;
		valueMoment1 += value;
		valueMoment2 += square( value );
		edgeMoment1 += edge;
		edgeMoment2 += square( edge );
		++count;
	}
	void finish()
	{
		if ( count == 0 ) {
			return;
		}
		valueAve = valueMoment1 / count;
		valueSD = ::sqrt( valueMoment2 / count - square( valueMoment1 / count ) );
		edgeAve = edgeMoment1 / count;
		edgeSD = ::sqrt( edgeMoment2 / count - square( edgeMoment1 / count ) );
	}
	double valueAverage() const
	{
		return valueAve;
	}
	double valueStandardDeviation() const
	{
		return valueSD;
	}
	double edgeAverage() const
	{
		return edgeAve;
	}
	double edgeStandardDeviation() const
	{
		return edgeSD;
	}
	double valueConstancy() const
	{
		return clamp<double>( 0, 1 - valueSD / valueAve, 1 );
	}
	double edgeConstancy() const
	{
		return clamp<double>( 0, 1 - edgeSD / absolute( edgeAve ) / 3.0, 1 );
	}
	double constancy() const
	{
		return valueConstancy() * edgeConstancy();
	}
	double likelyhood() const
	{
		return clamp<double>( 0, 1 - edgeSD / absolute( edgeAve ) / 3.0, 1 );
		double m2 = edgeMoment2 - 2 * edgeMoment1 * baseEdge + count * square( baseEdge );
		m2 /= count;
		return clamp<double>( 0, 1 - m2 / absolute( edgeAve ) / 10.0, 1 );
	}
};

class flicker
{
public:
	bool enable;
	double value;
	link lnk;
	flicker() : enable( false ), value( 0 )
	{
	}
	flicker( const flicker & c )
	{
		*this = c;
	}
	flicker & operator=( const flicker & c )
	{
		enable = c.enable;
		value = c.value;
		lnk = c.lnk;
		return *this;
	}
	bool operator==( const flicker & p ) const
	{
		return enable == p.enable && value == p.value && lnk == p.lnk;
	}
	bool operator!=( const flicker & p ) const
	{
		return ! ( *this == p );
	}
	void initialize()
	{
		enable = false;
		value = 0;
		lnk.initialize();
	}
};

typedef short int16;
typedef unsigned char uint8;
bool linepoints( const point2<int> & p1, const point2<int> & p2, list< point2<int> > & points );

//#include "semaphore.h"
