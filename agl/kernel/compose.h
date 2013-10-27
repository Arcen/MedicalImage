#ifdef __GLOBAL__

////////////////////////////////////////////////////////////////////////////////
// ノード記述子初期化

//メンバーの先頭からのアドレスを得るマクロ
//#define sgInstance(member) reinterpret_cast<unsigned int>(static_cast<sgMemberInterface*>(&(static_cast<thisClass *>(reinterpret_cast<sgNode *>(0))->member)))
#define sgInstance(member) (offsetof(thisClass,member))

//sgNodeDescriptorInstance::initializerでノードの記述情報を構築する時のためのマクロ群
//自分自身の属性宣言
#define sgDeclareThis()	thisClass::attributes->setAttribute( declareThis() );
//メンバーの宣言、initial：初期情報、retain：保存必要、Target：値は参照のみ、Node：メンバーはノード
#define sgDeclare(member,initial)	thisClass::attributes->member.setAttribute( declareMember( #member, sgInstance( member ), thisClass::attributes->member.getInitialForAttribute(), false, false ) );
#define sgDeclareRetain(member)	thisClass::attributes->member.setAttribute( declareMember( #member, sgInstance( member ), thisClass::attributes->member.getInitialForAttribute(), true, false ) );
#define sgDeclareInitial(member,initial)	thisClass::attributes->member.setAttribute( declareMember( #member, sgInstance( member ), thisClass::attributes->member.setInitialForAttribute( initial ), false, false ) );
#define sgDeclareInitialRetain(member,initial)	thisClass::attributes->member.setAttribute( declareMember( #member, sgInstance( member ), thisClass::attributes->member.setInitialForAttribute( initial ), true, false ) );
#define sgDeclareInitialTarget(member,initial)	thisClass::attributes->member.setAttribute( declareMember( #member, sgInstance( member ), thisClass::attributes->member.setInitialForAttribute( initial ), false, true ) );
#define sgDeclareNode(member,type)	thisClass::attributes->member.setAttribute( declareNode( #member, * type::descriptor, sgInstance( member ), false ) );
#define sgDeclareNodeTarget(member,type)	thisClass::attributes->member.setAttribute( declareNode( #member, * type::descriptor, sgInstance( member ), true ) );
//構築時の宣言、静的変数の実体宣言も隠れて行う
#define sgNodeDeclareInstance(classname) \
	classname::sgNodeDescriptorInstance * classname::descriptor = NULL;\
	classname * classname::attributes = NULL;\
	sgAttribute * classname::thisAttribute = NULL;\
	void classname::sgNodeDescriptorInstance::initializer()

//__COMPOSE__宣言を行って各ノードを読み込んでおくことで、構築する
#define __COMPOSE__
#include "elements.h"
#undef __COMPOSE__

//無駄なマクロを消去
#undef sgInstance
#undef sgDeclareThis
#undef sgDeclare
#undef sgDeclareRetain
#undef sgDeclareInitial
#undef sgDeclareInitialRetain
#undef sgDeclareInitialTarget
#undef sgDeclareNode
#undef sgDeclareNodeTarget

//sgGraph::initialize/finalizeでノード記述子の作成と削除を行うときのマクロ
#define sgNodeInitialize(classname); new classname::sgNodeDescriptorInstance;
#define sgNodeFinalize(classname); delete classname::descriptor;

