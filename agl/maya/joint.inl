////////////////////////////////////////////////////////////////////////////////
// JOINT


/*
matrix = [S] * [RO] * [R] * [JO] * [IS] * [T] 
These matrices are defined as follows:

[S] : scale by getScale
[RO] : rotateOrient (attribute name is rotateAxis) by getScaleOrientation
[R] : rotate by getRotation
[JO] : jointOrient by getOrientation
[IS] : parentScaleInverse by (the inverse of the getScale on the parent transformation matrix) 
[T] : translate by translation
*/

void convert( MFnIkJoint & fnIkJoint, vector3 & _scale, quaternion & _scaleOrientation, quaternion & _rotation, quaternion & _orientation, vector3 & _position, vector3 & _angle, MSpace::Space space = MSpace::kTransform )
{
	MStatus status;
	double scale[3], angle[3];
	MQuaternion scaleOrientation, rotation, orientation;
	MVector translation;
	MTransformationMatrix::RotationOrder order = MTransformationMatrix::kXYZ;
	Maya::statusError( fnIkJoint.getScale( scale ), "Fail MFnIkJoint::getScale" );
	Maya::statusError( fnIkJoint.getScaleOrientation( scaleOrientation ), "Fail MFnIkJoint::getScaleOrientation" );
	Maya::statusError( fnIkJoint.getRotation( rotation, space ), "Fail MFnIkJoint::getRotation" );
	Maya::statusError( fnIkJoint.getOrientation( orientation ), "Fail MFnIkJoint::getOrientation" );
	translation = fnIkJoint.translation( space, &status ); Maya::statusError( status, "Fail MFnIkJoint::translation" );
	Maya::statusError( fnIkJoint.getRotation( angle, order ), "Fail MFnIkJoint::getRotation" );

	_scale = ConvertScale( scale );
	_scaleOrientation = ConvertRotation( scaleOrientation );
	_rotation = ConvertRotation( rotation );
	_orientation = ConvertRotation( orientation );
	_position = ConvertPosition( translation );
	_angle = ConvertAngle( angle );
}

class jointAnimation : public animationNode
{
public:
	bool active;
	MDagPath dPath;
	MFnIkJoint fnIkJoint;
	sgMayaJointMatrix * matrix;
	array<quaternion> scaleOrientations, rotations, orientations;
	array<vector3> positions, scales, angles;
	string name;
	jointAnimation( const MDagPath & _dPath, sgMayaJointMatrix * _matrix, const char * _name ) 
		: dPath( _dPath ), fnIkJoint( _dPath ), matrix( _matrix ), name( _name )
	{
		active = true;
	}
	virtual void finish()
	{
		if ( ! active ) return;
		if ( isAnimated( scales ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$sclint";
			interpolator->data( scales );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaJointMatrix::attributes->scale );
		}
		if ( isAnimated( scaleOrientations ) ) {
			sgQuaternionLinearInterpolater * interpolator = new sgQuaternionLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$sorint";
			interpolator->data( scaleOrientations );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgQuaternionLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgQuaternionLinearInterpolater::attributes->current, *matrix, sgMayaJointMatrix::attributes->scaleOrientation );
		}
		if ( matrix->order() == sgMayaTransformMatrix::invalid && 
			isAnimated( rotations ) ) {
			sgQuaternionLinearInterpolater * interpolator = new sgQuaternionLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$rotint";
			interpolator->data( rotations );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgQuaternionLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgQuaternionLinearInterpolater::attributes->current, *matrix, sgMayaJointMatrix::attributes->rotation );
		}
		if ( isAnimated( orientations ) ) {
			sgQuaternionLinearInterpolater * interpolator = new sgQuaternionLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$ornint";
			interpolator->data( orientations );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgQuaternionLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgQuaternionLinearInterpolater::attributes->current, *matrix, sgMayaJointMatrix::attributes->orientation );
		}
		if ( isAnimated( positions ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$posint";
			interpolator->data( positions );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaJointMatrix::attributes->translation );
		}
		if ( matrix->order() != sgMayaTransformMatrix::invalid && 
			isAnimated( angles ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$angint";
			interpolator->data( angles );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaJointMatrix::attributes->angle );
		}
	}
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		scales.reserve( size );
		scaleOrientations.reserve( size );
		rotations.reserve( size );
		orientations.reserve( size );
		positions.reserve( size );
		angles.reserve( size );
	}
	virtual void set()
	{
		if ( ! active ) return;
		convert( fnIkJoint, scales.push_back( vector3() ), scaleOrientations.push_back( quaternion() ), 
			rotations.push_back( quaternion() ), orientations.push_back( quaternion() ), positions.push_back( vector3() ), angles.push_back( vector3() ) );
	}
};
sgBone * Maya::searchBone( MDagPath & dPath )
{
	string name = getName( dPath );
	MStatus status;
	MFnIkJoint fnIkJoint( dPath.node(), & status );
	if ( status == MS::kSuccess ) {
		string boneName = name + "$bone";
		for ( list<sgBone*>::iterator it( bones ); it; ++it ) {
			if ( it()->name == boneName ) {
				return it();
			}
		}
	}
	return NULL;
}

