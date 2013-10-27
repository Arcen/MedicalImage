////////////////////////////////////////////////////////////////////////////////
// シーングラフ・レンダリングのMediator

//レンダラーとシーングラフをつなぐ。また、そのサンプルソースとして
class graphRender
{
public:
	renderInterface * render;
	sgGraph * graph;
	matrix44 root, world, view, projection;
	array<matrix44> matrixList;
	array<int> matrixListCash;
	int matrixListIndex;
	map<const sgVertices*, retainer<renderInterface::rVertexBuffer> > VertexBufferList;
	map<const sgIndices*, retainer<renderInterface::rIndexBuffer> > IndexBufferList;
	map<const sgTexture*, retainer<renderInterface::rTexture> > textureList;
	class sgMeshData
	{
	public:
		sgPolygon * polygon;
		sgMaterial * material;
		renderInterface::rVertexBuffer* vertices;
		renderInterface::rIndexBuffer* indices;
		int primSize;
		sgMeshData() 
			: polygon( NULL ), material( NULL ), vertices( NULL ), indices( NULL ), primSize( 0 ) {}
		int getSize()
		{
			return indices->data.size;
		}
		renderInterface::rVertex * getVertex( int index )
		{
			return & vertices->data[int( indices->data[index] )];
		}
		void draw( renderInterface * render )
		{
			render->drawList( vertices, indices, primSize );
		}
		void draw( renderInterface * render, int offset )
		{
			render->drawList( vertices, indices, 1, offset );
		}
		bool operator==( const sgMeshData & src ) const { return memcmp( this, & src, sizeof( *this ) ) == 0; }
		bool operator!=( const sgMeshData & src ) const { return memcmp( this, & src, sizeof( *this ) ) != 0; }
	};
	map< sgPolygon*, retainer< array<sgMeshData> > > sgMeshDataList;

