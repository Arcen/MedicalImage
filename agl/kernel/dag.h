////////////////////////////////////////////////////////////////////////////////
// DAG情報

//シーンのツリー型のノード。ノードのポインタ情報のみをもち、ツリー構造を構築する
class sgDagNode
{
public:
	sgNode * node;
	list< retainer<sgDagNode> > children;//子供
	//dagのroot用のコンストラクタ
	sgDagNode() : node( NULL )
	{
	}
	//子供用のコンストラクタ
	sgDagNode( sgNode & _node ) : node( & _node )
	{
	}
	virtual ~sgDagNode()
	{
	}
	//追加：この関数で子供としてdagを作成
	sgDagNode * insert( sgNode & node )
	{
		return & children.push_back( new sgDagNode( node ) )();
	}
	//指定のノードを子供から削除
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
