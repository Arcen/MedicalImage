////////////////////////////////////////////////////////////////////////////////
// ƒ}ƒeƒŠƒAƒ‹

#ifndef __COMPOSE__

class sgMaterial : public sgNode
{
public:
	sgNodeDeclare( sgMaterial );
	sgVector3 ambient, diffuse, specular, emissive;
	sgDecimal transparency, specularPower;
	sgNodeMember<sgTexture> textures;
};

#else

sgNodeDeclareInstance( sgMaterial )
{
	sgDeclareThis();
	sgDeclareInitialRetain( ambient, vector3::gray );
	sgDeclareInitialRetain( diffuse, vector3::gray );
	sgDeclareInitialRetain( specular, vector3::white );
	sgDeclareInitialRetain( emissive, vector3::black );
	sgDeclareInitialRetain( transparency, 0 );
	sgDeclareInitialRetain( specularPower, 8 );
	sgDeclareNode( textures, sgTexture );
}

#endif
