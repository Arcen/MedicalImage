////////////////////////////////////////////////////////////////////////////////
// 型

//nodeのメンバーの型の情報を保持する記述子のインタフェース
class sgTypeDescriptor
{
public:
	static list<sgTypeDescriptor*> types;//全ての型のリスト
	//型を名前から検索する
	static sgTypeDescriptor * search( const char * name ) 
	{
		for ( list<sgTypeDescriptor*>::iterator it( types ); it; ++it ) {
			if ( it()->name == name ) {
				return it();
			}
		}
		return NULL;
	}
	string name;//型の名前
	//コンストラクタ
	sgTypeDescriptor( const char * _name ) : name( _name )
	{
		types.push_back( this );
	}
	//デストラクタ
	virtual ~sgTypeDescriptor()
	{
		types.pop( this );
	}
	//型の複製を行う純粋仮想関数
	virtual bool copyer( void * dst, const void * src ) const = 0;
	//型の比較を行う純粋仮想関数
	virtual bool compair( const void * left, const void * right ) const = 0;
	//型の初期化を確認する仮想関数
	virtual bool initialized( const void * src ) const { return false; }
	//型の初期化を行う仮想関数
	virtual bool initializer( void * src ) const { return false; }
	//型の比較
	bool operator==( const sgTypeDescriptor & right ) const { return this == & right; }
	bool operator==( const sgTypeDescriptor * right ) const { return this == right; }
	bool operator!=( const sgTypeDescriptor & right ) const { return this != & right; }
	bool operator!=( const sgTypeDescriptor * right ) const { return this != right; }
};

#ifdef __GLOBAL__
list<sgTypeDescriptor*> sgTypeDescriptor::types;
#endif

template<class T> const sgTypeDescriptor & getTypeDescriptor( const T * );

class sgAttribute;

//ノードのメンバーとなる型のインタフェース
class sgMemberInterface
{
public:
	virtual void * getInstance() { return NULL; }
	virtual void insertReference( void * src ) = 0;
	virtual void killReference( void * src ) = 0;
	virtual void setInitial( const void * src ){};
	virtual sgAttribute & setAttribute( sgAttribute & src ) = 0;//属性情報を設定する
	virtual sgAttribute & getAttribute() = 0;//属性情報を取得する
	operator sgAttribute & () { return getAttribute(); }//属性情報を取得する
};

//ノードのメンバとなる変数の型
template<class T>
class sgPrimitiveMember : public sgMemberInterface
{
	T instance, * reference;
	array<T*> references;
public:
	typedef typename array<T*>::iterator iterator;
	sgPrimitiveMember() : reference( & instance )
	{
	}
	void operator()( const T & src ) { *reference = src; }
	T & operator()(){ return *reference; }
	const T & operator()() const { return *reference; }
	int count() { return references.size ? references.size : 1; }
	operator sgAttribute & () { return getAttribute(); }

	virtual void * getInstance() { return reinterpret_cast<void *>( & instance ); }
	virtual void insertReference( void * src )
	{
		references.push_back( reinterpret_cast<T*>( src ) );
		reference = references.last();
	}
	virtual void killReference( void * src )
	{
		references.pop( reinterpret_cast<T*>( src ) );
		if ( ! references.size ) {
			reference = & instance;
		} else {
			reference = references.last();
		}
	}
	virtual void setInitial( const void * src ){ setInitialForAttribute( *reinterpret_cast<const T*>( src ) ); };

	//記述子のための属性のポインタをreferenceに保存
	virtual sgAttribute & setAttribute( sgAttribute & src ){ reference = reinterpret_cast<T*>( & src ); return getAttribute(); }
	virtual sgAttribute & getAttribute(){ return * reinterpret_cast<sgAttribute*>( reference ); }
	//記述子のための初期情報をinstanceに保存
	T & setInitialForAttribute( const T & src ){ instance = src; return getInitialForAttribute(); }
	T & getInitialForAttribute(){ return instance; }
};

//ノードのメンバとなる他のノードへのポインタ配列の型
template<class T>
class sgNodeMember : public sgMemberInterface
{
public:
	array<T*> reference;
	typedef typename array<T*>::iterator iterator;
	void operator()( const T & src ) { reference.push_back( & src ); }
	iterator operator()() { return iterator( reference ); }
	const iterator operator()() const { return iterator( reference ); }
	int count() { return reference.size; }
	operator sgAttribute & () { return getAttribute(); }

