#include "dicom_tag.h"
#include "dicom_uid.h"
#include <vector>
#include <list>
#include <map>
#include <string>

class dicom
{
    typedef MSXML2::IXMLDOMElementPtr IXMLDOMElementPtr;
    typedef MSXML2::IXMLDOMDocumentPtr IXMLDOMDocumentPtr;
    typedef MSXML2::IXMLDOMAttributePtr IXMLDOMAttributePtr;
    typedef MSXML2::IXMLDOMTextPtr IXMLDOMTextPtr;
    typedef MSXML2::IXMLDOMNodeListPtr IXMLDOMNodeListPtr;
    typedef MSXML2::IXMLDOMNodePtr IXMLDOMNodePtr;
public:
    class nodeType
    {
    public:
        string name;
        string value;
        std::map<std::string,std::string> attributes;
        std::list<nodeType> children;
    };
private:
//タグの名称
	enum specificTag {
		stImage = 0x7FE00010,
		stWidth = 0x00280010,
		stHeight = 0x00280011,
		stColorDepth = 0x00280100,
		stStoreBit = 0x00280101,
		stLow = 0x00280102,
		stHigh = 0x00280104,
		stLowS = 0x00280108,
		stHighS = 0x00280109,
		stDataSetTrailingPadding = 0xFFFCFFFC, 
		stItem = 0xFFFEE000, 
		stItemDelimitationItem = 0xFFFEE00D, 
		stSequenceDelimitationItem = 0xFFFEE0DD,
	};
	array<dicomTag> tags;
	array<dicomUID> UIDs;
	bool reverse, checkReverse;
public:
	dicom() : reverse( true ), checkReverse( true )
	{
		//タグ,UIDの初期化
		initializeTag( tags );
		initializeUID( UIDs );
	}
private:
	//強制バイト反転
	unsigned short forceReverse( unsigned short us )
	{
		return ( ( us & 0xff ) << 8 ) |
				( ( us & 0xff00 ) >> 8 );
	}
	//強制バイト反転
	unsigned long forceReverse( unsigned long ul )
	{
		return ( ( ul & 0xff ) << 24 ) |
				( ( ul & 0xff00 ) << 8 ) |
				( ( ul & 0xff0000 ) >> 8 ) |
				( ( ul & 0xff000000 ) >> 24 );
	}
	//強制バイト反転
	short forceReverse( short s )
	{
		return forceReverse( * reinterpret_cast<unsigned short*>( & s ) );
	}
	//強制バイト反転
	long forceReverse( long & l )
	{
		return forceReverse( * reinterpret_cast<unsigned long*>( & l ) );
	}
	//バイト反転が必要なら実行
	unsigned short fix( unsigned short & us )
	{
		if ( reverse ) {
			us = ( ( us & 0xff ) << 8 ) |
				( ( us & 0xff00 ) >> 8 );
		}
		return us;
	}
	//バイト反転が必要なら実行
	unsigned long fix( unsigned long & ul )
	{
		if ( reverse ) {
			ul = ( ( ul & 0xff ) << 24 ) |
				( ( ul & 0xff00 ) << 8 ) |
				( ( ul & 0xff0000 ) >> 8 ) |
				( ( ul & 0xff000000 ) >> 24 );
		}
		return ul;
	}
	//バイト反転が必要なら実行
	short fix( short & s )
	{
		unsigned short us = * reinterpret_cast<unsigned short*>( & s );
		fix( us );
		s = * reinterpret_cast<short*>( & us );
		return s;
	}
	//バイト反転が必要なら実行
	long fix( long & l )
	{
		unsigned short ul = * reinterpret_cast<unsigned long*>( & l );
		fix( ul );
		l = * reinterpret_cast<long*>( & ul );
		return l;
	}
	//バイト反転が必要なら実行
	long fix( float & f )
	{
		return fix( reinterpret_cast<long&>( f ) );
	}
	//バイト反転が必要なら実行
	long fix( double & d )
	{
		if ( reverse ) {
			unsigned long wk[2];
			memcpy( wk, & d, 8 );
			unsigned long _wk[2];
			_wk[0] = fix( wk[1] );
			_wk[1] = fix( wk[0] );
			memcpy( & d, _wk, 8 );
		}
		return d;
	}
	////////////////////////////////////////////////////////////////////////////////
	// XML関連
	IXMLDOMElementPtr createChildElement( IXMLDOMDocumentPtr & document, const char * name )
	{
		IXMLDOMElementPtr node = document->createElement( name );
		document->appendChild( node );
		return node;
	}
	IXMLDOMElementPtr createChildElement( IXMLDOMElementPtr & parent, const char * name )
	{
		IXMLDOMElementPtr node = parent->ownerDocument->createElement( name );
		parent->appendChild( node );
		return node;
	}
	IXMLDOMAttributePtr setAttribute( IXMLDOMElementPtr & element, const char * attribute, const char * value )
	{
		IXMLDOMAttributePtr attr = element->ownerDocument->createAttribute( attribute );
		attr->value = value;
		element->setAttributeNode( attr );
		return attr;
	}
	IXMLDOMTextPtr createChildText( IXMLDOMElementPtr & parent, const char * name )
	{
		IXMLDOMTextPtr node = parent->ownerDocument->createTextNode( name );
		parent->appendChild( node );
		return node;
	}
	string getAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return "";
		return string( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
	}
	int getIntAttribute( IXMLDOMElementPtr & element, const char * attribute )
	{
		IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( attribute ) );
		if ( attr == NULL ) return 0;
		return atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
	}
	////////////////////////////////////////////////////////////////////////////////
	// DICOM関連
	//タグの名称を取得
	const char * tagName( unsigned long tag )
	{
		unsigned short group = ( tag & 0xffff0000 ) >> 16;
		unsigned short element = ( tag & 0xffff );
		//不定の部分を強制的に変更
		if ( ( group & 0xff00 ) == 0x6000 || ( group & 0xff00 ) == 0x5000 ) {
			group &= 0xff00;
		}
		if ( group == 0x0020 && ( ( element & 0xff00 ) == 3100 ) ) {
			element &= 0xff00;
		}
		for ( int i = 0; i < tags.size; ++i ) {
			if ( tags[i].group == group && tags[i].element == element ) {
				if ( strlen( tags[i].comment ) ) return tags[i].comment;
				return tags[i].name;
			}
		}
		//elementが０ならばデータ長であると思われる
		if ( element == 0 ) return tagName( static_cast<unsigned long>( element ) );
		return "";
	}
	//型形式を取得
	const char * valueRepresentation( unsigned long tag )
	{
		unsigned short group = ( tag & 0xffff0000 ) >> 16;
		unsigned short element = ( tag & 0xffff );
		//不定の部分を強制的に変更
		if ( ( group & 0xff00 ) == 0x6000 || ( group & 0xff00 ) == 0x5000 ) {
			group &= 0xff00;
		}
		if ( group == 0x0020 && ( ( element & 0xff00 ) == 3100 ) ) {
			element &= 0xff00;
		}

		for ( int i = 0; i < tags.size; ++i ) {
			if ( tags[i].group == group && tags[i].element == element ) {
				return tags[i].valueRepresentation;
			}
		}
		//elementが０ならばデータ長であると思われる
		if ( element == 0 ) return valueRepresentation( static_cast<unsigned long>( element ) );
		return "";
	}
	//型形式が合っているか調べる
	bool valueRepresentation( const char * vr )
	{
		static const char vrs[][3] = { 
			"AE", "AS", "AT", "CS", "DA", "DS", "DT", "FL", "FD", "IS", 
			"LO", "LT", "OB", "OW", "PN", "SH", "SL", "SQ", "SS", "ST", 
			"TM", "UI", "UL", "UN", "US", "UT" };
		for ( int i = 0; i < sizeof( vrs ) / 3; ++i ) {
			if ( memcmp( vrs[i], vr, 2 ) == 0 ) return true;
		}
		return false;
	}
	//拡張型形式か調べる
	bool extendedValueRepresentation( const char * vr )
	{
		static const char vrs[][3] = { "OB", "OW", "SQ", "UN" };
		for ( int i = 0; i < sizeof( vrs ) / 3; ++i ) {
			if ( memcmp( vrs[i], vr, 2 ) == 0 ) return true;
		}
		return false;
	}
	//データが文字列型か調べる
	bool stringValueRepresentation( const char * vr )
	{
		if ( ! strlen( vr ) ) return false;
		static const char vrs[][3] = { 
			"AE", "AS", "CS", "DA", "DS", "DT", "LT", "SH", "ST", "TM", 
			"UI", "PN" };
		for ( int i = 0; i < sizeof( vrs ) / 3; ++i ) {
			if ( strstr( vr, vrs[i] ) ) return true;
		}
		return false;
	}
	//UIDを取得
	const char * uidName( const char * uid )
	{
		for ( int i = 0; i < UIDs.size; ++i ) {
			if ( strcmp( UIDs[i].uid, uid ) == 0 ) {
				if ( strlen( UIDs[i].comment ) ) return UIDs[i].comment;
				if ( strlen( UIDs[i].name ) ) return UIDs[i].name;
				break;
			}
		}
		return "";
	}
	//シーケンスを入力
	int readSQ( file & f, IXMLDOMElementPtr & parent, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			//タグを読み込む
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_item_position = f.tell();
			readTag( f, tag, group, element, string_tag );

			if ( f.eof() ) break;

			IXMLDOMElementPtr item = createChildElement( parent, "item" );
			setAttribute( item, "offset", string( start_item_position ) );
			
			//SQにはVRはない
			f.read( length ); fix( length );

			setAttribute( item, "tag", string_tag );
			setAttribute( item, "name", tagName( tag ) );
			setAttribute( item, "length", string( int( length ) ) );

			int start_position = f.tell();
			if ( tag == stItem ) {//項目タグ
				length = readDataSet( f, item, length );
			} else if ( tag == stSequenceDelimitationItem ) {//シーケンス区切りタグ（終了タグ）
				f.seek( start_position + length, SEEK_SET );
				break;//SQ終了
			} else {
				return 0;
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//データセットを出力
	int readDataSet( file & f, IXMLDOMElementPtr & parent, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_element_position = f.tell();
			readTag( f, tag, group, element, string_tag );

			if ( f.eof() ) break;

			char vr[1024];
			if ( ! readVRLength( f, tag, vr, length ) ) {
				//エンディアンを判別できたが遅かった
				f.seek( start_element_position, SEEK_SET );
				readTag( f, tag, group, element, string_tag );
				readVRLength( f, tag, vr, length );
			}

			IXMLDOMElementPtr domNode = createChildElement( parent, "element" );

			setAttribute( domNode, "offset", string( start_element_position ) );
			setAttribute( domNode, "tag", string_tag );
			setAttribute( domNode, "name", tagName( tag ) );
			setAttribute( domNode, "vr", vr );
			setAttribute( domNode, "length", string( int( length ) ) );

			int start_position = f.tell();
			if ( tag == stItemDelimitationItem ) {//項目区切りタグがあると終了
				f.seek( start_position + length, SEEK_SET );
				break;
			}

			if ( strstr( vr, "SQ" ) ) {
				int result = readSQ( f, domNode, length );
				if ( result == 0 && length == 0xFFFFFFFF ) return 0;
				if ( length == 0xFFFFFFFF ) length = result;
			} else {
				string value;
				if ( readValueByVR( f, vr, length, value ) ) {
					//valueにUIDがある場合，意味も付け加える
					if ( strstr( vr, "UI" ) ) {
						const char * name = uidName( value );
						if ( strlen( name ) ) {
							setAttribute( domNode, "UID", name );
						}
					}
					setAttribute( domNode, "value", value );
				}
				if ( tag == stImage ) {
					setAttribute( domNode, "value", string( start_position ) );
				}
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//シーケンスを入力
	int readSQ( file & f, nodeType & parent, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			//タグを読み込む
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_item_position = f.tell();
			readTag( f, tag, group, element, string_tag );

			if ( f.eof() ) break;

            parent.children.push_back( nodeType() );
            nodeType & item = parent.children.back();
            item.name = "item";
            item.attributes["offset"] = string( start_item_position ).chars();
			
			//SQにはVRはない
			f.read( length ); fix( length );

            item.attributes["tag"] = string_tag;
            item.attributes["name"] = tagName( tag );
            item.attributes["length"] = string( int( length ) ).chars();

			int start_position = f.tell();
			if ( tag == stItem ) {//項目タグ
				length = readDataSet( f, item, length );
			} else if ( tag == stSequenceDelimitationItem ) {//シーケンス区切りタグ（終了タグ）
				f.seek( start_position + length, SEEK_SET );
				break;//SQ終了
			} else {
				return 0;
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//データセットを出力
	int readDataSet( file & f, nodeType & parent, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_element_position = f.tell();
			readTag( f, tag, group, element, string_tag );

			if ( f.eof() ) break;

			char vr[1024];
			if ( ! readVRLength( f, tag, vr, length ) ) {
				//エンディアンを判別できたが遅かった
				f.seek( start_element_position, SEEK_SET );
				readTag( f, tag, group, element, string_tag );
				readVRLength( f, tag, vr, length );
			}
            parent.children.push_back( nodeType() );
            nodeType & node = parent.children.back();
            node.name = "element";
            node.attributes["offset"] = string( start_element_position ).chars();
            node.attributes["tag"] = string_tag;
            node.attributes["name"] = tagName( tag );
            node.attributes["vr"] = vr;
            node.attributes["length"] = string( int( length ) ).chars();

			int start_position = f.tell();
			if ( tag == stItemDelimitationItem ) {//項目区切りタグがあると終了
				f.seek( start_position + length, SEEK_SET );
				break;
			}

			if ( strstr( vr, "SQ" ) ) {
				int result = readSQ( f, node, length );
				if ( result == 0 && length == 0xFFFFFFFF ) return 0;
				if ( length == 0xFFFFFFFF ) length = result;
			} else {
				string value;
				if ( readValueByVR( f, vr, length, value ) ) {
					//valueにUIDがある場合，意味も付け加える
					if ( strstr( vr, "UI" ) ) {
						const char * name = uidName( value );
						if ( strlen( name ) ) {
                            node.attributes["UID"] = name;
						}
					}
                    node.attributes["value"] = value.chars();
				}
				if ( tag == stImage ) {
                    node.attributes["value"] = string( start_position ).chars();
				}
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//タグを読み込む
	void readTag( file & f, unsigned long & tag, unsigned short & group, unsigned short & element, char * str )
	{
		f.read( group ); fix( group );
		f.read( element ); fix( element );
		tag = ( static_cast<unsigned long>( group ) << 16 ) | element;
		//エンディアンの判定
		if ( checkReverse ) {
			unsigned short rg = forceReverse( group ), re = forceReverse( element );
			unsigned long rt = ( static_cast<unsigned long>( rg ) << 16 ) | re;
			if ( strlen( valueRepresentation( tag ) ) && 
				strlen( valueRepresentation( rt ) ) ) {
				//両方定義されていて判別不可能
			} else if ( strlen( valueRepresentation( tag ) ) == 0 && 
				strlen( valueRepresentation( rt ) ) == 0 ) {
				//両方定義されていなくて判別不可能
			} else if ( strlen( valueRepresentation( rt ) ) ) {//反転する
				reverse = ! reverse;
				group = rg; element = re; tag = rt;
				checkReverse = false;
			} else {
				checkReverse = false;
			}
		}
		sprintf( str, "%04X,%04X", group, element );
	}
	//タグを書き込む
	void writeTag( file & f, unsigned short group, unsigned short element )
	{
		fix( group );
		f.write( group ); 
		fix( element );
		f.write( element ); 
	}
	//VR長を読み込む
	bool readVRLength( file & f, const unsigned long tag, char * vr, unsigned long & length )
	{
		f.read( vr, 2 );//Value Representationをよむ
		vr[2] = '\0';

		//明示的VRか
		bool explicitVR = valueRepresentation( vr );
		bool two = false;
		if ( explicitVR ) {
			//拡張サイズ
			if ( extendedValueRepresentation( vr ) ) {
				f.seek( 2, SEEK_CUR );
				f.read( length );
				fix( length );
			} else {
				unsigned short us;
				f.read( us );
				length = fix( us );
				two = true;
			}
		} else {
			//暗黙的VR
			strcpy( vr, valueRepresentation( tag ) );
			f.seek( -2, SEEK_CUR );
			f.read( length );
			fix( length );
		}

		//エンディアンの判定
		if ( checkReverse ) {
			unsigned long rl = forceReverse( length );
			if ( two ) {
				unsigned short us = length;
				fix( us );
				rl = us;
			}
			if ( rl < length ) {
				reverse = ! reverse;
				checkReverse = false;
				return false;//強制で，前に戻ってやり直し
			}
		}
		return true;
	}
	//VR長を書き込む
	void writeVRLength( file & f, char * vr, unsigned long length )
	{
		//明示的VRか
		if ( strlen( vr ) == 2 ) {//暗黙的な場合には、説明用の文字が入っている
			f.write( vr, 2 );
			//拡張サイズ
			if ( extendedValueRepresentation( vr ) ) {
				unsigned short zero = 0;//2バイトの０を入れる
				f.write( zero );
				fix( length );
				f.write( length );
			} else {//拡張でない場合には２バイトの長さを入れる
				unsigned short us = static_cast<unsigned short>( length & 0xFFFF );
				fix( us );
				f.write( us );
			}
		} else {//暗黙的VRならば4バイトの長さを入れる
			//暗黙的VR
			fix( length );
			f.write( length );
		}
	}
	int readValueByVR( file & f, const char * vr, const int length, string & value )
	{
		if ( strstr( vr, "US" ) && 2 <= length ) {
			unsigned short us;
			f.read( us ); fix( us );
			value = string( int( us ) );
			return 2;
		} else if ( strstr( vr, "SS" ) && 2 <= length ) {
			short ss;
			f.read( ss ); fix( ss );
			value = string( int( ss ) );
			return 2;
		} else if ( ( strstr( vr, "UL" ) || strstr( vr, "AT" ) ) && 4 <= length ) {
			unsigned long ul;
			f.read( ul ); fix( ul );
			value = string( int( ul ) );
			return 4;
		} else if ( strstr( vr, "SL" ) && 4 <= length ) {
			long sl;
			f.read( sl ); fix( sl );
			value = string( int( sl ) );
			return 4;
		} else if ( strstr( vr, "FL" ) && 4 <= length ) {
			float flt;//4byte
			char four[4];
			f.read( four, 4 );
			flt = *reinterpret_cast<float *>( four );
			value = string( flt );
			return 4;
		} else if ( strstr( vr, "FD" ) && length == 8 ) {
			double dbl;//8byte
			char eight[8];
			f.read( eight, 8 );
			dbl = *reinterpret_cast<double *>( eight );
			value = string( dbl );
			return 8;
		} else if ( stringValueRepresentation( vr ) ) {
			value.allocate( length + 1 );
			f.read( value, length );
			value[length] = '\0';
			return length;
		}
		return 0;
	}
public:
	//XMLへ解析する
	bool convert( file & f, IXMLDOMDocumentPtr & domDocument )
	{
		domDocument->appendChild( domDocument->createProcessingInstruction( "xml", "version='1.0' encoding='Shift_JIS'" ) );
		IXMLDOMElementPtr domRoot = createChildElement( domDocument, "dicom" );

		reverse = true;
		// dicomファイルの中で，MacBin(128バイトオフセット)か，識別子無しかを判断する
		char id[1024];
		f.read( id, 4 );
		checkReverse = true;
		if ( memcmp( id, "DICM", 4 ) ) {
			f.seek( 128, SEEK_SET ); // MacBin対応
			f.read( id, 4 );
			if ( memcmp( id, "DICM", 4 ) ) {
				f.seek( 0, SEEK_SET );//最初からタグで入っている
				// Windows or Unix
			} else {
				// Mac Bin
				checkReverse = false;
				reverse = false;
			}
		} else {
			// Windows or Unix
		}
		if ( readDataSet( f, domRoot, 0xFFFFFFFF ) == 0 ) return false;
		return true;
	}
	//データを読み取る
	bool convert( file & f, nodeType & doc )
	{
        doc.name = "dicom";
		reverse = true;//エンディアンの設定.
		// dicomファイルの中で，MacBin(128バイトオフセット)か，識別子無しかを判断する
		char id[1024];
		f.read( id, 4 );
		checkReverse = true;
		if ( memcmp( id, "DICM", 4 ) ) {
			f.seek( 128, SEEK_SET ); // MacBin対応
			f.read( id, 4 );
			if ( memcmp( id, "DICM", 4 ) ) {
				f.seek( 0, SEEK_SET );//最初からタグで入っている
				// Windows or Unix
			} else {
				// Mac Bin
				checkReverse = false;
				reverse = false;
			}
		} else {
			// Windows or Unix
		}
		return readDataSet( f, doc, 0xFFFFFFFF ) != 0;
	}
	//解析後のXMLを出力
	bool export_( file & f, const char * outname )
	{
		IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
		if ( convert( f, domDocument ) ) {
			try {
				domDocument->save( _bstr_t( outname ) );
			} catch (...) {
				//セーブエラーはメッセージを表示するのみとする
			}
		}
		return true;
	}
	//ファイルから画像情報読み込み（XMLに解析して）
	bool read( file & f, int & width, int & height, int & depth, int & bitsAllocated, int & bitsStored, int & highBits, int & pixelRepresentation, int & imageOffset, int & rescaleIntercept, int & rescaleSlope, double & fieldOfViewMM )
	{
        nodeType doc;
		if ( convert( f, doc ) ) {
			return read( doc, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset, rescaleIntercept, rescaleSlope, fieldOfViewMM );
		}
		return false;
	}
	//解析後のDOMから画像情報読み込み
	bool read( IXMLDOMDocumentPtr & domDocument, int & width, int & height, int & depth, int & bitsAllocated, int & bitsStored, int & highBits, int & pixelRepresentation, int & imageOffset, int & rescaleIntercept, int & rescaleSlope, double & fieldOfViewMM )
	{
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		IXMLDOMNodeListPtr elementsList = domRoot->getElementsByTagName( _bstr_t( "element" ) );
		long length = elementsList->length;
		for ( int i = 0; i < length; ++i ) {
			IXMLDOMNodePtr & item = elementsList->item[i];
			IXMLDOMElementPtr element = static_cast<IXMLDOMElementPtr>( item );
			string name = getAttribute( element, "name" );
			if ( name == "列数" ) {
				width = getAttribute( element, "value" ).toi();
			} else if ( name == "行数" ) {
				height = getAttribute( element, "value" ).toi();
			} else if ( name == "割り当てビット" ) {
				bitsAllocated = getAttribute( element, "value" ).toi();
			} else if ( name == "格納ビット" ) {
				bitsStored = getAttribute( element, "value" ).toi();
			} else if ( name == "高位ビット" ) {
				highBits = getAttribute( element, "value" ).toi();
			} else if ( name == "画素表現" ) {
				pixelRepresentation = getAttribute( element, "value" ).toi();
			} else if ( name == "画素データ" ) {
				imageOffset = getAttribute( element, "value" ).toi();
			} else if ( name == "リスケール切片" ) {
				rescaleIntercept = getAttribute( element, "value" ).toi();
			} else if ( name == "リスケール傾斜" ) {
				rescaleSlope = getAttribute( element, "value" ).toi();
			} else if ( name == "画素間隔" ) {
				fieldOfViewMM = getAttribute( element, "value" ).tof();
			}
		}
		//無くてもいい物は省略できる用にしておく
		if ( bitsStored == -1 ) bitsStored = bitsAllocated;
		if ( highBits == -1 ) highBits = bitsStored - 1;
		//必要なノードがあるかどうか調べる
		if ( 0 <= imageOffset && 0 < width && 0 < height &&
			( bitsAllocated == 8 || bitsAllocated == 16 ) && 0 < bitsAllocated / 8 && 
			bitsStored <= bitsAllocated && highBits + 1 <= bitsStored &&
            rescaleSlope != 0 ) {
			return true;
		}
		return false;
	}
	//解析後のDOMから画像情報読み込み
	bool read( nodeType & parent, int & width, int & height, int & depth, int & bitsAllocated, int & bitsStored, int & highBits, int & pixelRepresentation, int & imageOffset, int & rescaleIntercept, int & rescaleSlope, double & fieldOfViewMM )
	{
        for ( std::list<nodeType>::iterator it = parent.children.begin(), sentinel = parent.children.end(); it != sentinel; ++it ) {
	        read( *it, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset, rescaleIntercept, rescaleSlope, fieldOfViewMM );
        }
        if ( parent.name == "element" ) {
            std::map<std::string,std::string>::iterator name_it = parent.attributes.find( "name" );
            std::map<std::string,std::string>::iterator value_it = parent.attributes.find( "value" );
            std::map<std::string,std::string>::iterator sentinel = parent.attributes.end();
            if ( name_it != sentinel && value_it != sentinel ) {
                const std::string & name = name_it->second;
                const std::string & value = value_it->second;
			    if ( name == "列数" ) {
				    width = string( value.c_str() ).toi();
			    } else if ( name == "行数" ) {
				    height = string( value.c_str() ).toi();
			    } else if ( name == "割り当てビット" ) {
				    bitsAllocated = string( value.c_str() ).toi();
			    } else if ( name == "格納ビット" ) {
				    bitsStored = string( value.c_str() ).toi();
			    } else if ( name == "高位ビット" ) {
				    highBits = string( value.c_str() ).toi();
			    } else if ( name == "画素表現" ) {
				    pixelRepresentation = string( value.c_str() ).toi();
			    } else if ( name == "画素データ" ) {
				    imageOffset = string( value.c_str() ).toi();
			    } else if ( name == "リスケール切片" ) {
				    rescaleIntercept = string( value.c_str() ).toi();
			    } else if ( name == "リスケール傾斜" ) {
				    rescaleSlope = string( value.c_str() ).toi();
			    } else if ( name == "画素間隔" ) {
				    fieldOfViewMM = string( value.c_str() ).tof();
			    }
            }
		}
        if ( parent.name == "dicom" ) {
		    //無くてもいい物は省略できる用にしておく
		    if ( bitsStored == -1 ) bitsStored = bitsAllocated;
		    if ( highBits == -1 ) highBits = bitsStored - 1;
		    //必要なノードがあるかどうか調べる
		    if ( 0 <= imageOffset && 0 < width && 0 < height &&
			    ( bitsAllocated == 8 || bitsAllocated == 16 ) && 0 < bitsAllocated / 8 && 
			    bitsStored <= bitsAllocated && highBits + 1 <= bitsStored &&
                rescaleSlope != 0 ) {
			    return true;
		    }
        }
		return false;
	}
	//解析後のXMLから値を探す
	bool read_value( IXMLDOMDocumentPtr & domDocument, const char * attribute, string & value )
	{
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		IXMLDOMNodeListPtr elementsList = domRoot->getElementsByTagName( _bstr_t( "element" ) );
		long length = elementsList->length;
		for ( int i = 0; i < length; ++i ) {
			IXMLDOMNodePtr & item = elementsList->item[i];
			IXMLDOMElementPtr element = static_cast<IXMLDOMElementPtr>( item );
			string name = getAttribute( element, "name" );
			if ( name == attribute ) {
				value = getAttribute( element, "value" );
				return true;
			}
		}
		return false;
	}
	bool read_value( IXMLDOMDocumentPtr & domDocument, const string & tag, int & value )
	{
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		IXMLDOMNodeListPtr elementsList = domRoot->getElementsByTagName( _bstr_t( "element" ) );
		long length = elementsList->length;
		for ( int i = 0; i < length; ++i ) {
			IXMLDOMNodePtr & item = elementsList->item[i];
			IXMLDOMElementPtr element = static_cast<IXMLDOMElementPtr>( item );
			string name = getAttribute( element, "name" );
			if ( name == tag ) {
				value = getAttribute( element, "value" ).toi();
                return true;
			}
		}
		return false;
	}
	bool read_value( IXMLDOMDocumentPtr & domDocument, const string & tag, decimal & value )
	{
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		IXMLDOMNodeListPtr elementsList = domRoot->getElementsByTagName( _bstr_t( "element" ) );
		long length = elementsList->length;
		for ( int i = 0; i < length; ++i ) {
			IXMLDOMNodePtr & item = elementsList->item[i];
			IXMLDOMElementPtr element = static_cast<IXMLDOMElementPtr>( item );
			string name = getAttribute( element, "name" );
			if ( name == tag ) {
				value = getAttribute( element, "value" ).tof();
                return true;
			}
		}
		return false;
	}
	bool read_value( IXMLDOMDocumentPtr & domDocument, const string & tag, double & value )
	{
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		IXMLDOMNodeListPtr elementsList = domRoot->getElementsByTagName( _bstr_t( "element" ) );
		long length = elementsList->length;
		for ( int i = 0; i < length; ++i ) {
			IXMLDOMNodePtr & item = elementsList->item[i];
			IXMLDOMElementPtr element = static_cast<IXMLDOMElementPtr>( item );
			string name = getAttribute( element, "name" );
			if ( name == tag ) {
				value = getAttribute( element, "value" ).tof();
                return true;
			}
		}
		return false;
	}
    bool read_value( nodeType & parent, const std::string & tag, int & value )
	{
        if ( parent.name == "element" ) {
            std::map<std::string,std::string>::iterator name_it = parent.attributes.find( "name" );
            std::map<std::string,std::string>::iterator value_it = parent.attributes.find( "value" );
            std::map<std::string,std::string>::iterator sentinel = parent.attributes.end();
            if ( name_it != sentinel && value_it != sentinel ) {
                if ( name_it->second == tag ) {
                    value = string( value_it->second.c_str() ).toi();
                    return true;
                }
            }
        }
        for ( std::list<nodeType>::iterator it = parent.children.begin(), sentinel = parent.children.end(); it != sentinel; ++it ) {
	        if ( read_value( *it, tag, value ) ) return true;
        }
		return false;
	}
    bool read_value( nodeType & parent, const std::string & tag, decimal & value )
	{
        if ( parent.name == "element" ) {
            std::map<std::string,std::string>::iterator name_it = parent.attributes.find( "name" );
            std::map<std::string,std::string>::iterator value_it = parent.attributes.find( "value" );
            std::map<std::string,std::string>::iterator sentinel = parent.attributes.end();
            if ( name_it != sentinel && value_it != sentinel ) {
                if ( name_it->second == tag ) {
                    value = string( value_it->second.c_str() ).tof();
                    return true;
                }
            }
        }
        for ( std::list<nodeType>::iterator it = parent.children.begin(), sentinel = parent.children.end(); it != sentinel; ++it ) {
	        if ( read_value( *it, tag, value ) ) return true;
        }
		return false;
	}
    bool read_value( nodeType & parent, const std::string & tag, double & value )
	{
        if ( parent.name == "element" ) {
            std::map<std::string,std::string>::iterator name_it = parent.attributes.find( "name" );
            std::map<std::string,std::string>::iterator value_it = parent.attributes.find( "value" );
            std::map<std::string,std::string>::iterator sentinel = parent.attributes.end();
            if ( name_it != sentinel && value_it != sentinel ) {
                if ( name_it->second == tag ) {
                    value = string( value_it->second.c_str() ).tof();
                    return true;
                }
            }
        }
        for ( std::list<nodeType>::iterator it = parent.children.begin(), sentinel = parent.children.end(); it != sentinel; ++it ) {
	        if ( read_value( *it, tag, value ) ) return true;
        }
		return false;
	}
    bool read_value( nodeType & parent, const std::string & tag, string & value )
	{
        if ( parent.name == "element" ) {
            std::map<std::string,std::string>::iterator name_it = parent.attributes.find( "name" );
            std::map<std::string,std::string>::iterator value_it = parent.attributes.find( "value" );
            std::map<std::string,std::string>::iterator sentinel = parent.attributes.end();
            if ( name_it != sentinel && value_it != sentinel ) {
                if ( name_it->second == tag ) {
                    value = value_it->second.c_str();
                    return true;
                }
            }
        }
        for ( std::list<nodeType>::iterator it = parent.children.begin(), sentinel = parent.children.end(); it != sentinel; ++it ) {
	        if ( read_value( *it, tag, value ) ) return true;
        }
		return false;
	}
	//個人情報を削除する
	bool removeInformation( file & f, file & out )
	{
		reverse = true;
		// dicomファイルの中で，MacBin(128バイトオフセット)か，識別子無しかを判断する
		char id[1024];
		f.read( id, 4 );
		checkReverse = true;
		if ( memcmp( id, "DICM", 4 ) ) {//DICMがない
			f.seek( 128, SEEK_SET ); // MacBin対応
			f.read( id, 4 );
			if ( memcmp( id, "DICM", 4 ) ) {//DICMがない
				f.seek( 0, SEEK_SET );//最初からタグで入っている
				// Windows or Unix
			} else {//DICMがあった
				// Mac Bin
				//Mac Binだが，想定不可能なばあいに対応・・・
				checkReverse = false;
				reverse = false;
				f.seek( 0, SEEK_SET ); // MacBin対応
				f.read( id, 132 );
				out.write( id, 132 );
			}
		} else {
			// Windows or Unix
			out.write( id, 4 );
		}
		if ( removeInformationDataSet( f, out, 0xFFFFFFFF ) == 0 ) return false;
		return true;
	}
private:
	//データセットを解析
	int removeInformationDataSet( file & f, file & out, unsigned long all_length )
	{
		int start_all_position = f.tell();
		int start_all_position_out = out.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_element_position = f.tell();
			readTag( f, tag, group, element, string_tag );//タグを読む

			if ( f.eof() ) break;//最後ならば、そのまま終わる

			char vr[1024];
			if ( ! readVRLength( f, tag, vr, length ) ) {
				//エンディアンを判別できたが遅かった
				f.seek( start_element_position, SEEK_SET );
				readTag( f, tag, group, element, string_tag );
				readVRLength( f, tag, vr, length );
			}
			writeTag( out, group, element );
			writeVRLength( out, vr, length );

			int start_position = f.tell();
			if ( tag == stItemDelimitationItem ) {//項目区切りタグがあると終了
				//データ長は０であるので下記のコードは実行されないはず
				if ( length ) {
					std::vector<char> value;
					value.resize( length );
					f.read( & value.front(), length );
					out.write( & value.front(), length );
				}
				break;
			}
			if ( strstr( vr, "SQ" ) ) {//シーケンスデータの場合
				int result = removeInformationSQ( f, out, length );
				if ( result == 0 && length == 0xFFFFFFFF ) return 0;
				if ( length == 0xFFFFFFFF ) length = result;
			} else {//その他のデータに関して
				std::vector<char> value;
				value.resize( length );
				f.read( & value.front(), length );

				//取り除く場合に、処理を行う

				out.write( & value.front(), length );
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//シーケンスを入力
	int removeInformationSQ( file & f, file & out, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			//タグを読み込む
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_item_position = f.tell();
			readTag( f, tag, group, element, string_tag );

			if ( f.eof() ) break;

			writeTag( out, group, element );

			//SQにはVRはない
			//4バイトの長さを読み込みそのまま出力
			f.read( length );
			out.write( length );
			fix( length );

			int start_position = f.tell();
			if ( tag == stItem ) {//項目タグ
				length = removeInformationDataSet( f, out, length );
			} else if ( tag == stSequenceDelimitationItem ) {//シーケンス区切りタグ（終了タグ）
				//データ長は０であるので下記のコードは実行されないはず
				if ( length ) {
					std::vector<char> value;
					value.resize( length );
					f.read( & value.front(), length );
					out.write( & value.front(), length );
				}
				break;//SQ終了
			} else {//明示的長さなはずだが，長さが指定されていない場合にここにくる
				return 0;//エラー
			}
			//処理が残っている文を処理
			int left_length = length - ( f.tell() - start_position );
			if ( left_length > 0 ) {//基本的にここにはこないはず
				if ( length != 0xFFFFFFFF ) {
					std::vector<char> value;
					value.resize( left_length );
					f.read( & value.front(), left_length );
					out.write( & value.front(), left_length );
				} else {
					//エラー
					return 0;
				}
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
public:
	//画像読み込み　メンバ関数のテンプレート化が出来ないので，必要な物を順次追加していく
	bool read( file & f, IXMLDOMDocumentPtr & domDocument, imageInterface< pixelLuminance<int16> > & img )
	{
		pixelLuminance<int16> p;
		int width = 0, height = 0, depth = 0;
		int bitsAllocated = -1, bitsStored = -1, highBits = -1, pixelRepresentation = 0;
		int imageOffset = -1;
        int rescaleIntercept = 0, rescaleSlope = 1;
        double fieldOfViewMM = 0;
		if ( read( domDocument, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset, rescaleIntercept, rescaleSlope, fieldOfViewMM ) ) {
			f.seek( imageOffset, SEEK_SET );
			img.create( width, height );
			uint32 mask = 0;
			for ( int i = 0; i < bitsStored; ++i ) mask = ( 1 | ( mask << 1 ) );
			int shift = ( highBits + 1 ) - bitsStored;
			if ( pixelRepresentation == 0 ) {
				switch ( bitsAllocated ) {
				case 8:
					{
						uint8 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk );
								p.y = uint8( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				case 16:
					{
						uint16 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk ); fix( wk );
								p.y = uint16( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				}
			} else {
				switch ( bitsAllocated ) {
				case 8:
					{
						int8 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk );
								p.y = int8( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				case 16:
					{
						int16 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk ); fix( wk );
								p.y = int16( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				}
			}
		}
		return true;
	}
	//画像読み込み
	bool read( file & f, imageInterface< pixelLuminance<int16> > & img )
	{
		pixelLuminance<int16> p;
		int width = 0, height = 0, depth = 0;
		int bitsAllocated = -1, bitsStored = -1, highBits = -1, pixelRepresentation = 0;
		int imageOffset = -1;
        int rescaleIntercept = 0, rescaleSlope = 1;
        double fieldOfViewMM = 0;
		if ( read( f, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset, rescaleIntercept, rescaleSlope, fieldOfViewMM ) ) {
			f.seek( imageOffset, SEEK_SET );
			img.create( width, height );
			uint32 mask = 0;
			for ( int i = 0; i < bitsStored; ++i ) mask = ( 1 | ( mask << 1 ) );
			int shift = ( highBits + 1 ) - bitsStored;
			if ( pixelRepresentation == 0 ) {
				switch ( bitsAllocated ) {
				case 8:
					{
						uint8 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk );
								p.y = uint8( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				case 16:
					{
						uint16 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk ); fix( wk );
								p.y = uint16( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				}
			} else {
				switch ( bitsAllocated ) {
				case 8:
					{
						int8 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk );
								p.y = int8( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				case 16:
					{
						int16 wk;
						for ( int y = 0; y < height; ++y ) {
							for ( int x = 0; x < width; ++x ) {
								f.read( wk ); fix( wk );
								p.y = int16( ( wk >> shift ) & mask );
                                p.y = rescaleSlope * p.y + rescaleIntercept;
								img.set( x, y, p );
							}
						}
					} break;
				}
			}
		}
		return true;
	}
};
