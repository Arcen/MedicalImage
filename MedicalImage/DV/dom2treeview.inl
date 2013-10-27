
void dom2treeview( const MSXML2::IXMLDOMElementPtr & firstElement, fc::treeview & tv, tree<fc::treeview::item>::node * parent )
{
	for ( MSXML2::IXMLDOMElementPtr element = firstElement; element != NULL; element = element->nextSibling ) {
		string name = static_cast<const char*>( element->nodeName );
		tree<fc::treeview::item>::node * current = parent;
		MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "tag" ) );
		if ( attr ) {
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( wk.length() ) name = wk;
		}
		attr = element->getAttributeNode( _bstr_t( "name" ) );
		if ( attr ) {
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( wk.length() ) name = wk;
		}
		attr = element->getAttributeNode( _bstr_t( "value" ) );
		if ( attr ) {
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( wk.length() ) {
				string dst;
				dst.print( "%s:%s", name.chars(), wk.chars() );
				name = dst;
			}
		}

		fc::treeview::item itm( name );
		MSXML2::IXMLDOMElementPtr wk = element;
		itm.data = & wk;//(IXMLDOMElement*)
		current = tv.push( itm, parent );
		if ( element->hasChildNodes() ) {
			dom2treeview( element->firstChild, tv, current );
		}
	}
}

void dom2treeview( MSXML2::IXMLDOMDocumentPtr & dom, fc::treeview & tv )
{
	if ( dom->hasChildNodes() ) dom2treeview( dom->documentElement, tv, NULL );
}
