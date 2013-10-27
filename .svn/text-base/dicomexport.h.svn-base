class dicomSeriesInformation
{
public:
	int targetOffset;
	array<string> files;
	double thickness;
	double fieldOfViewMM;
};

inline void dicomListup( const MSXML2::IXMLDOMElementPtr & root, int targetOffset, const string & fileprefix, array<dicomSeriesInformation> & output, dicomSeriesInformation * target = NULL )
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
	string referencefilename;
	double thickness = 0;
	double width = 0, pixelsize = 0;
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
		} else if ( name == "参照ファイルＩＤ" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			referencefilename = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		} else if ( name == "スライス厚さ" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			thickness = atof( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "列数" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			width = atof( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "画素間隔" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			pixelsize = atof( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		}
	}
	dicomSeriesInformation * child_target = NULL;
	if ( itemName.length() >= 6 && memcmp( itemName.chars(), "SERIES", 6 ) == 0 ) {
		output.push_back( dicomSeriesInformation() );
		child_target = & output.last();
		child_target->targetOffset = targetOffset;
	}
	if ( target && itemName.length() >= 5 && memcmp( itemName.chars(), "IMAGE", 5 ) == 0 ) {
		if ( referencefilename.length() ) {
			target->files.push_back( fileprefix + referencefilename );
			target->thickness = thickness;
			target->fieldOfViewMM = width * pixelsize;
		}
	}
	if ( firstChild ) dicomListup( root, firstChild, fileprefix, output, child_target );
	if ( nextSibling ) dicomListup( root, nextSibling, fileprefix, output, target );
}
