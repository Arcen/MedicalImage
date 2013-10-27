////////////////////////////////////////////////////////////////////////////////
// Global Variable Definition

#define __GLOBAL__

#include "agl.h"

namespace agl {
//#include "dxrender.h"
#include "oglrender.h"

	/*
	quaternion quaternion::euler( const decimal & roll, const decimal & pitch, const decimal & yaw )
	{
		const decimal cr = cos( roll / 2 );
		const decimal cp = cos( pitch / 2 );
		const decimal cy = cos( yaw / 2 );
		const decimal sr = sin( roll / 2 );
		const decimal sp = sin( pitch / 2 );
		const decimal sy = sin( yaw / 2 );
		const decimal cpcy = cp * cy, spsy = sp * sy;
		const decimal cpsy = cp * sy, spcy = sp * cy;
		return validate( quaternion( sr * cpcy - cr * spsy, cr * spcy + sr * cpsy, cr * cpsy - sr * spcy, cr * cpcy + sr * spsy ) );
	}
	*/
	quaternion quaternion::euler( const decimal & roll, const decimal & pitch, const decimal & yaw )
	{
		quaternion qr = quaternion( vector3( 0, 1, 0 ), roll );
		quaternion qp = quaternion( vector3( 1, 0, 0 ), pitch );
		quaternion qy = quaternion( vector3( 0, 0, 1 ), yaw );
		return qr * qp * qy;
	}

	void quaternion::euler( const quaternion & q, decimal & roll, decimal & pitch, decimal & yaw )
	{
		vector3 x( 1, 0, 0 ), z( 0, 0, 1 );

		matrix33 mrpy = matrix33::rotation( q );
		vector3 xrpy = x * mrpy, zrpy = z * mrpy;

		vector3 zrpy_on_xz = vector3( zrpy.x, 0, zrpy.z );
		if ( vector3::length3( zrpy_on_xz ) == 0 ) {
			roll = 0;
			pitch = zrpy.y > 0 ? pi_2 : -pi_2;
		} else {
			roll = vector3::argument3( z, vector3::normalize3( zrpy_on_xz ) );
			if ( zrpy.x < 0 ) roll *= -1;
			pitch = vector3::argument3( vector3::normalize3( zrpy_on_xz ), zrpy );
			if ( zrpy.y > 0 ) pitch *= -1;
		}
		matrix33 mrp = matrix33::rotation( quaternion::euler( roll, pitch, 0 ) );
		vector3 xrp = x * mrp;
		yaw = vector3::argument3( xrpy, xrp );
		matrix33 mrpyp = matrix33::rotation( quaternion::euler( roll, pitch, yaw ) );
		matrix33 mrpym = matrix33::rotation( quaternion::euler( roll, pitch, -yaw ) );
		vector3 xrpyp = x * mrpyp, xrpym = x * mrpym;
		if ( vector3::argument3( xrpyp, xrpy ) > vector3::argument3( xrpym, xrpy ) ) {
			yaw *= -1;
		}
	}
};