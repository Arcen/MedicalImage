#ifdef __GLOBAL__

//using namespace MSXML2;

class sceneGraphSaver
{
public:
    typedef MSXML2::IXMLDOMElementPtr IXMLDOMElementPtr;
    typedef MSXML2::IXMLDOMDocumentPtr IXMLDOMDocumentPtr;
    typedef MSXML2::IXMLDOMAttributePtr IXMLDOMAttributePtr;
    typedef MSXML2::IXMLDOMTextPtr IXMLDOMTextPtr;
	static IXMLDOMElementPtr createChildElement( IXMLDOMDocumentPtr & document, const char * name )
	{
		IXMLDOMElementPtr node = document->createElement( name );
		document->appendChild( node );
		return node;
	}
	static IXMLDOMElementPtr createChildElement( IXMLDOMElementPtr & parent, const char * name )
	{
		IXMLDOMElementPtr node = parent->ownerDocument->createElement( name );
		parent->appendChild( node );
		return node;
	}
	static IXMLDOMAttributePtr setAttribute( IXMLDOMElementPtr & element, const char * attribute, const char * value )
	{
		IXMLDOMAttributePtr attr = element->ownerDocument->createAttribute( attribute );
		attr->value = value;
		element->setAttributeNode( attr );
		return attr;
	}
	static IXMLDOMTextPtr createChildText( IXMLDOMElementPtr & parent, const char * name )
	{
		IXMLDOMTextPtr node = parent->ownerDocument->createTextNode( name );
		parent->appendChild( node );
		return node;
	}
	void save( int & instance, string & dst ) { dst.print( "%d", instance ); }
	void save( decimal & instance, string & dst ) { dst.print( "%g", instance ); }
	void save( vector2 & instance, string & dst ) { dst.print( "( %g %g )", instance.x, instance.y ); }
	void save( vector3 & instance, string & dst ) { dst.print( "( %g %g %g )", instance.x, instance.y, instance.z ); }
	void save( vector4 & instance, string & dst ) { dst.print( "( %g %g %g %g )", instance.x, instance.y, instance.z, instance.w ); }
	void save( quaternion & instance, string & dst ) { dst.print( "( %g %g %g %g )", instance.x, instance.y, instance.z, instance.w ); }
	void save( matrix22 & instance, string & dst ) { dst.print( "( %g %g %g %g )", 
		instance.m00, instance.m01, 
		instance.m10, instance.m11 ); }
	void save( matrix33 & instance, string & dst ) { dst.print( "( %g %g %g %g %g %g %g %g %g )", 
		instance.m00, instance.m01, instance.m02, 
		instance.m10, instance.m11, instance.m12, 
		instance.m20, instance.m21, instance.m22 ); }
	void save( matrix44 & instance, string & dst ) { dst.print( "( %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g )", 
		instance.m00, instance.m01, instance.m02, instance.m03, 
		instance.m10, instance.m11, instance.m12, instance.m13, 
		instance.m20, instance.m21, instance.m22, instance.m23, 
		instance.m30, instance.m31, instance.m32, instance.m33 ); }
	void save( string & instance, string & dst ) { dst.print( "%s", instance.chars() ); }
	template<class T>
	void saveArray( array<T> & instance, string & dst )
	{
		string a, e;
		for ( int i = 0; i < instance.size; i++ ) {
			save( instance[i], e );
			if ( i == 0 ) {
				a.reserve( instance.size * ( e.length() * 2 ) );
				a = e;
			} else {
				a = a + " " + e;
			}
		}
		dst.print( "{ %d : %s }", instance.size, a.chars() );
	}
	template<class T>
	void saveArrayArray( array< array<T> > & instance, string & dst )
	{
		string a, e;
		for ( int i = 0; i < instance.size; i++ ) {
			saveArray( instance[i], e );
			if ( i == 0 ) {
				a.reserve( instance.size * ( e.length() * 2 ) );
				a = e;
			} else {
				a = a + " " + e;
			}
		}
		dst.print( "{ %d : %s }", instance.size, a.chars() );
	}
	void save( const sgTypeDescriptor & type, void * instance, string & dst )
	{
		if ( getTypeDescriptor( ( const int * ) NULL ) == type ) { save( * reinterpret_cast<int*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const decimal * ) NULL ) == type ) { save( * reinterpret_cast<decimal*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const vector2 * ) NULL ) == type ) { save( * reinterpret_cast<vector2*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const vector3 * ) NULL ) == type ) { save( * reinterpret_cast<vector3*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const vector4 * ) NULL ) == type ) { save( * reinterpret_cast<vector4*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const quaternion * ) NULL ) == type ) { save( * reinterpret_cast<quaternion*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const matrix22 * ) NULL ) == type ) { save( * reinterpret_cast<matrix22*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const matrix33 * ) NULL ) == type ) { save( * reinterpret_cast<matrix33*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const matrix44 * ) NULL ) == type ) { save( * reinterpret_cast<matrix44*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const string * ) NULL ) == type ) { save( * reinterpret_cast<string*>( instance ), dst ); }

		if ( getTypeDescriptor( ( const array< int > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< int >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< decimal > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< decimal >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< vector2 > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< vector2 >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< vector3 > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< vector3 >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< vector4 > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< vector4 >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< quaternion > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< quaternion >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< matrix22 > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< matrix22 >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< matrix33 > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< matrix33 >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< matrix44 > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< matrix44 >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< string > * ) NULL ) == type ) { saveArray( * reinterpret_cast<array< string >*>( instance ), dst ); }

		if ( getTypeDescriptor( ( const array< array< int > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< int > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< decimal > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< decimal > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< vector2 > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< vector2 > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< vector3 > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< vector3 > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< vector4 > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< vector4 > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< quaternion > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< quaternion > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< matrix22 > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< matrix22 > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< matrix33 > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< matrix33 > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< matrix44 > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< matrix44 > >*>( instance ), dst ); }
		if ( getTypeDescriptor( ( const array< array< string > >* ) NULL ) == type ) { saveArrayArray( * reinterpret_cast<array< array< string > >*>( instance ), dst ); }
	}
	void save( sgTexture * node )
	{
		node->texture.save( saveDirectory + node->url() );
	}
	void save( sgNode * node )
	{
		if ( node->thisDescriptor == sgTexture::descriptor ) save( static_cast<sgTexture*>( node ) );
		IXMLDOMElementPtr domNode = createChildElement( domRoot, "node" );
		setAttribute( domNode, "type", node->thisDescriptor.name );
		setAttribute( domNode, "name", node->name );

		sgNodeDescriptor & descriptor = node->thisDescriptor;
		//プロパティ
		for ( array< retainer<sgAttribute> >::iterator it( descriptor.attributes ); it; ++it ) {
			sgAttribute & attribute = it()();
			if ( ! attribute.type ) continue;//this attribute
			if ( ! attribute.retain ) continue;
			if ( node->initialized( attribute ) ) continue;
			IXMLDOMElementPtr domAttribute = createChildElement( domNode, "attribute" );
			setAttribute( domAttribute, "name", attribute.name );
			string dst;
			save( * ( attribute.type ), node->getInstance( attribute ), dst );
			createChildText( domAttribute, dst );
		}
		//他のノードからの入力
		for ( array< list<sgConnection*> >::iterator itins( node->inputs ); itins; ++itins ) {
			list<sgConnection*> & connections = itins();
			for ( list<sgConnection*>::iterator itin( connections ); itin; ++itin ) {
				sgConnection * connection = itin();
				IXMLDOMElementPtr domConnection = createChildElement( domNode, "connection" );
				setAttribute( domConnection, "src", connection->src.name );
				setAttribute( domConnection, "output", connection->output.name );
//				setAttribute( domConnection, "dst", connection->dst.name );
				setAttribute( domConnection, "input", connection->input.name );
			}
		}
	}
	void save( IXMLDOMElementPtr & parent, sgDagNode & dag )
	{
		IXMLDOMElementPtr domNode = createChildElement( parent, "dag" );
		if ( dag.node ) setAttribute( domNode, "name", dag.node->name );
		for ( list< retainer<sgDagNode> >::iterator it( dag.children ); it; ++it ) {
			save( domNode, it()() );
		}
	}
	IXMLDOMDocumentPtr domDocument;
	IXMLDOMElementPtr domRoot;
	string saveDirectory;
	sceneGraphSaver( sgGraph * graph, const char * filename ) : domDocument( "MSXML.DOMDocument" )
	{
		char wk[MAX_PATH], drive[MAX_PATH], directory[MAX_PATH];
		_splitpath( filename, drive, directory, NULL, NULL );
		_makepath( wk, drive, directory, NULL, NULL );
		saveDirectory = wk;
		graph->alignment();
		domDocument->appendChild( domDocument->createProcessingInstruction( "xml", "version='1.0' encoding='Shift_JIS'" ) );
		domRoot = createChildElement( domDocument, "body" );
		// nodes
		for ( list< retainer<sgNode> >::iterator it( graph->nodes ); it; ++it ) save( & it()() );
		// dag
		save( domRoot, graph->root );
		domDocument->save( _bstr_t( filename ) );
	}
};

void sgGraph::save( const char * filename )
{
	sceneGraphSaver saver( this, filename );
}

#endif