sgDagNode * Maya::exportJoint( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & parentTransform )
{
	MStatus status;
	MFnIkJoint fnIkJoint( dPath, & status ); statusError( status, "Fail MFnIkJoint" );
	string name = getName( dPath );

	sgTransformMatrix * transformMatrix = new sgTransformMatrix( *graph );
	transformMatrix->name = name + "$transform";

	sgMayaJointMatrix * matrix = new sgMayaJointMatrix( *graph );
	matrix->name = name + "$joint";
	new sgConnection( *matrix, sgMayaJointMatrix::attributes->matrix, *transformMatrix, sgTransformMatrix::attributes->local );
	//親の行列を連結して、子供に渡す親の行列を更新
	if ( parentTransform ) new sgConnection( *parentTransform, sgTransformMatrix::attributes->global, *transformMatrix, sgTransformMatrix::attributes->parent );
	convert( fnIkJoint, matrix->scale(), matrix->scaleOrientation(), matrix->rotation(), matrix->orientation(), matrix->translation(), matrix->angle() );

	//制限情報を取得
	matrix->limitedMinimumScale().x = fnIkJoint.isLimited( MFnTransform::kScaleMinX, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumScale().x = fnIkJoint.isLimited( MFnTransform::kScaleMaxX, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMinimumScale().y = fnIkJoint.isLimited( MFnTransform::kScaleMinY, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumScale().y = fnIkJoint.isLimited( MFnTransform::kScaleMaxY, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMinimumScale().z = fnIkJoint.isLimited( MFnTransform::kScaleMinZ, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumScale().z = fnIkJoint.isLimited( MFnTransform::kScaleMaxZ, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	if ( matrix->limitedMinimumScale().x ) matrix->limitMinimumScale().x = fnIkJoint.limitValue( MFnTransform::kScaleMinX, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumScale().x ) matrix->limitMaximumScale().x = fnIkJoint.limitValue( MFnTransform::kScaleMaxX, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMinimumScale().y ) matrix->limitMinimumScale().y = fnIkJoint.limitValue( MFnTransform::kScaleMinY, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumScale().y ) matrix->limitMaximumScale().y = fnIkJoint.limitValue( MFnTransform::kScaleMaxY, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMinimumScale().z ) matrix->limitMinimumScale().z = fnIkJoint.limitValue( MFnTransform::kScaleMinZ, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumScale().z ) matrix->limitMaximumScale().z = fnIkJoint.limitValue( MFnTransform::kScaleMaxZ, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );

	matrix->limitedMinimumRotate().x = fnIkJoint.isLimited( MFnTransform::kRotateMinX, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumRotate().x = fnIkJoint.isLimited( MFnTransform::kRotateMaxX, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMinimumRotate().y = fnIkJoint.isLimited( MFnTransform::kRotateMinY, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumRotate().y = fnIkJoint.isLimited( MFnTransform::kRotateMaxY, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMinimumRotate().z = fnIkJoint.isLimited( MFnTransform::kRotateMinZ, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumRotate().z = fnIkJoint.isLimited( MFnTransform::kRotateMaxZ, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	if ( matrix->limitedMinimumRotate().x ) matrix->limitMinimumRotate().x = fnIkJoint.limitValue( MFnTransform::kRotateMinX, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumRotate().x ) matrix->limitMaximumRotate().x = fnIkJoint.limitValue( MFnTransform::kRotateMaxX, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMinimumRotate().y ) matrix->limitMinimumRotate().y = fnIkJoint.limitValue( MFnTransform::kRotateMinY, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumRotate().y ) matrix->limitMaximumRotate().y = fnIkJoint.limitValue( MFnTransform::kRotateMaxY, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMinimumRotate().z ) matrix->limitMinimumRotate().z = fnIkJoint.limitValue( MFnTransform::kRotateMinZ, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumRotate().z ) matrix->limitMaximumRotate().z = fnIkJoint.limitValue( MFnTransform::kRotateMaxZ, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );

	matrix->limitedMinimumTranslate().x = fnIkJoint.isLimited( MFnTransform::kTranslateMinX, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumTranslate().x = fnIkJoint.isLimited( MFnTransform::kTranslateMaxX, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMinimumTranslate().y = fnIkJoint.isLimited( MFnTransform::kTranslateMinY, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumTranslate().y = fnIkJoint.isLimited( MFnTransform::kTranslateMaxY, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMinimumTranslate().z = fnIkJoint.isLimited( MFnTransform::kTranslateMinZ, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	matrix->limitedMaximumTranslate().z = fnIkJoint.isLimited( MFnTransform::kTranslateMaxZ, & status ) ? 1 : 0; statusError( status, "Fail MFnIkJoint::isLimited" );
	if ( matrix->limitedMinimumTranslate().x ) matrix->limitMinimumTranslate().x = fnIkJoint.limitValue( MFnTransform::kTranslateMinX, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumTranslate().x ) matrix->limitMaximumTranslate().x = fnIkJoint.limitValue( MFnTransform::kTranslateMaxX, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMinimumTranslate().y ) matrix->limitMinimumTranslate().y = fnIkJoint.limitValue( MFnTransform::kTranslateMinY, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumTranslate().y ) matrix->limitMaximumTranslate().y = fnIkJoint.limitValue( MFnTransform::kTranslateMaxY, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMinimumTranslate().z ) matrix->limitMinimumTranslate().z = fnIkJoint.limitValue( MFnTransform::kTranslateMinZ, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );
	if ( matrix->limitedMaximumTranslate().z ) matrix->limitMaximumTranslate().z = fnIkJoint.limitValue( MFnTransform::kTranslateMaxZ, & status ); statusError( status, "Fail MFnIkJoint::limitValue" );

	//回転の許される軸を取得
	bool freeInX, freeInY, freeInZ;
	statusError( fnIkJoint.getDegreesOfFreedom( freeInX, freeInY, freeInZ ), "Fail MFnIkJoint::getDegreesOfFreedom" );
	matrix->freedom() = vector3( freeInX ? 1 : 0, freeInY ? 1 : 0, freeInZ ? 1 : 0 );

	//回転の順序を取得
	MTransformationMatrix::RotationOrder order = MTransformationMatrix::kXYZ;
	double angle[3];
	statusError( fnIkJoint.getRotation( angle, order ), "Fail MFnIkJoint::getRotation" );
	switch ( order ) {
	case MTransformationMatrix::kXYZ: matrix->order() = sgMayaTransformMatrix::xyz; break;
	case MTransformationMatrix::kYZX: matrix->order() = sgMayaTransformMatrix::yzx; break;
	case MTransformationMatrix::kZXY: matrix->order() = sgMayaTransformMatrix::zxy; break;
	case MTransformationMatrix::kXZY: matrix->order() = sgMayaTransformMatrix::xzy; break;
	case MTransformationMatrix::kYXZ: matrix->order() = sgMayaTransformMatrix::yxz; break;
	case MTransformationMatrix::kZYX: matrix->order() = sgMayaTransformMatrix::zyx; break;
	}
	matrix->rotation() = quaternion::identity;//回転は必要じゃない

	if ( parentTransform ) {
		sgMayaJointMatrix * parentJoint = ( sgMayaJointMatrix * ) parentTransform->getInput( sgTransformMatrix::attributes->local, sgMayaJointMatrix::descriptor );
		if ( parentJoint ) new sgConnection( *parentJoint, sgMayaJointMatrix::attributes->scale, *matrix, sgMayaJointMatrix::attributes->inverseScale );
	}
	matrix->update();
	transformMatrix->update();

	sgBone * bone = new sgBone( *graph );
	bones.push_back( bone );
	bone->name = name + "$bone";
	bone->initialMatrix( transformMatrix->global() );
	new sgConnection( *transformMatrix, sgTransformMatrix::attributes->global, *bone, sgBone::attributes->currentMatrix );
	bone->update();

	animationNodes.push_back( new jointAnimation( dPath, matrix, name ) );

	parentTransform = transformMatrix;
//	return parent;
	return parent->insert( *transformMatrix );
}
