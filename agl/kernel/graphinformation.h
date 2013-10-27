////////////////////////////////////////////////////////////////////////////////
// シーングラフ情報のMediator

//シーングラフからbbox情報を取り出す(update済みが条件)
class graphInformation
{
public:
	sgGraph * graph;
	matrix44 world;
	array<matrix44> matrixList;
	array<int> matrixListCash;
	int matrixListIndex;
	graphInformation() : world( matrix44::identity )
	{
	}
	void initialize()
	{
		matrixList.reserve( 1000 );//マトリックスの変更回数でメモリを用意する
		matrixListCash.reserve( 1000 );
		matrixListIndex = matrixList.size;
		matrixList.push_back( matrix44::identity );
		matrixListCash.push_back( matrixListIndex );
	}
	////////////////////////////////////////////////////////////////////////////////
	//行列
	void execute( sgTransformMatrix * transform )
	{
		world = transform->global();
		matrixListIndex = matrixList.size;
		matrixList.push_back( world );
		matrixListCash.push_back( matrixListIndex );
	}
	void dispose( sgTransformMatrix * transform )
	{
		matrixListCash.pop_back();
		matrixListIndex = matrixListCash.last();
	}
public:
	////////////////////////////////////////////////////////////////////////////////
	//BoundaryBox

	//ポリゴン
	void boundaryBox( sgPolygon * sgp, vector3 & mnmm, vector3 & mxmm, bool & yet )
	{
		if ( ! sgp->vertices.count() ) return;
		sgVertices & sgv = *( sgp->vertices()() );
		array<vector3> & positions = sgv.positions();
		for ( int i = 0; i < positions.size; i++ ) {
			vector3 p = ( vector4( positions[i] ) * world ).getVector3();
			if ( yet ) {
				mnmm = mxmm = p;
				yet = false;
			} else {
				mnmm = vector3( minimum( mnmm.x, p.x ), minimum( mnmm.y, p.y ), minimum( mnmm.z, p.z ) );
				mxmm = vector3( maximum( mxmm.x, p.x ), maximum( mxmm.y, p.y ), maximum( mxmm.z, p.z ) );
			}
		}
	}
	//DAG
	void boundaryBox( sgDagNode * dag, vector3 & mnmm, vector3 & mxmm, bool & yet )
	{
		if ( dag->node ) {
			if ( dag->node->thisDescriptor == sgTransformMatrix::descriptor ) {
				execute( static_cast<sgTransformMatrix*>( dag->node ) );
			} else if ( dag->node->thisDescriptor == sgPolygon::descriptor ) {
				boundaryBox( static_cast<sgPolygon*>( dag->node ), mnmm, mxmm, yet );
			}
		}
		//子供
		for ( list< retainer<sgDagNode> >::iterator it( dag->children ); it; ++it ) {
			boundaryBox( & it()(), mnmm, mxmm, yet );
		}
		//後始末
		if ( dag->node ) {
			if ( dag->node->thisDescriptor == sgTransformMatrix::descriptor ) {
				dispose( static_cast<sgTransformMatrix*>( dag->node ) );
			}
		}
	}
	bool boundaryBox( sgGraph & graph, vector3 & mnmm, vector3 & mxmm )
	{
		initialize();
		bool yet = true;
		world = matrix44::identity;
		boundaryBox( & graph.root, mnmm, mxmm, yet );
		return ! yet;
	}
};
