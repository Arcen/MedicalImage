////////////////////////////////////////////////////////////////////////////////
// TRANSFORM
decimal ConvertDecimal( decimal v )
{
	if ( between( - roughEpsilon, v, roughEpsilon ) ) return 0;
	return v;
}
vector3 ConvertPosition( const MVector & v )
{
	return vector3( ConvertDecimal( v.x ), ConvertDecimal( v.y ), ConvertDecimal( v.z ) );
}
vector3 ConvertScale( const double v[3] )
{
	return vector3( ConvertDecimal( v[0] ), ConvertDecimal( v[1] ), ConvertDecimal( v[2] ) );
}
vector3 ConvertShear( const double v[3] )
{
	return vector3( ConvertDecimal( v[0] ), ConvertDecimal( v[2] ), ConvertDecimal( v[1] ) );
}
quaternion ConvertRotation( const MQuaternion & v )
{
	if ( between<decimal>( - epsilon, v[0], epsilon ) && between<decimal>( - epsilon, v[1], epsilon ) && between<decimal>( - epsilon, v[2], epsilon ) ) return quaternion::identity;
	return quaternion::validate( quaternion( v[0], v[1], v[2], v[3] ) );
}
vector3 ConvertAngle( const double a[3] )
{
	return vector3( ConvertDecimal( a[0] ), ConvertDecimal( a[1] ), ConvertDecimal( a[2] ) );
}

sgTime * animationNode::time = NULL;

