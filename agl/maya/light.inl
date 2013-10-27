#include <maya/MFnAmbientLight.h>
#include <maya/MFnPointLight.h>
#include <maya/MFnSpotLight.h>
#include <maya/MFnDirectionalLight.h>
#include <maya/MFnAreaLight.h>

void convert( MFnAmbientLight & fnAmbientLight, vector3 & ambient )
{
	ambient = ConvertColor( fnAmbientLight.color() ).getVector3();
}

void convert( MFnPointLight & fnPointLight, vector3 & diffuse, vector3 & specular, vector3 & translation )
{
	vector3 color = ConvertColor( fnPointLight.color() );
	MPlug plug;
	bool emitDiffuse = true, emitSpecular = true;
	if ( Maya::getAttribute( fnPointLight.object(), plug, "emitDiffuse" ) ) {
		plug.getValue( emitDiffuse );
	}
	if ( Maya::getAttribute( fnPointLight.object(), plug, "emitSpecular" ) ) {
		plug.getValue( emitSpecular );
	}
	diffuse = emitDiffuse ? color : vector3::black;
	specular = emitSpecular ? color : vector3::black;

	translation = vector3::identity;
	MDagPath parent;
	fnPointLight.getPath( parent );
	if ( parent.pathCount() ) parent.pop();
	MStatus status;
	MFnTransform fnTransform( parent, & status );
	if ( status == MS::kSuccess ) {
		translation = ConvertPosition( fnTransform.translation( MSpace::kWorld ) );
	}
}

void convert( MFnDirectionalLight & fnDirectionalLight, vector3 & diffuse, vector3 & specular, vector3 & translation, vector3 & interest )
{
	vector3 color = ConvertColor( fnDirectionalLight.color() );
	MPlug plug;
	bool emitDiffuse = true, emitSpecular = true;
	if ( Maya::getAttribute( fnDirectionalLight.object(), plug, "emitDiffuse" ) ) {
		plug.getValue( emitDiffuse );
	}
	if ( Maya::getAttribute( fnDirectionalLight.object(), plug, "emitSpecular" ) ) {
		plug.getValue( emitSpecular );
	}
	diffuse = emitDiffuse ? color : vector3::black;
	specular = emitSpecular ? color : vector3::black;

	translation = vector3( 0, 0, 0 );
	interest = vector3( 0, 0, -1 );
	MDagPath parent;
	fnDirectionalLight.getPath( parent );
	if ( parent.pathCount() ) parent.pop();
	MStatus status;
	MFnTransform fnTransform( parent, & status );
	if ( status == MS::kSuccess ) {
		sgMayaTransformMatrix matrix( *Maya::graph );
		convert( fnTransform, matrix.scale(), matrix.rotateOrientation(), matrix.rotation(), matrix.translation(), 
			 matrix.shear(), matrix.scalePivot(), matrix.scalePivotTranslation(), matrix.rotatePivot(), matrix.rotatePivotTranslation(), matrix.angle(),
			 MSpace::kWorld );
		matrix.update();
		matrix.matrix( matrix44::removeOuter( matrix.matrix() ) );
		interest = vector3::normalize3( ( vector4( 0, 0, -1 ) * matrix.matrix() ).getVector3() );
	}
}

void convert( MFnSpotLight & fnSpotLight, vector3 & diffuse, vector3 & specular, vector3 & translation, vector3 & interest, float & spotInsideAngle, float & spotOutsideAngle )
{
	vector3 color = ConvertColor( fnSpotLight.color() );
	MPlug plug;
	bool emitDiffuse = true, emitSpecular = true;
	if ( Maya::getAttribute( fnSpotLight.object(), plug, "emitDiffuse" ) ) {
		plug.getValue( emitDiffuse );
	}
	if ( Maya::getAttribute( fnSpotLight.object(), plug, "emitSpecular" ) ) {
		plug.getValue( emitSpecular );
	}
	diffuse = emitDiffuse ? color : vector3::black;
	specular = emitSpecular ? color : vector3::black;

	spotInsideAngle = clamp<float>( 0, fnSpotLight.coneAngle() * 2, 360 );
	spotOutsideAngle = fnSpotLight.penumbraAngle() * 2;
	if ( 0 < spotOutsideAngle ) {
		spotOutsideAngle = clamp<float>( 0, spotInsideAngle + spotOutsideAngle, 360 );
	} else {
		spotOutsideAngle = clamp<float>( 0, spotInsideAngle - spotOutsideAngle, 360 );
		swap( spotInsideAngle, spotOutsideAngle );
	}

	translation = vector3::identity;
	interest = vector3( 0, 0, -1 );
	MDagPath parent;
	fnSpotLight.getPath( parent );
	if ( parent.pathCount() ) parent.pop();
	MStatus status;
	MFnTransform fnTransform( parent, & status );
	if ( status == MS::kSuccess ) {
		sgMayaTransformMatrix matrix( *Maya::graph );
		convert( fnTransform, matrix.scale(), matrix.rotateOrientation(), matrix.rotation(), matrix.translation(), 
			 matrix.shear(), matrix.scalePivot(), matrix.scalePivotTranslation(), matrix.rotatePivot(), matrix.rotatePivotTranslation(), matrix.angle(), 
			 MSpace::kWorld );
		matrix.update();
		matrix.matrix( matrix44::removeOuter( matrix.matrix() ) );
		interest = vector3::normalize3( ( vector4( 0, 0, -1 ) * matrix.matrix() ).getVector3() );
		interest = interest + translation;
	}
}

