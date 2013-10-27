
////////////////////////////////////////////////////////////////////////////////
// MATERIAL
float Color2Average( MColor & c )
{
	return ( c.r + c.g + c.b ) / 3;
}
struct MayaMaterial
{
	MObject material;
	MFnLambertShader * lambert;
	MFnBlinnShader * blinn;
	MFnPhongShader * phong;
	MFnReflectShader * reflect;
	MayaMaterial( MObject & mObj ) : lambert( NULL ), blinn( NULL ), phong( NULL ), reflect( NULL )
	{
		switch ( mObj.apiType() ) {
		case MFn::kLambert: lambert = new MFnLambertShader( mObj ); break;
		case MFn::kReflect: reflect = new MFnReflectShader( mObj ); break;
		case MFn::kBlinn: blinn = new MFnBlinnShader( mObj ); break;
		case MFn::kPhong: phong = new MFnPhongShader( mObj ); break;
		}
	}
	~MayaMaterial()
	{
		delete lambert;
		delete reflect;
		delete blinn;
		delete phong;
	}
	void set( sgMaterial * sgmaterial )
	{
		set( sgmaterial->ambient(), sgmaterial->diffuse(), sgmaterial->specular(), sgmaterial->emissive(), sgmaterial->specularPower(), sgmaterial->transparency() );
	}
	void set( vector3 & ambient, vector3 & diffuse, vector3 & specular, vector3 & emissive, float & specularPower, float & transparency )
	{
		vector4 a4, d4, s4, e4;
		set( a4, d4, s4, e4 );
		ambient = a4.getVector3();
		diffuse = d4.getVector3();
		specular = s4.getVector3();
		emissive = e4.getVector3();
		transparency = 1 - d4.w;
		specularPower = s4.w;
	}
	void set( vector4 & ambient, vector4 & diffuse, vector4 & specular, vector4 & emissive )
	{
		if ( lambert ) {
			ambient = ConvertColor( lambert->ambientColor() );
			diffuse = ConvertColor( lambert->color() );
			specular = vector4::black;
			emissive = ConvertColor( lambert->incandescence() );
			diffuse.w = 1 - Color2Average( lambert->transparency() );
			specular.w = 0;
		} else if ( phong ) {
			ambient = ConvertColor( phong->ambientColor() );
			diffuse = ConvertColor( phong->color() );
			specular = ConvertColor( phong->specularColor() );
			emissive = ConvertColor( phong->incandescence() );
			diffuse.w = 1 - Color2Average( phong->transparency() );
			specular.w = phong->cosPower();
		} else if ( blinn ) {
			ambient = ConvertColor( blinn->ambientColor() );
			diffuse = ConvertColor( blinn->color() );
			specular = ConvertColor( blinn->specularColor() );
			emissive = ConvertColor( blinn->incandescence() );
			diffuse.w = 1 - Color2Average( blinn->transparency() );
			specular.w = blinn->specularRollOff();
		} else if ( reflect ) {
			ambient = ConvertColor( reflect->ambientColor() );
			diffuse = ConvertColor( reflect->color() );
			specular = ConvertColor( reflect->specularColor() );
			emissive = ConvertColor( reflect->incandescence() );
			diffuse.w = 1 - Color2Average( reflect->transparency() );
			specular.w = 1;
		}
	}
};

void convert( MObject & material, vector4 & ambient, vector4 & diffuse, vector4 & specular, vector4 & emissive )
{
	MayaMaterial mm( material );
	mm.set( ambient, diffuse, specular, emissive );
}

void convert( MObject & material, vector3 & ambient, vector3 & diffuse, vector3 & specular, vector3 & emissive, float & specularPower, float & transparency  )
{
	MayaMaterial mm( material );
	mm.set( ambient, diffuse, specular, emissive, specularPower, transparency  );
}