	virtual void insertReference( void * src ){ reference.push_back( reinterpret_cast<T*>( src ) ); }
	virtual void killReference( void * src ){ reference.pop( reinterpret_cast<T*>( src ) ); }

	//記述子のための属性のポインタをreferenceに保存
	virtual sgAttribute & setAttribute( sgAttribute & src ){ if ( ! reference.size ) { reference.push_back( NULL ); } reference.first() = reinterpret_cast<T*>( & src ); return getAttribute(); }
	virtual sgAttribute & getAttribute(){ if ( ! reference.size ) { reference.push_back( NULL ); } return * reinterpret_cast<sgAttribute*>( reference.first() ); }
};

typedef sgPrimitiveMember< int >			sgInt;
typedef sgPrimitiveMember< decimal >		sgDecimal;
typedef sgPrimitiveMember< vector2 >		sgVector2;
typedef sgPrimitiveMember< vector3 >		sgVector3;
typedef sgPrimitiveMember< vector4 >		sgVector4;
typedef sgPrimitiveMember< quaternion >		sgQuaternion;
typedef sgPrimitiveMember< matrix22 >		sgMatrix22;
typedef sgPrimitiveMember< matrix33 >		sgMatrix33;
typedef sgPrimitiveMember< matrix44 >		sgMatrix44;
typedef sgPrimitiveMember< string >			sgString;

typedef sgPrimitiveMember< array< int > >			sgArrayInt;
typedef sgPrimitiveMember< array< decimal > >		sgArrayDecimal;
typedef sgPrimitiveMember< array< vector2 > >		sgArrayVector2;
typedef sgPrimitiveMember< array< vector3 > >		sgArrayVector3;
typedef sgPrimitiveMember< array< vector4 > >		sgArrayVector4;
typedef sgPrimitiveMember< array< quaternion > >	sgArrayQuaternion;
typedef sgPrimitiveMember< array< matrix22 > >		sgArrayMatrix22;
typedef sgPrimitiveMember< array< matrix33 > >		sgArrayMatrix33;
typedef sgPrimitiveMember< array< matrix44 > >		sgArrayMatrix44;
typedef sgPrimitiveMember< array< string > >		sgArrayString;

typedef sgPrimitiveMember< array< array<  int > > >			sgArrayArrayInt;
typedef sgPrimitiveMember< array< array<  decimal > > >		sgArrayArrayDecimal;
typedef sgPrimitiveMember< array< array<  vector2 > > >		sgArrayArrayVector2;
typedef sgPrimitiveMember< array< array<  vector3 > > >		sgArrayArrayVector3;
typedef sgPrimitiveMember< array< array<  vector4 > > >		sgArrayArrayVector4;
typedef sgPrimitiveMember< array< array<  quaternion > > >	sgArrayArrayQuaternion;
typedef sgPrimitiveMember< array< array<  matrix22 > > >	sgArrayArrayMatrix22;
typedef sgPrimitiveMember< array< array<  matrix33 > > >	sgArrayArrayMatrix33;
typedef sgPrimitiveMember< array< array<  matrix44 > > >	sgArrayArrayMatrix44;
typedef sgPrimitiveMember< array< array<  string > > >		sgArrayArrayString;

#ifdef __GLOBAL__

////////////////////////////////////////////////////////////////////////////////
// タイプ記述子
//型別に初期化などの操作を自動的に割り振るためのクラス群

//プリミティブタイプ記述子
template<class T>
class sgTypePrimitive : public sgTypeDescriptor
{
public:
	typedef T type;
	sgTypePrimitive( const char * _name ) : sgTypeDescriptor( _name ){}
	virtual bool initialized( const void * src ) const
	{
		return *reinterpret_cast<const type*>( src ) == type( 0 );
	}
	virtual bool initializer( void * dst ) const
	{
		*reinterpret_cast<type*>( dst ) = type( 0 );
		return true;
	}
};

