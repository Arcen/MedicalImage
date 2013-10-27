//インクルード
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

//MAYA関連
#include <maya/MFnPlugin.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MObjectArray.h>
#include <maya/MString.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MColorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MMatrix.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MGlobal.h>
#include <maya/MItDag.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MStringArray.h>
#include <maya/MDagPathArray.h>
#include <maya/MAnimControl.h>
#include <maya/MTime.h>
#include <maya/MPlug.h>
#include <maya/MAngle.h>
#include <maya/MFnTransform.h>
#include <maya/MAnimUtil.h>
#include <maya/MPlugArray.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MItGeometry.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnSet.h>
#include <maya/MFnWeightGeometryFilter.h>
#include <maya/MFnCamera.h>
#include <maya/MFnIkHandle.h>
#include <maya/MFnIkEffector.h>


//使用ライブラリ
#pragma comment( lib, "Foundation" )
#pragma comment( lib, "image" )
#pragma comment( lib, "libMDtAPI" )
#pragma comment( lib, "libMocap" )
#pragma comment( lib, "OpenMaya" )
#pragma comment( lib, "OpenMayaAnim" )
#pragma comment( lib, "OpenMayaFX" )
#pragma comment( lib, "OpenMayaRender" )
#pragma comment( lib, "OpenMayaUI" )

//IFF形式の読み込みツール
#include "iffreader.h"

#include "../kernel/agl.h"
using namespace agl;

//アニメーションが含まれるノードのインタフェースクラス
class animationNode
{
public:
	static sgTime * time;//時刻ノード
	virtual ~animationNode(){};
	virtual void reserve( int size ) = 0;//メモリを用意する
	virtual void set() = 0;
	virtual void finish() = 0;//処理の最後
};

//エンベロープの情報
class envelopeInfomation
{
public:
	string name;//名前
	MDagPath path;//位置
	sgPolygon * polygon;//ポリゴン情報
	matrix44 initialMatrix;//初期位置
	sgTransformMatrix *currentMatrix;//現在位置
	array<int> positionIndices;//頂点情報
	array<int> normalIndices;//法線情報
};

//インバースキネマティクスの情報
class ikInfomation
{
public:
	sgMayaIkHandle * ikhandle;//handle
	MDagPath path;//位置
};

//テクスチャ属性 place2d.inl
struct MayaTextureAttribute
{
	float repeatU, repeatV;//繰り返し
	float offsetU, offsetV;//オフセット
	float rotateUV;//回転

	float coverageU, coverageV;//クリップ
	float translateFrameU, translateFrameV;//オフセット
	float rotateFrame;//回転
	bool stagger;//市松模様？
	bool mirror;//鏡像
	bool wrapU, wrapV;//繰り返し
	matrix44 matrix;//回転行列
	MayaTextureAttribute()
	{
		initialize();
	}
	//初期化
	void initialize()
	{
		matrix = matrix44::identity;
		repeatU = repeatV = 1;
		offsetU = offsetU = 0;
		rotateUV = 0;
		coverageU = coverageV = 1;
		translateFrameU = translateFrameV = 0;
		rotateFrame = 0;
		stagger = mirror = false;
		wrapU = wrapV = true;
	}
	void initialize( MObject & mObj );
	void initializeByMaterial( MObject & material );
	void initializeByTexture( MObject & colorObject );
	void convert( vector2 & uv );
};

//名前のテーブル
struct nameTable
{
	string fullpath;//オブジェクトのフルパス
	string name;//オブジェクト名
	string unique;//一意な名前
};

//エクスポータの本体
class Maya : public MPxFileTranslator
{
public:
	list< retainer<animationNode> > animationNodes;//アニメーションが含まれるノード
	list< retainer<envelopeInfomation> > envelopes;//エンベロープが含まれるノード
	list< retainer<ikInfomation> > iks;//ikが含まれるノード
	static sgGraph * graph;
	sgMaterial * defaultMaterial;
	list<sgMaterial*> materials;
	list<sgTexture*> textures;
	list<sgBone*> bones;
	list< retainer<nameTable> > nameTables;
	Maya()
	{
	}
	virtual ~Maya()
	{
	}
	static void * creator(){ return new Maya(); };
	virtual bool haveReadMethod() const
	{
		return false;
	}
	virtual bool haveWriteMethod() const
	{
		return true;
	}
	virtual bool canBeOpened() const
	{
		return false;
	}
	virtual MString defaultExtension() const
	{
		return "agl";
	}
	virtual MPxFileTranslator::MFileKind identifyFile(const MFileObject& fileName, const char* buffer, short size) const
	{
		const char *name = fileName.name().asChar();
		int nameLength = strlen(name);
		return ( ( nameLength > 4 ) && ! stricmp( name + nameLength - 4, ".agl" ) ) ? kIsMyFileType : kNotMyFileType;
	}
	virtual MStatus writer(const MFileObject&, const MString&, FileAccessMode);