class ambientLightAnimation : public animationNode
{
public:
	bool active;
	MDagPath dPath;
	sgAmbientLight * light;
	array<vector3> ambients;
	string name;
	ambientLightAnimation( const MDagPath & _dPath, sgAmbientLight * _light, const char * _name ) : dPath( _dPath ), light( _light ), name( _name )
	{
		active = true;
	}
	virtual void finish()
	{
		if ( ! active ) return;
		if ( isAnimated( ambients ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$ambint";
			interpolator->data( ambients );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgAmbientLight::attributes->ambient );
		}
		active = false;
	}
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		ambients.reserve( size );
	}
	virtual void set()
	{
		if ( ! active ) return;
		MFnAmbientLight fnAmbientLight( dPath );
		convert( fnAmbientLight, ambients.push_back( vector3() ) );
	}
};

class positionLightAnimation : public animationNode
{
public:
	bool active;
	MDagPath dPath;
	sgPositionLight * light;
	array<vector3> diffuses, speculars, positions;
	string name;
	positionLightAnimation( const MDagPath & _dPath, sgPositionLight * _light, const char * _name ) : dPath( _dPath ), light( _light ), name( _name )
	{
		active = true;
	}
	virtual void finish()
	{
		if ( ! active ) return;
		if ( isAnimated( diffuses ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$dffint";
			interpolator->data( diffuses );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgPositionLight::attributes->diffuse );
		}
		if ( isAnimated( speculars ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$spcint";
			interpolator->data( speculars );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgPositionLight::attributes->specular );
		}
		if ( isAnimated( positions ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$posint";
			interpolator->data( positions );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgPositionLight::attributes->position );
		}
		active = false;
	}
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		diffuses.reserve( size );
		speculars.reserve( size );
		positions.reserve( size );
	}
	virtual void set()
	{
		if ( ! active ) return;
		MFnPointLight fnPointLight( dPath );
		convert( fnPointLight, diffuses.push_back( vector3() ), speculars.push_back( vector3() ), positions.push_back( vector3() ) );
	}
};

class directionLightAnimation : public animationNode
{
public:
	bool active;
	MDagPath dPath;
	sgDirectionLight * light;
	array<vector3> diffuses, speculars, positions, interests;
	string name;
	directionLightAnimation( const MDagPath & _dPath, sgDirectionLight * _light, const char * _name ) : dPath( _dPath ), light( _light ), name( _name )
	{
		active = true;
	}
	virtual void finish()
	{
		if ( ! active ) return;
		if ( isAnimated( diffuses ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$dffint";
			interpolator->data( diffuses );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgDirectionLight::attributes->diffuse );
		}
		if ( isAnimated( speculars ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$spcint";
			interpolator->data( speculars );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgDirectionLight::attributes->specular );
		}
		if ( isAnimated( positions ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$posint";
			interpolator->data( positions );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgDirectionLight::attributes->position );
		}
		if ( isAnimated( interests ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$intint";
			interpolator->data( interests );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgDirectionLight::attributes->interest );
		}
		active = false;
	}
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		diffuses.reserve( size );
		speculars.reserve( size );
		positions.reserve( size );
		interests.reserve( size );
	}
	virtual void set()
	{
		if ( ! active ) return;
		MFnDirectionalLight fnDirectionalLight( dPath );
		convert( fnDirectionalLight, diffuses.push_back( vector3() ), speculars.push_back( vector3() ), positions.push_back( vector3() ), interests.push_back( vector3() ) );
	}
};

