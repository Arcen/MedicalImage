////////////////////////////////////////////////////////////////////////////////
// �G���x���[�v

#ifndef __COMPOSE__

////////////////////////////////////////////////////////////////////////////////
// �{�[��
class sgBone : public sgNode
{
public:
	sgNodeDeclare( sgBone );
	matrix44 matrix;//���s���Ɏg�p����ό`�̂��߂̍s��
	sgMatrix44 currentMatrix;//�{�[���𐢊E���W�n�֕ϊ�����s��
	sgMatrix44 initialMatrix;//�{�[���𐢊E���W�ł̏����ʒu�֕ϊ�����s��
	friend class sgEnvelope;
	sgBone();
	virtual void update()
	{
		//���[���h���珉���ʒu�̃{�[���̃��[�J���ɕϊ����A
		//���ꂪ���݂̃{�[���̃��[�J���Ɖ��肵�ă��[���h�֕ϊ�����s����쐬
		matrix = matrix44::inverse( initialMatrix() ) * currentMatrix();
	}
};

////////////////////////////////////////////////////////////////////////////////
// �G���x���[�v�@����Ń��b�V���p�̒��_�����X�V����
class sgEnvelope : public sgNode
{
public:
	sgNodeDeclare( sgEnvelope );

	friend class sgBone;
	//�����̃{�[���ɑΉ����A���x���m�ۂ��邽�ߤ�ǂݍ��ݎ��ɔz��ɓ���Ă���
	array<matrix44> forPosition;
	array<matrix33> forNormal;

	sgNodeMember<sgBone> bones;//���͌��ɂȂ�{�[��
	sgNodeMember<sgVertices> srcVertices;//�ό`���ɂȂ钸�_�z��
	sgNodeMember<sgVertices> dstVertices;//�ό`��̒��_�z��
	sgMatrix44 currentMatrix;//�G���x���[�v�����݂̐��E���W�֕ϊ�����s��
	sgMatrix44 initialMatrix;//�G���x���[�v�����蓖�Ď��̐��E���W�֕ϊ�����s��

	sgInt srcOffsetVertex;//�\�[�X�̒��_�z��̃I�t�Z�b�g
	sgInt dstOffsetVertex;//�^�[�Q�b�g�̒��_�z��̃I�t�Z�b�g
	sgInt countVertex;//�ό`���钸�_��
	sgArrayArrayDecimal weights;//�d��
	sgInt transformPosition;//�ʒu��ό`����
	sgInt transformNormal;//�@����ό`����

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
			//�G���x���[�v�̃��[�J�������[���h�ɕϊ����A�{�[���ό`��̃��[���h�֕ϊ����A���݂̃��[�J���ɕϊ�����
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
			array< array< float > > & w = weights();//�d��
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
