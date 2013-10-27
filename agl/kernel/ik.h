////////////////////////////////////////////////////////////////////////////////
// �C���o�[�X�L�l�}�e�B�N�X

#ifndef __COMPOSE__

//maya�p��IK���Z�N���X�Djoint/effector�����X�g�Ƃ��Ă����C
//handle�̈ʒu��effector������悤��joint�̊p�x��ύX������C
//handle���X�i�b�v(effector�̈ʒu�ɕύX)����
class sgMayaIkHandle : public sgNode
{
	array<sgMayaJointMatrix*> jointsArray;
	array<sgTransformMatrix*> jointMatricies;
public:
	sgNodeDeclare( sgMayaIkHandle );
	sgNodeMember<sgTransformMatrix> parent;//�W���C���g�̐e�̃m�[�h�i�Œ���̍X�V�����҂���j
	sgNodeMember<sgMayaJointMatrix> joints;//���͏o�͐��joint���D�擪����Ceffector�܂ł�����D
	sgInt valid;//�n���h���̈ʒu�ɃW���C���g���ǂ������邩�ǂ���
	sgVector3 position;//�����Ɍ����āC���� �O���[�o�����W
	sgDecimal epsiron;//�������v�Z����Ƃ��̔����p�x[rad]
	sgDecimal movability;//�n���h���̓����Ղ�[rad]
	sgInt numberJointEstimate;//�W���C���g�̐���񐔁F�P�̂̉�]������������A�P�Ȃ�ΑS�̂̍X�V�p�x�̑傫���������ɂȂ�悤�ɓ���
	sgInt numberJointsEstimate;//�W���C���g�S�̂̐���񐔁F�W���C���g�S�̂̃n���h���ւ̐ڋ߂������Ȃ�

