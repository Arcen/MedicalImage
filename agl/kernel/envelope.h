////////////////////////////////////////////////////////////////////////////////
// エンベロープ

#ifndef __COMPOSE__

////////////////////////////////////////////////////////////////////////////////
// ボーン
class sgBone : public sgNode
{
public:
	sgNodeDeclare( sgBone );
	matrix44 matrix;//実行時に使用する変形のための行列
	sgMatrix44 currentMatrix;//ボーンを世界座標系へ変換する行列
	sgMatrix44 initialMatrix;//ボーンを世界座標での初期位置へ変換する行列
	friend class sgEnvelope;
	sgBone();
	virtual void update()
	{
		//ワールドから初期位置のボーンのローカルに変換し、
		//それが現在のボーンのローカルと仮定してワールドへ変換する行列を作成
		matrix = matrix44::inverse( initialMatrix() ) * currentMatrix();
	}
};

////////////////////////////////////////////////////////////////////////////////
// エンベロープ　これでメッシュ用の頂点情報を更新する
class sgEnvelope : public sgNode
{
public:
	sgNodeDeclare( sgEnvelope );

	friend class sgBone;
	//複数のボーンに対応し、速度も確保するため､読み込み時に配列に入れておく
	array<matrix44> forPosition;
	array<matrix33> forNormal;

	sgNodeMember<sgBone> bones;//入力元になるボーン
	sgNodeMember<sgVertices> srcVertices;//変形元になる頂点配列
	sgNodeMember<sgVertices> dstVertices;//変形先の頂点配列
	sgMatrix44 currentMatrix;//エンベロープを現在の世界座標へ変換する行列
	sgMatrix44 initialMatrix;//エンベロープを割り当て時の世界座標へ変換する行列

	sgInt srcOffsetVertex;//ソースの頂点配列のオフセット
	sgInt dstOffsetVertex;//ターゲットの頂点配列のオフセット
	sgInt countVertex;//変形する頂点数
	sgArrayArrayDecimal weights;//重み
	sgInt transformPosition;//位置を変形する
	sgInt transformNormal;//法線を変形する

	virtual void update()
	{
		const int bonesCount = bones.count();
		if ( ! bonesCount ) return;
		forPosition.allocate( bonesCount );
		forNormal.allocate( bonesCount );
		matrix44 inverseInputCurrentMatrix = matrix44::inverse( currentMatrix() );
		int i = 0;
		for ( sgNodeMember<sgBone>::iterator it( bones() ); it; ++it, ++i ) {
			sgBone & bone = *it();
			//エンベロープのローカルをワールドに変換し、ボーン変形後のワールドへ変換し、現在のローカルに変換する
			forPosition[i] = initialMatrix() * bone.matrix * inverseInputCurrentMatrix;
			forNormal[i] = forPosition[i].getMatrix33();
		}
		const int cv = countVertex();
		if ( bonesCount == 1 ) {
			if ( transformPosition() ) {
				vector3 * src = srcVertices()()->positions().data + srcOffsetVertex();
				vector3 * dst = dstVertices()()->positions().data + dstOffsetVertex();
				matrix44 & p = forPosition[0];
				for ( int i = 0; i < cv; i++ ) {
					dst[i] = vector3( vector4( src[i] ) * p );
				}
			}
			if ( transformNormal() ) {
				vector3 * src = srcVertices()()->normals().data + srcOffsetVertex();
				vector3 * dst = dstVertices()()->normals().data + dstOffsetVertex();
				matrix33 & n = forNormal[0];
				for ( int i = 0; i < cv; i++ ) {
					dst[i] = vector3::normalize3( src[i] * n );
				}
			}
		} else {
			array< array< float > > & w = weights();//重み
			if ( transformPosition() ) {
				vector3 * src = srcVertices()()->positions().data + srcOffsetVertex();
				vector3 * dst = dstVertices()()->positions().data + dstOffsetVertex();
				for ( int i = 0; i < cv; i++ ) {
					dst[i] = vector3::identity;
					for ( int j = 0; j < bonesCount; j++ ) {
						if ( w[i][j] ) dst[i] += vector3( vector4( src[i] ) * forPosition[j] ) * w[i][j];
					}
				}
			}
			if ( transformNormal() ) {
				vector3 * src = srcVertices()()->normals().data + srcOffsetVertex();
				vector3 * dst = dstVertices()()->normals().data + dstOffsetVertex();
				vector3 wk;
				for ( int i = 0; i < cv; i++ ) {
					wk = vector3::identity;
					for ( int j = 0; j < bonesCount; j++ ) {
						if ( w[i][j] ) wk += vector3::normalize3( src[i] * forNormal[j] ) * w[i][j];
					}
					dst[i] = vector3::normalize3( wk );
				}
			}
		}
	}
};

#else

sgNodeDeclareInstance( sgBone )
{
	sgDeclareThis();
	sgDeclareInitialRetain( currentMatrix, matrix44::identity );
	sgDeclareInitialRetain( initialMatrix, matrix44::identity );
}

sgNodeDeclareInstance( sgEnvelope )
{
	sgDeclareThis();
	sgDeclareNode( bones, sgBone );
	sgDeclareNode( srcVertices, sgVertices );
	sgDeclareNodeTarget( dstVertices, sgVertices );

	sgDeclareInitialRetain( currentMatrix, matrix44::identity );
	sgDeclareInitialRetain( initialMatrix, matrix44::identity );

	sgDeclareInitialRetain( srcOffsetVertex, 0 );
	sgDeclareInitialRetain( dstOffsetVertex, 0 );
	sgDeclareInitialRetain( countVertex, 0 );
	sgDeclareRetain( weights );
	sgDeclareInitialRetain( transformPosition, 1 );
	sgDeclareInitialRetain( transformNormal, 1 );
}

#endif