////////////////////////////////////////////////////////////////////////////////
// グラフ登録
//bool sgGraph::CoInitialized = false;//Mayaでのバグのため初期化を省いてみた。
void sgGraph::initialize()
{
//	CoInitialized = ( S_OK == CoInitialize( NULL ) );
	if ( sgGraph::attributeGraph ) return;
	sgGraph::attributeGraph = new sgGraph();

	sgTypePrimitiveInitialize( int );
	sgTypePrimitiveInitialize( decimal );
	sgTypeClassInitialize( vector2 );
	sgTypeClassInitialize( vector3 );
	sgTypeClassInitialize( vector4 );
	sgTypeClassInitialize( quaternion );
	sgTypeClassInitialize( matrix22 );
	sgTypeClassInitialize( matrix33 );
	sgTypeClassInitialize( matrix44 );
	sgTypeStringInitialize();
	
	sgNodeInitialize( sgTime );
	sgNodeInitialize( sgIntConstantInterpolater );
	sgNodeInitialize( sgDecimalConstantInterpolater );
	sgNodeInitialize( sgVector2ConstantInterpolater );
	sgNodeInitialize( sgVector3ConstantInterpolater );
	sgNodeInitialize( sgVector4ConstantInterpolater );
	sgNodeInitialize( sgQuaternionConstantInterpolater );
	sgNodeInitialize( sgIntLinearInterpolater );
	sgNodeInitialize( sgDecimalLinearInterpolater );
	sgNodeInitialize( sgVector2LinearInterpolater );
	sgNodeInitialize( sgVector3LinearInterpolater );
	sgNodeInitialize( sgVector4LinearInterpolater );
	sgNodeInitialize( sgQuaternionLinearInterpolater );
	sgNodeInitialize( sgOrtho );
	sgNodeInitialize( sgPerspective );
	sgNodeInitialize( sgFrustum );
	sgNodeInitialize( sgCamera );
	sgNodeInitialize( sgAmbientLight );
	sgNodeInitialize( sgPositionLight );
	sgNodeInitialize( sgDirectionLight );
	sgNodeInitialize( sgSpotLight );
	sgNodeInitialize( sgTexture );
	sgNodeInitialize( sgMaterial );
	sgNodeInitialize( sgVertices );
	sgNodeInitialize( sgIndices );
	sgNodeInitialize( sgPolygon );
	sgNodeInitialize( sgBone );
	sgNodeInitialize( sgEnvelope );
	sgNodeInitialize( sgTransformMatrix );
	sgNodeInitialize( sg3DSMAXTransformMatrix );
	sgNodeInitialize( sgMayaTransformMatrix );
	sgNodeInitialize( sgMayaJointMatrix );
	sgNodeInitialize( sgMayaIkHandle );
}

void sgGraph::finalize()
{
	if ( ! sgGraph::attributeGraph ) return;
	sgNodeFinalize( sgMayaIkHandle );
	sgNodeFinalize( sgMayaJointMatrix );
	sgNodeFinalize( sgMayaTransformMatrix );
	sgNodeFinalize( sg3DSMAXTransformMatrix );
	sgNodeFinalize( sgTransformMatrix );
	sgNodeFinalize( sgEnvelope );
	sgNodeFinalize( sgBone );
	sgNodeFinalize( sgPolygon );
	sgNodeFinalize( sgIndices );
	sgNodeFinalize( sgVertices );
	sgNodeFinalize( sgMaterial );
	sgNodeFinalize( sgTexture );
	sgNodeFinalize( sgSpotLight );
	sgNodeFinalize( sgDirectionLight );
	sgNodeFinalize( sgPositionLight );
	sgNodeFinalize( sgAmbientLight );
	sgNodeFinalize( sgCamera );
	sgNodeFinalize( sgPerspective );
	sgNodeFinalize( sgFrustum );
	sgNodeFinalize( sgOrtho );
	sgNodeFinalize( sgQuaternionLinearInterpolater );
	sgNodeFinalize( sgVector4LinearInterpolater );
	sgNodeFinalize( sgVector3LinearInterpolater );
	sgNodeFinalize( sgVector2LinearInterpolater );
	sgNodeFinalize( sgDecimalLinearInterpolater );
	sgNodeFinalize( sgIntLinearInterpolater );
	sgNodeFinalize( sgQuaternionConstantInterpolater );
	sgNodeFinalize( sgVector4ConstantInterpolater );
	sgNodeFinalize( sgVector3ConstantInterpolater );
	sgNodeFinalize( sgVector2ConstantInterpolater );
	sgNodeFinalize( sgDecimalConstantInterpolater );
	sgNodeFinalize( sgIntConstantInterpolater );
	sgNodeFinalize( sgTime );

	sgTypePrimitiveFinalize( int );
	sgTypePrimitiveFinalize( decimal );
	sgTypeClassFinalize( vector2 );
	sgTypeClassFinalize( vector3 );
	sgTypeClassFinalize( vector4 );
	sgTypeClassFinalize( quaternion );
	sgTypeClassFinalize( matrix22 );
	sgTypeClassFinalize( matrix33 );
	sgTypeClassFinalize( matrix44 );
	sgTypeStringFinalize();

	delete sgGraph::attributeGraph;
	sgGraph::attributeGraph = NULL;
//	if ( CoInitialized ) CoUninitialize();
}

#endif
