
void MayaTextureAttribute::initialize( MObject & mObj )
{
	initialize();

	MStatus status;
	MFnDependencyNode dependNode( mObj, &status ); Maya::statusError( status, "Fail fnDepencencyNode" );

	MPlug & repeatUV = dependNode.findPlug( "repeatUV", & status ); Maya::statusError( status, "Fail repeatUV" );
	repeatUV.child(0).getValue( repeatU );
	repeatUV.child(1).getValue( repeatV );

	MPlug & offset = dependNode.findPlug( "offset", & status ); Maya::statusError( status, "Fail offset" );
	offset.child(0).getValue( offsetU );
	offset.child(1).getValue( offsetV );
/*
	MPlug & noiseUV = dependNode.findPlug( "noiseUV", & status ); Maya::statusError( status, "Fail noiseUV" );
	noiseUV.child(0).getValue( noiseU );
	noiseUV.child(1).getValue( noiseV );
*/
	MAngle a;
	MPlug & rotateUVPlug = dependNode.findPlug( "rotateUV", & status ); Maya::statusError( status, "Fail rotateUV" );
	rotateUVPlug.getValue(a);
	rotateUV = a.value();

	MPlug & coverage = dependNode.findPlug( "coverage", & status ); Maya::statusError( status, "Fail coverage" );
	coverage.child(0).getValue( coverageU );
	coverage.child(1).getValue( coverageV );

	MPlug & translateFrame = dependNode.findPlug( "translateFrame", & status ); Maya::statusError( status, "Fail translateFrame" );
	translateFrame.child(0).getValue( translateFrameU );
	translateFrame.child(1).getValue( translateFrameV );

	MPlug & rotateFramePlug = dependNode.findPlug( "rotateFrame", & status ); Maya::statusError( status, "Fail rotateFrame" );
	rotateFramePlug.getValue(a);
	rotateFrame = a.value();

	MPlug & staggerPlug = dependNode.findPlug( "stagger", & status ); Maya::statusError( status, "Fail stagger" );
	staggerPlug.getValue( stagger );

	MPlug & mirrorPlug = dependNode.findPlug( "mirror", & status ); Maya::statusError( status, "Fail mirror" );
	mirrorPlug.getValue( mirror );

	MPlug & wrapUPlug = dependNode.findPlug( "wrapU", & status ); Maya::statusError( status, "Fail wrapU" );
	wrapUPlug.getValue( wrapU );

	MPlug & wrapVPlug = dependNode.findPlug( "wrapV", & status ); Maya::statusError( status, "Fail wrapV" );
	wrapVPlug.getValue( wrapV );

	const vector3 half( 0.5, 0.5, 0 );
	matrix = 
		matrix44::translate( -half ) * 
		matrix44::rotation( -rotateFrame ) * 
		matrix44::translate( half ) * 
		matrix44::translate( vector3( -translateFrameU, -translateFrameV, 0 ) ) * 
		matrix44::scale( vector3( repeatU/coverageU, repeatV/coverageV, 1 ) ) *
		matrix44::translate( vector3( offsetU, offsetV, 0 ) ) * 
		matrix44::translate( -half ) * 
		matrix44::rotation( -rotateUV ) * 
		matrix44::translate( half ) * 
		matrix44::scale( vector3( 1, -1, 1 ) ) * matrix44::translate( vector3( 0, 1, 0 ) );
}

void MayaTextureAttribute::initializeByMaterial( MObject & material )
{
	initializeByTexture( Maya::getInputObject( material, "color" ) );
}

void MayaTextureAttribute::initializeByTexture( MObject & colorObject )
{
	initialize();

	if ( colorObject == MObject::kNullObj ) return;

	MStatus status;
	MItDependencyGraph place2DTextureIt( colorObject, MFn::kPlace2dTexture, MItDependencyGraph::kUpstream, MItDependencyGraph::kDepthFirst, MItDependencyGraph::kNodeLevel, & status ); Maya::statusError( status, "Fail MItDependencyGraph" );
	place2DTextureIt.disablePruningOnFilter();
	if ( place2DTextureIt.isDone() ) return;

	initialize( place2DTextureIt.thisNode() );
}

