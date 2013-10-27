////////////////////////////////////////////////////////////////////////////////
// �O���t�m�[�h�̃x�[�X
class sgNode
{
public:
	sgGraph & graph;//��������O���t
	sgNodeDescriptor & thisDescriptor;//�m�[�h��ʗp�L�q�q�istatic sgNodeDescriptorInstance * descriptor�ւ̎Q�Ɓj
	void * base;//���̂̃A�h���X
	int ID;//�V�[���O���t���ň�ӂ�ID
	void * external;//�m�[�h�̊O���f�[�^�p�̃|�C���^
	bool dirty;//smart��update���s�����߂́A���̃m�[�h���A�b�v�f�[�g���K�v���ǂ���
	string name;//���O
	array< list<sgConnection*> > inputs;//���̃m�[�h�ւ̓���
	array< list<sgConnection*> > outputs;//���̃m�[�h����̏o��

	////////////////////////////////////////////////////////////////////////////////
	//�R���X�g���N�^�^�f�X�g���N�^
	sgNode( sgGraph & _graph, sgNodeDescriptor & _descriptor, void * _base );//graph.h
	virtual ~sgNode();//graph.h
	////////////////////////////////////////////////////////////////////////////////
	//�m�[�h�̃A�b�v�f�[�g
	virtual void update(){}
	////////////////////////////////////////////////////////////////////////////////
	//sgAttribute�ւ�proxy
	void * getInstance( sgAttribute & attrbt ) { return attrbt.getInstance( base ); }
	void insertReference( sgAttribute & attrbt, void * src ) { attrbt.insertReference( base, src ); }
	void killReference( sgAttribute & attrbt, void * src ) { attrbt.killReference( base, src ); }
	void setInitial( sgAttribute & attrbt, const void * src ) { attrbt.setInitial( base, src ); }
	////////////////////////////////////////////////////////////////////////////////
	//���o�͎擾
	sgNode * getInput( sgAttribute & attribute, sgNodeDescriptor * srcDescriptor = NULL )
	{
		for ( list<sgConnection*>::iterator it( inputs[attribute.ID] ); it; ++it ) {
			if ( srcDescriptor && it()->src.thisDescriptor != srcDescriptor ) continue;
			return & ( it()->src );
		}
		return NULL;
	}
	void enumrateInputs( list<sgNode*> & results, sgAttribute * attribute = NULL, sgNodeDescriptor * srcDescriptor = NULL )
	{
		results.release();
		if ( attribute ) {
			for ( list<sgConnection*>::iterator it( inputs[attribute->ID] ); it; ++it ) {
				if ( srcDescriptor && it()->src.thisDescriptor != srcDescriptor ) continue;
				results.push_back( & it()->src );
			}
		} else {
			for ( array< list<sgConnection*> >::iterator its( inputs ); its; ++its ) {
				for ( list<sgConnection*>::iterator it( its() ); it; ++it ) {
					if ( srcDescriptor && it()->src.thisDescriptor != srcDescriptor ) continue;
					results.push_back( & it()->src );
				}
			}
		}
	}
	sgNode * getOutput( sgAttribute & attribute, sgNodeDescriptor * dstDescriptor = NULL )
	{
		for ( list<sgConnection*>::iterator it( outputs[attribute.ID] ); it; ++it ) {
			if ( dstDescriptor && it()->dst.thisDescriptor != dstDescriptor ) continue;
			return & ( it()->dst );
		}
		return NULL;
	}
	void enumrateOutputs( list<sgNode*> & results, sgAttribute * attribute = NULL, sgNodeDescriptor * dstDescriptor = NULL )
	{
		results.release();
		if ( attribute ) {
			for ( list<sgConnection*>::iterator it( outputs[attribute->ID] ); it; ++it ) {
				if ( dstDescriptor && it()->dst.thisDescriptor != dstDescriptor ) continue;
				results.push_back( & it()->dst );
			}
		} else {
			for ( array< list<sgConnection*> >::iterator its( outputs ); its; ++its ) {
				for ( list<sgConnection*>::iterator it( its() ); it; ++it ) {
					if ( dstDescriptor && it()->dst.thisDescriptor != dstDescriptor ) continue;
					results.push_back( & it()->dst );
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	//�m�[�h�̕���
	virtual sgNode * clone( int ancestor = 0 )
	{
		sgNode * c = thisDescriptor.creator( graph );
		static int unique = 0;
		c->name = name + "$clone" + string( unique++ );

		for ( array< list<sgConnection*> >::iterator iit( inputs ); iit; ++iit ) {
			for ( list<sgConnection*>::iterator it( iit() ); it; ++it ) {
				sgConnection * cnnctn = it();
				sgNode * src = & ( cnnctn->src );
				if ( & cnnctn->src == & cnnctn->dst ) {
					src = c;
				} else {
					if ( ancestor ) src = src->clone( ancestor - 1 );
				}
				new sgConnection( *src, cnnctn->output, *c, cnnctn->input );
			}
		}
		for ( array< retainer<sgAttribute> >::iterator ait( thisDescriptor.attributes ); ait; ++ait ) {
			sgAttribute & attrbt = ait()();
			if ( ! attrbt.type ) continue;
			attrbt.type->copyer( c->getInstance( attrbt ), getInstance( attrbt ) );
		}
		return c;
	}
	////////////////////////////////////////////////////////////////////////////////
	// �����o�[���������ς݂��ǂ����𒲂ׂ�
	bool initialized( sgAttribute & attrbt )
	{
		if ( ! attrbt.type ) return true;
		if ( attrbt.initial ) return attrbt.type->compair( getInstance( attrbt ), attrbt.initial );
		return attrbt.type->initialized( getInstance( attrbt ) );
	}
	////////////////////////////////////////////////////////////////////////////////
	// �����o�[������������
	void initializeAttribute()
	{
		for ( array< retainer<sgAttribute> >::iterator it( thisDescriptor.attributes ); it; ++it ) {
			sgAttribute & attrbt = it()();
			if ( ! attrbt.type ) continue;
			//�����ݒ�
			if ( attrbt.initial ) {
				setInitial( attrbt, attrbt.initial );
			} else {
				attrbt.type->initializer( getInstance( attrbt ) );
			}
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
// sgNode�̐ڑ����
inline sgConnection::sgConnection( sgNode & _src, sgAttribute & _output, sgNode & _dst, sgAttribute & _input ) 
	: src( _src ), output( _output ), dst( _dst ), input( _input )
{
	//�^�L�q�q�ƃm�[�h�L�q�q�͓����ł���K�v������B
	assert( input.type == output.type && input.node == output.node );
	dst.inputs[input.ID].push_back( this );
	src.outputs[output.ID].push_back( this );
	if ( output.target ) {
		//�^�[�Q�b�g���ύX���鎞�ɂ́A�Ώۂ̃|�C���^���\�[�X���ŕێ�����
		src.insertReference( output, dst.getInstance( input ) );
	} else {
		//�\�[�X����Ώۑ��ŎQ�Ƃ�ۑ�����
		dst.insertReference( input, src.getInstance( output ) );
	}
}

//�Ȃ���̍폜
inline sgConnection::~sgConnection()
{
	dst.inputs[input.ID].pop( this );
	src.outputs[output.ID].pop( this );
}

////////////////////////////////////////////////////////////////////////////////
// �m�[�h�̔h���I�u�W�F�N�g�̃f�t�H���g
#define sgNodeDeclare(classname);	\
	classname( sgGraph & graph ) : sgNode( graph, *descriptor, this ) { thisDescriptor.initialize( this ); }\
	virtual ~classname(){};\
	typedef classname thisClass;\
	class sgNodeDescriptorInstance : public sgNodeDescriptor { public:\
		sgNodeDescriptorInstance() : sgNodeDescriptor( #classname ){\
			assert( ! classname::descriptor );\
			assert( ! classname::attributes );\
			classname::descriptor = this;\
			classname::attributes = new thisClass( * sgGraph::attributeGraph );\
			initializer(); }\
		virtual ~sgNodeDescriptorInstance(){\
			assert( classname::descriptor );\
			assert( classname::attributes );\
			finalizer();\
			classname::descriptor = NULL;\
			delete classname::attributes;\
			classname::attributes = NULL; }\
		virtual sgNode * creator( sgGraph & graph ) { return new thisClass( graph ); }\
		virtual void initializer();\
		virtual void finalizer(){};\
	};\
	static sgNodeDescriptorInstance * descriptor;\
	static thisClass * attributes;\
	static sgAttribute * thisAttribute;\
	static sgAttribute & setAttribute( sgAttribute & src ){ thisAttribute = & src; return getAttribute(); }\
	static sgAttribute & getAttribute(){ return *thisAttribute; }\
	operator sgAttribute & () { return getAttribute(); }
//sgNodeDescriptorInstance//�m�[�h�̋L�q�������ۂɕۑ�����N���X
//sgNodeDescriptorInstance::initializer//��������ݒ肵�A�m�[�h�̋L�q�����\�z����֐��B
//descriptor//�m�[�h�̋L�q�q��ێ�����
//attributes//�m�[�h�̃����o�[�p�̑��������e�����o�[�̎Q�Ƃ̈ʒu�ɋL�^���邽�߂Ƀm�[�h�Ɠ����N���X�𗘗p����
//thisAttribute//�m�[�h���g�̑������

