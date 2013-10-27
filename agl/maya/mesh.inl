////////////////////////////////////////////////////////////////////////////////
// MESH
vector3 ConvertPosition( const MPoint & v )
{
	return vector3( ConvertDecimal( v.x ), ConvertDecimal( v.y ), ConvertDecimal( v.z ) );
}
vector3 ConvertNormal( const MFloatVector & v )
{
	return vector3( ConvertDecimal( v.x ), ConvertDecimal( v.y ), ConvertDecimal( v.z ) );
}
vector2 ConvertTextureCoordinate( const float & u, const float & v )
{
	return vector2( ConvertDecimal( u ), ConvertDecimal( v ) );
}
vector4 ConvertColor( const MColor & v )
{
	return vector4( ConvertDecimal( v.r ), ConvertDecimal( v.g ), ConvertDecimal( v.b ), ConvertDecimal( v.a ) );
}

struct MayaVertices
{
	MPointArray v;
	MFloatVectorArray n;
	MFloatArray tu, tv;
	MColorArray c;
	void initialize( MFnMesh & fnMesh )
	{
		Maya::statusError( fnMesh.getPoints( v, MSpace::kObject ), "Fail getPoints" );
		Maya::statusError( fnMesh.getVertexColors( c ), "Fail getVertexColors" );
		Maya::statusError( fnMesh.getNormals( n ), "Fail getNormals" );
		Maya::statusError( fnMesh.getUVs( tu, tv ), "Fail getUVs" );
	}
};

//インデックス情報
struct MayaIndices
{
	array< array<int> > v, n, t, c;//頂点、法線、テクスチャ、色のインデックス
	array< array<int> > indices;
	int allVertexSize;//全頂点のサイズ
	bool enableColor;//頂点カラーが有効かどうか
	void initialize( MDagPath & dPath, MayaVertices & mv )
	{
		allVertexSize = 0;
		MStatus status;
		MItMeshPolygon polyIt( dPath.node(), & status ); Maya::statusError( status, "Fail MItMeshPolygon" );
		indices.allocate( polyIt.count() );//頂点数分の情報確保
		v.allocate( indices.size );
		n.allocate( indices.size );
		t.allocate( indices.size );
		c.allocate( indices.size );
		int numColors = 0;//設定してある色数
		if ( polyIt.numColors( numColors ) != MStatus::kSuccess ) numColors = 0;
		enableColor = numColors ? true : false;
		for ( int i = 0; !polyIt.isDone(); i++, polyIt.next() ) {
			array<int> & index = indices[i];
			index.allocate( polyIt.polygonVertexCount() );
			allVertexSize += index.size;
			v[i].allocate( index.size );
			n[i].allocate( index.size );
			t[i].allocate( index.size );
			for ( int j = 0; j < index.size; j++ ) {
				//頂点と法線のインデックス取得
				v[i][j] = polyIt.vertexIndex(j);
				n[i][j] = polyIt.normalIndex(j);
				int uv;//テクスチャ座標取得
				if ( polyIt.getUVIndex( j, uv ) == MStatus::kSuccess ) {
					t[i][j] = uv;
				} else {
					t[i][j] = 0;
				}
			}
			//頂点カラー
			c[i].allocate( index.size );
			for ( int j = 0; j < index.size; j++ ) c[i][j] = 0;//初期化
			if ( ! numColors ) continue;
			if ( ! mv.c.length() ) continue;
			for ( int j = 0; j < index.size; j++ ) {
				MColor color;
				//頂点カラーを取得
				if ( polyIt.getColor( color, j ) == MStatus::kSuccess ) {
					//色のリストから探す。
					for ( int k = 0; k < mv.c.length(); ++k ) {
						if ( color == mv.c[k] ) {
							c[i][j] = k;
							break;
						}
					}
				}
			}
		}
	}
};

struct MayaMaterials
{
	array< MObject > materials;
	array< array<int> > faceIndex;
	void initialize( MDagPath & dPath, MFnMesh & fnMesh )
	{
		MIntArray indices;
		MObjectArray shaders;
		Maya::statusError( fnMesh.getConnectedShaders( dPath.instanceNumber(), shaders, indices ), "Fail getConnectedShaders" );
		materials.allocate( shaders.length() );
		for ( int i = 0; i < materials.size; i++ ) {
			materials[i] = Maya::getInputObject( shaders[i], "surfaceShader" );
		}

		faceIndex.allocate( materials.size + 1 );
		for ( int i = 0; i < faceIndex.size; i++ ) faceIndex[i].reserve( indices.length() );
		for ( int i = 0; i < indices.length(); i++ ) {
			if ( between( 0, indices[i], materials.size - 1 ) ) {
				faceIndex[ indices[i] ].push_back( i );
			} else {
				faceIndex.last().push_back( i );
			}
		}
		if ( ! faceIndex.last().size ) faceIndex.allocate( faceIndex.size - 1 );
	}
};