void MayaTextureAttribute::convert( vector2 & uv )
{
	if ( ! stagger && ! mirror ) {
		uv = ( vector4( uv.x, uv.y, 0, 1 ) * matrix ).getVector2();
	} else {
		const vector2 half( 0.5, 0.5 );
		uv -= half;
		uv = uv * matrix22::rotation( -rotateFrame );
		uv += half;
		uv -= vector2( translateFrameU, translateFrameV );

		//coverageはレンダリングの設定でも不可能

		//flagはレンダリングのほうで設定しないと境界がおかしくなる
		if ( 0 ) {
			if ( ( ! wrapU && ! between<decimal>( 0, uv.x, 1 ) ) ||
				( ! wrapV && ! between<decimal>( 0, uv.y, 1 ) ) ) {
				uv = vector2::identity;
				return;
			}
		}

		uv *= vector2( repeatU, repeatV );
		uv += vector2( offsetU, offsetV );

		if ( 1 ) {
			if ( stagger ) if ( ( ( int ) floor( uv.y ) ) & 1 ) uv.x += 0.5;
			if ( mirror ) {
				if ( ( ( int ) floor( uv.x ) ) & 1 ) uv.x = 2 * floor( uv.x ) + 1.0 - uv.x;
				if ( ( ( int ) floor( uv.y ) ) & 1 ) uv.y = 2 * floor( uv.y ) + 1.0 - uv.y;
			}
		}

		uv -= half;
		uv = uv * matrix22::rotation( -rotateUV );
		uv += half; 

		uv.y = 1-uv.y;
	}
}
/*

http://www.highend3d.com/maya/devarchive/sp.3d?mail_id=2645

struct rtPlace2dTexture_t
{
	vector2 coverage;
	vector2 translate_frame;
	float rotate_frame;
	bool mirror;
	bool stagger;
	vector2 repeat_uv;
	vector2 offset;
	float rotate_uv;
};
bool rtPlace2dTexture( vector2 *result, miState *state, rtPlace2dTexture_t *param)
{
	vector2 coverage = param->coverage;
	vector2 translate_frame = param->translate_frame;
	float rotate_frame = param->rotate_frame;
	bool mirror = param->mirror;
	bool stagger = param->stagger;
	vector2 repeat_uv = param->repeat_uv;
	vector2 offset = param->offset;
	float rotate_uv = param->rotate_uv;
	
	float outU, outV, outUf, outVf, n, nx, ny;
	float pi=355.0/113.0;
	float d2r=pi/180.0;
	
	bool inside;
	
	{
		// rotate frame
		
		outU = (uv.y-0.5)*sin(-rotate_frame*d2r) + (uv.x-0.5)*cos(-rotate_frame*d2r) + 0.5;
		outV = (uv.y-0.5)*cos(-rotate_frame*d2r) - (uv.x-0.5)*sin(-rotate_frame*d2r) + 0.5;
		
		inside = true;
		
		if (coverage.x < 1.0) {
			
			if (outU >= 1.0) outU -= floor(outU);
			else if (outU < 0.0) outU = outU - floor(outU) + 1.0;
			
			outU -= translate_frame.x;
			
			if (wrap_u)
			{
				if (outU >= coverage.x) { outU -= 1.0; }
				else if (outU < 0.0) { outU += 1.0; }
			}
			
			outU /= coverage.x;
			
			if (outU < 0.0 || outU >= 1.0) inside = false;
		}
		else
		{
			outU = (outU - translate_frame.x) / coverage.x;
			if (!wrap_u) {
				if (outU < 0.0 || outU >= 1.0)
					inside = false;
			}
		}
		outU = outU * repeat_uv.x + offset.x;
		
		if (coverage.y < 1.0) {
			
			if (outV >= 1.0) { outV -= floor(outV); }
			else { if (outV < 0.0) outV = outV - floor(outV) + 1.0; }
			
			outV -= translate_frame.y;
			// outV = outV - (translate_frame.y - floor(translate_frame.y));
			
			if (wrap_v)
			{
				if (outV >= coverage.y) { outV -= 1.0; }
				else { if (outV < 0.0) { outV += 1.0; } }
			}
			
			outV /= coverage.y;
			
			if (outV < 0.0 || outV >= 1.0) inside = false;
		}
		else
		{
			outV = (outV - translate_frame.y) / coverage.y;
			if (!wrap_v) {
				if (outV < 0.0 || outV >= 1.0)
					inside = false;
			}
		} 
		
		outV = outV * repeat_uv.y + offset.y;
		
		if (inside) {
			
			if (stagger) if ((int)floor(outV) % 2) outU += 0.5;
				
			if (mirror) {
				if ((int)floor(outU) % 2) outU = 2 * floor(outU) + 1.0 - outU;
				if ((int)floor(outV) % 2) outV = 2 * floor(outV) + 1.0 - outV;
			}
			
			float tmp = (outU-0.5)*cos(rotate_uv*d2r) - (outV-0.5)*sin(rotate_uv*d2r) + 0.5;
			outV = (outU-0.5)*sin(rotate_uv*d2r) + (outV-0.5)*cos(rotate_uv*d2r) + 0.5;
			outU = tmp;
			
			// bombing
			
			outUf=floor(outU);
			outVf=floor(outV);
			
			outU-=outUf;
			outV-=outVf;
			
			if ((tna.x>0.0)||(tna.y>0.0)) {
				vector2 t;
				
				t.x=(floor(outUf)*tnf.x)-tno.x;
				t.y=(floor(outVf)*tnf.y)-tno.y;
				
				outU-=(mi_noise_2d( t.x, t.y )-0.5)*tna.x;
				outV-=(mi_noise_2d( t.y, t.x )-0.5)*tna.y;
				
				switch(tmu)
				{
				case 1: { if (outU<0.0) outU=0.0; if (outU>=1.0) outU=0.99999; } break;
				case 2: outU-=floor(outU); break;
				}
				
				switch(tmv)
				{
				case 1: { if (outV<0.0) outV=0.0; if (outV>=1.0) outV=0.99999; } break;
				case 2: outV-=floor(outV); break;
				}
			}
		}
	}

	result->x=outU;
	result->y=outV;
	result->z=0.0;

	return(miTRUE);
}

*/