////////////////////////////////////////////////////////////////////////////////
// ノード属性情報

class sgNode;
class sgGraph;

//ノードの記述子（この中にノードのメンバー等の属性を入れる）
class sgNodeDescriptor
{
public:
	static list<sgNodeDescriptor*> nodes;//記述子のリスト
	//名前からの検索
	static sgNodeDescriptor * search( const char * name )
	{
		for ( list<sgNodeDescriptor*>::iterator it( nodes ); it; ++it ) {
			if ( it()->name == name ) {
				return it();
			}
		}
		return NULL;
	}
	//属性のリスト
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
	//属性の取得
	sgAttribute * getAttribute( const char * name ) 
	{
		for ( array< retainer<sgAttribute> >::iterator it( attributes ); it; ++it ) {
			if ( it()->name == name ) {
				return & it()();
			}
		}
		return NULL;
	}
	//ノードのメンバーの初期化
	void initialize( sgNode * node );
	//記述子の構築用の純粋仮想関数
	virtual void initializer() = 0;
	//ノードの生成子用の純粋仮想関数
	virtual sgNode * creator( sgGraph & graph ) = 0;

	//ノードの型を宣言する
	sgAttribute & declareThis()
	{
		return ( attributes.push_back( new sgAttribute( attributes.size, *this ) ) )();
	}
	//メンバーの型を宣言する
	template<class T>
	sgAttribute & declareMember( const char * name, unsigned int instance, T & initial, bool retain, bool target )
	{
		return ( attributes.push_back( new sgAttribute( attributes.size, name, getTypeDescriptor( ( const T * ) NULL ), instance, & initial, retain, target ) ) )();
	}
	sgAttribute & declareNode( const char * name, sgNodeDescriptor & node, unsigned int instance, bool target )
	{
		return ( attributes.push_back( new sgAttribute( attributes.size, name, node, instance, target ) ) )();
	}
	//比較
	bool operator==( const sgNodeDescriptor & right ) const { return this == & right; }
	bool operator==( const sgNodeDescriptor * right ) const { return this == right; }
	bool operator!=( const sgNodeDescriptor & right ) const { return this != & right; }
	bool operator!=( const sgNodeDescriptor * right ) const { return this != right; }
};

#ifdef __GLOBAL__
list<sgNodeDescriptor*> sgNodeDescriptor::nodes;
#endif

