////////////////////////////////////////////////////////////////////////////////
// ŽžŠÔ

#ifndef __COMPOSE__

template<class T>
inline void constantInterpolater( const decimal & time, const T & current, const array<T> & data )
{
	if ( data.size ) const_cast<T&>( current ) = static_cast<T>( data[ clamp<int>( 0, static_cast<int>( time ), data.size - 1 ) ] );
};

template<class T>
inline void linearInterpolater( const decimal & time, const T & current, const array<T> & data )
{
	if ( data.size ) {
		const int index = clamp<int>( 0, static_cast<int>( time ), data.size - 1 );
		const decimal alpha = time - index;
		const_cast<T&>( current ) = static_cast<T>( 
			data[ clamp<int>( 0, index, data.size - 1 ) ] * ( 1 - alpha ) + 
			data[ clamp<int>( 0, index + 1, data.size - 1 ) ] * alpha );
	}
};

inline void linearQuaternionInterpolater( const decimal & time, const quaternion & current, const array<quaternion> & data )
{
	if ( data.size ) {
		const int index = clamp<int>( 0, static_cast<int>( time ), data.size - 1 );
		const decimal alpha = time - index;
		const_cast<quaternion&>( current ) = quaternion::slerp( alpha, data[ clamp<int>( 0, index, data.size - 1 ) ], data[ clamp<int>( 0, index + 1, data.size - 1 ) ] );
	}
};

#define sgInterpolaterClass(classname,type,interpolater,initial)	\
class classname : public sgNode\
{\
public:\
	sgNodeDeclare( classname );\
	sgDecimal time;\
	sgPrimitiveMember<type> current;\
	sgPrimitiveMember< array<type> > data;\
	virtual void update() { interpolater( time(), current(), data() ); }\
};

#else

#define sgInterpolaterClass(classname,type,interpolater,initial)	\
sgNodeDeclareInstance( classname )\
{\
	sgDeclareThis();\
	sgDeclareInitial( time, 0 );\
	sgDeclareInitial( current, initial );\
	sgDeclareRetain( data );\
}

#endif

sgInterpolaterClass( sgIntConstantInterpolater, int, constantInterpolater, 0 );
sgInterpolaterClass( sgDecimalConstantInterpolater, decimal, constantInterpolater, 0 );
sgInterpolaterClass( sgVector2ConstantInterpolater, vector2, constantInterpolater, vector2::identity );
sgInterpolaterClass( sgVector3ConstantInterpolater, vector3, constantInterpolater, vector3::identity );
sgInterpolaterClass( sgVector4ConstantInterpolater, vector4, constantInterpolater, vector4::identity );
sgInterpolaterClass( sgQuaternionConstantInterpolater, quaternion, constantInterpolater, quaternion::identity );

sgInterpolaterClass( sgIntLinearInterpolater, int, linearInterpolater, 0 );
sgInterpolaterClass( sgDecimalLinearInterpolater, decimal, linearInterpolater, 0 );
sgInterpolaterClass( sgVector2LinearInterpolater, vector2, linearInterpolater, vector2::identity );
sgInterpolaterClass( sgVector3LinearInterpolater, vector3, linearInterpolater, vector3::identity );
sgInterpolaterClass( sgVector4LinearInterpolater, vector4, linearInterpolater, vector4::identity );
sgInterpolaterClass( sgQuaternionLinearInterpolater, quaternion, linearQuaternionInterpolater, quaternion::identity );

#undef sgInterpolaterClass
