////////////////////////////////////////////////////////////////////////////////
// éûä‘

#ifndef __COMPOSE__

class sgVertices : public sgNode
{
public:
	sgNodeDeclare( sgVertices );
	sgArrayVector3 positions;
	sgArrayVector3 normals;
	sgArrayVector2 textureCoordinates;
	sgArrayVector4 colors;
};

class sgIndices : public sgNode
{
public:
	sgNodeDeclare( sgIndices );
	sgArrayInt indices;
};

class sgPolygon : public sgNode
{
public:
	sgNodeDeclare( sgPolygon );
	sgNodeMember<sgVertices> vertices;
	sgNodeMember<sgIndices> indices;
	sgNodeMember<sgMaterial> materials;
	sgInt valid;//É|ÉäÉSÉìÇèëÇ≠Ç©Ç«Ç§Ç©
};

#else

sgNodeDeclareInstance( sgVertices )
{
	sgDeclareThis();
	sgDeclareRetain( positions );
	sgDeclareRetain( normals );
	sgDeclareRetain( textureCoordinates );
	sgDeclareRetain( colors );
}

sgNodeDeclareInstance( sgIndices )
{
	sgDeclareThis();
	sgDeclareRetain( indices );
}

sgNodeDeclareInstance( sgPolygon )
{
	sgDeclareThis();
	sgDeclareNode( vertices, sgVertices );
	sgDeclareNode( indices, sgIndices );
	sgDeclareNode( materials, sgMaterial );
	sgDeclareInitialRetain( valid, 1 );
}

#endif
