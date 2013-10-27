////////////////////////////////////////////////////////////////////////////////
// グラフノードのベース
class sgNode
{
public:
	sgGraph & graph;//所属するグラフ
	sgNodeDescriptor & thisDescriptor;//ノード種別用記述子（static sgNodeDescriptorInstance * descriptorへの参照）
	void * base;//実体のアドレス
	int ID;//シーングラフ内で一意なID
	void * external;//ノードの外部データ用のポインタ
	bool dirty;//smartなupdateを行うための、このノードがアップデートが必要かどうか
	string name;//名前
	array< list<sgConnection*> > inputs;//このノードへの入力
	array< list<sgConnection*> > outputs;//このノードからの出力

	////////////////////////////////////////////////////////////////////////////////
	//コンストラクタ／デストラクタ
	sgNode( sgGraph & _graph, sgNodeDescriptor & _descriptor, void * _base );//graph.h
	virtual ~sgNode();//graph.h
	////////////////////////////////////////////////////////////////////////////////
	//ノードのアップデート
	virtual void update(){}
	////////////////////////////////////////////////////////////////////////////////
	//sgAttributeへのproxy
	void * getInstance( sgAttribute & attrbt ) { return attrbt.getInstance( base ); }
	void insertReference( sgAttribute & attrbt, void * src ) { attrbt.insertReference( base, src ); }
	void killReference( sgAttribute & attrbt, void * src ) { attrbt.killReference( base, src ); }
	void setInitial( sgAttribute & attrbt, const void * src ) { attrbt.setInitial( base, src ); }
	////////////////////////////////////////////////////////////////////////////////
	//入出力取得
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
	//ノードの複製
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
	// メンバーが初期化済みかどうかを調べる
	bool initialized( sgAttribute & attrbt )
	{
		if ( ! attrbt.type ) return true;
		if ( attrbt.initial ) return attrbt.type->compair( getInstance( attrbt ), attrbt.initial );
		return attrbt.type->initialized( getInstance( attrbt ) );
	}
	////////////////////////////////////////////////////////////////////////////////
	// メンバーを初期化する
	void initializeAttribute()
	{
		for ( array< retainer<sgAttribute> >::iterator it( thisDescriptor.attributes ); it; ++it ) {
			sgAttribute & attrbt = it()();
			if ( ! attrbt.type ) continue;
			//初期設定
			if ( attrbt.initial ) {
				setInitial( attrbt, attrbt.initial );
			} else {
				attrbt.type->initializer( getInstance( attrbt ) );
			}
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
// sgNodeの接続情報
inline sgConnection::sgConnection( sgNode & _src, sgAttribute & _output, sgNode & _dst, sgAttribute & _input ) 
	: src( _src ), output( _output ), dst( _dst ), input( _input )
{
	//型記述子とノード記述子は同じである必要がある。
	assert( input.type == output.type && input.node == output.node );
	dst.inputs[input.ID].push_back( this );
	src.outputs[output.ID].push_back( this );
	if ( output.target ) {
		//ターゲット先を変更する時には、対象のポインタをソース側で保持する
		src.insertReference( output, dst.getInstance( input ) );
	} else {
		//ソース側を対象側で参照を保存する
		dst.insertReference( input, src.getInstance( output ) );
	}
}

//つながりの削除
inline sgConnection::~sgConnection()
{
	dst.inputs[input.ID].pop( this );
	src.outputs[output.ID].pop( this );
}

////////////////////////////////////////////////////////////////////////////////
// ノードの派生オブジェクトのデフォルト
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
//sgNodeDescriptorInstance//ノードの記述情報を実際に保存するクラス
//sgNodeDescriptorInstance::initializer//属性情報を設定し、ノードの記述情報を構築する関数。
//descriptor//ノードの記述子を保持する
//attributes//ノードのメンバー用の属性情報を各メンバーの参照の位置に記録するためにノードと同じクラスを利用する
//thisAttribute//ノード自身の属性情報

