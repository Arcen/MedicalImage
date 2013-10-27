////////////////////////////////////////////////////////////////////////////////
// グラフ
class sgGraph
{
//	static bool CoInitialized;
public:
	string name;//グラフの名前
	int maxID;//ノードのID
	list< retainer<sgNode> > nodes;//ノードのリスト（削除を保証する）
	friend class sgNode;
	static sgGraph *attributeGraph;//グラフの記述子（実体はノード）を保持するためのグラフ
	sgDagNode root;//DAGのルートノード（空）
	sgGraph() : maxID( 0 )
	{
	}
	//ノードの挿入
	void insert( sgNode * node ) { nodes.push_back( node ); }
	//ノードの取り外し（削除は行わない）
	void remove( sgNode * node ) 
	{
		retainer<sgNode> wk( node );//検索用に型をあわせる
		list< retainer<sgNode> >::iterator it = nodes.search( wk );
		wk.loot();//検索用なので、削除されないように取り外す。
		if ( it ) {//見つかっていた場合
			it().loot();//メモリを削除されないように取り外して
			nodes.pop( it );//リストから削除する
		}
	}
	//列挙　記述子を引数に与えた場合にはその種類のノードのみを列挙する
	void enumrate( list<sgNode*> & dst, sgNodeDescriptor * descriptor = NULL )
	{
		dst.release();
		for ( list< retainer<sgNode> >::iterator it( nodes ); it; ++it ) {
			if ( descriptor && it()->thisDescriptor != descriptor ) continue;
			dst.push_back( & it()() );
		}
	}
	//名前による検索
	sgNode * search( const char * name ) 
	{
		for ( list< retainer<sgNode> >::iterator it( nodes ); it; ++it ) {
			if ( it()->name == name ) {
				return & it()();
			}
		}
		return NULL;
	}
	//入力元から順にnodesを並べる（updateするには並べなおさないといけない）
	//入力が循環している場合には、無限ループするので、グラフを作成するときは気をつける。
	void alignment()
	{
		list< retainer<sgNode> > temp;
		//tempにメモリを壊さないように、移動する
		while ( nodes.size ) temp.push_back( nodes.loot_front() );
		while ( temp.size ) {
			sgNode * target = temp.loot_front().loot();
			bool found = false;
			//対象の入力元を列挙
			for ( array< list<sgConnection*> >::iterator itcs( target->inputs ); itcs; ++itcs ) {
				for ( list<sgConnection*>::iterator itc( itcs() ); itc; ++itc ) {
					//入力元
					sgNode & src = itc()->src;
					//入力元が未だリストに残っているかどうかを調べる
					for ( list< retainer<sgNode> >::iterator itn( temp ); itn; ++itn ) {
						if ( itn() == & src ) {
							temp.push_back( target );//対象を後ろに回す。
							found = true; break;
						}
					}
					if ( found ) break;
				}
				if ( found ) break;
			}
			if ( found ) continue;//見つかった場合は先頭にあるものから見直す
			nodes.push_back( target );
		}
	}
	//更新する（alignmentされてる必要がある）
	bool update( bool smart = false )
	{
		if ( smart ) {
			//sgTimeなど、更新したノードはdirtyをtrueにする必要がある。
			//ダーティフラグがなければ、更新しない。また、更新されたら出力先に、更新するように、ダーティにする。
			bool change = false;
			for ( list< retainer<sgNode> >::iterator itn( nodes ); itn; ++itn ) {
				sgNode & node = itn()();
				if ( node.dirty == false ) continue;
				node.update();//更新
				node.dirty = false;//更新済み
				change = true;//何かしらのノードが更新された
				for ( array< list<sgConnection*> >::iterator itcs( node.outputs ); itcs; ++itcs ) {
					for ( list<sgConnection*>::iterator itc( itcs() ); itc; ++itc ) {
						itc()->dst.dirty = true;//出力先を更新する
					}
				}
			}
			return change;
		} else {
			//全てを更新
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

//ノードのコンストラクタ
inline sgNode::sgNode( sgGraph & _graph, sgNodeDescriptor & _thisDescriptor, void * _base ) : 
	graph( _graph ), thisDescriptor( _thisDescriptor ), base( _base ), ID( _graph.maxID++ ), external( NULL ), dirty( true )
{
	graph.insert( this );//グラフのリストに挿入
	//接続情報をメンバ数＋自分の分だけ確保する
	outputs.allocate( thisDescriptor.attributes.size );
	inputs.allocate( thisDescriptor.attributes.size );
	name.print( "%p", this );//メモリのアドレスからとりあえず一意な名前をつける
}

//ノードのデストラクタ
inline sgNode::~sgNode()
{
	//接続情報があれば、削除する
	for ( array< list<sgConnection*> >::iterator oit( outputs ); oit; ++oit ) {
		list<sgConnection*> & connections = oit();
		while ( connections.size ) delete connections.first();
	}
	for ( array< list<sgConnection*> >::iterator iit( inputs ); iit; ++iit ) {
		list<sgConnection*> & connections = iit();
		while ( connections.size ) delete connections.first();
	}
	//グラフから取り除く
	graph.remove( this );
}

//ノードの記述子によるメンバーの初期化
inline void sgNodeDescriptor::initialize( sgNode * nd )
{
	if ( sgGraph::attributeGraph == & nd->graph ) return;//記述子自体の場合には初期化しない
	for ( array< retainer<sgAttribute> >::iterator it( attributes ); it; ++it ) {
		sgAttribute & attrbt = it()();
		if ( ! attrbt.instance ) continue;//this property
		//初期設定
		//インスタンスにデフォルトを設定
		if ( attrbt.initial ) attrbt.type->copyer( nd->getInstance( attrbt ), attrbt.initial );
	}
}

#ifdef __GLOBAL__
sgGraph *sgGraph::attributeGraph = NULL;
#endif
