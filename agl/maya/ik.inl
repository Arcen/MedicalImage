sgDagNode * Maya::exportIk( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & parentTransform )
{
	MStatus status;
	MFnIkHandle fnIkHandle( dPath, & status ); statusError( status, "Fail MFnIkHandle" );
	string name = getName( dPath );

	sgMayaIkHandle * ikhandle = new sgMayaIkHandle( *graph );
	ikhandle->name = name + "$ikhandle";

	ikInfomation * ik = new ikInfomation();
	ik->ikhandle = ikhandle;
	ik->path = dPath;
	iks.push_back( ik );

	sgDagNode * result = exportDagTransform( dPath, parent, parentTransform );
	sgTransformMatrix * tm = static_cast<sgTransformMatrix*>( result->node );
	new sgConnection( *tm, sgTransformMatrix::attributes->center, *ikhandle, sgMayaIkHandle::attributes->position );
	return result;
}

bool Maya::exportIk( ikInfomation & ik )
{
	sgMayaIkHandle * ikhandle = ik.ikhandle;

	MStatus status;
	MFnIkHandle fnIkHandle( ik.path, & status ); statusError( status, "Fail MFnIkHandle" );
	MObject ikHandleObject = fnIkHandle.object();

	//startJoint
	MObject startJointObject = Maya::getInputObject( ikHandleObject, "startJoint" );
	MDagPath dPathStartJoint = getDagPath( startJointObject );
	MFnIkJoint fnStartJoint( dPathStartJoint, & status ); statusError( status, "Fail MFnJoint" );

	//endEffector
	MObject endEffectorObject = Maya::getInputObject( ikHandleObject, "endEffector" );
	MDagPath dEndEffector = getDagPath( endEffectorObject );
	MFnIkEffector fnEffector( dEndEffector, & status ); statusError( status, "Fail MFnIkEffector" );

	//endJoint
	MObject endJointObject = Maya::getInputObject( endEffectorObject, "translateX" );
	MDagPath dPathEndJoint = getDagPath( endJointObject );
	MFnIkJoint fnEndJoint( dPathEndJoint, & status ); statusError( status, "Fail MFnIkJoint" );

	if ( ! fnStartJoint.isParentOf( endJointObject ) ) return false;
	//startJoint����endJoint�܂ł����X�g�A�b�v����
	for ( MObject current = startJointObject; ! current.isNull(); ) {
		string name = getName( getDagPath( current ) );
		sgNode * joint = this->graph->search( name + "$joint" );
		if ( ! joint ) return false;
		if ( current == startJointObject ) {
			//���[�g�W���C���g�̑Ή�����s����Œ���X�V���K�v�Ȑe�m�[�h�Ƃ��ēo�^
			sgTransformMatrix * mat = ( sgTransformMatrix * ) joint->getOutput( sgMayaJointMatrix::attributes->matrix, sgTransformMatrix::descriptor );
			if ( mat ) {
				new sgConnection( *mat, *sgTransformMatrix::attributes, *ikhandle, sgMayaIkHandle::attributes->parent );
			}
		}
		new sgConnection( *joint, *sgMayaJointMatrix::attributes, *ikhandle, sgMayaIkHandle::attributes->joints );
		//�I������
		if ( current == endJointObject ) {
			return true;
		} else {
			//���ɏI�[���܂�joint�̎q����{��
			MObject next = MObject::kNullObj;
			MFnIkJoint fnJoint( current, & status ); statusError( status, "Fail MFnIkJoint" );
			for ( int i = 0; i < fnJoint.childCount(); ++i ) {
				MObject child = fnJoint.child( i, & status ); statusError( status, "Fail child" );
				if ( child.apiType() != MFn::kJoint ) continue;
				MFnIkJoint fnChildJoint( child, & status ); statusError( status, "Fail MFnIkJoint" );
				if ( child != endJointObject &&
					! fnChildJoint.isParentOf( endJointObject ) ) continue;
				next = child;
				break;
			}
			current = next;
		}
	}
	return false;
}

void Maya::exportIks()
{
	for ( list< retainer<ikInfomation> >::iterator it( iks ); it; ++it ) {
		ikInfomation & ik = it()();
		if ( ! exportIk( ik ) ) {
			delete ik.ikhandle;//�s�v�Ȃ̂ō폜
		}
	}
}
