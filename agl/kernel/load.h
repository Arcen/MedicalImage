#ifdef __GLOBAL__

class sceneGraphLoader
{
public:
    typedef MSXML2::IXMLDOMElementPtr IXMLDOMElementPtr;
    typedef MSXML2::IXMLDOMDocumentPtr IXMLDOMDocumentPtr;
    typedef MSXML2::IXMLDOMAttributePtr IXMLDOMAttributePtr;
    typedef MSXML2::IXMLDOMTextPtr IXMLDOMTextPtr;
	static string getAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return "";
		return string( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
	}
	static string getChildText( IXMLDOMElementPtr & element )
	{
		return string( (const char*)( element->text ) );
	}
	int load( int & instance, string & src ) { return src.scan( "%d", & instance ); }
	int load( decimal & instance, string & src ) { return src.scan( "%g", & instance ); }
	int load( vector2 & instance, string & src ) { return src.scan( "( %g %g )", & instance.x, & instance.y ); }
	int load( vector3 & instance, string & src ) { return src.scan( "( %g %g %g )", & instance.x, & instance.y, & instance.z ); }
	int load( vector4 & instance, string & src ) { return src.scan( "( %g %g %g %g )", & instance.x, & instance.y, & instance.z, & instance.w ); }
	int load( quaternion & instance, string & src ) { return src.scan( "( %g %g %g %g )", & instance.x, & instance.y, & instance.z, & instance.w ); }
	int load( matrix22 & instance, string & src ) { return src.scan( "( %g %g %g %g )", 
		& instance.m00, & instance.m01, 
		& instance.m10, & instance.m11 ); }
	int load( matrix33 & instance, string & src ) { return src.scan( "( %g %g %g %g %g %g %g %g %g )", 
		& instance.m00, & instance.m01, & instance.m02, 
		& instance.m10, & instance.m11, & instance.m12, 
		& instance.m20, & instance.m21, & instance.m22 ); }
	int load( matrix44 & instance, string & src ) { return src.scan( "( %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g )", 
		& instance.m00, & instance.m01, & instance.m02, & instance.m03, 
		& instance.m10, & instance.m11, & instance.m12, & instance.m13, 
		& instance.m20, & instance.m21, & instance.m22, & instance.m23, 
		& instance.m30, & instance.m31, & instance.m32, & instance.m33 ); }
	int load( string & instance, string & src )
	{
		instance.reserve( src.length() + 1 );
		return src.scan( "%s", instance.chars() );
	}
	template<class T>
	int loadArray( array<T> & instance, string & src )
	{
		int size;
		int offset = src.scan( "{ %d : ", & size );
		if ( offset < 0 ) return -1;
		string rest = src.chars() + offset;
		instance.allocate( size );
		for ( int i = 0; i < instance.size; i++ ) {
			rest = src.chars() + offset;
			int additive = load( instance[i], rest );
			if ( additive < 0 ) return -1;
			offset += additive;
		}
		rest = src.chars() + offset;
		int additive = rest.scan( " }" );
		if ( additive < 0 ) return -1;
		offset += additive;
		return offset;
	}
	template<class T>
	int loadArrayArray( array< array<T> > & instance, string & src )
	{
		int size;
		int offset = src.scan( "{ %d : ", & size );
		if ( offset < 0 ) return -1;
		string rest = src.chars() + offset;
		instance.allocate( size );
		for ( int i = 0; i < instance.size; i++ ) {
			rest = src.chars() + offset;
			int additive = loadArray( instance[i], rest );
			if ( additive < 0 ) return -1;
			offset += additive;
		}
		rest = src.chars() + offset;
		int additive = rest.scan( " }" );
		if ( additive < 0 ) return -1;
		offset += additive;
		return offset;
	}
	void load( const sgTypeDescriptor & type, void * instance, string & src )
	{
		if ( getTypeDescriptor( ( const int * ) NULL ) == type ) { load( * reinterpret_cast<int*>( instance ), src ); }
		if ( getTypeDescriptor( ( const decimal * ) NULL ) == type ) { load( * reinterpret_cast<decimal*>( instance ), src ); }
		if ( getTypeDescriptor( ( const vector2 * ) NULL ) == type ) { load( * reinterpret_cast<vector2*>( instance ), src ); }
		if ( getTypeDescriptor( ( const vector3 * ) NULL ) == type ) { load( * reinterpret_cast<vector3*>( instance ), src ); }
		if ( getTypeDescriptor( ( const vector4 * ) NULL ) == type ) { load( * reinterpret_cast<vector4*>( instance ), src ); }
		if ( getTypeDescriptor( ( const quaternion * ) NULL ) == type ) { load( * reinterpret_cast<quaternion*>( instance ), src ); }
		if ( getTypeDescriptor( ( const matrix22 * ) NULL ) == type ) { load( * reinterpret_cast<matrix22*>( instance ), src ); }
		if ( getTypeDescriptor( ( const matrix33 * ) NULL ) == type ) { load( * reinterpret_cast<matrix33*>( instance ), src ); }
		if ( getTypeDescriptor( ( const matrix44 * ) NULL ) == type ) { load( * reinterpret_cast<matrix44*>( instance ), src ); }
		if ( getTypeDescriptor( ( const string * ) NULL ) == type ) { load( * reinterpret_cast<string*>( instance ), src ); }

		if ( getTypeDescriptor( ( const array< int > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< int >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< decimal > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< decimal >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< vector2 > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< vector2 >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< vector3 > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< vector3 >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< vector4 > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< vector4 >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< quaternion > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< quaternion >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< matrix22 > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< matrix22 >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< matrix33 > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< matrix33 >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< matrix44 > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< matrix44 >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< string > * ) NULL ) == type ) { loadArray( * reinterpret_cast<array< string >*>( instance ), src ); }

		if ( getTypeDescriptor( ( const array< array< int > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< int > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< decimal > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< decimal > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< vector2 > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< vector2 > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< vector3 > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< vector3 > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< vector4 > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< vector4 > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< quaternion > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< quaternion > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< matrix22 > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< matrix22 > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< matrix33 > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< matrix33 > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< matrix44 > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< matrix44 > >*>( instance ), src ); }
		if ( getTypeDescriptor( ( const array< array< string > >* ) NULL ) == type ) { loadArrayArray( * reinterpret_cast<array< array< string > >*>( instance ), src ); }
	}
	bool createNode( IXMLDOMElementPtr & element )
	{
		string nodeName = element->nodeName;
		if ( element->nodeName != _bstr_t( "node" ) ) return true;
		string type = getAttribute( element, "type" );
		sgNodeDescriptor * desc = sgNodeDescriptor::search( type );
		if ( ! desc ) return false;
		sgNode * node = desc->creator( *graph );
		node->name = getAttribute( element, "name" );
		return true;
	}
	bool load( sgTexture * texture )
	{
		texture->texture.load( loadDirectory + texture->url() );
		return true;
	}
	bool loadNode( IXMLDOMElementPtr & element )
	{
		string name = getAttribute( element, "name" );
		sgNode * node = graph->search( name );
		if ( ! node ) return false;
		sgNodeDescriptor & thisDescriptor = node->thisDescriptor;
		for ( IXMLDOMElementPtr child = element->firstChild; child != 0; child = child->nextSibling ) {
			if ( child->nodeName == _bstr_t( "attribute" ) ) {
				string name = getAttribute( child, "name" );
				sgAttribute * attribute = thisDescriptor.getAttribute( name );
				if ( ! attribute ) return false;
				if ( ! attribute->type ) return false;
				string data = getChildText( child );
				load( *attribute->type, node->getInstance( *attribute ), data );
			} else if ( child->nodeName == _bstr_t( "connection" ) ) {
				string srcName = getAttribute( child, "src" );
				string outputName = getAttribute( child, "output" );
				//string dstName = getAttribute( child, "dst" );
				string inputName = getAttribute( child, "input" );
				sgNode * srcNode = graph->search( srcName );
				if ( ! srcNode ) return false;
				sgAttribute * output = srcNode->thisDescriptor.getAttribute( outputName );
				if ( ! output ) return false;
				sgAttribute * input = thisDescriptor.getAttribute( inputName );
				if ( ! input ) return false;
				new sgConnection( *srcNode, *output, *node, *input );
  			}
		}
		if ( node->thisDescriptor == sgTexture::descriptor ) load( static_cast<sgTexture*>( node ) );
		return true;
	}
	void loadDag( IXMLDOMElementPtr & element, sgDagNode & parent )
	{
		string name = getAttribute( element, "name" );
		sgNode * node = graph->search( name );
		if ( ! node ) return;
		sgDagNode * dag = parent.insert( *node );
		for ( IXMLDOMElementPtr child = element->firstChild; child != 0; child = child->nextSibling ) {
			loadDag( child, *dag );
		}
		return;
	}
	void load( IXMLDOMElementPtr & node )
	{
		if ( node->nodeName == _bstr_t( "node" ) ) {
			loadNode( node );
		} else if ( node->nodeName == _bstr_t( "dag" ) ) {
			for ( IXMLDOMElementPtr child = node->firstChild; child != 0; child = child->nextSibling ) {
				loadDag( child, graph->root );
			}
		}
	}
	IXMLDOMDocumentPtr domDocument;
	sgGraph * graph;
	string loadDirectory;
	sceneGraphLoader() : domDocument( "MSXML.DOMDocument" ), graph( NULL )
	{
	}
	sgGraph * load( const char * filename )
	{
		char wk[MAX_PATH], drive[MAX_PATH], directory[MAX_PATH];
		_splitpath( filename, drive, directory, NULL, NULL );
		_makepath( wk, drive, directory, NULL, NULL );
		loadDirectory = wk;

		domDocument->validateOnParse = VARIANT_FALSE;
		domDocument->load( _bstr_t( filename ) );
		graph = new sgGraph();
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		for ( IXMLDOMElementPtr child = domRoot->firstChild; child != 0; child = child->nextSibling ) {
			createNode( child );
		}
		for ( IXMLDOMElementPtr child = domRoot->firstChild; child != 0; child = child->nextSibling ) {
			load( child );
		}
		graph->alignment();
		return graph;
	}
};

sgGraph * sgGraph::load( const char * filename )
{
	sceneGraphLoader loader;
	return loader.load( filename );
}

#endif
