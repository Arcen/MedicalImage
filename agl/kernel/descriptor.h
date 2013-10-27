////////////////////////////////////////////////////////////////////////////////
// �m�[�h�������

class sgNode;
class sgGraph;

//�m�[�h�̋L�q�q�i���̒��Ƀm�[�h�̃����o�[���̑���������j
class sgNodeDescriptor
{
public:
	static list<sgNodeDescriptor*> nodes;//�L�q�q�̃��X�g
	//���O����̌���
	static sgNodeDescriptor * search( const char * name )
	{
		for ( list<sgNodeDescriptor*>::iterator it( nodes ); it; ++it ) {
			if ( it()->name == name ) {
				return it();
			}
		}
		return NULL;
	}
	//�����̃��X�g
	array< retainer<sgAttribute> > attributes;
	string name;
	sgNodeDescriptor( const char * _name )
		: name( _name )
	{
		nodes.push_back( this );
	}
	virtual ~sgNodeDescriptor()
	{
		nodes.pop( this );
	}
	//�����̎擾
	sgAttribute * getAttribute( const char * name ) 
	{
		for ( array< retainer<sgAttribute> >::iterator it( attributes ); it; ++it ) {
			if ( it()->name == name ) {
				return & it()();
			}
		}
		return NULL;
	}
	//�m�[�h�̃����o�[�̏�����
	void initialize( sgNode * node );
	//�L�q�q�̍\�z�p�̏������z�֐�
	virtual void initializer() = 0;
	//�m�[�h�̐����q�p�̏������z�֐�
	virtual sgNode * creator( sgGraph & graph ) = 0;

	//�m�[�h�̌^��錾����
	sgAttribute & declareThis()
	{
		return ( attributes.push_back( new sgAttribute( attributes.size, *this ) ) )();
	}
	//�����o�[�̌^��錾����
	template<class T>
	sgAttribute & declareMember( const char * name, unsigned int instance, T & initial, bool retain, bool target )
	{
		return ( attributes.push_back( new sgAttribute( attributes.size, name, getTypeDescriptor( ( const T * ) NULL ), instance, & initial, retain, target ) ) )();
	}
	sgAttribute & declareNode( const char * name, sgNodeDescriptor & node, unsigned int instance, bool target )
	{
		return ( attributes.push_back( new sgAttribute( attributes.size, name, node, instance, target ) ) )();
	}
	//��r
	bool operator==( const sgNodeDescriptor & right ) const { return this == & right; }
	bool operator==( const sgNodeDescriptor * right ) const { return this == right; }
	bool operator!=( const sgNodeDescriptor & right ) const { return this != & right; }
	bool operator!=( const sgNodeDescriptor * right ) const { return this != right; }
};

#ifdef __GLOBAL__
list<sgNodeDescriptor*> sgNodeDescriptor::nodes;
#endif