	static void statusError( const MStatus & status, const char * fmt, ... )
	{
		if ( status != MStatus::kSuccess ) {
			char wk[1024];
			va_list args;
			va_start( args, fmt );
			vsprintf( wk, fmt, args );
			va_end( args );
			throw ( const char * ) wk;
		}
	}
	static void getRelativeObjects( MObject & object, MObjectArray & objects )
	{
		objects.clear();

		MPlugArray connections;
		MFnDependencyNode fnDepencencyNode( object );
		fnDepencencyNode.getConnections( connections );
		for ( int i = 0; i < connections.length(); i++ ) {
			MPlug & plug = connections[i];
			if ( ! plug.isNull() ) {
				MFnAttribute fnAttribute( plug.attribute() );
				string attributeName = fnAttribute.name().asChar();
				MPlugArray inputs;
				plug.connectedTo( inputs, true, false );
				for ( int j = 0; j < inputs.length(); j++ ) {
					objects.append( inputs[j].node() );
				}
				MPlugArray outputs;
				plug.connectedTo( outputs, false, true );
				for ( int j = 0; j < outputs.length(); j++ ) {
					objects.append( outputs[j].node() );
				}
			}
		}
	}
	static void getAttributes( MObject & object, MObjectArray & objects )
	{
		objects.clear();

		MFnDependencyNode fnDepencencyNode( object );
		for ( int i = 0; i < fnDepencencyNode.attributeCount(); i++ ) {
			objects.append( fnDepencencyNode.attribute( i ) );
		}
	}
	static bool getAttribute( MObject & object, MPlug & plug, const char * attribute )
	{
		MFnDependencyNode fnDepencencyNode( object );
		for ( int i = 0; i < fnDepencencyNode.attributeCount(); i++ ) {
			MFnAttribute fnAttribute( fnDepencencyNode.attribute( i ) );
			string attributeName = fnAttribute.name().asChar();
			if ( attributeName == attribute ) {
				plug = fnDepencencyNode.findPlug( ( const char * ) attributeName );
				return true;
			}
		}
		return false;
	}
	static MObject getInputObject( MObject & object, const char * attribute )
	{
		MPlugArray connections;
		MFnDependencyNode fnDepencencyNode( object );
		fnDepencencyNode.getConnections( connections );
		for ( int i = 0; i < connections.length(); i++ ) {
			MPlug & plug = connections[i];
			if ( ! plug.isNull() ) {
				MFnAttribute fnAttribute( plug.attribute() );
				string attributeName = fnAttribute.name().asChar();
				if ( attributeName != attribute ) continue;
				MPlugArray inputs;
				plug.connectedTo( inputs, true, false );
				if ( inputs.length() ) return inputs[0].node();
			}
		}
		return MObject::kNullObj;
	}
	static MObject getOutputObject( MObject & object, const char * attribute )
	{
		MPlugArray connections;
		MFnDependencyNode fnDepencencyNode( object );
		fnDepencencyNode.getConnections( connections );
		for ( int i = 0; i < connections.length(); i++ ) {
			MPlug & plug = connections[i];
			if ( ! plug.isNull() ) {
				MFnAttribute fnAttribute( plug.attribute() );
				string attributeName = fnAttribute.name().asChar();
				if ( attributeName != attribute ) continue;
				MPlugArray outputs;
				plug.connectedTo( outputs, false, true );
				if ( outputs.length() ) return outputs[0].node();
			}
		}
		return MObject::kNullObj;
	}
	static MDagPath getDagPath( const MObject & mObj )
	{
		MDagPath dPath;
		MItDag dagIt;
		statusError( dagIt.reset( mObj ), "Fail getDagPath" );
		statusError( dagIt.getPath( dPath ), "Fail getDagPath" );
		return dPath;
	}
	static MObject getDGFirstItem( MObjectArray & inSrcObj, MFn::Type inFilter = MFn::kInvalid, 
						   MItDependencyGraph::Direction inDirection = MItDependencyGraph::kDownstream, 
						   MItDependencyGraph::Traversal inTraversal = MItDependencyGraph::kDepthFirst )
	{
		for ( int i = 0; i < inSrcObj.length(); i++ ) {
			MObject object = getDGFirstItem( inSrcObj[i], inFilter, inDirection, inTraversal );
			if ( object != MObject::kNullObj ) return object;
		}
		return MObject::kNullObj;
	}
	static MObject getDGFirstItem( MObject& inSrcObj, MFn::Type inFilter = MFn::kInvalid, 
						   MItDependencyGraph::Direction inDirection = MItDependencyGraph::kDownstream, 
						   MItDependencyGraph::Traversal inTraversal = MItDependencyGraph::kDepthFirst )
	{
		MStatus status;
		MItDependencyGraph dgIt(inSrcObj, inFilter, inDirection, inTraversal, MItDependencyGraph::kNodeLevel, &status );
		if ( status != MS::kSuccess ) return MObject::kNullObj;
		if ( dgIt.disablePruningOnFilter() != MS::kSuccess ) return MObject::kNullObj;
		if ( dgIt.isDone() != MS::kSuccess ) return MObject::kNullObj;
		return dgIt.thisNode();
	}
	static void addDGAllItems( MObjectArray& inDstObjs, MObject& inSrcObj, MFn::Type inFilter = MFn::kInvalid, 
						   MItDependencyGraph::Direction inDirection = MItDependencyGraph::kDownstream, 
						   MItDependencyGraph::Traversal inTraversal = MItDependencyGraph::kDepthFirst )
	{
		MItDependencyGraph dgIt( inSrcObj, inFilter, inDirection, inTraversal, MItDependencyGraph::kNodeLevel );
		for ( ; !dgIt.isDone(); dgIt.next() ) {
			inDstObjs.append(dgIt.thisNode());
		}
	}
	static void getDGAllItems( MObjectArray& inDstObjs, MObjectArray & inSrcObj, MFn::Type inFilter = MFn::kInvalid, 
						   MItDependencyGraph::Direction inDirection = MItDependencyGraph::kDownstream, 
						   MItDependencyGraph::Traversal inTraversal = MItDependencyGraph::kDepthFirst )
	{
		inDstObjs.clear();
		for ( int i = 0; i < inSrcObj.length(); i++ ) {
			addDGAllItems( inDstObjs, inSrcObj[i], inFilter, inDirection, inTraversal );
		}
	}
	static void getDGAllItems( MObjectArray& inDstObjs, MObject& inSrcObj, MFn::Type inFilter = MFn::kInvalid, 
						   MItDependencyGraph::Direction inDirection = MItDependencyGraph::kDownstream, 
						   MItDependencyGraph::Traversal inTraversal = MItDependencyGraph::kDepthFirst )
	{
		inDstObjs.clear();
		addDGAllItems( inDstObjs, inSrcObj, inFilter, inDirection, inTraversal );
	}