class spotLightAnimation : public animationNode
{
public:
	bool active;
	MDagPath dPath;
	sgSpotLight * light;
	array<vector3> diffuses, speculars, positions, interests;
	array<decimal> insideAngles, outsideAngles;
	string name;
	spotLightAnimation( const MDagPath & _dPath, sgSpotLight * _light, const char * _name ) : dPath( _dPath ), light( _light ), name( _name )
	{
		active = true;
	}
	virtual void finish()
	{
		if ( ! active ) return;
		if ( isAnimated( diffuses ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$dffint";
			interpolator->data( diffuses );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgSpotLight::attributes->diffuse );
		}
		if ( isAnimated( speculars ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$spcint";
			interpolator->data( speculars );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgSpotLight::attributes->specular );
		}
		if ( isAnimated( positions ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$posint";
			interpolator->data( positions );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgSpotLight::attributes->position );
		}
		if ( isAnimated( interests ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$intint";
			interpolator->data( interests );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *light, sgSpotLight::attributes->interest );
		}
		if ( isAnimated( insideAngles ) ) {
			sgDecimalLinearInterpolater * interpolator = new sgDecimalLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$insint";
			interpolator->data( insideAngles );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgDecimalLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgDecimalLinearInterpolater::attributes->current, *light, sgSpotLight::attributes->insideAngle );
		}
		if ( isAnimated( outsideAngles ) ) {
			sgDecimalLinearInterpolater * interpolator = new sgDecimalLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$outint";
			interpolator->data( outsideAngles );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgDecimalLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgDecimalLinearInterpolater::attributes->current, *light, sgSpotLight::attributes->outsideAngle );
		}
		active = false;
	}
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		diffuses.reserve( size );
		speculars.reserve( size );
		positions.reserve( size );
		interests.reserve( size );
		insideAngles.reserve( size );
		outsideAngles.reserve( size );
	}
	virtual void set()
	{
		if ( ! active ) return;
		MFnSpotLight fnSpotLight( dPath );
		convert( fnSpotLight, diffuses.push_back( vector3() ), speculars.push_back( vector3() ), positions.push_back( vector3() ), interests.push_back( vector3() ), insideAngles.push_back( 0 ), outsideAngles.push_back( 0 ) );
	}
};

void Maya::exportAmbientLight( MDagPath & dPath )
{
	MFnAmbientLight fnAmbientLight( dPath );
	if ( fnAmbientLight.ambientShade() ) return;//少しでもポイントライトに近くなるならば無効
	string name = getName( dPath );
	sgAmbientLight * light = new sgAmbientLight( *graph );
	light->name = name + "$ambientLight";
	convert( fnAmbientLight, light->ambient() );
	animationNodes.push_back( new ambientLightAnimation( dPath, light, name ) );
}

void Maya::exportPointLight( MDagPath & dPath )
{
	MFnPointLight fnPointLight( dPath );
	string name = getName( dPath );
	sgPositionLight * light = new sgPositionLight( *graph );
	light->name = name + "$positionLight";
	convert( fnPointLight, light->diffuse(), light->specular(), light->position() );
	animationNodes.push_back( new positionLightAnimation( dPath, light, name ) );
}

void Maya::exportDirectionalLight( MDagPath & dPath )
{
	MFnDirectionalLight fnDirectionalLight( dPath );
	string name = getName( dPath );
	sgDirectionLight * light = new sgDirectionLight( *graph );
	light->name = name + "$directionLight";
	convert( fnDirectionalLight, light->diffuse(), light->specular(), light->position(), light->interest() );
	animationNodes.push_back( new directionLightAnimation( dPath, light, name ) );
}

void Maya::exportSpotLight( MDagPath & dPath )
{
	MFnSpotLight fnSpotLight( dPath );
	string name = getName( dPath );
	sgSpotLight * light = new sgSpotLight( *graph );
	light->name = name + "$spotLight";
	convert( fnSpotLight, light->diffuse(), light->specular(), light->position(), light->interest(), light->insideAngle(), light->outsideAngle() );
	animationNodes.push_back( new spotLightAnimation( dPath, light, name ) );
}

void Maya::exportLight( MDagPath & dPath )
{
	switch ( dPath.node().apiType() ) {
	case MFn::kAmbientLight: exportAmbientLight( dPath ); break;
	case MFn::kDirectionalLight: exportDirectionalLight( dPath ); break;
	case MFn::kPointLight: exportPointLight( dPath ); break;
	case MFn::kSpotLight: exportSpotLight( dPath ); break;
	case MFn::kAreaLight: break;
	}
}