class materialAnimation : public animationNode
{
public:
	bool active;
	MObject mObj;
	sgMaterial * material;
	array<vector3> ambients, diffuses, speculars, emissives;
	array<float> specularPowers, transparencys;
	string name;
	materialAnimation( const MObject & _mObj, sgMaterial * _material, const char * _name ) : mObj( _mObj ), material( _material ), name( _name )
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
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *material, sgMaterial::attributes->ambient );
		}
		if ( isAnimated( diffuses ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$dffint";
			interpolator->data( diffuses );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *material, sgMaterial::attributes->diffuse );
		}
		if ( isAnimated( speculars ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$spcint";
			interpolator->data( speculars );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *material, sgMaterial::attributes->specular );
		}
		if ( isAnimated( emissives ) ) {
			sgVector3LinearInterpolater * interpolator = new sgVector3LinearInterpolater( * Maya::graph );
			interpolator->name = name + "$emsint";
			interpolator->data( emissives );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgVector3LinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgVector3LinearInterpolater::attributes->current, *material, sgMaterial::attributes->emissive );
		}
		if ( isAnimated( specularPowers ) ) {
			sgDecimalLinearInterpolater * interpolator = new sgDecimalLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$pwrint";
			interpolator->data( specularPowers );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgDecimalLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgDecimalLinearInterpolater::attributes->current, *material, sgMaterial::attributes->specularPower );
		}
		if ( isAnimated( transparencys ) ) {
			sgDecimalLinearInterpolater * interpolator = new sgDecimalLinearInterpolater( * Maya::graph );
			interpolator->name = name + "$trnint";
			interpolator->data( transparencys );
			new sgConnection( *time, sgTime::attributes->current, *interpolator, sgDecimalLinearInterpolater::attributes->time );
			new sgConnection( *interpolator, sgDecimalLinearInterpolater::attributes->current, *material, sgMaterial::attributes->transparency );
		}
	}
	virtual void reserve( int size )
	{
		if ( ! active ) return;
		ambients.reserve( size );
		diffuses.reserve( size );
		speculars.reserve( size );
		emissives.reserve( size );
		specularPowers.reserve( size );
		transparencys.reserve( size );
	}
	virtual void set()
	{
		if ( ! active ) return;
		convert( mObj, ambients.push_back( vector3() ), diffuses.push_back( vector3() ), speculars.push_back( vector3() ), emissives.push_back( vector3() ), 
			specularPowers.push_back( 8 ), transparencys.push_back( 0 ) );
	}
};

sgMaterial * Maya::exportDefaultMaterial()
{
	if ( defaultMaterial ) {
		defaultMaterial = new sgMaterial( *graph );
		defaultMaterial->name = "$material";
	}
	return defaultMaterial;
}
sgMaterial * Maya::exportMaterial( MObject & mObj )
{
	if ( 0 ) return exportDefaultMaterial();
	MFn::Type apiType = mObj.apiType();
	MStatus status;
	MFnDependencyNode fnDepencencyNode( mObj, &status ); statusError( status, "Fail fnDepencencyNode" );
	string name = getName( fnDepencencyNode );
	
	switch ( mObj.apiType() ) {
	case MFn::kLambert: name = name + "$lambert"; break;
	case MFn::kBlinn: name = name + "$blinn"; break;
	case MFn::kPhong: name = name + "$phong"; break;
	case MFn::kReflect: name = name + "$reflect"; break;
	default: return exportDefaultMaterial();
	}
	for ( list<sgMaterial*>::iterator it( materials ); it; ++it ) {
		if ( it()->name == name ) return it();
	}
	MayaMaterial mm( mObj );
	sgMaterial * material = new sgMaterial( *graph );
	material->name = name;
	mm.set( material );

	MObject colorObject = getInputObject( mObj, "color" );
	if ( colorObject != MObject::kNullObj ) {
		sgTexture * texture = exportTexture( colorObject );
		if ( texture ) {
			new sgConnection( *texture, *sgTexture::attributes, *material, sgMaterial::attributes->textures );
			material->diffuse() = vector3::white;
		}
	}
	materials.push_back( material );
	animationNodes.push_back( new materialAnimation( mObj, material, name ) );
	return material;
}