class Maya2XeiosMeshConverter
{
public:
	class indexVertex
	{
	public:
		indexVertex() 
			: vertexIndex( 0 ), normalIndex( 0 ), textureCoordinateIndex( 0 ), colorIndex( 0 ), materialIndex( 0 )
		{
		}
		indexVertex( int vi, int ni, int ti, int ci, int mi ) 
			: vertexIndex( vi ), normalIndex( ni ), textureCoordinateIndex( ti ), colorIndex( ci ), materialIndex( 0 )
		{
		}
		int vertexIndex;
		int normalIndex;
		int textureCoordinateIndex;
		int colorIndex;
		int materialIndex;
		bool operator==( const indexVertex & r ) const
		{
			return vertexIndex == r.vertexIndex && 
				normalIndex == r.normalIndex && 
				textureCoordinateIndex == r.textureCoordinateIndex && 
				colorIndex == r.colorIndex && 
				materialIndex == r.materialIndex;
		}
	};
	MayaVertices * mv;
	MayaIndices * mi;
	MayaMaterials * mm;
	sgVertices * xv;
	array<int> positionIndices;
	array<int> normalIndices;
	array<indexVertex> indexVertices;
	array< array<int> > indices;
	MayaTextureAttribute mta;
	void initialize( MayaVertices & _mv, MayaIndices & _mi, MayaMaterials & _mm, sgVertices * _xv )
	{
		mv = & _mv; mi = & _mi; mm = & _mm; xv = _xv;

		indexVertices.reserve( mi->allVertexSize );
		xv->positions().reserve( mi->allVertexSize );
		xv->normals().reserve( mi->allVertexSize );
		xv->textureCoordinates().reserve( mi->allVertexSize );
		xv->colors().reserve( mi->allVertexSize );
		positionIndices.reserve( mi->allVertexSize );
		normalIndices.reserve( mi->allVertexSize );
		indices.allocate( mi->indices.size );
		//マテリアルの順に並べておき、あとから並び替える必要をなくす
		for ( int m = 0; m < mm->faceIndex.size; m++ ) {
			if ( mm->materials.size ) {
				mta.initializeByMaterial( mm->materials[m] );
			} else {
				mta.initialize();
			}
			for ( int n = 0; n < mm->faceIndex[m].size; n++ ) {
				int i = mm->faceIndex[m][n];//face index

				array<int> & src = mi->indices[i];
				array<int> & dst = indices[i];
				if ( src.size < 3 ) continue;
				dst.reserve( ( src.size - 2 ) * 3 );
				int first = registerIndex( i, 0, m );
				int second = registerIndex( i, 1, m );
				for ( int j = 2; j < src.size; ++j ) {
					int third = registerIndex( i, j, m );
					dst.push_back( first );
					dst.push_back( second );
					dst.push_back( third );
					second = third;
				}
			}
		}
	}
	int registerIndex( int i, int j, int im )
	{
		static MColor white( 1, 1, 1, 1 );
		static MFloatVector z( 0, 0, 1 );
		int iv = mi->v[i][j];
		int in = mi->n[i][j];
		int it = mi->t[i][j];
		int ic = mi->c[i][j];
		return registerIndex( iv, in, it, ic, im, mv->v[iv], mv->n.length() ? mv->n[in] : z, mv->tu.length() ? mv->tu[it] : 0, mv->tv.length() ? mv->tv[it] : 0, mv->c.length() ? mv->c[ic] : white );
	}
	int registerIndex( int vi, int ni, int ti, int ci, int im, 
		const MPoint & v, const MFloatVector & n, const float & tu, const float & tv, const MColor & c )
	{
		indexVertex iv( vi, ni, ti, ci, im );
		for ( int i = 0; i < indexVertices.size; i++ ) {
			if ( indexVertices[i] == iv ) return i;
		}
		xv->positions().push_back( ConvertPosition( v ) );
		xv->normals().push_back( ConvertNormal( n ) );
		vector2 & uv = xv->textureCoordinates().push_back( ConvertTextureCoordinate( tu, tv ) );
		xv->colors().push_back( ConvertColor( c ) );
		positionIndices.push_back( vi );
		normalIndices.push_back( ni );
		mta.convert( uv );
		indexVertices.push_back( iv );
		return indexVertices.size - 1;
	}
};

