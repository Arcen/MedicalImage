
void convert( MFnCamera & fnCamera, vector3 & position, vector3 & interest, vector3 & upper )
{
	position = ConvertPosition( fnCamera.eyePoint( MSpace::kWorld ) );
	interest = ConvertPosition( fnCamera.centerOfInterestPoint( MSpace::kWorld ) );
	upper = ConvertPosition( fnCamera.upDirection( MSpace::kWorld ) );
}

class cameraAnimation : public animationNode
{
public:
	bool active;
	MDagPath dPath;
	sgCamera * camera;
	array<vector3> positions, interests, uppers;
	string name;
	cameraAnimation( const MDagPath & _dPath, sgCamera * _camera, const char * _name ) : dPath( _dPath ), camera( _camera ), name( _name )
	{
		active = true;
	}
	virtual void finish()
	{
		if ( ! active ) return;
		if ( isAnimated( positions ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$posint";
			interpolator->data( positions );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *camera, sgCamera::attributes->position );
		}
		if ( isAnimated( interests ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$intint";
			interpolator->data( interests );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *camera, sgCamera::attributes->interest );
		}
		if ( isAnimated( uppers ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$upint";
			interpolator->data( uppers );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *camera, sgCamera::attributes->upper );
		}
	}
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		positions.reserve( size );
		interests.reserve( size );
		uppers.reserve( size );
	}
	virtual void set()
	{
		if ( ! active ) return;
		MFnCamera fnCamera( dPath );
		convert( fnCamera, positions.push_back( vector3() ), interests.push_back( vector3() ), uppers.push_back( vector3() ) );
	}
};

void Maya::exportCamera( MDagPath & dPath )
{
	MFnCamera fnCamera( dPath.node() );
	string name = getName( dPath );

	bool ortho = fnCamera.isOrtho();

	sgCamera * camera = new sgCamera( *graph );
	camera->name = name + "$camera";
	animationNodes.push_back( new cameraAnimation( dPath, camera, name ) );
	convert( fnCamera, camera->position(), camera->interest(), camera->upper() );
	if ( ortho ) {
		sgOrtho * ortho = new sgOrtho( *graph );
		ortho->name = name + "$ortho";
		ortho->nearZ( fnCamera.nearClippingPlane() );
		ortho->farZ( fnCamera.farClippingPlane() );
		ortho->aspect( fnCamera.aspectRatio() );
		ortho->sizeY( fnCamera.orthoWidth() / ortho->aspect() );
		new sgConnection( *ortho, *sgOrtho::attributes, *camera, sgCamera::attributes->ortho );
	} else {
		sgPerspective * perspective = new sgPerspective( *graph );
		perspective->name = name + "$perspective";
		perspective->nearZ( fnCamera.nearClippingPlane() );
		perspective->farZ( fnCamera.farClippingPlane() );
		perspective->aspect( fnCamera.aspectRatio() );
		perspective->fieldOfViewY( rad2deg( fnCamera.verticalFieldOfView() ) );
		new sgConnection( *perspective, *sgPerspective::attributes, *camera, sgCamera::attributes->perspective );
	}
	return;
}

