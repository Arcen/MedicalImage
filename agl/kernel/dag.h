////////////////////////////////////////////////////////////////////////////////
// DAG���

//�V�[���̃c���[�^�̃m�[�h�B�m�[�h�̃|�C���^���݂̂������A�c���[�\�����\�z����
class sgDagNode
{
public:
	sgNode * node;
	list< retainer<sgDagNode> > children;//�q��
	//dag��root�p�̃R���X�g���N�^
	sgDagNode() : node( NULL )
	{
	}
	//�q���p�̃R���X�g���N�^
	sgDagNode( sgNode & _node ) : node( & _node )
	{
	}
	virtual ~sgDagNode()
	{
	}
	//�ǉ��F���̊֐��Ŏq���Ƃ���dag���쐬
	sgDagNode * insert( sgNode & node )
	{
		return & children.push_back( new sgDagNode( node ) )();
	}
	//�w��̃m�[�h���q������폜
	void remove( sgNode & node )
	{
		for ( list< retainer<sgDagNode> >::iterator it( children ); it; ++it ) {
			if ( it()->node == & node ) {
				children.pop( it );
				return;
			}
		}
	}
};
