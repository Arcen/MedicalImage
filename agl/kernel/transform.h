////////////////////////////////////////////////////////////////////////////////
// ïœå`

#ifndef __COMPOSE__

class sgTransformMatrix : public sgNode
{
public:
	sgNodeDeclare( sgTransformMatrix );
	sgMatrix44 parent, local, global;
	sgVector3 center;
	sgInt valid;//Ç±ÇÃêÊÇé¿çsÇ∑ÇÈÇ©Ç«Ç§Ç©
	virtual void update()
	{
		global( local() * parent() );
		center( global().translation() );
	}
};

class sg3DSMAXTransformMatrix : public sgNode
{
public:
	sgNodeDeclare( sg3DSMAXTransformMatrix );
	sgQuaternion scaleRotation;
	sgVector3 scale;
	sgQuaternion rotation;
	sgVector3 translation;
	sgMatrix44 matrix;
	virtual void update()
	{
		if ( scale() == vector3( 1, 1, 1 ) ) {
			matrix( matrix44::rotation( rotation() ) * matrix44::translate( translation() ) );
		} else if ( vector3::same( scale() ) ) {
			matrix( matrix44::scale( scale() ) * matrix44::rotation( rotation() ) * matrix44::translate( translation() ) );
		} else {
			matrix( matrix44::rotation( scaleRotation() ) * matrix44::scale( scale() ) * 
				matrix44::rotation( quaternion::conjugation( scaleRotation() ) ) * 
				matrix44::rotation( rotation() ) * matrix44::translate( translation() ) );
		}
	}
};

class sgMayaTransformMatrix : public sgNode
{
public:
	sgNodeDeclare( sgMayaTransformMatrix );
	sgVector3 scalePivot, scale, shear, scalePivotTranslation, rotatePivot, rotatePivotTranslation, translation;
	sgQuaternion rotation, rotateOrientation;
	sgMatrix44 matrix;
	//euleräpÇ…ÇÊÇÈâÒì]êßå‰èÓïÒ
	sgVector3 angle;//rotationÇÃäpìx[rad]
	enum {
		invalid = 0,//rotationÇóòópÅidefaultÅj
		xyz, yzx, zxy, xzy, yxz, zyx//angleÇóòóp
	};
	sgInt order;//âÒì]èáèò
	//êßå¿èÓïÒ
	sgVector3 limitedMinimumTranslate, limitedMaximumTranslate;
	sgVector3 limitedMinimumRotate, limitedMaximumRotate;
	sgVector3 limitedMinimumScale, limitedMaximumScale;
	sgVector3 limitMinimumTranslate, limitMaximumTranslate;
	sgVector3 limitMinimumRotate, limitMaximumRotate;
	sgVector3 limitMinimumScale, limitMaximumScale;
	//âÒì]ÇåvéZÇ∑ÇÈ
	static quaternion composeRotation( const quaternion & rotation, int order, const vector3 & angle )
	{
		//åvéZèáèòÇ™ñ≥Ç¢Ç∆Ç´Ç…ÇÕÅAÇ‹Ç∏ÇªÇÃÇ‹Ç‹ï‘Ç∑
		if ( order == invalid ) return rotation;

		quaternion qx( vector3( 1, 0, 0 ), angle.x );
		quaternion qy( vector3( 0, 1, 0 ), angle.y );
		quaternion qz( vector3( 0, 0, 1 ), angle.z );
		switch ( order ) {
		case xyz: return qz * qy * qx;
		case yzx: return qx * qz * qy;
		case zxy: return qy * qx * qz;
		case xzy: return qy * qz * qx;
		case yxz: return qz * qx * qy;
		case zyx: return qx * qy * qz;
		default: return rotation;
		}
	}
	//êßå¿Çâ¡Ç¶ÇÈ
	void limitTranslation( vector3 & translation )
	{
		if ( limitedMinimumTranslate().x ) translation.x = maximum( translation.x, limitMinimumTranslate().x );
		if ( limitedMaximumTranslate().x ) translation.x = minimum( translation.x, limitMaximumTranslate().x );
		if ( limitedMinimumTranslate().y ) translation.y = maximum( translation.y, limitMinimumTranslate().y );
		if ( limitedMaximumTranslate().y ) translation.y = minimum( translation.y, limitMaximumTranslate().y );
		if ( limitedMinimumTranslate().z ) translation.z = maximum( translation.z, limitMinimumTranslate().z );
		if ( limitedMaximumTranslate().z ) translation.z = minimum( translation.z, limitMaximumTranslate().z );
	}
	void limitAngle( vector3 & angle )
	{
		if ( limitedMinimumRotate().x && limitedMaximumRotate().x ) angle.x = clamp( limitMinimumRotate().x, angle.x, limitMaximumRotate().x );
		if ( limitedMinimumRotate().y && limitedMaximumRotate().y ) angle.y = clamp( limitMinimumRotate().y, angle.y, limitMaximumRotate().y );
		if ( limitedMinimumRotate().z && limitedMaximumRotate().z ) angle.z = clamp( limitMinimumRotate().z, angle.z, limitMaximumRotate().z );
	}
	void limitScale( vector3 & scale )
	{
		if ( limitedMinimumScale().x ) scale.x = maximum( scale.x, limitMinimumScale().x );
		if ( limitedMaximumScale().x ) scale.x = minimum( scale.x, limitMaximumScale().x );
		if ( limitedMinimumScale().y ) scale.y = maximum( scale.y, limitMinimumScale().y );
		if ( limitedMaximumScale().y ) scale.y = minimum( scale.y, limitMaximumScale().y );
		if ( limitedMinimumScale().z ) scale.z = maximum( scale.z, limitMinimumScale().z );
		if ( limitedMaximumScale().z ) scale.z = minimum( scale.z, limitMaximumScale().z );
	}
	quaternion composeRotation( const vector3 & angle )
	{
		quaternion qx( vector3( 1, 0, 0 ), angle.x );
		quaternion qy( vector3( 0, 1, 0 ), angle.y );
		quaternion qz( vector3( 0, 0, 1 ), angle.z );
		switch ( order() ) {
		case xyz: return qz * qy * qx;
		case yzx: return qx * qz * qy;
		case zxy: return qy * qx * qz;
		case xzy: return qy * qz * qx;
		case yxz: return qz * qx * qy;
		case zyx: return qx * qy * qz;
		default: return rotation();
		}
	}
	virtual void update()
	{
		matrix( matrix44::translate( -scalePivot() ) * 
			matrix44::scale( scale() ) * 
			matrix44::shear( shear() ) * 
			matrix44::translate( scalePivot() + scalePivotTranslation() - rotatePivot() ) *
			matrix44::rotation( rotateOrientation() ) * 
			matrix44::rotation( composeRotation( angle() ) ) * 
			matrix44::translate( rotatePivot() + rotatePivotTranslation() + translation() ) );
	}
};