//文字列タイプ記述子
class sgTypeString : public sgTypeDescriptor
{
public:
	typedef string type;
	sgTypeString( const char * _name ) : sgTypeDescriptor( _name ){}
	virtual bool initialized( const void * src ) const
	{
		return *reinterpret_cast<const type*>( src ) == "";
	}
	virtual bool initializer( void * dst ) const
	{
		*reinterpret_cast<type*>( dst ) = "";
		return true;
	}
};

//クラスタイプ記述子
template<class T>
class sgTypeClass : public sgTypeDescriptor
{
public:
	typedef T type;
	sgTypeClass( const char * _name ) : sgTypeDescriptor( _name ){}
	virtual bool initialized( const void * src ) const
	{
		return *reinterpret_cast<const type*>( src ) == type::identity;
	}
	virtual bool initializer( void * dst ) const
	{
		*reinterpret_cast<type*>( dst ) = type::identity;
		return true;
	}
};

//配列型記述子
template<class T>
class sgTypeArray : public sgTypeDescriptor
{
public:
	typedef array<T> type;
	sgTypeArray( const char * _name ) : sgTypeDescriptor( _name ){}
	virtual bool initialized( const void * src ) const
	{
		return reinterpret_cast<const type*>( src )->size == 0;
	}
	virtual bool initializer( void * src ) const
	{
		reinterpret_cast<type*>( src )->allocate( 0 );
		return true;
	}
};

//２重配列型記述子
template<class T>
class sgTypeArrayArray : public sgTypeDescriptor
{
public:
	typedef array< array<T> > type;
	sgTypeArrayArray( const char * _name ) : sgTypeDescriptor( _name ){}
	virtual bool initialized( const void * src ) const
	{
		return reinterpret_cast<const type*>( src )->size == 0;
	}
	virtual bool initializer( void * src ) const
	{
		reinterpret_cast<type*>( src )->allocate( 0 );
		return true;
	}
};

//記述子作成者
template<class T>
class sgTypeConstructor : public T
{
public:
	static sgTypeConstructor * typeDescriptor;
	virtual bool compair( const void * left, const void * right ) const	
	{
		return *reinterpret_cast<const type*>( left ) == *reinterpret_cast<const type*>( right );
	}
	virtual bool copyer( void * dst, const void * src ) const 
	{
		*reinterpret_cast<type*>( dst ) = *reinterpret_cast<const type*>( src );
		return true;
	}
	sgTypeConstructor( const char * _name ) : T( _name )
	{
		assert( ! typeDescriptor );
		typeDescriptor = this;
	}
	~sgTypeConstructor()
	{
		assert( typeDescriptor );
		typeDescriptor = NULL;
	}
};

//型の宣言用のマクロ。実体保持用の静的変数の宣言。テンプレート関数の特殊化により、型のポインタから型の記述子を得ることが出来る
#define sgTypePrimitiveDeclare(primitive);	\
	template<> const sgTypeDescriptor & getTypeDescriptor( const primitive * ) { return * sgTypeConstructor<sgTypePrimitive<primitive> >::typeDescriptor; } \
	template<> const sgTypeDescriptor & getTypeDescriptor( const array< primitive > * ) { return * sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor; } \
	template<> const sgTypeDescriptor & getTypeDescriptor( const array< array< primitive > > * ) { return * sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor; } \
	sgTypeConstructor< sgTypePrimitive<primitive> > * sgTypeConstructor< sgTypePrimitive<primitive> >::typeDescriptor = NULL;\
	sgTypeConstructor< sgTypeArray<primitive> > * sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor = NULL; \
	sgTypeConstructor< sgTypeArrayArray<primitive> > * sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor = NULL;

#define sgTypeStringDeclare();	\
	template<> const sgTypeDescriptor & getTypeDescriptor( const string * ) { return * sgTypeConstructor<sgTypeString>::typeDescriptor; } \
	template<> const sgTypeDescriptor & getTypeDescriptor( const array< string > * ) { return * sgTypeConstructor< sgTypeArray<string> >::typeDescriptor; } \
	template<> const sgTypeDescriptor & getTypeDescriptor( const array< array< string > > * ) { return * sgTypeConstructor< sgTypeArrayArray<string> >::typeDescriptor; } \
	sgTypeConstructor< sgTypeString > * sgTypeConstructor< sgTypeString >::typeDescriptor = NULL;\
	sgTypeConstructor< sgTypeArray<string> > * sgTypeConstructor< sgTypeArray<string> >::typeDescriptor = NULL; \
	sgTypeConstructor< sgTypeArrayArray<string> > * sgTypeConstructor< sgTypeArrayArray<string> >::typeDescriptor = NULL;

