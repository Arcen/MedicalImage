////////////////////////////////////////////////////////////////////////////////
// �^�̑���

//�m�[�h�̋L�q�q�i���̒��Ƀm�[�h�̃����o�[���̑���������j
class sgNodeDescriptor;

//�����̃N���X
class sgAttribute
{
	//�R�s�[�R���X�g���N�^�̋֎~�p
	sgAttribute();
	sgAttribute( const sgAttribute & value );
	sgAttribute & operator=( const sgAttribute & value );
public:
	int ID;//�esgNodeDescriptor���ł�attributes�z��̃C���f�b�N�X
	string name;//������
	const sgTypeDescriptor * type;//�f�[�^�̌^
	const sgNodeDescriptor * node;//�m�[�h�̌^
	unsigned int instance;//���̂̃N���X�̐擪����̃I�t�Z�b�g
	const void * initial;//�f�[�^�̏����f�[�^
	bool retain;//�t�@�C������̓��o�͂��s�����ǂ���
	bool target;// false : �\�[�X�̃C���X�^���X���f�X�e�B�l�[�V�����̃|�C���^�ɑ��, true : �f�X�e�B�l�[�V�����̃C���X�^���X���\�[�X�̃|�C���^�ɑ��
	//this�ւ̎Q��
	sgAttribute( int _id, const sgNodeDescriptor & _nd )
		: ID( _id ), name( "this" ), type( NULL ), node( & _nd ), instance( 0 ), initial( NULL ), retain( false ), target( false ) {}
	//���̂ւ̎Q��
	sgAttribute( int _id, const char * _name, const sgTypeDescriptor & _td, unsigned int _instance, const void * _initial, bool _retain, bool _target )
		: ID( _id ), name( _name ), type( & _td ), node( NULL ), instance( _instance ), initial( _initial ), retain( _retain ), target( _target ) {}
	//�z��ł̃m�[�h�ւ̎Q��
	sgAttribute( int _id, const char * _name, const sgNodeDescriptor & _nd, unsigned int _instance, bool _target )
		: ID( _id ), name( _name ), type( NULL ), node( & _nd ), instance( _instance ), initial( NULL ), retain( false ), target( _target ) {}
	//�����o�[�̎擾
	sgMemberInterface * getMember( void * base ) const { return instance ? reinterpret_cast<sgMemberInterface *>( reinterpret_cast<unsigned int>( base ) + instance ) : NULL; }
	//�����o�[�̎��̂̎擾
	void * getInstance( void * base )
	{
		sgMemberInterface * mi = getMember( base );
		return mi ? mi->getInstance() : base;
	}
	//�����o�[�̎Q�Ƃ̒ǉ�
	void insertReference( void * base, void * src )
	{
		sgMemberInterface * mi = getMember( base );
		if ( mi ) mi->insertReference( src );
	}
	//�����o�[�̎Q�Ƃ̍폜
	void killReference( void * base, void * src )
	{
		sgMemberInterface * mi = getMember( base );
		if ( mi ) mi->killReference( src );
	}
	//�����o�[�̎��Ԃ̏�����
	void setInitial( void * base, const void * src )
	{
		sgMemberInterface * mi = getMember( base );
		if ( mi ) mi->setInitial( src );
	}
	//��r
	bool operator==( const sgAttribute & src ) const {
		return ( ID == src.ID && name == src.name && type == src.type && 
			node == src.node && instance == src.instance );
	}
	bool operator!=( const sgAttribute & src ) const {
		return ( ID != src.ID || name != src.name || type != src.type || 
			node != src.node || instance != src.instance );
	}
};
