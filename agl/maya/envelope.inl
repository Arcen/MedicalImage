
////////////////////////////////////////////////////////////////////////////////
// ENVELOPE
void Maya::exportEnvelopes()
{
	for ( list< retainer<envelopeInfomation> >::iterator it( envelopes ); it; ++it ) {
		exportEnvelope( & it()() );
	}
}

struct MayaSkinCluster
{
	MObject skinClusterObj;
	MFnSkinCluster * fnSkinCluster;
	unsigned skinIndex;
	MDagPathArray geometries;
	MayaSkinCluster( MDagPath & dPath, array<MDagPath> & joints, array< array< float > > & weights )
	{
		MStatus status;
		skinClusterObj = Maya::getDGFirstItem( dPath.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream );

		MFnGeometryFilter fnGeometryFilter( skinClusterObj );
		float envelopeScale = fnGeometryFilter.envelope();
		unsigned nOutConnects = fnGeometryFilter.numOutputConnections();
		MDagPath path;
		for(int i = 0; i < nOutConnects; i++) {
			fnGeometryFilter.getPathAtIndex( i, path );
			geometries.append( path );
		}

		fnSkinCluster = new MFnSkinCluster( skinClusterObj );
		skinIndex = fnSkinCluster->indexForOutputShape( dPath.node() );
		MDagPathArray _joints;
		_joints.clear();
		fnSkinCluster->influenceObjects( _joints, & status );
		int numJoints = _joints.length();
		joints.allocate( numJoints );
		for ( int i = 0; i < numJoints; i++ ) joints[i] = _joints[i];

		MDagPath & geometry = geometries[skinIndex];
		MItGeometry geomIt( geometry );
		int count = geomIt.count();
		weights.allocate( count );
		for ( int i = 0; ! geomIt.isDone(); geomIt.next(), ++i ) {
			unsigned cInf;
			MFloatArray fa;
			fnSkinCluster->getWeights( geometry, geomIt.component(), fa, cInf );
			array< float > & weight = weights[i];
			weight.allocate( int( fa.length() ) );
			for ( int j = 0; j < fa.length(); ++j ) {
				if ( fa[j] * envelopeScale > roughEpsilon ) {
					weight[j] = fa[j] * envelopeScale;
				} else {
					weight[j] = 0;
				}
			}
		}
		
		array<int> endJointNum;
		for ( int i = 0; i < joints.size; i++ ) {
			if ( ! joints[i].childCount() ) endJointNum.push_back( i );
		}
		for ( int i = 0; i < weights.size; i++ ) {
			dumpEffectorJointWeight( endJointNum, weights[i] );
		}
	}
	~MayaSkinCluster()
	{
		delete fnSkinCluster;
	}
	void dumpEffectorJointWeight( array<int> & endJointNum, array< float > & w )
	{
		float total = 0.0;
		for ( int i = 0; i < w.size; i++ ) total += w[i];
		float endTotal = 0.0;
		for ( int i = 0; i < endJointNum.size; i++) {
			float & c = w[endJointNum[i]];
			if ( c == total ) {
			} else if( c > 0.0 ) {
				endTotal += c;
				c = 0.0;
			}
		}
		float cWeight = total-endTotal;
		if ( cWeight == 1.0 ) return;
		for ( int i = 0; i < w.size; i++ ) {
			if(w[i] > 0.0) {
				w[i] /= cWeight;
			}
		}
	}
};
struct MayaJointCluster
{
	MObjectArray jointClusters;
	bool abnormal;
	MayaJointCluster( MDagPath & dPath, array<MDagPath> & joints, array< array< float > > & weights )
	{
		abnormal = true;
		MFnMesh fnMesh( dPath.node() );
		weights.allocate( fnMesh.numVertices() );

		MStatus status;
		Maya::getDGAllItems( jointClusters, dPath.node(), MFn::kJointCluster, MItDependencyGraph::kUpstream );
		joints.allocate( jointClusters.length() );
		for ( int i = 0; i < jointClusters.length(); i++ ) {
			MFnWeightGeometryFilter fnWeightGeometryFilter( jointClusters[i] );
			MObject jointObject = Maya::getDGFirstItem( jointClusters[i], MFn::kJoint, MItDependencyGraph::kUpstream );
			joints[i] = Maya::getDagPath( jointObject );
		}
		for ( int i = 0; i < weights.size; i++ ) {
			array< float > & weight = weights[i];
			weight.allocate( joints.size );
			for ( int j = 0; j < weight.size; j++ ) weight[j] = 0;
		}

		for ( int i = 0; i < jointClusters.length(); i++ ) {
			MFnWeightGeometryFilter fnWeightGeometryFilter( jointClusters[i] );
			unsigned oGeomIdx = fnWeightGeometryFilter.indexForOutputShape( dPath.node(), & status );
			float envelopeScale = fnWeightGeometryFilter.envelope();

			//MDagPath shapePath;
			//fnWeightGeometryFilter.getPathAtIndex( oGeomIdx, shapePath );

			MFnSet fnSet( fnWeightGeometryFilter.deformerSet() );
			MSelectionList members;
			fnSet.getMembers( members, false );
			
			MDagPath dag;
			MObject component;
			for ( MItSelectionList sIt(members); ; sIt.next() ) {
				if ( sIt.isDone() ) {
					return;
				}
				sIt.getDagPath( dag, component );
				if ( dag == dPath ) break;
			}

			MItGeometry geomIt( dag, component, & status );
			for ( int j = 0; ! geomIt.isDone(); geomIt.next(), j++ ) {
				MFloatArray wghts;
				fnWeightGeometryFilter.getWeights( dag, geomIt.component(), wghts );
				int index = geomIt.index();
				int wgthsLength = wghts.length();
				if ( wghts.length() != 1 ) {
					return;
				}
				//if ( wghts[0] * envelopeScale != 1 ) return;//rigidなので、影響のあるほうは１００％である。
				weights[index][i] = 1;
			}
		}
		abnormal = false;
	}
};

bool getEnvelopeResource( envelopeInfomation * envelope, array<MDagPath> & joints, array< array< float > > & weights )
{
	MStatus status;
	//スムーススキニング
	if ( hasDependencyGraphItem( envelope->path.node(), MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream ) ) {
		MayaSkinCluster msc( envelope->path, joints, weights );
		return true;
	//リジッドスキニング
	} else if ( hasDependencyGraphItem( envelope->path.node(), MFn::kJointCluster, MItDependencyGraph::kUpstream ) ) {
		MayaJointCluster mjc( envelope->path, joints, weights );
		return ! mjc.abnormal;
	}
	return false;
}

void Maya::exportEnvelope( envelopeInfomation * envelope )
{
	array<MDagPath> joints;
	array< array< float > > weights;
	if ( ! getEnvelopeResource( envelope, joints, weights ) ) return;
	sgEnvelope * sgenvelope = new sgEnvelope( *graph );
	sgenvelope->name = envelope->name + "$envelope";
	sgVertices * dstVertices = envelope->polygon->vertices()();
	sgVertices * srcVertices = ( sgVertices * ) dstVertices->clone();
	srcVertices->name = envelope->name + "$srcVertices";
	new sgConnection( *srcVertices, *sgVertices::attributes, *sgenvelope, sgEnvelope::attributes->srcVertices );
	new sgConnection( *sgenvelope, sgEnvelope::attributes->dstVertices, *dstVertices, *sgVertices::attributes );

	int i;
	//boneの取得
	array<sgBone*> sgbones;
	array<int> boneIndex;
	sgbones.reserve( joints.size );
	boneIndex.reserve( joints.size );
	for ( i = 0; i < joints.size; i++ ) {
		MDagPath & joint = joints[i];
		sgBone * bone = searchBone( joint );
		if ( ! bone ) continue;
		try {
			for ( int j = 0; j < weights.size; j++ ) {
				if ( weights[j][i] ) throw true;
			}
		} catch ( bool ) {
			sgbones.push_back( bone );
			boneIndex.push_back( i );
			new sgConnection( *bone, *sgBone::attributes, *sgenvelope, sgEnvelope::attributes->bones );
		}
	}
	int countVertex = srcVertices->positions().size;
	sgenvelope->srcOffsetVertex( 0 );
	sgenvelope->dstOffsetVertex( 0 );
	sgenvelope->countVertex( countVertex );
	sgenvelope->initialMatrix( envelope->initialMatrix );
	new sgConnection( *envelope->currentMatrix, sgTransformMatrix::attributes->global, *sgenvelope, sgEnvelope::attributes->currentMatrix );
	sgenvelope->transformPosition( 1 );
	sgenvelope->transformNormal( 1 );
	sgenvelope->weights().allocate( countVertex );
	for ( i = 0; i < countVertex; i++ ) {
		array<float> & dst = sgenvelope->weights()[i];
		dst.allocate( sgbones.size );
		int vertexIndex = int( envelope->positionIndices[i] );
		array<float> & src = weights[vertexIndex];
		for ( int j = 0; j < sgbones.size; j++ ) {
			dst[j] = src[boneIndex[j]];
		}
	}
}