#define sgTypeClassDeclare(primitive);	\
	template<> const sgTypeDescriptor & getTypeDescriptor( const primitive * ) { return * sgTypeConstructor<sgTypeClass<primitive> >::typeDescriptor; } \
	template<> const sgTypeDescriptor & getTypeDescriptor( const array< primitive > * ) { return * sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor; } \
	template<> const sgTypeDescriptor & getTypeDescriptor( const array< array< primitive > > * ) { return * sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor; } \
	sgTypeConstructor< sgTypeClass<primitive> > * sgTypeConstructor< sgTypeClass<primitive> >::typeDescriptor = NULL;\
	sgTypeConstructor< sgTypeArray<primitive> > * sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor = NULL; \
	sgTypeConstructor< sgTypeArrayArray<primitive> > * sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor = NULL;

//型の記述子の宣言
sgTypePrimitiveDeclare( int );
sgTypePrimitiveDeclare( decimal );
sgTypeClassDeclare( vector2 );
sgTypeClassDeclare( vector3 );
sgTypeClassDeclare( vector4 );
sgTypeClassDeclare( quaternion );
sgTypeClassDeclare( matrix22 );
sgTypeClassDeclare( matrix33 );
sgTypeClassDeclare( matrix44 );
sgTypeStringDeclare();

//マクロの後片付け
#undef sgTypePrimitiveDeclare
#undef sgTypeClassDeclare
#undef sgTypeStringDeclare

//型の記述子の初期化と終了化のマクロ
#define sgTypePrimitiveInitialize(primitive);	\
	new sgTypeConstructor< sgTypePrimitive<primitive> >( #primitive ); \
	new sgTypeConstructor< sgTypeArray<primitive> >( #primitive "[]" ); \
	new sgTypeConstructor< sgTypeArrayArray<primitive> >( #primitive "[][]" );

#define sgTypePrimitiveFinalize(primitive);	\
	delete sgTypeConstructor< sgTypePrimitive<primitive> >::typeDescriptor; \
	delete sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor; \
	delete sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor; \
	sgTypeConstructor< sgTypePrimitive<primitive> >::typeDescriptor = NULL;\
	sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor = NULL; \
	sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor = NULL;

#define sgTypeStringInitialize();	\
	new sgTypeConstructor< sgTypeString >( "string" ); \
	new sgTypeConstructor< sgTypeArray<string> >( "string" "[]" ); \
	new sgTypeConstructor< sgTypeArrayArray<string> >( "string" "[][]" );

#define sgTypeStringFinalize();	\
	delete sgTypeConstructor< sgTypeString >::typeDescriptor; \
	delete sgTypeConstructor< sgTypeArray<string> >::typeDescriptor; \
	delete sgTypeConstructor< sgTypeArrayArray<string> >::typeDescriptor; \
	sgTypeConstructor< sgTypeString >::typeDescriptor = NULL;\
	sgTypeConstructor< sgTypeArray<string> >::typeDescriptor = NULL; \
	sgTypeConstructor< sgTypeArrayArray<string> >::typeDescriptor = NULL;

#define sgTypeClassInitialize(primitive);	\
	new sgTypeConstructor< sgTypeClass<primitive> >( #primitive ); \
	new sgTypeConstructor< sgTypeArray<primitive> >( #primitive "[]" ); \
	new sgTypeConstructor< sgTypeArrayArray<primitive> >( #primitive "[][]" );

#define sgTypeClassFinalize(primitive);	\
	delete sgTypeConstructor< sgTypeClass<primitive> >::typeDescriptor; \
	delete sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor; \
	delete sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor; \
	sgTypeConstructor< sgTypeClass<primitive> >::typeDescriptor = NULL;\
	sgTypeConstructor< sgTypeArray<primitive> >::typeDescriptor = NULL; \
	sgTypeConstructor< sgTypeArrayArray<primitive> >::typeDescriptor = NULL;


#endif
