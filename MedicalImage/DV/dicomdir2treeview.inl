
void dicomdir2treeview( const MSXML2::IXMLDOMElementPtr & root, fc::treeview & tv, tree<fc::treeview::item>::node * parent, int targetOffset )
{
	MSXML2::IXMLDOMElementPtr element;
	//検索（ディレクトリの項目）
	for ( element = root->firstChild; element != NULL; element = element->nextSibling ) {
		MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "offset" ) );
		if ( attr ) {
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( wk.toi() == targetOffset ) break;
		}
	}
	if ( element ) {} else return;
	//列挙（項目内のエレメントについて）
	int nextSibling = 0, firstChild = 0;
	string itemName;
	for ( MSXML2::IXMLDOMElementPtr item = element->firstChild; item != NULL; item = item->nextSibling ) {
		MSXML2::IXMLDOMAttributePtr attr = item->getAttributeNode( _bstr_t( "name" ) );
		if ( attr ) {} else continue;
		string name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		if ( name == "次のディレクトリレコードのオフセット" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			nextSibling = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "参照下位ディレクトリエンティティのオフセット" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			firstChild = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "ディレクトリレコードタイプ" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			itemName = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		}
	}
	if ( ! itemName.length() ) return;
	fc::treeview::item itm( itemName );
	MSXML2::IXMLDOMElementPtr wk = element;
	itm.data = & wk;//(IXMLDOMElement*)
	tree<fc::treeview::item>::node * current = tv.push( itm, parent );

	for ( MSXML2::IXMLDOMElementPtr item = element->firstChild; item != NULL; item = item->nextSibling ) {
		MSXML2::IXMLDOMAttributePtr attr = item->getAttributeNode( _bstr_t( "name" ) );
		string name;
		if ( attr ) {
			name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( name == "次のディレクトリレコードのオフセット" || 
				name == "参照下位ディレクトリエンティティのオフセット" || 
				name == "グループ長" || 
				name == "レコード使用中フラグ" || 
				name == "ディレクトリレコードタイプ" ||
				name == "グループ長（汎用）" ) {
				continue;
			}
		}
		if ( ! name.length() ) {
			attr = item->getAttributeNode( _bstr_t( "tag" ) );
			if ( attr ) {
				string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
				if ( wk.length() ) name = wk;
			}
		}
		attr = item->getAttributeNode( _bstr_t( "value" ) );
		if ( attr ) {
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( wk.length() ) {
				string dst;
				dst.reserve( 1024 );
				dst.print( "%s:%s", name.chars(), wk.chars() );
				name = dst;
			}
		}

		fc::treeview::item itm( name );
		MSXML2::IXMLDOMElementPtr wk = item;
		itm.data = & wk;//(IXMLDOMElement*)
		tv.push( itm, current );
	}

	if ( nextSibling ) dicomdir2treeview( root, tv, parent, nextSibling );
	if ( firstChild ) dicomdir2treeview( root, tv, current, firstChild );
}

void dicomdir2treeview( MSXML2::IXMLDOMDocumentPtr & dom, fc::treeview & tv )
{
	int firstChildOffset = 0;
	MSXML2::IXMLDOMElementPtr element = dom->documentElement;
	if ( element ) {} else return;
	for ( element = element->firstChild; element != NULL; element = element->nextSibling ) {
		string name = static_cast<const char*>( static_cast<_bstr_t>( element->nodeName ) );
		MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "name" ) );
		if ( attr ) {} else continue;
		name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		if ( name == "ルートディレクトリエンティティの最初のディレクトリレコードのオフセット" ) {
			attr = element->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			firstChildOffset = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "ディレクトリレコードシーケンス" ) {
			if ( firstChildOffset && element->hasChildNodes() ) {
				fc::treeview::item itm( name );
				MSXML2::IXMLDOMElementPtr wk = element;
				itm.data = & wk;//(IXMLDOMElement*)
				dicomdir2treeview( element, tv, tv.push( itm, NULL ), firstChildOffset );
			}
		}
	}
}
