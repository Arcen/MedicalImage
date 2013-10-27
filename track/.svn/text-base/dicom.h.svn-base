#pragma once
#include "main.h"

#include <agl/kernel/agl.h>
using namespace agl;
//using namespace MSXML;

class dicom
{
public:
    typedef MSXML2::IXMLDOMDocumentPtr IXMLDOMDocumentPtr;
    typedef MSXML2::IXMLDOMElementPtr IXMLDOMElementPtr;
    typedef MSXML2::IXMLDOMAttributePtr IXMLDOMAttributePtr;
    typedef MSXML2::IXMLDOMTextPtr IXMLDOMTextPtr;
    typedef MSXML2::IXMLDOMNode IXMLDOMNode;
    typedef MSXML2::IXMLDOMNodePtr IXMLDOMNodePtr;
    typedef MSXML2::IXMLDOMNodeList IXMLDOMNodeList;
    typedef MSXML2::IXMLDOMNodeListPtr IXMLDOMNodeListPtr;
    
	struct dicomTag {
		unsigned short group; // 50xx:, 0x60xx: この二つは不定でもいい
		unsigned short element; // 0020:31xx これも不定でいい
		char comment[1024];
		char name[1024];
		char valueRepresentation[1024];
		bool valueMultiplicity;
	};
	struct dicomUID {
		char uid[1024];
		char name[1024];
		char comment[1024];
		char type[1024];
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
	//シングルトンにより生成
	static dicom & get()
	{
		static dicom dcm;
		return dcm;
	}
private:
	dicom() : reverse( true ), checkReverse( true )
	{
		//タグ,UIDの初期化
		initializeTag();
		initializeUID();
	}
private:
	void initializeTag();
	void initializeUID();
	unsigned short forceReverse( unsigned short us )
	{
		return ( ( us & 0xff ) << 8 ) |
				( ( us & 0xff00 ) >> 8 );
	}
	unsigned long forceReverse( unsigned long ul )
	{
		return ( ( ul & 0xff ) << 24 ) |
				( ( ul & 0xff00 ) << 8 ) |
				( ( ul & 0xff0000 ) >> 8 ) |
				( ( ul & 0xff000000 ) >> 24 );
	}
	short forceReverse( short s )
	{
		return forceReverse( * reinterpret_cast<unsigned short*>( & s ) );
	}
	long forceReverse( long & l )
	{
		return forceReverse( * reinterpret_cast<unsigned long*>( & l ) );
	}
	unsigned short fix( unsigned short & us )
	{
		if ( reverse ) {
			us = ( ( us & 0xff ) << 8 ) |
				( ( us & 0xff00 ) >> 8 );
		}
		return us;
	}
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
	short fix( short & s )
	{
		unsigned short us = * reinterpret_cast<unsigned short*>( & s );
		fix( us );
		s = * reinterpret_cast<short*>( & us );
		return s;
	}
	long fix( long & l )
	{
		unsigned short ul = * reinterpret_cast<unsigned long*>( & l );
		fix( ul );
		l = * reinterpret_cast<long*>( & ul );
		return l;
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
	//型形式が合っているか
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
	//拡張型形式か
	bool extendedValueRepresentation( const char * vr )
	{
		static const char vrs[][3] = { "OB", "OW", "SQ", "UN" };
		for ( int i = 0; i < sizeof( vrs ) / 3; ++i ) {
			if ( memcmp( vrs[i], vr, 2 ) == 0 ) return true;
		}
		return false;
	}
	//データが文字列型か
	bool stringValueRepresentation( const char * vr )
	{
		if ( ! strlen( vr ) ) return false;
		static const char vrs[][3] = { 
			"AE", "AS", "CS", "DA", "DS", "DT", "IS", "LO", "LT", "SH", "ST", "TM", 
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
	//シーケンスを出力
	int exportSQ( file & f, IXMLDOMElementPtr & parent, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
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
				length = exportDataSet( f, item, length );
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
	int exportDataSet( file & f, IXMLDOMElementPtr & parent, unsigned long all_length )
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
			

			const char * tag_name = tagName( tag );
			setAttribute( domNode, "offset", string( start_element_position ) );
			setAttribute( domNode, "tag", string_tag );
			setAttribute( domNode, "name", tag_name );
			setAttribute( domNode, "vr", vr );
			setAttribute( domNode, "length", string( int( length ) ) );

			static int done_position = -1;
			int current_position = f.tell();
			if ( current_position < done_position || current_position - done_position > 1024 * 128 ) {
				char wk[1024];
				sprintf(wk,"DICOM処理中(%d[byte])(%s)",current_position,tag_name);
				statusBar->set( wk );
				done_position = current_position;
			}

			int start_position = f.tell();
			if ( tag == stItemDelimitationItem ) {//項目区切りタグがあると終了
				f.seek( start_position + length, SEEK_SET );
				break;
			}

			if ( strstr( vr, "SQ" ) ) {
				int result = exportSQ( f, domNode, length );
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
				} else {
					setAttribute( domNode, "value", "-0" );
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
			} else if ( strlen( valueRepresentation( rt ) ) ) {
				reverse = ! reverse;
				group = rg; element = re; tag = rt;
				checkReverse = false;
			} else {
				checkReverse = false;
			}
		}
		sprintf( str, "%04X,%04X", group, element );
	}
	bool readVRLength( file & f, const unsigned long tag, char * vr, unsigned long & length )
	{
		f.read( vr, 2 );
		vr[2] = '\0';

		//明示的VRか
		bool explicitVR = valueRepresentation( vr );
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
			if ( rl < length ) {
				reverse = ! reverse;
				checkReverse = false;
				return false;//強制で，前に戻ってやり直し
			}
		}
		return true;
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
		if ( exportDataSet( f, domRoot, 0xFFFFFFFF ) == 0 ) return false;
		return true;
	}
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
	bool getAttribute( IXMLDOMDocumentPtr & domDocument, const char * attribute, int & value )
	{
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		IXMLDOMNodeListPtr elementsList = domRoot->getElementsByTagName( _bstr_t( "element" ) );
		long length = elementsList->length;
		for ( int i = 0; i < length; ++i ) {
			IXMLDOMNodePtr & item = elementsList->item[i];
			IXMLDOMElementPtr element = static_cast<IXMLDOMElementPtr>( item );
			string name = getAttribute( element, "name" );
			if ( name == attribute ) {
				string valueString = getAttribute( element, "value" );
				if ( ! valueString.length() ) return false;
				value = valueString.toi();
				return true;
			}
		}
		return false;
	}
	bool getAttribute( IXMLDOMDocumentPtr & domDocument, const char * attribute, string & value )
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
	bool read( file & f, int & width, int & height, int & depth, int & bitsAllocated, int & bitsStored, int & highBits, int & pixelRepresentation, int & imageOffset )
	{
		IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
		if ( convert( f, domDocument ) ) {
			return read( domDocument, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset );
		}
		return false;
	}
	bool read( IXMLDOMDocumentPtr & domDocument, int & width, int & height, int & depth, int & bitsAllocated, int & bitsStored, int & highBits, int & pixelRepresentation, int & imageOffset )
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
			}
		}
		//無くてもいい物は省略できる用にしておく
		if ( bitsStored == -1 ) bitsStored = bitsAllocated;
		if ( highBits == -1 ) highBits = bitsStored - 1;
		//必要なノードがあるかどうか調べる
		if ( 0 <= imageOffset && 0 < width && 0 < height &&
			( bitsAllocated == 8 || bitsAllocated == 16 ) && 0 < bitsAllocated / 8 && 
			bitsStored <= bitsAllocated && highBits + 1 <= bitsStored ) {
			return true;
		}
		return false;
	}
	//読み込み　メンバ関数のテンプレート化が出来ないので，必要な物を順次追加していく
	bool read( file & f, IXMLDOMDocumentPtr & domDocument, imageInterface< pixelLuminance<int16> > & img )
	{
		pixelLuminance<int16> p;
		int width = 0, height = 0, depth = 0;
		int bitsAllocated = -1, bitsStored = -1, highBits = -1, pixelRepresentation = 0;
		int imageOffset = -1;
		if ( read( domDocument, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset ) ) {
			f.seek( imageOffset, SEEK_SET );
			img.create( width, height );
			uint32 mask = 0;
			string value;
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
								img.set( x, y, p );
							}
						}
					} break;
				}
			}
		}
		return true;
	}
	bool read( file & f, imageInterface< pixelLuminance<int16> > & img )
	{
		pixelLuminance<int16> p;
		int width = 0, height = 0, depth = 0;
		int bitsAllocated = -1, bitsStored = -1, highBits = -1, pixelRepresentation = 0;
		int imageOffset = -1;
		if ( ! read( f, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset ) ) return false;

		f.seek( imageOffset, SEEK_SET );
		img.create( width, height );
		uint32 mask = 0;
		for ( int i = 0; i < bitsStored; ++i ) mask = ( 1 | ( mask << 1 ) );
		int shift = bitsAllocated - ( highBits + 1 );
		if ( pixelRepresentation == 0 ) {
			switch ( bitsAllocated ) {
			case 8:
				{
					uint8 wk;
					for ( int y = 0; y < height; ++y ) {
						for ( int x = 0; x < width; ++x ) {
							f.read( wk );
							p.y = uint8( ( wk & mask ) << shift );
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
							p.y = uint16( ( wk & mask ) << shift );
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
							p.y = int8( ( wk & mask ) << shift );
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
							p.y = int16( wk & mask );
							img.set( x, y, p );
						}
					}
				} break;
			}
		}
		return true;
	}
};
