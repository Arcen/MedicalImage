#include "Maya.h"

MStatus initializePlugin(MObject mObj)
{
	sgGraph::initialize();
	sgGraph * g = new sgGraph();
	sgTime * wk = new sgTime( *g );
	delete g;
	MFnPlugin plugin( mObj, "Akira", "1.0" );
	return plugin.registerFileTranslator( "aglMaya", "", Maya::creator, NULL, NULL, true );
}

MStatus uninitializePlugin(MObject mObj)
{
	sgGraph::finalize();
	MFnPlugin plugin( mObj );
	return plugin.deregisterFileTranslator( "aglMaya" );
}

inline void strrep( char * string, const char find, const char replace )
{
	unsigned int length = strlen( string );
	for ( unsigned i = 0; i < length; i++ ) if ( string[i] == find ) string[i] = replace;
}

sgGraph * Maya::graph = NULL;

MStatus Maya::writer(const MFileObject& file, const MString& options, FileAccessMode mode)
{
	try {
		sgGraph::initialize();
		materials.release();
		textures.release();
		animationNodes.release();
		animationNode::time = NULL;
		envelopes.release();
		iks.release();
		bones.release();
		nameTables.release();

		MDagPathArray dPathArray;
		switch ( mode ) {
		case MPxFileTranslator::kExportAccessMode: exportAll(dPathArray); break;
		case MPxFileTranslator::kExportActiveAccessMode: exportSelection(dPathArray); break;
		default: statusError( MS::kFailure, "Illigal FileAccessMode" );
		}
		graph = new sgGraph();
		animationNode::time = new sgTime( *graph );
		animationNode::time->name = "time$time";
		exportDagPath(dPathArray);
		exportEnvelopes();
		exportIks();
		exportAnimation();
		graph->alignment();
		graph->save( file.fullName().asChar() );
		if ( 1 ) {
			express e( NULL, graph );
		} else {
			delete graph;
		}
	} catch ( const char * errString ) {
		fprintf( stderr, errString );
		sgGraph::finalize();
		return MS::kFailure;
	}
	return MS::kSuccess;
}

void Maya::exportAll( MDagPathArray & dPathArray )
{
	MStatus status;
	MItDag dagIt( MItDag::kDepthFirst, MFn::kInvalid, & status ); statusError( status, "Fail MItDag" );
	MDagPath dPath;
	statusError( dagIt.getPath(dPath), "Fail getPath" );
	dPathArray.append( dPath );
}

void Maya::exportSelection( MDagPathArray & dPathArray )
{
	MStatus status;
	MSelectionList selectList;
	statusError( MGlobal::getActiveSelectionList( selectList ), "MGlobal::getActiveSelectionList" );

	MItSelectionList selectIt( selectList, MFn::kInvalid, &status ); statusError( status, "MItSelectionList" );
	MItDag dagIt( MItDag::kDepthFirst, MFn::kInvalid, &status ); statusError( status, "MItDag" );

	list<MObject> cArray;
	for ( ; !selectIt.isDone(); selectIt.next() ) {
		MObject rootObj;
		statusError( selectIt.getDependNode(rootObj), "getDependNode" );
		list<MObject>::iterator it( cArray.search( rootObj ) );
		if ( it ) continue;
		cArray.push_back( rootObj );
		if ( MS::kSuccess != dagIt.reset( rootObj ) ) continue;

		for ( ; !dagIt.isDone(); dagIt.next() ) {
			list<MObject>::iterator it( cArray.search( dagIt.item() ) );
			if ( ! it ) {
				cArray.push_back( dagIt.item() );
			}
		}
		MDagPath dPath;
		statusError( MFnDagNode( rootObj ).getPath(dPath), "Fail getPath" );
		dPathArray.append( dPath );
	}
}

void Maya::exportDagPath( MDagPathArray & dPathArray )
{
	for ( unsigned i = 0; i < dPathArray.length(); i++ ) {
		exportDagNode( dPathArray[i], & graph->root, NULL );
	}
}

