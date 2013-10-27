////////////////////////////////////////////////////////////////////////////////
// �O���t
class sgGraph
{
//	static bool CoInitialized;
public:
	string name;//�O���t�̖��O
	int maxID;//�m�[�h��ID
	list< retainer<sgNode> > nodes;//�m�[�h�̃��X�g�i�폜��ۏ؂���j
	friend class sgNode;
	static sgGraph *attributeGraph;//�O���t�̋L�q�q�i���̂̓m�[�h�j��ێ����邽�߂̃O���t
	sgDagNode root;//DAG�̃��[�g�m�[�h�i��j
	sgGraph() : maxID( 0 )
	{
	}
	//�m�[�h�̑}��
	void insert( sgNode * node ) { nodes.push_back( node ); }
	//�m�[�h�̎��O���i�폜�͍s��Ȃ��j
	void remove( sgNode * node ) 
	{
		retainer<sgNode> wk( node );//�����p�Ɍ^�����킹��
		list< retainer<sgNode> >::iterator it = nodes.search( wk );
		wk.loot();//�����p�Ȃ̂ŁA�폜����Ȃ��悤�Ɏ��O���B
		if ( it ) {//�������Ă����ꍇ
			it().loot();//���������폜����Ȃ��悤�Ɏ��O����
			nodes.pop( it );//���X�g����폜����
		}
	}
	//�񋓁@�L�q�q�������ɗ^�����ꍇ�ɂ͂��̎�ނ̃m�[�h�݂̂�񋓂���
	void enumrate( list<sgNode*> & dst, sgNodeDescriptor * descriptor = NULL )
	{
		dst.release();
		for ( list< retainer<sgNode> >::iterator it( nodes ); it; ++it ) {
			if ( descriptor && it()->thisDescriptor != descriptor ) continue;
			dst.push_back( & it()() );
		}
	}
	//���O�ɂ�錟��
	sgNode * search( const char * name ) 
	{
		for ( list< retainer<sgNode> >::iterator it( nodes ); it; ++it ) {
			if ( it()->name == name ) {
				return & it()();
			}
		}
		return NULL;
	}
	//���͌����珇��nodes����ׂ�iupdate����ɂ͕��ׂȂ����Ȃ��Ƃ����Ȃ��j
	//���͂��z���Ă���ꍇ�ɂ́A�������[�v����̂ŁA�O���t���쐬����Ƃ��͋C������B
	void alignment()
	{
		list< retainer<sgNode> > temp;
		//temp�Ƀ��������󂳂Ȃ��悤�ɁA�ړ�����
		while ( nodes.size ) temp.push_back( nodes.loot_front() );
		while ( temp.size ) {
			sgNode * target = temp.loot_front().loot();
			bool found = false;
			//�Ώۂ̓��͌����
			for ( array< list<sgConnection*> >::iterator itcs( target->inputs ); itcs; ++itcs ) {
				for ( list<sgConnection*>::iterator itc( itcs() ); itc; ++itc ) {
					//���͌�
					sgNode & src = itc()->src;
					//���͌����������X�g�Ɏc���Ă��邩�ǂ����𒲂ׂ�
					for ( list< retainer<sgNode> >::iterator itn( temp ); itn; ++itn ) {
						if ( itn() == & src ) {
							temp.push_back( target );//�Ώۂ����ɉ񂷁B
							found = true; break;
						}
					}
					if ( found ) break;
				}
				if ( found ) break;
			}
			if ( found ) continue;//���������ꍇ�͐擪�ɂ�����̂��猩����
			nodes.push_back( target );
		}
	}
	//�X�V����ialignment����Ă�K�v������j
	bool update( bool smart = false )
	{
		if ( smart ) {
			//sgTime�ȂǁA�X�V�����m�[�h��dirty��true�ɂ���K�v������B
			//�_�[�e�B�t���O���Ȃ���΁A�X�V���Ȃ��B�܂��A�X�V���ꂽ��o�͐�ɁA�X�V����悤�ɁA�_�[�e�B�ɂ���B
			bool change = false;
			for ( list< retainer<sgNode> >::iterator itn( nodes ); itn; ++itn ) {
				sgNode & node = itn()();
				if ( node.dirty == false ) continue;
				node.update();//�X�V
				node.dirty = false;//�X�V�ς�
				change = true;//��������̃m�[�h���X�V���ꂽ
				for ( array< list<sgConnection*> >::iterator itcs( node.outputs ); itcs; ++itcs ) {
					for ( list<sgConnection*>::iterator itc( itcs() ); itc; ++itc ) {
						itc()->dst.dirty = true;//�o�͐���X�V����
					}
				}
			}
			return change;
		} else {
			//�S�Ă��X�V
			for ( list< retainer<sgNode> >::iterator it( nodes ); it; ++it ) {
				it()->update();
			}
			return true;
		}
	}
	static void initialize();
	static void finalize();
	void save( const char * filename );
	static sgGraph * load( const char * filename );
};

//�m�[�h�̃R���X�g���N�^
inline sgNode::sgNode( sgGraph & _graph, sgNodeDescriptor & _thisDescriptor, void * _base ) : 
	graph( _graph ), thisDescriptor( _thisDescriptor ), base( _base ), ID( _graph.maxID++ ), external( NULL ), dirty( true )
{
	graph.insert( this );//�O���t�̃��X�g�ɑ}��
	//�ڑ����������o���{�����̕������m�ۂ���
	outputs.allocate( thisDescriptor.attributes.size );
	inputs.allocate( thisDescriptor.attributes.size );
	name.print( "%p", this );//�������̃A�h���X����Ƃ肠������ӂȖ��O������
}

//�m�[�h�̃f�X�g���N�^
inline sgNode::~sgNode()
{
	//�ڑ���񂪂���΁A�폜����
	for ( array< list<sgConnection*> >::iterator oit( outputs ); oit; ++oit ) {
		list<sgConnection*> & connections = oit();
		while ( connections.size ) delete connections.first();
	}
	for ( array< list<sgConnection*> >::iterator iit( inputs ); iit; ++iit ) {
		list<sgConnection*> & connections = iit();
		while ( connections.size ) delete connections.first();
	}
	//�O���t�����菜��
	graph.remove( this );
}

//�m�[�h�̋L�q�q�ɂ�郁���o�[�̏�����
inline void sgNodeDescriptor::initialize( sgNode * nd )
{
	if ( sgGraph::attributeGraph == & nd->graph ) return;//�L�q�q���̂̏ꍇ�ɂ͏��������Ȃ�
	for ( array< retainer<sgAttribute> >::iterator it( attributes ); it; ++it ) {
		sgAttribute & attrbt = it()();
		if ( ! attrbt.instance ) continue;//this property
		//�����ݒ�
		//�C���X�^���X�Ƀf�t�H���g��ݒ�
		if ( attrbt.initial ) attrbt.type->copyer( nd->getInstance( attrbt ), attrbt.initial );
	}
}

#ifdef __GLOBAL__
sgGraph *sgGraph::attributeGraph = NULL;
#endif
