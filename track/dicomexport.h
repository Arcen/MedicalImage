#pragma once
#include "main.h"

class dicomSeriesInformation
{
public:
	int series_id;
	int targetOffset;
	array<string> files;
	array<int> image_ids;
	double thickness;
	double fieldOfViewMM;
};

inline int dicomListup( const MSXML2::IXMLDOMElementPtr & root, int targetOffset, const string & fileprefix, array<dicomSeriesInformation> & output, dicomSeriesInformation * target = NULL, MSXML2::IXMLDOMElementPtr search_position = 0 )
{
	MSXML2::IXMLDOMElementPtr element;
	//�����i�f�B���N�g���̍��ځj
	if ( search_position ) {
		for ( element = search_position; element != NULL; element = element->nextSibling ) {
			MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "offset" ) );
			if ( attr ) {
				if ( atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) ) == targetOffset ) break;
			}
		}
	}
	if ( element ) {
	} else {
		for ( element = root->firstChild; element != NULL; element = element->nextSibling ) {
			MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "offset" ) );
			if ( attr ) {
				if ( atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) ) == targetOffset ) break;
			}
		}
	}
	if ( element ) {} else return 0;
	//�񋓁i���ړ��̃G�������g�ɂ��āj
	int nextSibling = 0, firstChild = 0;
	string itemName;
	int series_id = -1;
	int image_id = -1;
	string referencefilename;
	double thickness = 0;
	double width = 0, pixelsize = 0;
	for ( MSXML2::IXMLDOMElementPtr item = element->firstChild; item != NULL; item = item->nextSibling ) {
		MSXML2::IXMLDOMAttributePtr attr = item->getAttributeNode( _bstr_t( "name" ) );
		if ( attr ) {} else continue;
		string name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		if ( name == "���̃f�B���N�g�����R�[�h�̃I�t�Z�b�g" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			nextSibling = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "�Q�Ɖ��ʃf�B���N�g���G���e�B�e�B�̃I�t�Z�b�g" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			firstChild = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "�f�B���N�g�����R�[�h�^�C�v" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			itemName = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		} else if ( name == "�Q�ƃt�@�C���h�c" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			referencefilename = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		} else if ( name == "�X���C�X����" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			thickness = atof( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "��" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			width = atof( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "��f�Ԋu" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			pixelsize = atof( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "�V���[�Y�ԍ�" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			char * p = wk.chars();
			while ( *p && *p == ' ' ) ++p;
			series_id = atoi( p );
		} else if ( name == "�C���X�^���X�ԍ�" ) {
			attr = item->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			string wk = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			char * p = wk.chars();
			while ( *p && *p == ' ' ) ++p;
			image_id = atoi( p );
		}
	}
	dicomSeriesInformation * child_target = NULL;
	if ( itemName.length() >= 6 && memcmp( itemName.chars(), "SERIES", 6 ) == 0 ) {
		//�����V���[�Y�����邩�T��
		if ( series_id >= 0 ) {
			for ( int i = 0; i < output.size; ++i ) {
				if ( output[i].series_id == series_id ) {
					child_target = & output[i];
					child_target->targetOffset = targetOffset;
					child_target->series_id = series_id;
					break;
				}
			}
		}
		if ( ! child_target ) {
			output.push_back( dicomSeriesInformation() );
			child_target = & output.last();
			child_target->targetOffset = targetOffset;
			child_target->series_id = series_id;
		}
		//char wk[1024];
		//sprintf(wk,"�V���[�Y���R�[�h��񏈗���(%d)",series_id);
		//statusBar->set( wk );
	}
	if ( target && itemName.length() >= 5 && memcmp( itemName.chars(), "IMAGE", 5 ) == 0 ) {
		if ( referencefilename.length() ) {
			target->files.push_back( fileprefix + referencefilename );
			if ( image_id >= 0 ) target->image_ids.push_back( image_id );
			target->thickness = thickness;
			target->fieldOfViewMM = width * pixelsize;
			char wk[1024];
			sprintf(wk,"���R�[�h��񏈗���(�V���[�Y%d/�摜%d)",target->series_id,image_id);
			statusBar->set( wk );
		}
	}
	int currentChild = firstChild;
	while ( currentChild ) {
		currentChild = dicomListup( root, currentChild, fileprefix, output, child_target, element );//���̌Z�킪�߂�l
	}
	return nextSibling;
}
