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
//�^�O�̖���
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
		//�^�O,UID�̏�����
		initializeTag( tags );
		initializeUID( UIDs );
	}
private:
	//�����o�C�g���]
	unsigned short forceReverse( unsigned short us )
	{
		return ( ( us & 0xff ) << 8 ) |
				( ( us & 0xff00 ) >> 8 );
	}
	//�����o�C�g���]
	unsigned long forceReverse( unsigned long ul )
	{
		return ( ( ul & 0xff ) << 24 ) |
				( ( ul & 0xff00 ) << 8 ) |
				( ( ul & 0xff0000 ) >> 8 ) |
				( ( ul & 0xff000000 ) >> 24 );
	}
	//�����o�C�g���]
	short forceReverse( short s )
	{
		return forceReverse( * reinterpret_cast<unsigned short*>( & s ) );
	}
	//�����o�C�g���]
	long forceReverse( long & l )
	{
		return forceReverse( * reinterpret_cast<unsigned long*>( & l ) );
	}
	//�o�C�g���]���K�v�Ȃ���s
	unsigned short fix( unsigned short & us )
	{
		if ( reverse ) {
			us = ( ( us & 0xff ) << 8 ) |
				( ( us & 0xff00 ) >> 8 );
		}
		return us;
	}
	//�o�C�g���]���K�v�Ȃ���s
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
	//�o�C�g���]���K�v�Ȃ���s
	short fix( short & s )
	{
		unsigned short us = * reinterpret_cast<unsigned short*>( & s );
		fix( us );
		s = * reinterpret_cast<short*>( & us );
		return s;
	}
	//�o�C�g���]���K�v�Ȃ���s
	long fix( long & l )
	{
		unsigned short ul = * reinterpret_cast<unsigned long*>( & l );
		fix( ul );
		l = * reinterpret_cast<long*>( & ul );
		return l;
	}
	//�o�C�g���]���K�v�Ȃ���s
	long fix( float & f )
	{
		return fix( reinterpret_cast<long&>( f ) );
	}
	//�o�C�g���]���K�v�Ȃ���s
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
	// XML�֘A
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
	// DICOM�֘A
	//�^�O�̖��̂��擾
	const char * tagName( unsigned long tag )
	{
		unsigned short group = ( tag & 0xffff0000 ) >> 16;
		unsigned short element = ( tag & 0xffff );
		//�s��̕����������I�ɕύX
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
		//element���O�Ȃ�΃f�[�^���ł���Ǝv����
		if ( element == 0 ) return tagName( static_cast<unsigned long>( element ) );
		return "";
	}
	//�^�`�����擾
	const char * valueRepresentation( unsigned long tag )
	{
		unsigned short group = ( tag & 0xffff0000 ) >> 16;
		unsigned short element = ( tag & 0xffff );
		//�s��̕����������I�ɕύX
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
		//element���O�Ȃ�΃f�[�^���ł���Ǝv����
		if ( element == 0 ) return valueRepresentation( static_cast<unsigned long>( element ) );
		return "";
	}
	//�^�`���������Ă��邩���ׂ�
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
	//�g���^�`�������ׂ�
	bool extendedValueRepresentation( const char * vr )
	{
		static const char vrs[][3] = { "OB", "OW", "SQ", "UN" };
		for ( int i = 0; i < sizeof( vrs ) / 3; ++i ) {
			if ( memcmp( vrs[i], vr, 2 ) == 0 ) return true;
		}
		return false;
	}
	//�f�[�^��������^�����ׂ�
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
	//UID���擾
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
	//�V�[�P���X�����
	int readSQ( file & f, IXMLDOMElementPtr & parent, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			//�^�O��ǂݍ���
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_item_position = f.tell();
			readTag( f, tag, group, element, string_tag );

			if ( f.eof() ) break;

			IXMLDOMElementPtr item = createChildElement( parent, "item" );
			setAttribute( item, "offset", string( start_item_position ) );
			
			//SQ�ɂ�VR�͂Ȃ�
			f.read( length ); fix( length );

			setAttribute( item, "tag", string_tag );
			setAttribute( item, "name", tagName( tag ) );
			setAttribute( item, "length", string( int( length ) ) );

			int start_position = f.tell();
			if ( tag == stItem ) {//���ڃ^�O
				length = readDataSet( f, item, length );
			} else if ( tag == stSequenceDelimitationItem ) {//�V�[�P���X��؂�^�O�i�I���^�O�j
				f.seek( start_position + length, SEEK_SET );
				break;//SQ�I��
			} else {
				return 0;
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//�f�[�^�Z�b�g���o��
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
				//�G���f�B�A���𔻕ʂł������x������
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
			if ( tag == stItemDelimitationItem ) {//���ڋ�؂�^�O������ƏI��
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
					//value��UID������ꍇ�C�Ӗ����t��������
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
	//�V�[�P���X�����
	int readSQ( file & f, nodeType & parent, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			//�^�O��ǂݍ���
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
			
			//SQ�ɂ�VR�͂Ȃ�
			f.read( length ); fix( length );

            item.attributes["tag"] = string_tag;
            item.attributes["name"] = tagName( tag );
            item.attributes["length"] = string( int( length ) ).chars();

			int start_position = f.tell();
			if ( tag == stItem ) {//���ڃ^�O
				length = readDataSet( f, item, length );
			} else if ( tag == stSequenceDelimitationItem ) {//�V�[�P���X��؂�^�O�i�I���^�O�j
				f.seek( start_position + length, SEEK_SET );
				break;//SQ�I��
			} else {
				return 0;
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//�f�[�^�Z�b�g���o��
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
				//�G���f�B�A���𔻕ʂł������x������
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
			if ( tag == stItemDelimitationItem ) {//���ڋ�؂�^�O������ƏI��
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
					//value��UID������ꍇ�C�Ӗ����t��������
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
	//�^�O��ǂݍ���
	void readTag( file & f, unsigned long & tag, unsigned short & group, unsigned short & element, char * str )
	{
		f.read( group ); fix( group );
		f.read( element ); fix( element );
		tag = ( static_cast<unsigned long>( group ) << 16 ) | element;
		//�G���f�B�A���̔���
		if ( checkReverse ) {
			unsigned short rg = forceReverse( group ), re = forceReverse( element );
			unsigned long rt = ( static_cast<unsigned long>( rg ) << 16 ) | re;
			if ( strlen( valueRepresentation( tag ) ) && 
				strlen( valueRepresentation( rt ) ) ) {
				//������`����Ă��Ĕ��ʕs�\
			} else if ( strlen( valueRepresentation( tag ) ) == 0 && 
				strlen( valueRepresentation( rt ) ) == 0 ) {
				//������`����Ă��Ȃ��Ĕ��ʕs�\
			} else if ( strlen( valueRepresentation( rt ) ) ) {//���]����
				reverse = ! reverse;
				group = rg; element = re; tag = rt;
				checkReverse = false;
			} else {
				checkReverse = false;
			}
		}
		sprintf( str, "%04X,%04X", group, element );
	}
	//�^�O����������
	void writeTag( file & f, unsigned short group, unsigned short element )
	{
		fix( group );
		f.write( group ); 
		fix( element );
		f.write( element ); 
	}
	//VR����ǂݍ���
	bool readVRLength( file & f, const unsigned long tag, char * vr, unsigned long & length )
	{
		f.read( vr, 2 );//Value Representation�����
		vr[2] = '\0';

		//�����IVR��
		bool explicitVR = valueRepresentation( vr );
		bool two = false;
		if ( explicitVR ) {
			//�g���T�C�Y
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
			//�ÖٓIVR
			strcpy( vr, valueRepresentation( tag ) );
			f.seek( -2, SEEK_CUR );
			f.read( length );
			fix( length );
		}

		//�G���f�B�A���̔���
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
				return false;//�����ŁC�O�ɖ߂��Ă�蒼��
			}
		}
		return true;
	}
	//VR������������
	void writeVRLength( file & f, char * vr, unsigned long length )
	{
		//�����IVR��
		if ( strlen( vr ) == 2 ) {//�ÖٓI�ȏꍇ�ɂ́A�����p�̕����������Ă���
			f.write( vr, 2 );
			//�g���T�C�Y
			if ( extendedValueRepresentation( vr ) ) {
				unsigned short zero = 0;//2�o�C�g�̂O������
				f.write( zero );
				fix( length );
				f.write( length );
			} else {//�g���łȂ��ꍇ�ɂ͂Q�o�C�g�̒���������
				unsigned short us = static_cast<unsigned short>( length & 0xFFFF );
				fix( us );
				f.write( us );
			}
		} else {//�ÖٓIVR�Ȃ��4�o�C�g�̒���������
			//�ÖٓIVR
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
	//XML�։�͂���
	bool convert( file & f, IXMLDOMDocumentPtr & domDocument )
	{
		domDocument->appendChild( domDocument->createProcessingInstruction( "xml", "version='1.0' encoding='Shift_JIS'" ) );
		IXMLDOMElementPtr domRoot = createChildElement( domDocument, "dicom" );

		reverse = true;
		// dicom�t�@�C���̒��ŁCMacBin(128�o�C�g�I�t�Z�b�g)���C���ʎq�������𔻒f����
		char id[1024];
		f.read( id, 4 );
		checkReverse = true;
		if ( memcmp( id, "DICM", 4 ) ) {
			f.seek( 128, SEEK_SET ); // MacBin�Ή�
			f.read( id, 4 );
			if ( memcmp( id, "DICM", 4 ) ) {
				f.seek( 0, SEEK_SET );//�ŏ�����^�O�œ����Ă���
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
	//�f�[�^��ǂݎ��
	bool convert( file & f, nodeType & doc )
	{
        doc.name = "dicom";
		reverse = true;//�G���f�B�A���̐ݒ�.
		// dicom�t�@�C���̒��ŁCMacBin(128�o�C�g�I�t�Z�b�g)���C���ʎq�������𔻒f����
		char id[1024];
		f.read( id, 4 );
		checkReverse = true;
		if ( memcmp( id, "DICM", 4 ) ) {
			f.seek( 128, SEEK_SET ); // MacBin�Ή�
			f.read( id, 4 );
			if ( memcmp( id, "DICM", 4 ) ) {
				f.seek( 0, SEEK_SET );//�ŏ�����^�O�œ����Ă���
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
	//��͌��XML���o��
	bool export_( file & f, const char * outname )
	{
		IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
		if ( convert( f, domDocument ) ) {
			try {
				domDocument->save( _bstr_t( outname ) );
			} catch (...) {
				//�Z�[�u�G���[�̓��b�Z�[�W��\������݂̂Ƃ���
			}
		}
		return true;
	}
	//�t�@�C������摜���ǂݍ��݁iXML�ɉ�͂��āj
	bool read( file & f, int & width, int & height, int & depth, int & bitsAllocated, int & bitsStored, int & highBits, int & pixelRepresentation, int & imageOffset, int & rescaleIntercept, int & rescaleSlope, double & fieldOfViewMM )
	{
        nodeType doc;
		if ( convert( f, doc ) ) {
			return read( doc, width, height, depth, bitsAllocated, bitsStored, highBits, pixelRepresentation, imageOffset, rescaleIntercept, rescaleSlope, fieldOfViewMM );
		}
		return false;
	}
	//��͌��DOM����摜���ǂݍ���
	bool read( IXMLDOMDocumentPtr & domDocument, int & width, int & height, int & depth, int & bitsAllocated, int & bitsStored, int & highBits, int & pixelRepresentation, int & imageOffset, int & rescaleIntercept, int & rescaleSlope, double & fieldOfViewMM )
	{
		IXMLDOMElementPtr domRoot = domDocument->documentElement;
		IXMLDOMNodeListPtr elementsList = domRoot->getElementsByTagName( _bstr_t( "element" ) );
		long length = elementsList->length;
		for ( int i = 0; i < length; ++i ) {
			IXMLDOMNodePtr & item = elementsList->item[i];
			IXMLDOMElementPtr element = static_cast<IXMLDOMElementPtr>( item );
			string name = getAttribute( element, "name" );
			if ( name == "��" ) {
				width = getAttribute( element, "value" ).toi();
			} else if ( name == "�s��" ) {
				height = getAttribute( element, "value" ).toi();
			} else if ( name == "���蓖�ăr�b�g" ) {
				bitsAllocated = getAttribute( element, "value" ).toi();
			} else if ( name == "�i�[�r�b�g" ) {
				bitsStored = getAttribute( element, "value" ).toi();
			} else if ( name == "���ʃr�b�g" ) {
				highBits = getAttribute( element, "value" ).toi();
			} else if ( name == "��f�\��" ) {
				pixelRepresentation = getAttribute( element, "value" ).toi();
			} else if ( name == "��f�f�[�^" ) {
				imageOffset = getAttribute( element, "value" ).toi();
			} else if ( name == "���X�P�[���ؕ�" ) {
				rescaleIntercept = getAttribute( element, "value" ).toi();
			} else if ( name == "���X�P�[���X��" ) {
				rescaleSlope = getAttribute( element, "value" ).toi();
			} else if ( name == "��f�Ԋu" ) {
				fieldOfViewMM = getAttribute( element, "value" ).tof();
			}
		}
		//�����Ă��������͏ȗ��ł���p�ɂ��Ă���
		if ( bitsStored == -1 ) bitsStored = bitsAllocated;
		if ( highBits == -1 ) highBits = bitsStored - 1;
		//�K�v�ȃm�[�h�����邩�ǂ������ׂ�
		if ( 0 <= imageOffset && 0 < width && 0 < height &&
			( bitsAllocated == 8 || bitsAllocated == 16 ) && 0 < bitsAllocated / 8 && 
			bitsStored <= bitsAllocated && highBits + 1 <= bitsStored &&
            rescaleSlope != 0 ) {
			return true;
		}
		return false;
	}
	//��͌��DOM����摜���ǂݍ���
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
			    if ( name == "��" ) {
				    width = string( value.c_str() ).toi();
			    } else if ( name == "�s��" ) {
				    height = string( value.c_str() ).toi();
			    } else if ( name == "���蓖�ăr�b�g" ) {
				    bitsAllocated = string( value.c_str() ).toi();
			    } else if ( name == "�i�[�r�b�g" ) {
				    bitsStored = string( value.c_str() ).toi();
			    } else if ( name == "���ʃr�b�g" ) {
				    highBits = string( value.c_str() ).toi();
			    } else if ( name == "��f�\��" ) {
				    pixelRepresentation = string( value.c_str() ).toi();
			    } else if ( name == "��f�f�[�^" ) {
				    imageOffset = string( value.c_str() ).toi();
			    } else if ( name == "���X�P�[���ؕ�" ) {
				    rescaleIntercept = string( value.c_str() ).toi();
			    } else if ( name == "���X�P�[���X��" ) {
				    rescaleSlope = string( value.c_str() ).toi();
			    } else if ( name == "��f�Ԋu" ) {
				    fieldOfViewMM = string( value.c_str() ).tof();
			    }
            }
		}
        if ( parent.name == "dicom" ) {
		    //�����Ă��������͏ȗ��ł���p�ɂ��Ă���
		    if ( bitsStored == -1 ) bitsStored = bitsAllocated;
		    if ( highBits == -1 ) highBits = bitsStored - 1;
		    //�K�v�ȃm�[�h�����邩�ǂ������ׂ�
		    if ( 0 <= imageOffset && 0 < width && 0 < height &&
			    ( bitsAllocated == 8 || bitsAllocated == 16 ) && 0 < bitsAllocated / 8 && 
			    bitsStored <= bitsAllocated && highBits + 1 <= bitsStored &&
                rescaleSlope != 0 ) {
			    return true;
		    }
        }
		return false;
	}
	//��͌��XML����l��T��
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
	//�l�����폜����
	bool removeInformation( file & f, file & out )
	{
		reverse = true;
		// dicom�t�@�C���̒��ŁCMacBin(128�o�C�g�I�t�Z�b�g)���C���ʎq�������𔻒f����
		char id[1024];
		f.read( id, 4 );
		checkReverse = true;
		if ( memcmp( id, "DICM", 4 ) ) {//DICM���Ȃ�
			f.seek( 128, SEEK_SET ); // MacBin�Ή�
			f.read( id, 4 );
			if ( memcmp( id, "DICM", 4 ) ) {//DICM���Ȃ�
				f.seek( 0, SEEK_SET );//�ŏ�����^�O�œ����Ă���
				// Windows or Unix
			} else {//DICM��������
				// Mac Bin
				//Mac Bin�����C�z��s�\�Ȃ΂����ɑΉ��E�E�E
				checkReverse = false;
				reverse = false;
				f.seek( 0, SEEK_SET ); // MacBin�Ή�
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
	//�f�[�^�Z�b�g�����
	int removeInformationDataSet( file & f, file & out, unsigned long all_length )
	{
		int start_all_position = f.tell();
		int start_all_position_out = out.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_element_position = f.tell();
			readTag( f, tag, group, element, string_tag );//�^�O��ǂ�

			if ( f.eof() ) break;//�Ō�Ȃ�΁A���̂܂܏I���

			char vr[1024];
			if ( ! readVRLength( f, tag, vr, length ) ) {
				//�G���f�B�A���𔻕ʂł������x������
				f.seek( start_element_position, SEEK_SET );
				readTag( f, tag, group, element, string_tag );
				readVRLength( f, tag, vr, length );
			}
			writeTag( out, group, element );
			writeVRLength( out, vr, length );

			int start_position = f.tell();
			if ( tag == stItemDelimitationItem ) {//���ڋ�؂�^�O������ƏI��
				//�f�[�^���͂O�ł���̂ŉ��L�̃R�[�h�͎��s����Ȃ��͂�
				if ( length ) {
					std::vector<char> value;
					value.resize( length );
					f.read( & value.front(), length );
					out.write( & value.front(), length );
				}
				break;
			}
			if ( strstr( vr, "SQ" ) ) {//�V�[�P���X�f�[�^�̏ꍇ
				int result = removeInformationSQ( f, out, length );
				if ( result == 0 && length == 0xFFFFFFFF ) return 0;
				if ( length == 0xFFFFFFFF ) length = result;
			} else {//���̑��̃f�[�^�Ɋւ���
				std::vector<char> value;
				value.resize( length );
				f.read( & value.front(), length );

				//��菜���ꍇ�ɁA�������s��

				out.write( & value.front(), length );
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
	//�V�[�P���X�����
	int removeInformationSQ( file & f, file & out, unsigned long all_length )
	{
		int start_all_position = f.tell();
		while ( f.eof() == false && ( all_length == 0xFFFFFFFF || f.tell() - start_all_position < all_length ) ) {
			//�^�O��ǂݍ���
			unsigned long tag, length;
			unsigned short group, element;
			char string_tag[1024];
			int start_item_position = f.tell();
			readTag( f, tag, group, element, string_tag );

			if ( f.eof() ) break;

			writeTag( out, group, element );

			//SQ�ɂ�VR�͂Ȃ�
			//4�o�C�g�̒�����ǂݍ��݂��̂܂܏o��
			f.read( length );
			out.write( length );
			fix( length );

			int start_position = f.tell();
			if ( tag == stItem ) {//���ڃ^�O
				length = removeInformationDataSet( f, out, length );
			} else if ( tag == stSequenceDelimitationItem ) {//�V�[�P���X��؂�^�O�i�I���^�O�j
				//�f�[�^���͂O�ł���̂ŉ��L�̃R�[�h�͎��s����Ȃ��͂�
				if ( length ) {
					std::vector<char> value;
					value.resize( length );
					f.read( & value.front(), length );
					out.write( & value.front(), length );
				}
				break;//SQ�I��
			} else {//�����I�����Ȃ͂������C�������w�肳��Ă��Ȃ��ꍇ�ɂ����ɂ���
				return 0;//�G���[
			}
			//�������c���Ă��镶������
			int left_length = length - ( f.tell() - start_position );
			if ( left_length > 0 ) {//��{�I�ɂ����ɂ͂��Ȃ��͂�
				if ( length != 0xFFFFFFFF ) {
					std::vector<char> value;
					value.resize( left_length );
					f.read( & value.front(), left_length );
					out.write( & value.front(), left_length );
				} else {
					//�G���[
					return 0;
				}
			}
			f.seek( start_position + length, SEEK_SET );
		}
		int end_all_position = f.tell();
		return end_all_position - start_all_position;
	}
public:
	//�摜�ǂݍ��݁@�����o�֐��̃e���v���[�g�����o���Ȃ��̂ŁC�K�v�ȕ��������ǉ����Ă���
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
	//�摜�ǂݍ���
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