class sgMayaJointMatrix : public sgNode
{
public:
	sgNodeDeclare( sgMayaJointMatrix );
	sgVector3 scale, inverseScale, translation;
	sgQuaternion scaleOrientation, rotation, orientation;
	sgMatrix44 matrix;
	//euleräpÇ…ÇÊÇÈâÒì]êßå‰èÓïÒ
	sgVector3 angle;//rotationÇÃäpìx[rad]
	sgInt order;//âÒì]èáèò
	//êßå¿èÓïÒ
	sgVector3 limitedMinimumTranslate, limitedMaximumTranslate;
	sgVector3 limitedMinimumRotate, limitedMaximumRotate;
	sgVector3 limitedMinimumScale, limitedMaximumScale;
	sgVector3 limitMinimumTranslate, limitMaximumTranslate;
	sgVector3 limitMinimumRotate, limitMaximumRotate;
	sgVector3 limitMinimumScale, limitMaximumScale;
	//âÒì]é≤ÇÃêßå¿
	sgVector3 freedom;
	//êßå¿Çâ¡Ç¶ÇÈ
	void limitTranslation( vector3 & translation )
	{
		if ( limitedMinimumTranslate().x ) translation.x = maximum( translation.x, limitMinimumTranslate().x );
		if ( limitedMaximumTranslate().x ) translation.x = minimum( translation.x, limitMaximumTranslate().x );
		if ( limitedMinimumTranslate().y ) translation.y = maximum( translation.y, limitMinimumTranslate().y );
		if ( limitedMaximumTranslate().y ) translation.y = minimum( translation.y, limitMaximumTranslate().y );
		if ( limitedMinimumTranslate().z ) translation.z = maximum( translation.z, limitMinimumTranslate().z );
		if ( limitedMaximumTranslate().z ) translation.z = minimum( translation.z, limitMaximumTranslate().z );
	}
	void limitAngle( vector3 & angle )
	{
		if ( limitedMinimumRotate().x && limitedMaximumRotate().x ) angle.x = clamp( limitMinimumRotate().x, angle.x, limitMaximumRotate().x );
		if ( limitedMinimumRotate().y && limitedMaximumRotate().y ) angle.y = clamp( limitMinimumRotate().y, angle.y, limitMaximumRotate().y );
		if ( limitedMinimumRotate().z && limitedMaximumRotate().z ) angle.z = clamp( limitMinimumRotate().z, angle.z, limitMaximumRotate().z );
	}
	void limitScale( vector3 & scale )
	{
		if ( limitedMinimumScale().x ) scale.x = maximum( scale.x, limitMinimumScale().x );
		if ( limitedMaximumScale().x ) scale.x = minimum( scale.x, limitMaximumScale().x );
		if ( limitedMinimumScale().y ) scale.y = maximum( scale.y, limitMinimumScale().y );
		if ( limitedMaximumScale().y ) scale.y = minimum( scale.y, limitMaximumScale().y );
		if ( limitedMinimumScale().z ) scale.z = maximum( scale.z, limitMinimumScale().z );
		if ( limitedMaximumScale().z ) scale.z = minimum( scale.z, limitMaximumScale().z );
	}
	quaternion composeRotation( const vector3 & angle )
	{
		quaternion qx( vector3( 1, 0, 0 ), angle.x );
		quaternion qy( vector3( 0, 1, 0 ), angle.y );
		quaternion qz( vector3( 0, 0, 1 ), angle.z );
		switch ( order() ) {
		case sgMayaTransformMatrix::xyz: return qz * qy * qx;
		case sgMayaTransformMatrix::yzx: return qx * qz * qy;
		case sgMayaTransformMatrix::zxy: return qy * qx * qz;
		case sgMayaTransformMatrix::xzy: return qy * qz * qx;
		case sgMayaTransformMatrix::yxz: return qz * qx * qy;
		case sgMayaTransformMatrix::zyx: return qx * qy * qz;
		default: return rotation();
		}
	}
	virtual void update()
	{
		vector3 & is = inverseScale();
		matrix( matrix44::scale( scale() ) * 
			matrix44::rotation( scaleOrientation() ) * 
			matrix44::rotation( composeRotation( angle() ) ) * 
			matrix44::rotation( orientation() ) * 
			matrix44::scale( vector3( 1/is.x, 1/is.y, 1/is.z ) ) * 
			matrix44::translate( translation() ) );
	}
	//âÒì]èàóùÇÊÇËå„ÇÃçsóÒ
	matrix44 postRotation()
	{
		vector3 & is = inverseScale();
		return matrix44::rotation( orientation() ) * 
			matrix44::scale( vector3( 1/is.x, 1/is.y, 1/is.z ) ) * 
			matrix44::translate( translation() );
	}
	//âÒì]èàóùÇ‹Ç≈ÇÃçsóÒ
	matrix44 preRotation()
	{
		return matrix44::scale( scale() ) * 
			matrix44::rotation( scaleOrientation() );
	}
};