inline bool hasDependencyGraphItem( MObject& inSrcObj, MFn::Type inFilter = MFn::kInvalid, 
								   MItDependencyGraph::Direction inDirection = MItDependencyGraph::kDownstream, 
								   MItDependencyGraph::Traversal inTraversal = MItDependencyGraph::kDepthFirst, 
								   MItDependencyGraph::Level inLevel = MItDependencyGraph::kNodeLevel )
{
	MStatus status;
	MItDependencyGraph dgIt( inSrcObj, inFilter, inDirection, inTraversal, inLevel, &status );
	if ( status != MS::kSuccess ) return false;
	if ( dgIt.disablePruningOnFilter() != MS::kSuccess ) return false;
	return ! dgIt.isDone();
}

inline bool hasEnvelope( MDagPath & dPath )
{
	if ( ! hasDependencyGraphItem( dPath.node(), MFn::kJoint, MItDependencyGraph::kUpstream ) ) return false;
	if ( hasDependencyGraphItem( dPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream ) ) return true;
	if ( hasDependencyGraphItem( dPath.node(), MFn::kJointCluster, MItDependencyGraph::kUpstream ) ) return true;
	return false;
}

sgDagNode * Maya::exportDagMesh( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & transform )
{
	MStatus status;

	//中間オブジェクトなので出力しない
	MFnDagNode fnDagNode( dPath, &status ); statusError( status, "Fail MFnDagNode" );
	if ( fnDagNode.isIntermediateObject() ) return parent;

	MFnMesh fnMesh( dPath, &status ); statusError( status, "Fail MFnMesh" );
	string name = getName( dPath );

	sgPolygon * sgpolygon = new sgPolygon( *graph );
	sgpolygon->name = name + "$polygon";
	parent->insert( *sgpolygon );

	sgVertices * sgvertices = new sgVertices( *graph );
	sgvertices->name = name + "$vertices";

	MayaVertices mv; mv.initialize( fnMesh );
	MayaIndices mi; mi.initialize( dPath, mv );
	MayaMaterials mm; mm.initialize( dPath, fnMesh );
	Maya2XeiosMeshConverter mxmc; mxmc.initialize( mv, mi, mm, sgvertices );

	bool enableTexture = false;
	int meshIndex = 0;
	for ( int i = 0; i < mm.faceIndex.size; i++ ) {
		if ( ! mm.faceIndex[i].size ) continue;
		sgMaterial * material = between( 0, i, mm.materials.size - 1 ) ? 
			exportMaterial( mm.materials[i] ) : exportDefaultMaterial();
		if ( material->textures() ) enableTexture = true;
		new sgConnection( *material, *sgMaterial::attributes, *sgpolygon, sgPolygon::attributes->materials );
		new sgConnection( *sgvertices, *sgVertices::attributes, *sgpolygon, sgPolygon::attributes->vertices );
		sgIndices * sgindices = new sgIndices( *graph );
		sgindices->name = name + "$indices" + string( meshIndex );
		new sgConnection( *sgindices, *sgIndices::attributes, *sgpolygon, sgPolygon::attributes->indices );
		int numIndex = 0;
		for ( int j = 0; j < mm.faceIndex[i].size; ++j ) {
			numIndex += mxmc.indices[mm.faceIndex[i][j]].size;
		}
		sgindices->indices().reserve( numIndex );
		for ( int j = 0; j < mm.faceIndex[i].size; ++j ) {
			array<int> & indices = mxmc.indices[mm.faceIndex[i][j]];
			for ( int k = 0; k < indices.size; k++ ) {
				sgindices->indices().push_back( indices[k] );
			}
		}
		meshIndex++;
	}
	if ( ! enableTexture ) sgvertices->textureCoordinates().release();
	if ( ! mi.enableColor ) sgvertices->colors().release();
	
	if ( hasEnvelope( dPath ) ) {
		envelopeInfomation * envelope = & ( ( envelopes.push_back( new envelopeInfomation ) )() );
		envelope->name = name;
		envelope->path = dPath;
		envelope->polygon = sgpolygon;
		envelope->currentMatrix = transform;
		envelope->initialMatrix = transform->global();
		envelope->positionIndices = mxmc.positionIndices;
		envelope->normalIndices = mxmc.normalIndices;
	}

	return parent;
}