	void exportAll(MDagPathArray&);
	void exportSelection(MDagPathArray&);
	void exportDagPath(MDagPathArray&);

	void exportDagNode( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * transform );
	sgDagNode * exportDagTransform( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & transform );
	sgDagNode * exportWorld( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & transform );
	sgDagNode * exportDagMesh( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & transform );
	sgMaterial * exportMaterial( MObject & mObj );
	sgMaterial * exportDefaultMaterial();
	sgTexture * exportTexture( MObject & mObj );
	sgBone * searchBone( MDagPath & dPath );
	sgDagNode * exportJoint( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & transform );
	sgDagNode * exportIk( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * & transform );
	void exportEnvelopes();
	void exportEnvelope( envelopeInfomation * envelope );
	bool exportIk( ikInfomation & ik );
	void exportIks();
	string getName( MDagPath & dPath );
	string getName( MFnDependencyNode & fnDependencyNode );
	bool isUnique( const char * unique );
	nameTable * searchNameTableByFullpath( string & fullpath );
	nameTable * searchNameTableByName( string & name );
	void exportAnimation();
	void exportCamera( MDagPath & dPath );
	void exportLight( MDagPath & dPath );
	void exportAmbientLight( MDagPath & dPath );
	void exportPointLight( MDagPath & dPath );
	void exportDirectionalLight( MDagPath & dPath );
	void exportSpotLight( MDagPath & dPath );
};

