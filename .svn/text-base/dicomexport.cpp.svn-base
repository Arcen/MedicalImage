#include <agl/kernel/agl.h>
using namespace agl;
using namespace MSXML;

#include "dicom.h"
#include "dicomexport.h"
/*
void dicomExport( const char * dicomfilename )
{
	// debug DICOM情報書き出し
	char drive[MAX_PATH], path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( dicomfilename, drive, path, filename, extension );
	char output[MAX_PATH];
	_makepath( output, drive, path, filename, ".xml" );
	retainer<file> f = file::initialize( dicomfilename, false, true );
	if ( ! f ) return;
	static dicom dcm;
	dcm.export( f(), output );
}

void dicomListupFiles( const IXMLDOMElementPtr & root, int targetOffset, const string & fileprefix, array<string> & filenames )
{
	IXMLDOMElementPtr element;
	//検索（ディレクトリの項目）
	for ( element = root->firstChild; element != NULL; element = element->nextSibling ) {
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "offset" ) );
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
	for ( IXMLDOMElementPtr item = element->firstChild; item != NULL; item = item->nextSibling ) {
		IXMLDOMAttributePtr attr = item->getAttributeNode( _bstr_t( "name" ) );
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
		}
	}
	if ( ! itemName.length() ) return;
	if ( itemName.length() >= 5 && memcmp( itemName.chars(), "IMAGE", 5 ) == 0 ) {
		if ( referencefilename.length() ) {
			filenames.push_back( fileprefix + referencefilename );
		}
	}
	if ( firstChild ) dicomListupFiles( root, firstChild, fileprefix, filenames );
	if ( nextSibling ) dicomListupFiles( root, nextSibling, fileprefix, filenames );
}
*/
void dicomListup( const IXMLDOMElementPtr & root, int targetOffset, const string & fileprefix, array<dicomSeriesInformation> & output, dicomSeriesInformation * target )
{
	IXMLDOMElementPtr element;
	//検索（ディレクトリの項目）
	for ( element = root->firstChild; element != NULL; element = element->nextSibling ) {
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "offset" ) );
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
	for ( IXMLDOMElementPtr item = element->firstChild; item != NULL; item = item->nextSibling ) {
		IXMLDOMAttributePtr attr = item->getAttributeNode( _bstr_t( "name" ) );
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
		} else if ( name == "横行" ) {
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
/*
bool dicomExtractFilenames( const char * dicomfilename, array<string> & filenames )
{
	// debug DICOM情報書き出し
	char drive[MAX_PATH], path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( dicomfilename, drive, path, filename, extension );
	string fileprefix = string( drive ) + path;
	retainer<file> f = file::initialize( dicomfilename, false, true );
	if ( ! f ) return false;
	static dicom dcm;
	IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
	if ( ! dcm.convert( f(), domDocument ) ) return false;
	int firstChildOffset = 0;
	IXMLDOMElementPtr element = domDocument->documentElement;
	for ( element = element->firstChild; element != NULL; element = element->nextSibling ) {
		string name = static_cast<const char*>( static_cast<_bstr_t>( element->nodeName ) );
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "name" ) );
		if ( attr ) {} else continue;
		name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		if ( name == "ルートディレクトリエンティティの最初のディレクトリレコードのオフセット" ) {
			attr = element->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			firstChildOffset = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "ディレクトリレコードシーケンス" ) {
			if ( firstChildOffset && element->hasChildNodes() ) {
				dicomListupFiles( element, firstChildOffset, fileprefix, filenames );
			}
		} else if ( name == "画素データ" ) {
			filenames.push_back( dicomfilename );
		}
	}
	return true;
}
*/