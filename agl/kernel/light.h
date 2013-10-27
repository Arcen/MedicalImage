////////////////////////////////////////////////////////////////////////////////
// ƒ‰ƒCƒg

#ifndef __COMPOSE__

class sgAmbientLight : public sgNode
{
public:
	sgNodeDeclare( sgAmbientLight );
	sgVector3 ambient;
};

class sgPositionLight : public sgNode
{
public:
	sgNodeDeclare( sgPositionLight );
	sgVector3 diffuse;
	sgVector3 specular;
	sgVector3 position;
	sgVector3 attenuation;
	sgDecimal range;
};

class sgDirectionLight : public sgNode
{
public:
	sgNodeDeclare( sgDirectionLight );
	sgVector3 diffuse;
	sgVector3 specular;
	sgVector3 position;
	sgVector3 interest;
};

class sgSpotLight : public sgNode
{
public:
	sgNodeDeclare( sgSpotLight );
	sgVector3 diffuse;
	sgVector3 specular;
	sgVector3 position;
	sgVector3 interest;
	sgVector3 attenuation;
	sgDecimal range;
	sgDecimal insideAngle;//[deg]
	sgDecimal outsideAngle;//[deg]
	sgDecimal falloff;
};

#else

sgNodeDeclareInstance( sgAmbientLight )
{
	sgDeclareThis();
	sgDeclareInitialRetain( ambient, vector3::gray );
}

sgNodeDeclareInstance( sgPositionLight )
{
	sgDeclareThis();
	sgDeclareInitialRetain( diffuse, vector3::white );
	sgDeclareInitialRetain( specular, vector3::white );
	sgDeclareInitialRetain( position, vector3::identity );
	sgDeclareInitialRetain( attenuation, vector3( 1, 0, 0 ) );
	sgDeclareInitialRetain( range, sqrt( FLT_MAX ) );
}

sgNodeDeclareInstance( sgDirectionLight )
{
	sgDeclareThis();
	sgDeclareInitialRetain( diffuse, vector3::white );
	sgDeclareInitialRetain( specular, vector3::white );
	sgDeclareInitialRetain( position, vector3( 0, 0, -1 ) );
	sgDeclareInitialRetain( interest, vector3( 0, 0, 0 ) );
}

sgNodeDeclareInstance( sgSpotLight )
{
	sgDeclareThis();
	sgDeclareInitialRetain( diffuse, vector3::white );
	sgDeclareInitialRetain( specular, vector3::white );
	sgDeclareInitialRetain( position, vector3( 0, 0, -1 ) );
	sgDeclareInitialRetain( interest, vector3( 0, 0, 0 ) );
	sgDeclareInitialRetain( attenuation, vector3( 1, 0, 0 ) );
	sgDeclareInitialRetain( range, sqrt( FLT_MAX ) );
	sgDeclareInitialRetain( insideAngle, 360 );
	sgDeclareInitialRetain( outsideAngle, 360 );
	sgDeclareInitialRetain( falloff, 1 );
}

#endif