	class transparencyTriangle
	{
	public:
		float z;
		int offset, world;
		sgMeshData * mesh;
		transparencyTriangle() : z( 0 ), offset( 0 ), world( 0 ), mesh( NULL )
		{
		}
		transparencyTriangle( const transparencyTriangle & value )
		{
			*this = value;
		}
		transparencyTriangle & operator=( const transparencyTriangle & value )
		{
			z = value.z;
			offset = value.offset;
			world = value.world;
			mesh = value.mesh;
			return *this;
		}
	};
	class transparencyMesh
	{
	public:
		int world;
		sgMeshData * mesh;
		transparencyMesh() : world( 0 ), mesh( NULL )
		{
		}
		transparencyMesh( const transparencyTriangle & tt )
		{
			world = tt.world;
			mesh = tt.mesh;
		}
		transparencyMesh & operator=( const transparencyMesh & value )
		{
			world = value.world;
			mesh = value.mesh;
			return *this;
		}
		transparencyMesh( const transparencyMesh & value )
		{
			*this = value;
		}
	};
	graphRender() : root( matrix44::identity ), world( matrix44::identity ), view( matrix44::identity ), projection( matrix44::identity )
	{
	}
	void initialize()
	{
		matrixList.reserve( 1000 );//マトリックスの変更回数でメモリを用意する
		matrixListCash.reserve( 1000 );
		matrixListIndex = matrixList.size;
		matrixList.push_back( matrix44::identity );
		matrixListCash.push_back( matrixListIndex );
		
	//	transparencyTriangleList.reserve( 10000 );
	//	transparencyDagMeshList.reserve( 1000 );
	}
	////////////////////////////////////////////////////////////////////////////////
	// カメラ
	virtual void defaultCamera()
	{
		render->setView( vector3( 0, 0, -100 ), vector3( 0, 0, 0 ), vector3( 0, 1, 0 ) );
	}
	bool execute( sgCamera * camera )
	{
		render->setView( camera->position(), camera->interest(), camera->upper() );
		return true;
	}
	bool execute( sgPerspective * perspective )
	{
		render->setPerspective( perspective->fieldOfViewY(), perspective->aspect(), perspective->nearZ(), perspective->farZ() );
		return true;
	}
	bool execute( sgOrtho * ortho )
	{
		render->setOrtho( ortho->sizeY(), ortho->aspect(), ortho->nearZ(), ortho->farZ() );
		return true;
	}
	bool execute( sgFrustum * frustum )
	{
		render->setFrustum( frustum->minimum(), frustum->maximum() );
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////
	// ライト
	int lightIndex;
	virtual void defaultLight()
	{
		render->lightAmbient( vector4::gray );
		render->lightEnable( true );
		render->lightEnable( 0, true );
		for ( int i = 1; i < 8; i++ ) {
			render->lightEnable( i, false );
		}
		render->lightAmbient( vector4::gray );
		render->lightEnable( 1, true );
		render->lightPosition( 1, vector4::black, vector4::white, vector4::white, vector3( 100, 100, 100 ), sqrt( FLT_MAX ), vector3( 1, 0, 0 ) );
		lightIndex = 0;
	}
	bool execute( sgAmbientLight * light )
	{
		if ( lightIndex == 0 ) {
			render->lightEnable( 0, true );
			render->lightAmbient( light->ambient() );
			lightIndex++;
			return true;
		}
		return false;
	}
	bool execute( sgPositionLight * light )
	{
		if ( between( 0, lightIndex, 7 ) ) {
			if ( lightIndex == 0 ) lightIndex++;
			render->lightEnable( lightIndex, true );
			render->lightPosition( lightIndex, vector4::black, light->diffuse(), light->specular(), light->position(), light->range(), light->attenuation() );
			lightIndex++;
			return true;
		}
		return false;
	}
	bool execute( sgDirectionLight * light )
	{
		if ( between( 0, lightIndex, 7 ) ) {
			if ( lightIndex == 0 ) lightIndex++;
			render->lightEnable( lightIndex, true );
			render->lightDirection( lightIndex, vector4::black, light->diffuse(), light->specular(), vector3::normalize3( light->interest() - light->position() ) );
			lightIndex++;
			return true;
		}
		return false;
	}
	bool execute( sgSpotLight * light )
	{
		if ( between( 0, lightIndex, 7 ) ) {
			if ( lightIndex == 0 ) lightIndex++;
			render->lightEnable( lightIndex, true );
			render->lightSpot( lightIndex, vector4::black, light->diffuse(), light->specular(), light->position(), vector3::normalize3( light->interest() - light->position() ), light->range(), light->falloff(), light->attenuation(), light->insideAngle(), light->outsideAngle() );
			lightIndex++;
			return true;
		}
		return false;
	}
	////////////////////////////////////////////////////////////////////////////////
	//行列
	bool execute( sgTransformMatrix * transform )
	{
		if ( ! transform->valid() ) return false;
		render->pushMatrix();
		matrix44 w = transform->global() * root;
		render->setMatrix( w );
		matrixListIndex = matrixList.size;
		matrixList.push_back( w );
		render->getMatrix( world );
		matrixListCash.push_back( matrixListIndex );
		return true;
	}
	void dispose( sgTransformMatrix * transform )
	{
		if ( ! transform->valid() ) return;
		render->popMatrix();
		render->getMatrix( world );
		matrixListCash.pop_back();
		matrixListIndex = matrixListCash.last();
	}
	////////////////////////////////////////////////////////////////////////////////
	//テクスチャ
	bool execute( sgTexture * texture )
	{
		if ( texture ) {
			retainer<renderInterface::rTexture> & rrt = textureList[texture];
			if ( rrt ) {
				renderInterface::rTexture & rt = rrt();
				render->setTexture( 0, & rt );
				return true;
			}
		}
		render->setWhiteTexture( 0 );
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////
	//マテリアル
	bool execute( sgMaterial * material )
	{
		render->material( material->ambient(), material->diffuse(), material->specular(), material->emissive(), material->transparency(), material->specularPower() );
		sgTexture * texture = material->textures.count() ? material->textures()() : NULL;
		if ( texture ) {
			if ( ! texture->valid() ) return false;
		}
		execute( texture );
		return true;
	}
	////////////////////////////////////////////////////////////////////////////////
	//ポリゴン
	bool execute( sgPolygon * polygon, bool reverse = false )
	{
		if ( ! polygon->valid() ) return false;
		array<sgMeshData> & polygonExs = sgMeshDataList[ polygon ]();
		if ( ! reverse ) {
			for ( array<sgMeshData>::iterator it( polygonExs ); it; ++it ) {
				sgMeshData & mesh = it();
				if ( execute( mesh.material ) ) {
					mesh.draw( render );
				}
			}
		} else {
			for ( array<sgMeshData>::iterator it( polygonExs, false ); it; --it ) {
				sgMeshData & mesh = it();
				if ( execute( mesh.material ) ) {
					mesh.draw( render );
				}
			}
		}
		return true;
	}
	void defaultTest()
	{
		render->alphaTest( true, 0, renderInterface::testFuncGreater );
		render->depthTest( true, true, renderInterface::testFuncLessEqual );
		//render->blend( renderInterface::blendZero, renderInterface::blendOne );
		render->blend( renderInterface::blendInverseSourceAlpha, renderInterface::blendSourceAlpha );
	}
	////////////////////////////////////////////////////////////////////////////////
	// 前準備
public:
	void prepare( sgGraph * graph )
	{
		list<sgNode*> lists;
		{
			graph->enumrate( lists, sgVertices::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				sgVertices * sgv = static_cast<sgVertices *>( it() );
				if ( ! sgv->getOutput( sgVertices::attributes->getAttribute(), sgPolygon::descriptor ) ) continue;
				retainer<renderInterface::rVertexBuffer> & vb = VertexBufferList[sgv];
				int flag = 
					( sgv->positions().size ? renderInterface::rVertexBuffer::enableVertex : 0 ) |
					( sgv->normals().size ? renderInterface::rVertexBuffer::enableNormal : 0 ) |
					( true ? renderInterface::rVertexBuffer::enableTexture0 : 0 ) |
					( sgv->colors().size ? renderInterface::rVertexBuffer::enableDiffuse | renderInterface::rVertexBuffer::enableSpecular : 0 );
				int size = sgv->positions().size;
				if ( ! vb ) vb = render->createVertexBuffer( size, flag );
				vector3 * srcPosition = sgv->positions().size ? sgv->positions().data : NULL;
				vector3 * srcNormal = sgv->normals().size ? sgv->normals().data : NULL;
				vector2 * srcTextureCoordinate = sgv->textureCoordinates().size ? sgv->textureCoordinates().data : NULL;
				vector4 * srcColor = sgv->colors().size ? sgv->colors().data : NULL;
				renderInterface::rVertex * dst = vb->data.data;
				for ( int i = 0; i < size; i++ ) {
					if ( srcPosition ) {
						dst->position[0] = srcPosition->x;
						dst->position[1] = srcPosition->y;
						dst->position[2] = srcPosition->z;
						++srcPosition;
					}
					if ( srcNormal ) {
						dst->normal[0] = srcNormal->x;
						dst->normal[1] = srcNormal->y;
						dst->normal[2] = srcNormal->z;
						++srcNormal;
					}
					if ( srcTextureCoordinate ) {
						dst->texture[0][0] = srcTextureCoordinate->x;
						dst->texture[0][1] = srcTextureCoordinate->y;
						++srcTextureCoordinate;
					}
					if ( srcColor ) {
						dst->diffuse[0] = srcColor->x;
						dst->diffuse[1] = srcColor->y;
						dst->diffuse[2] = srcColor->z;
						dst->diffuse[3] = srcColor->w;
						dst->specular[0] = dst->specular[1] = dst->specular[2] = dst->specular[3] = 0;
						++srcColor;
					} else {
						dst->diffuse[0] = dst->diffuse[1] = dst->diffuse[2] = 1;
						dst->diffuse[3] = 1;
						dst->specular[0] = dst->specular[1] = dst->specular[2] = 0;
						dst->specular[3] = 1;
					}
					++dst;
				}
				vb->update();
			}
		}
		{
			graph->enumrate( lists, sgIndices::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				sgIndices * sgi = static_cast<sgIndices *>( it() );
				retainer<renderInterface::rIndexBuffer> & ib = IndexBufferList[sgi];
				if ( ! ib ) {
					int size = sgi->indices().size;
					ib = render->createIndexBuffer( sgi->indices().size );
					int * src = sgi->indices().data;
					renderInterface::rIndexBuffer::element * dst = ib->data.data;
					for ( int i = 0; i < size; i++ ) {
						dst[i] = static_cast<renderInterface::rIndexBuffer::element>( src[i] );
					}
					ib->update();
				}
			}
		}
		{
			graph->enumrate( lists, sgPolygon::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				sgPolygon * polygon = static_cast<sgPolygon *>( it() );
				retainer< array<sgMeshData> > & polygonExs = sgMeshDataList[ polygon ];
				if ( ! polygonExs ) {
					polygonExs = new array<sgMeshData>;
					polygonExs->reserve( polygon->materials.count() );
					array<sgVertices*>::iterator itv( polygon->vertices() );
					array<sgIndices*>::iterator iti( polygon->indices() );
					for ( array<sgMaterial*>::iterator itm( polygon->materials() ); itm && iti && itv; ++itm, ++iti, ++itv ) {
						sgVertices * sgv = itv();
						sgIndices * sgi = iti();
						sgMaterial * sgm = itm();
						sgMeshData & mesh = polygonExs->put_back();
						mesh.polygon = polygon;
						mesh.material = sgm;
						mesh.vertices = & VertexBufferList[sgv]();
						mesh.indices = & IndexBufferList[sgi]();
						mesh.primSize = sgi->indices().size / 3;
					}
				}
			}
		}
		{
			graph->enumrate( lists, sgTexture::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				sgTexture * sgt = static_cast<sgTexture *>( it() );
				retainer<renderInterface::rTexture> & texture = textureList[sgt];
				if ( ! texture ) {
					texture = render->createTexture( sgt->url(), sgt->texture.width, sgt->texture.height, ( unsigned long * ) sgt->texture.data, 0 );
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	// 後始末
	void dispose( sgGraph * graph )
	{
		list<sgNode*> lists;
		{
			graph->enumrate( lists, sgVertices::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				sgVertices * sgv = static_cast<sgVertices *>( it() );
				if ( ! sgv->getOutput( sgVertices::attributes->getAttribute(), sgPolygon::descriptor ) ) continue;
				VertexBufferList.pop( sgv );
			}
		}
		{
			graph->enumrate( lists, sgIndices::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				IndexBufferList.pop( static_cast<sgIndices *>( it() ) );
			}
		}
		{
			graph->enumrate( lists, sgPolygon::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				sgMeshDataList.pop( static_cast<sgPolygon *>( it() ) );
			}
		}
		{
			graph->enumrate( lists, sgTexture::descriptor );
			for ( list<sgNode*>::iterator it( lists ); it; ++it ) {
				sgTexture * sgt = static_cast<sgTexture *>( it() );
				retainer<renderInterface::rTexture> & texture = textureList[sgt];
				if ( texture ) texture = NULL;
				textureList.pop( sgt );
			}
		}
	}
	void dispose( sgTexture * sgt )
	{
		retainer<renderInterface::rTexture> & texture = textureList[sgt];
		if ( texture ) texture = NULL;
		textureList.pop( sgt );
	}
	////////////////////////////////////////////////////////////////////////////////
	//DAG
	bool execute( sgDagNode * dag, bool reverse = false )
	{
		if ( dag->node ) {
			if ( dag->node->thisDescriptor == sgTransformMatrix::descriptor ) {
				if ( ! execute( static_cast<sgTransformMatrix*>( dag->node ) ) ) return true;
			} else if ( dag->node->thisDescriptor == sgPolygon::descriptor ) {
				execute( static_cast<sgPolygon*>( dag->node ) );
			}
		}
		if ( reverse ) {
			for ( list< retainer<sgDagNode> >::iterator it( dag->children, false ); it; --it ) {
				execute( & it()(), reverse );
			}
		} else {
			for ( list< retainer<sgDagNode> >::iterator it( dag->children ); it; ++it ) {
				execute( & it()(), reverse );
			}
		}
		if ( dag->node ) {
			if ( dag->node->thisDescriptor == sgTransformMatrix::descriptor ) {
				dispose( static_cast<sgTransformMatrix*>( dag->node ) );
			}
		}
		return true;
	}
	bool execute( sgDagNode & dag, bool reverse = false )
	{
		render->getEachMatrix( world, view, projection );
		execute( & dag, reverse );
		return true;
	}
};
