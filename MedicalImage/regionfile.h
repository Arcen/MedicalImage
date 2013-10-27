
class miDocument;
class ctSlices;
class miPattern;

class regionIO
{
public:
#if 0
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
	static string getText( IXMLDOMElementPtr & element )
	{
		return string( static_cast<const char*>( static_cast<_bstr_t>( element->text ) ) );
	}
	static void setText( IXMLDOMElementPtr & element, string text )
	{
		element->text = static_cast<_bstr_t>( text );
	}
	static string getAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return "";
		return string( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
	}
	static int getIntAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return 0;
		return atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
	}
	static void setRegion( IXMLDOMElementPtr & element, const ctRegion & rgn )
	{
		for ( array< ctRegion::rectangle >::iterator it( rgn.rectangleIterator() ); it; ++it ) {
			IXMLDOMElementPtr domNode = createChildElement( element, "rectangle" );
			setAttribute( domNode, "left", string( it->left ) );
			setAttribute( domNode, "top", string( it->top ) );
			setAttribute( domNode, "right", string( it->right ) );
			setAttribute( domNode, "bottom", string( it->bottom ) );
		}
	}
	static void getRegion( IXMLDOMElementPtr & element, ctRegion & rgn )
	{
		rgn.initialize();
		int count = element->childNodes->length;
		//for ( IXMLDOMElementPtr child = element->firstChild; child != 0; child = child->nextSibling ) ++count;
		rgn.reserve( count );

		for ( IXMLDOMElementPtr child = element->firstChild; child != 0; child = child->nextSibling ) {
			rgn.push_back( ctRegion::rectangle( getIntAttribute( child, "left" ), getIntAttribute( child, "top" ), getIntAttribute( child, "right" ), getIntAttribute( child, "bottom" ) ) );
		}
	}
	IXMLDOMDocumentPtr domDocument;
	regionIO() : domDocument( "MSXML.DOMDocument" )
	{
	}
	void save( const ctRegion & rgn, const char * filename )
	{
		domDocument->appendChild( domDocument->createProcessingInstruction( "xml", "version='1.0' encoding='Shift_JIS'" ) );
		IXMLDOMElementPtr domRoot = createChildElement( domDocument, "region" );
		setRegion( domRoot, rgn );
		try {
			domDocument->save( _bstr_t( filename ) );
		} catch (...) {
			//セーブエラーはメッセージを表示するのみとする
		}
	}
	void load( ctRegion & rgn, const char * filename )
	{
		rgn.initialize();
		domDocument->validateOnParse = VARIANT_FALSE;
		if ( ! domDocument->load( _bstr_t( filename ) ) ) return;
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		if ( string( "region" ) != ( const char * ) domRoot->nodeName ) return;
		getRegion( domRoot, rgn );
	}
	void save( const ctSlices & slices, const char * filename );
	bool load( ctSlices & slices, const char * filename );
	void save( const miPattern & teacher, const char * filename );
	bool load( miPattern & teacher, const char * filename );
#else
	static void save( const ctSlices & slices, const char * filename );
	static bool load( ctSlices & slices, const char * filename );
	static void save( const miPattern & teacher, const char * filename );
	static bool load( miPattern & teacher, const char * filename );
	static bool loadLabel( ctSlices & slices, const char * filename );
#endif
};
