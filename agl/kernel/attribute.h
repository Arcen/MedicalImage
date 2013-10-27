////////////////////////////////////////////////////////////////////////////////
// 型の属性

//ノードの記述子（この中にノードのメンバー等の属性を入れる）
class sgNodeDescriptor;

//属性のクラス
class sgAttribute
{
	//コピーコンストラクタの禁止用
	sgAttribute();
	sgAttribute( const sgAttribute & value );
	sgAttribute & operator=( const sgAttribute & value );
public:
	int ID;//各sgNodeDescriptor内でのattributes配列のインデックス
	string name;//属性名
	const sgTypeDescriptor * type;//データの型
	const sgNodeDescriptor * node;//ノードの型
	unsigned int instance;//実体のクラスの先頭からのオフセット
	const void * initial;//データの初期データ
	bool retain;//ファイルからの入出力を行うかどうか
	bool target;// false : ソースのインスタンスをデスティネーションのポインタに代入, true : デスティネーションのインスタンスをソースのポインタに代入
	//thisへの参照
	sgAttribute( int _id, const sgNodeDescriptor & _nd )
		: ID( _id ), name( "this" ), type( NULL ), node( & _nd ), instance( 0 ), initial( NULL ), retain( false ), target( false ) {}
	//実体への参照
	sgAttribute( int _id, const char * _name, const sgTypeDescriptor & _td, unsigned int _instance, const void * _initial, bool _retain, bool _target )
		: ID( _id ), name( _name ), type( & _td ), node( NULL ), instance( _instance ), initial( _initial ), retain( _retain ), target( _target ) {}
	//配列でのノードへの参照
	sgAttribute( int _id, const char * _name, const sgNodeDescriptor & _nd, unsigned int _instance, bool _target )
		: ID( _id ), name( _name ), type( NULL ), node( & _nd ), instance( _instance ), initial( NULL ), retain( false ), target( _target ) {}
	//メンバーの取得
	sgMemberInterface * getMember( void * base ) const { return instance ? reinterpret_cast<sgMemberInterface *>( reinterpret_cast<unsigned int>( base ) + instance ) : NULL; }
	//メンバーの実体の取得
	void * getInstance( void * base )
	{
		sgMemberInterface * mi = getMember( base );
		return mi ? mi->getInstance() : base;
	}
	//メンバーの参照の追加
	void insertReference( void * base, void * src )
	{
		sgMemberInterface * mi = getMember( base );
		if ( mi ) mi->insertReference( src );
	}
	//メンバーの参照の削除
	void killReference( void * base, void * src )
	{
		sgMemberInterface * mi = getMember( base );
		if ( mi ) mi->killReference( src );
	}
	//メンバーの実態の初期化
	void setInitial( void * base, const void * src )
	{
		sgMemberInterface * mi = getMember( base );
		if ( mi ) mi->setInitial( src );
	}
	//比較
	bool operator==( const sgAttribute & src ) const {
		return ( ID == src.ID && name == src.name && type == src.type && 
			node == src.node && instance == src.instance );
	}
	bool operator!=( const sgAttribute & src ) const {
		return ( ID != src.ID || name != src.name || type != src.type || 
			node != src.node || instance != src.instance );
	}
};