	class axisInformation
	{
	public:
		int jointIndex;
		int axisIndex;
		decimal mnmm, mxmm;
		bool limited;
	};
	//����_�ɑ΂���joint�̉�]�݂̂��w��p�x�ōs��
	vector3 rotate( const vector3 & angle, const vector4 & position, sgMayaJointMatrix * joint )
	{
		return ( position * matrix44::rotation( joint->composeRotation( angle ) ) ).getVector3();
	}
	//�ŋ}�~���@�ɂ��A�W���C���g�̉�]�p�x�̐���
	bool estimate( const vector4 & handle, const vector4 & effector, sgMayaJointMatrix * joint )
	{
		vector3 & angle = joint->angle();//�p�x
		const vector3 e = rotate( angle, effector, joint );//�G�t�F�N�^�̉�]��̈ʒu
		const vector3 h = ( handle ).getVector3();
		const matrix33 jacobian( matrix33::zero );
		vector3 & freedom = joint->freedom();//��]�����R�Ȏ�
		const vector3 epsiron3[3] = { vector3( epsiron(), 0, 0 ), vector3( 0, epsiron(), 0 ), vector3( 0, 0, epsiron() ) };
		vector3 delta( vector3::zero );
		const vector3 h_e = h - e;
		for ( int i = 0; i < 3; ++i ) {
			if ( ! freedom[i] ) continue;
			//�ŋ}�s���@
			delta[i] = vector3::dot3( h_e, // �}�C�i�X������ɓ���č���������B�{����- ( e - h ) * transpose(jacobian);
				//���R�r�A��
				//�e�x�N�g���͂��̗v�f��epsiron���������ɓ��������̃G�t�F�N�^�̉�]��̈ʒu�̍�
				// ���K������̂Ŋ���Z���ȗ� jacobian = ( plus - minus ) / ( 2 * epsiron );
				rotate( angle + epsiron3[i], effector, joint ) - 
				rotate( angle - epsiron3[i], effector, joint ) );
		}
		//�p�x�������Ȃ�I���
		if ( vector3::zero == delta ) return true;
		//���K��
		delta = vector3::normalize3( delta ) * movability();

		vector3 newangle = angle + delta;//�V�����p�x
		joint->limitAngle( newangle );//�������s��
		if ( newangle == angle ) return true;//���łɌ��E
		const vector3 enew = rotate( newangle, effector, joint );
		if ( vector3::length3( e - h ) < vector3::length3( enew - h ) ) {
			return true;//���������ʉ��������Ă���
		}
		//�p�x�̍X�V
		angle = newangle;
		return false;
	}
	//solver�𓮍삳����
	//�㗬�ɂ���DAG����n�܂��Ă���solver���瓮�삳����K�v������D
	//�����āC��𓮍삳������ɁCdirty�t���O���������ꍇ�ɂ́Csmart��update���āC������solver�𓮍삳����
	virtual void update()
	{
		if ( ! valid() ) return;
		if ( joints.count() < 2 ) return;//��]����K�v������
		//�W���C���g�̏��
		if ( ! jointsArray.size ) {
			jointsArray.reserve( joints.count() );
			for ( sgNodeMember<sgMayaJointMatrix>::iterator it( joints() ); it; ++it ) {
				jointsArray.push_back( it() );
			}
			jointMatricies.allocate( jointsArray.size );
			for ( int i = 0; i < jointsArray.size; ++i ) {
				jointMatricies[i] = ( sgTransformMatrix * ) jointsArray[i]->getOutput( sgMayaJointMatrix::attributes->matrix, sgTransformMatrix::descriptor );
			}
		}
		//�^�[�Q�b�g����X�V����K�v������
		for ( int i = 0; i < jointMatricies.size; ++i ) {
			jointsArray[i]->update();
			jointMatricies[i]->update();
		}

		for ( int count = 0; count < numberJointsEstimate(); ++count ) {
			matrix44 prerot = jointMatricies.last()->local();//�G�t�F�N�^�̍��W�n����Ώۂ̃W���C���g�܂ł̏C�����ꂽ�ϊ��s�������킷�悤�ɂ���
			for ( int i = jointsArray.size - 2; i >= 0; --i ) {
				sgMayaJointMatrix * joint = jointsArray[i];//��]����W���C���g
				sgTransformMatrix * tm = jointMatricies[i];//���̉�]�Ɛe���p
				//��]�܂ł̃��[�J�����W�ւ̕ϊ�
				//�ڕW�̈ʒu�i���[���h���W�n�̖ڕW�̈ʒu���t�ϊ��ŃW���C���g�̉�]��̍��W�n�i��]�̉e���͎󂯂Ȃ��j�ɕϊ��j
				vector4 handle = vector4( position() ) * matrix44::inverse( joint->postRotation() * tm->parent() );
				//���݂̈ʒu�ieffector�̃��[�J���ʒu����]�O�̍��W�n�̈ʒu�ɕϊ��j
				vector4 effector = vector4( 0, 0, 0, 1 ) * prerot * joint->preRotation();
				// effector * rotation == handle �ɂȂ�悤�ȉ�]�ɕύX����悤�ɂ���

				//�ŋ}�~���@�ɂāA�p�x���X�V���Ă����B
				bool changed = false;
				for ( int k = 0; k < numberJointEstimate(); ++k ) {//�W���C���g�P�̂̍X�V��
					if ( estimate( handle, effector, joint ) ) break;
					changed = true;
				}
				if ( changed ) {
					joint->dirty = true;//�ʂɉ�]����Ƃ��̓t���O���Y��ɂȂ�Ȃ����A�Ō�ɃG���x���[�v�Ȃǂɂ����ĉe�����o��悤�ɉ����Ă���
					//joint->graph.update( true );//�S�̂��X�V����ق������S�����A�O���t�ɂ���Ă͈ȉ��̕��������ŁAIK�ɂ����Ă͏\��
					//�W���C���g�{�̂̍X�V
					jointsArray[i]->update();
					for ( int j = i; j < jointMatricies.size; ++j ) {
						//����ȉ��̃W���C���g�̍s��ɂ���global���X�V���Ă���
						jointMatricies[j]->update();
					}
				}
				prerot = prerot * tm->local();
			}
		}
	}
};

#else

sgNodeDeclareInstance( sgMayaIkHandle )
{
	sgDeclareThis();
	sgDeclareNodeTarget( parent, sgTransformMatrix );
	sgDeclareNodeTarget( joints, sgMayaJointMatrix );
	sgDeclareInitialRetain( valid, 0 );
	sgDeclareInitialRetain( position, vector3::identity );
	sgDeclareInitialRetain( epsiron, deg2rad( 1.0 ) );
	sgDeclareInitialRetain( movability, deg2rad( 1.0 ) );
	sgDeclareInitialRetain( numberJointEstimate, 1 );
	sgDeclareInitialRetain( numberJointsEstimate, 5 );
}

#endif