#else

sgNodeDeclareInstance( sgTransformMatrix )
{
	sgDeclareThis();
	sgDeclareInitialRetain( local, matrix44::identity );
	sgDeclareInitial( parent, matrix44::identity );
	sgDeclareInitial( global, matrix44::identity );
	sgDeclareInitialRetain( valid, 1 );
	sgDeclareInitial( center, vector3::identity );
}

sgNodeDeclareInstance( sg3DSMAXTransformMatrix )
{
	sgDeclareThis();
	sgDeclareInitialRetain( scaleRotation, quaternion::identity );
	sgDeclareInitialRetain( scale, vector3( 1, 1, 1 ) );
	sgDeclareInitialRetain( rotation, quaternion::identity );
	sgDeclareInitialRetain( translation, vector3( 0, 0, 0 ) );
	sgDeclareInitial( matrix, matrix44::identity );
}

sgNodeDeclareInstance( sgMayaTransformMatrix )
{
	sgDeclareThis();
	sgDeclareInitialRetain( scalePivot, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( scale, vector3( 1, 1, 1 ) );
	sgDeclareInitialRetain( shear, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( scalePivotTranslation, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( rotatePivot, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( rotation, quaternion::identity );
	sgDeclareInitialRetain( rotateOrientation, quaternion::identity );
	sgDeclareInitialRetain( rotatePivotTranslation, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( translation, vector3( 0, 0, 0 ) );
	sgDeclareInitial( matrix, matrix44::identity );

	sgDeclareInitialRetain( angle, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( order, invalid );

	sgDeclareInitialRetain( limitedMinimumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMaximumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMinimumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMaximumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMinimumScale, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMaximumScale, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMinimumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMaximumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMinimumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMaximumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMinimumScale, vector3( 1, 1, 1 ) );
	sgDeclareInitialRetain( limitMaximumScale, vector3( 1, 1, 1 ) );
}

sgNodeDeclareInstance( sgMayaJointMatrix )
{
	sgDeclareThis();
	sgDeclareInitialRetain( scale, vector3( 1, 1, 1 ) );
	sgDeclareInitialRetain( scaleOrientation, quaternion::identity );
	sgDeclareInitialRetain( rotation, quaternion::identity );
	sgDeclareInitialRetain( orientation, quaternion::identity );
	sgDeclareInitialRetain( inverseScale, vector3( 1, 1, 1 ) );
	sgDeclareInitialRetain( translation, vector3( 0, 0, 0 ) );
	sgDeclareInitial( matrix, matrix44::identity );

	sgDeclareInitialRetain( angle, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( order, sgMayaTransformMatrix::invalid );

	sgDeclareInitialRetain( limitedMinimumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMaximumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMinimumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMaximumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMinimumScale, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitedMaximumScale, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMinimumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMaximumTranslate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMinimumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMaximumRotate, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( limitMinimumScale, vector3( 1, 1, 1 ) );
	sgDeclareInitialRetain( limitMaximumScale, vector3( 1, 1, 1 ) );

	sgDeclareInitialRetain( freedom, vector3( 1, 1, 1 ) );
}

#endif