bool Maya::isUnique( const char * unique )
{
	for ( list< retainer<nameTable> >::iterator it( nameTables ); it; ++it ) {
		if ( it()->unique == unique ) return false;
	}
	return true;
}

nameTable * Maya::searchNameTableByFullpath( string & fullpath )
{
	for ( list< retainer<nameTable> >::iterator it( nameTables ); it; ++it ) {
		if ( it()->fullpath == fullpath ) return & ( it()() );
	}
	nameTable * nt = new nameTable;
	nt->fullpath = fullpath;
	const char * lasttok = strrchr( fullpath, '|' );
	if ( lasttok ) {
		nt->name = lasttok + 1;
	}
	if ( isUnique( nt->name ) ) {
		nt->unique = nt->name;
	} else {
		int index = 0;
		string uniqueName;
		do {
			uniqueName = nt->name + "$" + string( index++ );
		} while ( ! isUnique( uniqueName ) );
		nt->unique = uniqueName;
	}
	nameTables.push_back( nt );
	return nt;
}

nameTable * Maya::searchNameTableByName( string & name )
{
	for ( list< retainer<nameTable> >::iterator it( nameTables ); it; ++it ) {
		if ( it()->name == name ) return & ( it()() );
	}
	nameTable * nt = new nameTable;
	nt->fullpath = name;
	nt->name = name;
	if ( isUnique( nt->name ) ) {
		nt->unique = nt->name;
	} else {
		int index = 0;
		string uniqueName;
		do {
			uniqueName = nt->name + "$" + string( index++ );
		} while ( ! isUnique( uniqueName ) );
		nt->unique = uniqueName;
	}
	nameTables.push_back( nt );
	return nt;
}

string Maya::getName( MDagPath & dPath )
{
	string fullpath = dPath.fullPathName().asChar();
	nameTable * nt = searchNameTableByFullpath( fullpath );
	return nt->unique;
}

string Maya::getName( MFnDependencyNode & fnDependencyNode )
{
	string name = fnDependencyNode.name().asChar();
	nameTable * nt = searchNameTableByName( name );
	return nt->unique;
}

void Maya::exportDagNode( MDagPath & dPath, sgDagNode * parent, sgTransformMatrix * transform )
{
	MFn::Type apiType = dPath.apiType();
	switch ( dPath.apiType() ) {
	case MFn::kWorld: parent = exportWorld( dPath, parent, transform ); break;
	case MFn::kJoint: parent = exportJoint( dPath, parent, transform ); break;
	case MFn::kTransform: parent = exportDagTransform( dPath, parent, transform ); break;
	case MFn::kMesh: parent = exportDagMesh( dPath, parent, transform ); break;
	case MFn::kCamera: exportCamera( dPath ); break;
	case MFn::kAmbientLight:
	case MFn::kDirectionalLight:
	case MFn::kPointLight:
	case MFn::kSpotLight:
//	case MFn::kAreaLight:
		exportLight( dPath ); break;
	case MFn::kIkHandle: parent = exportIk( dPath, parent, transform ); break;
	default:
		apiType = apiType;
		break;
	}

	MItDag dagIt;
	statusError( dagIt.reset( dPath, MItDag::kBreadthFirst ), "Illigal MItDag" );
	dagIt.next();
	for ( ; dagIt.depth() == 1; dagIt.next() ) {
		MDagPath cPath;
		statusError( dagIt.getPath( cPath ), "Fail getPath" );
		exportDagNode( cPath, parent, transform );
	}
}

#include "transform.inl"
#include "mesh.inl"
#include "material.inl"
#include "texture.inl"
#include "joint.inl"
#include "ik.inl"
#include "envelope.inl"
#include "animation.inl"
#include "world.inl"
#include "camera.inl"
#include "light.inl"
#include "place2d.inl"