template<class T>
inline bool isAnimated( array<T> & a )
{
	if ( ! a.size ) return false;
	T & a0 = a[0];
	for ( int i = 1; i < a.size; i++ ) {
		if ( a0 != a[i] ) return true;
	}
	return false;
}
void convert( MFnTransform & fnTransform, vector3 & _scale, quaternion & _rotateOrientation, quaternion & _rotation, vector3 & _translation, 
			 vector3 & _shear, vector3 & _scalePivot, vector3 & _scalePivotTranslation, vector3 & _rotatePivot, vector3 & _rotatePivotTranslation, 
			 vector3 & _angle, 
			 MSpace::Space space = MSpace::kTransform )
{
	MStatus status;

	double scale[3], shear[3], angle[3];
	MQuaternion rotation, rotateOrientation;
	MVector translation, scalePivotTranslation, rotatePivotTranslation;
	MPoint scalePivot, rotatePivot;
	MTransformationMatrix::RotationOrder order = MTransformationMatrix::kXYZ;
	Maya::statusError( fnTransform.getScale( scale ), "Fail MFnTransform::getScale" );
	Maya::statusError( fnTransform.getShear( shear ), "Fail MFnTransform::getShear" );
	Maya::statusError( fnTransform.getRotation( rotation, space ), "Fail MFnTransform::getRotation" );
	rotateOrientation = fnTransform.rotateOrientation( space, & status ); Maya::statusError( status, "Fail MFnTransform::rotateOrientation" );
	translation = fnTransform.translation( space, & status ); Maya::statusError( status, "Fail MFnTransform::translation" );
	scalePivot = fnTransform.scalePivot( space, & status ); Maya::statusError( status, "Fail MFnTransform::scalePivot" );
	scalePivotTranslation = fnTransform.scalePivotTranslation( space, & status ); Maya::statusError( status, "Fail MFnTransform::scalePivotTranslation" );
	rotatePivot = fnTransform.rotatePivot( space, & status ); Maya::statusError( status, "Fail MFnTransform::rotatePivot" );
	rotatePivotTranslation = fnTransform.rotatePivotTranslation( space, & status ); Maya::statusError( status, "Fail MFnTransform::rotatePivotTranslation" );
	Maya::statusError( fnTransform.getRotation( angle, order ), "Fail MFnTransform::getRotation" );

	_scale = ConvertScale( scale );
	_rotateOrientation = ConvertRotation( rotateOrientation );
	_rotation = ConvertRotation( rotation );
	_translation = ConvertPosition( translation );

	_shear = ConvertShear( shear );
	_scalePivot = ConvertPosition( scalePivot );
	_scalePivotTranslation = ConvertPosition( scalePivotTranslation );
	_rotatePivot = ConvertPosition( rotatePivot );
	_rotatePivotTranslation = ConvertPosition( rotatePivotTranslation );

	_angle = ConvertPosition( angle );
}
//transformアニメーション情報処理
class transformAnimation : public animationNode
{
public:
	//情報が必要かどうか
	bool active;
	//オブジェクトのパス
	MDagPath dPath;
	MFnTransform fnTransform;
	//ノードへのポインタ（これに補間クラスから出力する）
	sgMayaTransformMatrix * matrix;
	//各配列には同じキーフレーム分の情報が入っている
	array<quaternion> rotateOrientations, rotations;
	array<vector3> positions, scales, angles;
	array<vector3> scalePivots, scalePivotTranslations, rotatePivots, rotatePivotTranslations, shears;
	//名前
	string name;
	//コンストラクタ
	transformAnimation( const MDagPath & _dPath, sgMayaTransformMatrix * _matrix, const char * _name ) 
		: dPath( _dPath ), fnTransform( _dPath ), matrix( _matrix ), name( _name )
	{
		active = true;
	}
	//終了化
	virtual void finish()
	{
		//アニメーション情報が必要かチェック
		if ( ! active ) return;

		//データが単一の情報のみをもっている場合には，補間は必要ない
		int size = rotations.size;
		if ( isAnimated( rotateOrientations ) ) {
			//補間用のクラスを作成
			sgQuaternionLinearInterpolater * interpolator = new sgQuaternionLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$rorint";//名前を設定
			interpolator->data( rotateOrientations );//情報を設定
			//時間情報を補間クラスに接続
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgQuaternionLinearInterpolater::attributes->time );
			//補間後の情報を対象に接続
			new sgConnection( *interpolator, sgQuaternionLinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->rotateOrientation );
		}
		if ( matrix->order() == sgMayaTransformMatrix::invalid && 
			isAnimated( rotations ) ) {
			sgQuaternionLinearInterpolater * interpolator = new sgQuaternionLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$rotint";
			interpolator->data( rotations );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgQuaternionLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgQuaternionLinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->rotation );
		}
		if ( isAnimated( positions ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$posint";
			interpolator->data( positions );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->translation );
		}
		if ( isAnimated( scales ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$sclint";
			interpolator->data( scales );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->scale );
		}
		if ( isAnimated( shears ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$shrint";
			interpolator->data( shears );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->shear );
		}
		if ( isAnimated( scalePivots ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$spvint";
			interpolator->data( scalePivots );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->scalePivot );
		}
		if ( isAnimated( scalePivotTranslations ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$srtint";
			interpolator->data( scalePivotTranslations );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->scalePivotTranslation );
		}
		if ( isAnimated( rotatePivots ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$rpvint";
			interpolator->data( rotatePivots );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->rotatePivot );
		}
		if ( isAnimated( rotatePivotTranslations ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$rptint";
			interpolator->data( rotatePivotTranslations );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->rotatePivotTranslation );
		}
		//回転の順番が指定されているときには角度情報を使う．
		if ( matrix->order() != sgMayaTransformMatrix::invalid && 
			isAnimated( angles ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$angint";
			interpolator->data( angles );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *matrix, sgMayaTransformMatrix::attributes->angle );
		}
		active = false;
	}
	//メモリを確保する
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		rotateOrientations.reserve( size );
		rotations.reserve( size );
		positions.reserve( size );
		scales.reserve( size );
		shears.reserve( size );
		scalePivots.reserve( size );
		scalePivotTranslations.reserve( size );
		rotatePivots.reserve( size );
		rotatePivotTranslations.reserve( size );
		angles.reserve( size );
	}
	//各フレームでの情報を取得
	virtual void set()
	{
		if ( ! active ) return;
		convert( fnTransform, scales.push_back( vector3() ), rotateOrientations.push_back( quaternion() ), rotations.push_back( quaternion() ), positions.push_back( vector3() ), 
			shears.push_back( vector3() ), scalePivots.push_back( vector3() ), scalePivotTranslations.push_back( vector3() ), rotatePivots.push_back( vector3() ), rotatePivotTranslations.push_back( vector3() ), angles.push_back( vector3() ) );
	}
};
sgDagNode * Maya::exportDagTransform( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & parentTransform )
{
	MStatus status;
	MFnTransform fnTransform( dPath, & status ); statusError( status, "Fail MFnTransform" );
	string name = getName( dPath );

	sgTransformMatrix * transformMatrix = new sgTransformMatrix( *graph );
	transformMatrix->name = name + "$transform";

	sgMayaTransformMatrix * matrix = new sgMayaTransformMatrix( *graph );
	matrix->name = name + "$matrix";
	new sgConnection( *matrix, sgMayaTransformMatrix::attributes->matrix, *transformMatrix, sgTransformMatrix::attributes->local );
	//親の行列を連結して、子供に渡す親の行列を更新
	if ( parentTransform ) new sgConnection( *parentTransform, sgTransformMatrix::attributes->global, *transformMatrix, sgTransformMatrix::attributes->parent );

	convert( fnTransform, matrix->scale(), matrix->rotateOrientation(), matrix->rotation(), 
		matrix->translation(), matrix->shear(), 
		matrix->scalePivot(), matrix->scalePivotTranslation(), 
		matrix->rotatePivot(), matrix->rotatePivotTranslation(), matrix->angle() );
	matrix->angle() = vector3( 0, 0, 0 );//角度は必要じゃない

	matrix->limitedMinimumScale().x = fnTransform.isLimited( MFnTransform::kScaleMinX, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumScale().x = fnTransform.isLimited( MFnTransform::kScaleMaxX, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMinimumScale().y = fnTransform.isLimited( MFnTransform::kScaleMinY, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumScale().y = fnTransform.isLimited( MFnTransform::kScaleMaxY, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMinimumScale().z = fnTransform.isLimited( MFnTransform::kScaleMinZ, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumScale().z = fnTransform.isLimited( MFnTransform::kScaleMaxZ, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	if ( matrix->limitedMinimumScale().x ) matrix->limitMinimumScale().x = fnTransform.limitValue( MFnTransform::kScaleMinX, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumScale().x ) matrix->limitMaximumScale().x = fnTransform.limitValue( MFnTransform::kScaleMaxX, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMinimumScale().y ) matrix->limitMinimumScale().y = fnTransform.limitValue( MFnTransform::kScaleMinY, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumScale().y ) matrix->limitMaximumScale().y = fnTransform.limitValue( MFnTransform::kScaleMaxY, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMinimumScale().z ) matrix->limitMinimumScale().z = fnTransform.limitValue( MFnTransform::kScaleMinZ, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumScale().z ) matrix->limitMaximumScale().z = fnTransform.limitValue( MFnTransform::kScaleMaxZ, & status ); statusError( status, "Fail MFnTransform::limitValue" );

	matrix->limitedMinimumRotate().x = fnTransform.isLimited( MFnTransform::kRotateMinX, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumRotate().x = fnTransform.isLimited( MFnTransform::kRotateMaxX, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMinimumRotate().y = fnTransform.isLimited( MFnTransform::kRotateMinY, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumRotate().y = fnTransform.isLimited( MFnTransform::kRotateMaxY, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMinimumRotate().z = fnTransform.isLimited( MFnTransform::kRotateMinZ, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumRotate().z = fnTransform.isLimited( MFnTransform::kRotateMaxZ, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	if ( matrix->limitedMinimumRotate().x ) matrix->limitMinimumRotate().x = fnTransform.limitValue( MFnTransform::kRotateMinX, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumRotate().x ) matrix->limitMaximumRotate().x = fnTransform.limitValue( MFnTransform::kRotateMaxX, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMinimumRotate().y ) matrix->limitMinimumRotate().y = fnTransform.limitValue( MFnTransform::kRotateMinY, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumRotate().y ) matrix->limitMaximumRotate().y = fnTransform.limitValue( MFnTransform::kRotateMaxY, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMinimumRotate().z ) matrix->limitMinimumRotate().z = fnTransform.limitValue( MFnTransform::kRotateMinZ, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumRotate().z ) matrix->limitMaximumRotate().z = fnTransform.limitValue( MFnTransform::kRotateMaxZ, & status ); statusError( status, "Fail MFnTransform::limitValue" );

	matrix->limitedMinimumTranslate().x = fnTransform.isLimited( MFnTransform::kTranslateMinX, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumTranslate().x = fnTransform.isLimited( MFnTransform::kTranslateMaxX, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMinimumTranslate().y = fnTransform.isLimited( MFnTransform::kTranslateMinY, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumTranslate().y = fnTransform.isLimited( MFnTransform::kTranslateMaxY, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMinimumTranslate().z = fnTransform.isLimited( MFnTransform::kTranslateMinZ, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	matrix->limitedMaximumTranslate().z = fnTransform.isLimited( MFnTransform::kTranslateMaxZ, & status ) ? 1 : 0; statusError( status, "Fail MFnTransform::isLimited" );
	if ( matrix->limitedMinimumTranslate().x ) matrix->limitMinimumTranslate().x = fnTransform.limitValue( MFnTransform::kTranslateMinX, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumTranslate().x ) matrix->limitMaximumTranslate().x = fnTransform.limitValue( MFnTransform::kTranslateMaxX, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMinimumTranslate().y ) matrix->limitMinimumTranslate().y = fnTransform.limitValue( MFnTransform::kTranslateMinY, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumTranslate().y ) matrix->limitMaximumTranslate().y = fnTransform.limitValue( MFnTransform::kTranslateMaxY, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMinimumTranslate().z ) matrix->limitMinimumTranslate().z = fnTransform.limitValue( MFnTransform::kTranslateMinZ, & status ); statusError( status, "Fail MFnTransform::limitValue" );
	if ( matrix->limitedMaximumTranslate().z ) matrix->limitMaximumTranslate().z = fnTransform.limitValue( MFnTransform::kTranslateMaxZ, & status ); statusError( status, "Fail MFnTransform::limitValue" );

	matrix->update();
	transformMatrix->update();

	animationNodes.push_back( new transformAnimation( dPath, matrix, name ) );

	parentTransform = transformMatrix;
	return parent->insert( *transformMatrix );
}
