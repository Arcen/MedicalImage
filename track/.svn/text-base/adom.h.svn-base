#include <assert.h>
#include <tl/tl.h>

namespace adom
{
    typedef MSXML2::IXMLDOMDocumentPtr IXMLDOMDocumentPtr;
    typedef MSXML2::IXMLDOMElementPtr IXMLDOMElementPtr;
    typedef MSXML2::IXMLDOMAttributePtr IXMLDOMAttributePtr;
    typedef MSXML2::IXMLDOMTextPtr IXMLDOMTextPtr;
    typedef MSXML2::IXMLDOMNode IXMLDOMNode;
    typedef MSXML2::IXMLDOMNodeList IXMLDOMNodeList;
    
	////////////////////////////////////////////////////////////////////////////////
	// XML関連

	//ルートノード作成
	inline IXMLDOMElementPtr createChildElement( IXMLDOMDocumentPtr & document, const char * name )
	{
		IXMLDOMElementPtr node = document->createElement( name );
		document->appendChild( node );
		return node;
	}
	//子ノード作成
	inline IXMLDOMElementPtr createChildElement( IXMLDOMElementPtr & parent, const char * name )
	{
		IXMLDOMElementPtr node = parent->ownerDocument->createElement( name );
		parent->appendChild( node );
		return node;
	}
	//属性設定
	inline IXMLDOMAttributePtr setAttribute( IXMLDOMElementPtr & element, const char * attribute, const char * value )
	{
		IXMLDOMAttributePtr attr = element->ownerDocument->createAttribute( attribute );
		attr->value = value;
		element->setAttributeNode( attr );
		return attr;
	}
	//テキスト設定
	inline IXMLDOMTextPtr createChildText( IXMLDOMElementPtr & parent, const char * name )
	{
		IXMLDOMTextPtr node = parent->ownerDocument->createTextNode( name );
		parent->appendChild( node );
		return node;
	}
	//属性取得
	inline string getAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return "";
		return string( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
	}
	//属性取得
	inline bool existAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return false;
		return true;
	}
	//int型に変換しての属性取得
	inline int getIntAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return 0;
		return atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
	}
	inline const string getNodeName( IXMLDOMElementPtr & element )
	{
		return string( static_cast<const char*>( static_cast<_bstr_t>( element->nodeName ) ) );
	}
	inline bool save( IXMLDOMDocumentPtr & document, const char * filename )
	{
		bool result = false;
		try {
			document->save( _bstr_t( filename ) );
		} catch (...) {
			return false;
		}
		return true;
	}
	inline bool load( IXMLDOMDocumentPtr & document, const char * filename )
	{
		document->validateOnParse = VARIANT_FALSE;
		bool result = false;
		try {
			result = document->load( _bstr_t( filename ) ) ? true : false;
		} catch (...) {
			return false;
		}
		return result;
	}
};
