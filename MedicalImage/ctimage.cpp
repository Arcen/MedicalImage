////////////////////////////////////////////////////////////////////////////////
//CT�摜
#include "main.h"
#include "dicom.h"
#include "slicehdr.h"
#include "activate.h"
#include <algorithm>
#include "parallel_for.h"
extern int thread_count;

static managementActive<ctSlice> ctSliceManager;
static managementActive<ctSliceView> ctSliceViewManager;

//������
bool ctSlice::initialize()
{
	ctSliceManager.initialize( this );
	airCalibrate = waterCalibrate = false;
	const int numberRegion = miDocument::get()->numberRegion();
	_regions.allocate( numberRegion );
	_left.allocate( numberRegion );
	_right.allocate( numberRegion );
	clusterArea.allocate( numberRegion );
	clusterAreaLeft.allocate( numberRegion );
	clusterAreaRight.allocate( numberRegion );
	for ( int i = 0; i < numberRegion; ++i ) {
		_regions[i].initialize();
		_left[i].initialize();
		_right[i].initialize();
		clusterArea[i] = clusterAreaLeft[i] = clusterAreaRight[i] = 0;
	}
	return true;
}
//slice.data�̏�񂩂珉����.
bool ctSlice::initialize( const char * filename, int offset, int width, int height, double position )
{
	finalize();
	this->position = position;//�B�e�ʒu

	imageSource = imageSourceSliceData;//�ǂݍ��݈ʒu���(slice.data�𗘗p����)
	imageSourcePath = filename;
	imageSourceOffset = offset;
	imageSize.x = width;
	imageSize.y = height;
	//�e�̈揉����
	initialize();
	return true;
}
bool ctSlice::initialize( const char * filename )
{
	finalize();
	imageSource = imageSourceDicom;//�ǂݍ��݈ʒu���
	imageSourcePath = filename;
	//�e�̈揉����
	initialize();

    //�X���C�X�ʒu�͓ǂݍ��܂Ȃ���΂Ȃ�Ȃ�.
	dicom dcm;
	retainer<file> f = file::initialize( imageSourcePath, false, true );
	if ( ! f ) return false;
    dicom::nodeType dcmNode;
	if ( ! dcm.convert( f(), dcmNode ) ) return false;
	double pos;
    if ( dcm.read_value( dcmNode, "�X���C�X�ʒu", pos ) ) {
		this->position = pos;
	}
	return true;
}
//�I����
bool ctSlice::finalize()
{
	//�X���C�X�z��ł̖����I�ȊJ���̂��ߎ���
    check_critical_section ccs( cs );//���̃X���b�h�Ń��b�N���͊J�����Ȃ�.
	rimg.finalize();
	ctSliceManager.finalize( this );
	return true;
}
void ctSlice::inactivate()
{
	ctSliceManager.inactivate();
}
//�摜�ǂݍ���
bool ctSlice::reload()
{
	ctSliceManager.activate( this );
	imageInterface< pixelLuminance<int16> > ctimg;//�摜
	ctimg.topdown = true;
	switch ( imageSource ) 
	{
	case imageSourceSliceData:
		{
			retainer<file> f = file::initialize( imageSourcePath, false, true );
			if ( ! f ) return false;
			f->seek( imageSourceOffset, SEEK_SET );
			ctimg.create( imageSize.x, imageSize.y );
			f->read( ctimg.data, imageSize.x * imageSize.y * 2 );
			//�Q�o�C�g�̑O�㔽�]
			_swab( ( char * ) ctimg.data, ( char * ) ctimg.data, imageSize.x * imageSize.y * 2 );
		}
		break;
	case imageSourceDicom:
		{
			dicom dcm;
			retainer<file> f = file::initialize( imageSourcePath, false, true );
			if ( ! f ) return false;
			if ( ! dcm.read( f(), ctimg ) ) return false;
		}
		break;
	default:
		return true;
	}
	//�ǂ݊ԈႢ�����邩������Ȃ��̂ŁA�X�V
	imageSize.x = ctimg.width;
	imageSize.y = ctimg.height;
	if ( regions[regionHuman].empty() ) {
		rimg = ctimg;
	} else {
		rimg.create( regions[regionHuman] );
		rimg.width = ctimg.width;
		rimg.height = ctimg.height;
		rimg.copy( ctimg );
	}

	//�L�����u���[�V�������K�v�ȏꍇ�ɂ͒�������
	calibrate();
	return true;
}
//�L�����u���[�V�����̎�����
void ctSlice::uncalibrate()
{
	//���ɒ����ς݂̏ꍇ��
	if ( this->airCalibrate || this->waterCalibrate ) {
		//�������Ȃ��悤�t���O��ύX
		airCalibrate = false;
		waterCalibrate = false;
		//�摜���ēǂݍ��݂��邽�߂ɊJ��
		finalize();
		regionChange = true;
	}
}
//�L�����u���[�V����
void ctSlice::calibrate( int16 air, int16 water, bool airCalibrate, bool waterCalibrate )
{
	//�L�����u���[�V��������K�v���Ȃ��ꍇ�ɂ͕ς��Ȃ�
	if ( this->airCalibrate == airCalibrate && 
		this->waterCalibrate == waterCalibrate ) {
		if ( ( ! airCalibrate || air == this->air ) && 
			( ! waterCalibrate || water == this->water ) ) return;
	}
	//���ɒ����ς݂̏ꍇ��
	if ( this->airCalibrate || this->waterCalibrate ) {
		//�摜���ēǂݍ��݂��邽�߂ɊJ��
		finalize();
	}
	//�V���������p�̒l��ݒ�
	this->air = air;
	this->water = water;
	this->airCalibrate = airCalibrate;
	this->waterCalibrate = waterCalibrate;
	regionChange = true;
	if ( active ) {//�A�N�e�B�u�Ȃ�Β������Ă����Ȃ���΂Ȃ�Ȃ��D�t�Ȃ�reload���ɒ��������
		calibrate();
	}
}
//�L�����u���[�V�����̎��s
void ctSlice::calibrate()
{
	//�������Ȃ��Ƃ��ɂ͂��̂܂�
	if ( ! airCalibrate && ! waterCalibrate ) return;
	//�摜���擾�D���̂Ƃ��ɃL�����u���[�V���������s����Ă��܂��D
	bool ac = airCalibrate, wc = waterCalibrate;
	airCalibrate = waterCalibrate = false;//�L�����u���[�V���������s���Ȃ��悤�Ƀt���O��ύX
	ctImage & ctimg = getImage();
	//�摜�擾��C�t���O��߂�
	airCalibrate = ac; waterCalibrate = wc;

	regionChange = true;
	pixelLuminance<int16> p;
	if ( airCalibrate && waterCalibrate ) {
		for ( ctImage::iterator it( ctimg ); it; ++it ) {
			p = ctimg.getInternal( it() );
			// -1000->air, 0->water
			if ( -2048 < p.y && p.y < 2048 ) {
				p.y = clamp<int16>( -2048, int16( double( p.y - water ) / ( air - water ) * -1000.0 ), 2048 );
			}
			ctimg.setInternal( it(), p );
		}
	} else if ( airCalibrate && ! waterCalibrate ) {
		for ( ctImage::iterator it( ctimg ); it; ++it ) {
			p = ctimg.getInternal( it() );
			// -1000->air, 0->water
			if ( -2048 < p.y && p.y < 2048 ) {
				p.y = clamp<int16>( -2048, p.y - air - 1000, 2048 );
			}
			ctimg.setInternal( it(), p );
		}
	} else if ( ! airCalibrate && waterCalibrate ) {
		for ( ctImage::iterator it( ctimg ); it; ++it ) {
			p = ctimg.getInternal( it() );
			// -1000->air, 0->water
			if ( -2048 < p.y && p.y < 2048 ) {
				p.y = clamp<int16>( -2048, p.y - water, 2048 );
			}
			ctimg.setInternal( it(), p );
		}
	}
}

//���E�̈撲���i�f�t�H���g�͑S�̈�j
void ctSlice::adjustSide( int index )
{
	const int numberRegion = miDocument::get()->numberRegion();
	if ( index == -1 ) {
		for ( int i = 0; i < numberRegion; ++i ) {
			adjustSide( i );
		}
		return;
	}
	switch ( index ) {
	case regionHuman:
		_left[index] = regions[index];
		_right[index] = regions[index];
		break;
	case regionLung:
		{
            std::list<ctRegion> multiple;
			regions[index].divideMultiRegion( multiple );
			_left[index].initialize();
			_right[index].initialize();
			for ( std::list<ctRegion>::iterator it = multiple.begin(); it != multiple.end(); ++it ) {
				ctRegion::point p = it->center();
				if ( p.x > imageSize.x / 2 ) {
					_left[index] |= *it;
				} else {
					_right[index] |= *it;
				}
			}
		}
		break;
	default:
		_left[index] = left[regionLung] & regions[index];
		_right[index] = right[regionLung] & regions[index];
		break;
	}
}
//�̈撲��
void ctSlice::adjustRegion( int priority )
{
	const int numberRegion = miDocument::get()->numberRegion();
	//�x��͐l�̓�
	_regions[regionLung] &= regions[regionHuman];
	//�e�̈�͔x��������A�D��̈�͕ύX���ɂ���
	ctRegion rest = regions[regionLung];//�������̈�
	if ( regionNormal < priority ) {
		//���̗̈�D��Ȃ�ΗD��̈�����肵�Ă���A�D��O�̕������ꂼ�ꏈ�����āA�c��͔x�E�Ƃ���B
		_regions[priority] &= rest;
		rest -= regions[priority];
		for ( int i = regionNormal + 1; i < numberRegion; ++i ) {
			if ( priority != i ) {
				_regions[i] &= rest;
				rest -= regions[i];
			}
		}
		_regions[regionNormal] = rest;
	} else if ( priority == regionHuman ) {
		//�l�̂͒������Ȃ�
	} else {
		//�x�E�D��Ȃ�΁A�x�E�������m�肵�c��͂��̎c���p���āA�Ō�Ɏc���x�E�ɉ�����B���ʂƂ��Ĕx�E�����邱�Ƃ͏o���Ȃ��B
		//�ʏ폈���Ȃ�΁A�x�E��D��Ƃ���
		priority = regionNormal;
		_regions[priority] &= rest;
		rest -= regions[priority];
		for ( int i = regionNormal + 1; i < numberRegion; ++i ) {
			_regions[i] &= rest;
			rest -= regions[i];
		}
		_regions[regionNormal] |= rest;
	}
	adjustSide();
	regionChange = true;
}
//�폜����
void ctSlice::removeRegion( int index, const ctRegion & rgn )
{
	if ( rgn.empty() ) return;
	_regions[index] -= rgn;
	if ( index == regionHuman ) {
		//�폜����ꍇ�ɂ͍ēǂݍ��݂͕K�v�Ȃ��̂ŁC�ǂݍ��ݍς݂̏ꍇ�ɂ͏C������
		if ( active ) {
			ctImage & ctimg = getImage();
			ctImage wk = ctimg;
			ctimg.create( _regions[regionHuman] );
			ctimg.width = wk.width;
			ctimg.height = wk.height;
			ctimg.copy( wk );
		}
	}
	adjustRegion( index );
}
//�ǉ�����
void ctSlice::appendRegion( int index, const ctRegion & rgn )
{
	if ( rgn.empty() ) return;
	_regions[index] |= rgn;
	if ( index == regionHuman ) {
		//�̈悪���������C�ēǂݍ��݂�����D
		finalize();//�ǂݍ��݂Ȃ���
	}
	adjustRegion( index );
}
//�ύX����
void ctSlice::changeRegion( int index, const ctRegion & rgn )
{
	setRegion( index, rgn );
	adjustRegion( index );
}
//���������ɐݒ肷��
void ctSlice::setRegion( int index, const ctRegion & rgn )
{
	bool additive = false;
	if ( index == regionHuman ) {
		if ( ! ( ( rgn - _regions[index] ).empty() ) ) {
			additive = true;//�ǉ��C��������
		}
	}
	_regions[index] = rgn;
	if ( index == regionHuman ) {
		if ( additive ) {//�ǉ��C��������
			finalize();//�ǂݍ��݂Ȃ���
		} else {//�폜�����ŏ\��
			if ( active ) {//�ǂݍ��ݍς݂̏ꍇ�����C�C�����Ă���
				ctImage & ctimg = getImage();
				ctImage wk = ctimg;
				ctimg.create( _regions[regionHuman] );
				ctimg.width = wk.width;
				ctimg.height = wk.height;
				ctimg.copy( wk );
			}
		}
	}
	regionChange = true;
}

//CT�摜�ǂݍ���
bool ctSlices::initialize( int slicesize )
{
	int numberRegion = miDocument::get()->numberRegion();
	clusterVolume.allocate( numberRegion );
	clusterVolumeLeft.allocate( numberRegion );
	clusterVolumeRight.allocate( numberRegion );

	slices.allocate( slicesize );
	for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
		it->initialize();
	}
	return true;
}
bool ctSlices::initialize( const char * slice, const char * header )
{
	index = 0;
	ctHeader cth;
	int numberRegion = miDocument::get()->numberRegion();
	clusterVolume.allocate( numberRegion );
	clusterVolumeLeft.allocate( numberRegion );
	clusterVolumeRight.allocate( numberRegion );
	if ( ! cth.initialize( header ) ) return false;
	slices.allocate( cth.depth );
	fieldOfViewMM = cth.fieldOfViewMM;
	{
		retainer<file> f = file::initialize( slice, false, true );
		if ( ! f ) return false;
	}
	statusBar->reset();
	statusBar->expand( slices.size );
	const bool forward = ( 1 < slices.size ) && ( cth.position.first() < cth.position.last() );
	array<double>::iterator itp( cth.position, forward );
	int offset = 0;
	for ( array<ctSlice>::iterator it( slices, forward ); it; forward ? ( ++it, ++itp ) : ( --it, --itp ) ) {
		//it->initialize( & f(), cth.width, cth.height, itp() );
		it->initialize( slice, offset, cth.width, cth.height, itp() );
		offset += cth.width * cth.height * 2;
//		it->searchHuman();
		statusBar->progress();
	}
	//�������ݒ�
	if ( slices.size ) {
		this->numberTop = ( slices.size - 1 ) * 3.0 / 4;
		this->numberCenter = ( slices.size - 1 ) / 2;
		this->numberBottom = ( slices.size - 1 ) * 1.0 / 4;
		this->prone = false;
		this->bottomFirst = true;
	}
	statusBar->reset();
	return true;
}
#include "dicomexport.h"
#include "dicomseries.h"
dicomSeriesSelect * dicomSeriesSelect::_this = NULL;

bool ctSlices::initialize_dicom_dir( const char * dicomdir )
{
	index = 0;
	ctHeader cth;
	int numberRegion = miDocument::get()->numberRegion();
	clusterVolume.allocate( numberRegion );
	clusterVolumeLeft.allocate( numberRegion );
	clusterVolumeRight.allocate( numberRegion );

	// DICOM���擾
	char drive[MAX_PATH], path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( dicomdir, drive, path, filename, extension );
	string fileprefix = string( drive ) + path;
	retainer<file> f = file::initialize( dicomdir, false, true );
	if ( ! f ) return false;
	static dicom dcm;
	MSXML2::IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
	if ( ! dcm.convert( f(), domDocument ) ) return false;

	//�f�B���N�g���擾
	int firstChildOffset = 0;
	MSXML2::IXMLDOMElementPtr elementDirecotry = NULL;
	MSXML2::IXMLDOMElementPtr element = domDocument->documentElement;
	for ( element = element->firstChild; element != NULL; element = element->nextSibling ) {
		string name = static_cast<const char*>( static_cast<_bstr_t>( element->nodeName ) );
		MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "name" ) );
		if ( attr ) {} else continue;
		name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
		if ( name == "���[�g�f�B���N�g���G���e�B�e�B�̍ŏ��̃f�B���N�g�����R�[�h�̃I�t�Z�b�g" ) {
			attr = element->getAttributeNode( _bstr_t( "value" ) );
			if ( attr ) {} else continue;
			firstChildOffset = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
		} else if ( name == "�f�B���N�g�����R�[�h�V�[�P���X" ) {
			elementDirecotry = element;
		}
	}
	array<dicomSeriesInformation> series;
	dicomListup( elementDirecotry, firstChildOffset, fileprefix, series );
	if ( series.size == 0 ) return false;
	dicomSeriesSelect dss( mainWindow->hwnd );
	dss.series = & series;
	dss.index = series.size - 1;
	if ( ! dss.call() ) return false;
	if ( dss.index == -1 ) return false;

	array<string> & filenames = series[dss.index].files;
	int readoffset = dss.start;//�ǂݍ��݊J�n�ʒu
	int readstep = dss.space;//�ǂݍ��ݎ��̃X���C�X�Ԃ̊Ԋu
	int readsize = maximum<int>( 0, filenames.size - readoffset );//�ǂݍ��݂��s���X���C�X��
	if ( readsize ) {
		readsize = ( readsize - 1 ) / readstep + 1;//��΂����߂ɓǂݍ��ރX���C�X��������
		readsize = minimum<int>( readsize, dss.maximumNumber );//���x�𒴂��Ȃ��悤�ɐݒ�
	}
	slices.allocate( readsize );//�������m��
	if ( ! readsize ) return false;
	fieldOfViewMM = series[dss.index].fieldOfViewMM;
	statusBar->reset();
	statusBar->expand( readsize );
    array<string> skipped_filenames;
    {
        skipped_filenames.reserve( readsize );
	    array<string>::iterator itn( filenames, false );//�O��x�ꕔ���ɂ���p�ɋt���ɂ��邪�A���ۂ͂ǂ��炩�킩��Ȃ�.
	    for ( int i = 0; i < readoffset; ++i ) --itn;
        for ( int i = 0; i < readsize; ++i, --itn )
        {
            skipped_filenames.push_back( itn() );
		    for ( int i = 1; i < readstep; ++i ) --itn;//�X���C�X��ǂݔ�΂�
        }
    }
	point2<int16> imageSize( 512, 512 );
    if ( readsize )
    {
		//�V���[�Y�擪�̂݃T�C�Y���v��
		retainer<file> f = file::initialize( skipped_filenames.first(), false, true );
		if ( f ) {
			int w, h, d, ba, bs, hb, pr, io, ri = 0, rs = 1;
            double fov;
			if ( dcm.read( f(), w, h, d, ba, bs, hb, pr, io, ri, rs, fov ) ) {
				imageSize.x = int16( w );
				imageSize.y = int16( h );
			}
		}
    }
	array<string>::iterator itn( skipped_filenames );
	for ( array<ctSlice>::iterator it( slices ); it; ++it, ++itn ) {
		it->imageSize = imageSize;
		it->initialize( itn() );
		statusBar->progress();
	}
	//�������ݒ�
	this->numberTop = ( readsize - 1 ) * 3.0 / 4;
	this->numberCenter = ( readsize - 1 ) / 2;
	this->numberBottom = ( readsize - 1 ) * 1.0 / 4;
	this->prone = false;
	this->bottomFirst = false;
	statusBar->reset();
	return true;
}
bool string_lt( const void * e1, const void * e2 )
{
	return stricmp( ( const char * ) e1, ( const char * ) e2 ) < 0;
}
void sort( array<string> & data )
{
	array<char *> chars;
	chars.reserve( data.size );
	for ( int i = 0; i < data.size; ++i ) {
		chars.push_back( data[i].chars() );
	}
	std::sort( chars.data, chars.data + chars.size, string_lt );
	array<string> new_data;
	new_data.reserve( data.size );
	for ( int i = 0; i < data.size; ++i ) {
		new_data.push_back( string( chars[i] ) );
	}
	data = new_data;
}
bool ctSlices::initialize_directory( const char * dicompath )
{
	index = 0;
	ctHeader cth;
    miDocument * doc = miDocument::get();
	int numberRegion = doc->numberRegion();
	clusterVolume.allocate( numberRegion );
	clusterVolumeLeft.allocate( numberRegion );
	clusterVolumeRight.allocate( numberRegion );

	// DICOM���擾
	string fileprefix = dicompath;
	static dicom dcm;

	//�f�B���N�g���擾
    list<string> files;
	WIN32_FIND_DATA wfd;
	HANDLE fff = FindFirstFile( fileprefix + "*.dcm", & wfd );
	if ( fff != INVALID_HANDLE_VALUE ) {
		do {
			if ( 
				stricmp( ".", wfd.cFileName ) != 0 &&
				stricmp( "..", wfd.cFileName ) != 0  ) {
                files.push_back( fileprefix + string( wfd.cFileName ) );
			}
		} while( FindNextFile( fff, & wfd ) );
	}
	FindClose( fff );
	array<dicomSeriesInformation> series;
	series.push_back( dicomSeriesInformation() );
	dicomSeriesInformation & dsi = series.first();
	dsi.targetOffset = 0;
	dsi.thickness = 1;
	dsi.fieldOfViewMM = 1;
    dsi.files.reserve( files.size );
    for ( list<string>::iterator it( files ); it; ++it ) {
		dsi.files.push_back( *it ); 
    }
	sort( dsi.files );
	if ( dsi.files.size == 0 ) return false;
	if ( series.size == 0 ) return false;
	dicomSeriesSelect dss( mainWindow->hwnd );
	dss.series = & series;
	dss.index = series.size - 1;
	if ( ! dss.call() ) return false;
	if ( dss.index == -1 ) return false;

	array<string> & filenames = series[dss.index].files;
	int readoffset = dss.start;//�ǂݍ��݊J�n�ʒu
	int readstep = dss.space;//�ǂݍ��ݎ��̃X���C�X�Ԃ̊Ԋu
	int readsize = maximum<int>( 0, filenames.size - readoffset );//�ǂݍ��݂��s���X���C�X��
	if ( readsize ) {
		readsize = ( readsize - 1 ) / readstep + 1;//��΂����߂ɓǂݍ��ރX���C�X��������
		readsize = minimum<int>( readsize, dss.maximumNumber );//���x�𒴂��Ȃ��悤�ɐݒ�
	}
	slices.allocate( readsize );//�������m��
	if ( ! readsize ) return false;
    //��΂����t�@�C�������쐻.
    array<string> skipped_filenames;
    {
        skipped_filenames.reserve( readsize );
	    array<string>::iterator itn( filenames, false );//�O��x�ꕔ���ɂ���p�ɋt���ɂ��邪�A���ۂ͂ǂ��炩�킩��Ȃ�.
	    for ( int i = 0; i < readoffset; ++i ) --itn;
        for ( int i = 0; i < readsize; ++i, --itn )
        {
            skipped_filenames.push_back( itn() );
		    for ( int i = 1; i < readstep; ++i ) --itn;//�X���C�X��ǂݔ�΂�
        }
    }
    fieldOfViewMM = series[dss.index].fieldOfViewMM;
	statusBar->reset();
	statusBar->expand( readsize );

	point2<int16> imageSize( 512, 512 );//�f�t�H���g�T�C�Y.
    if ( readsize ) {//�擪�̃t�@�C��������擾.
		retainer<file> f = file::initialize( skipped_filenames.first(), false, true );
		if ( f ) {
			int w, h, d, ba, bs, hb, pr, io, ri = 0, rs = 1;
	        if ( dcm.read( *f, w, h, d, ba, bs, hb, pr, io, ri, rs, fieldOfViewMM ) )
            {
			    imageSize.x = int16( w );
			    imageSize.y = int16( h );
                fieldOfViewMM *= imageSize.x;
            }
		}
    }
#if 1
    {
        struct sliceInitializer {
            miDocument * doc;
	        point2<int16> imageSize;
            array<string> * skipped_filenames;
	        array<ctSlice> * slices;
            static void process( void * data, int index )
            {
                sliceInitializer * self = ( sliceInitializer * ) data;
                ctSlice & slice = (*(self->slices))[index];
                string & srcpath = (*(self->skipped_filenames))[index];
		        slice.imageSize = self->imageSize;
		        slice.initialize( srcpath );
                self->doc->progress();
            }
        } status;
        status.doc = doc;
        status.imageSize = imageSize;
        status.skipped_filenames = & skipped_filenames;
        status.slices = & slices;
        parallel_for pf( mainWindow, thread_count, 0, readsize, sliceInitializer::process, & status );
        pf.begin();
        pf.join();
    }
#else
    for ( int i = 0; i < readsize; ++i ) {
        const string & srcpath = skipped_filenames[i];
        ctSlice & slice = slices[i];
		slice.imageSize = imageSize;
		slice.initialize( srcpath );
		statusBar->progress();
	}
#endif
	//�������ݒ�
	this->numberTop = ( readsize - 1 ) * 3.0 / 4;
	this->numberCenter = ( readsize - 1 ) / 2;
	this->numberBottom = ( readsize - 1 ) * 1.0 / 4;
	this->prone = false;
	this->bottomFirst = false;
	statusBar->reset();
	return true;
}

//臒l����
void ctSlices::search( int index, int down, int up, int mode, bool progress )
{
	if ( progress ) {
		statusBar->reset();
		statusBar->expand( slices.size );
	}
	for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
		it->search( index, down, up, mode );
        if ( progress ) statusBar->progress();
	}
	if ( progress ) statusBar->reset();
}
//�C������
void ctSlices::modify( int index, int closing, int opening, bool progress )
{
	if ( progress ) {
		statusBar->reset();
		statusBar->expand( slices.size );
	}
	for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
		it->modify( index, closing, opening );
        if ( progress ) statusBar->progress();
	}
	if ( progress ) statusBar->reset();
}

void ctSlices::searchHuman( bool progress )
{
	if ( progress ) {
		statusBar->reset();
		statusBar->expand( slices.size );
	}
	for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
		it->searchHuman();
		if ( progress ) statusBar->progress();
	}
	if ( progress ) statusBar->reset();
}
void ctSlices::continuousRegion( int x, int y, int z, int rtype, array<ctRegion> & contRgn, bool progress )
{
	if ( progress ) {
		statusBar->reset();
		statusBar->expand( slices.size );
	}
	contRgn.release();
	contRgn.allocate( slices.size );
	ctRegion start;
	slices[z].regions[rtype].select( start, x, y );
	if ( start.empty() ) return;
	array<ctRegion> wholeRgn( slices.size );
	for ( int i = 0; i < slices.size; ++i ) {
		wholeRgn[i] = slices[i].regions[rtype];
	}
	list<int> targetSlice;
	targetSlice.push_back( z );
	contRgn[z] = start;
	wholeRgn[z] -= start;
    std::list<ctRegion> parts;
	while ( ! targetSlice.empty() ) {
		int ts = targetSlice.loot_back();
		ctRegion & t = contRgn[ts];
		for ( int i = ts-1; i <= ts+1; i += 2 ) {
			if ( ! between( 0, i, slices.size - 1 ) ) continue;
			wholeRgn[i].divideMultiRegion( parts );
			wholeRgn[i].release();
			bool changed = false;
            for ( std::list<ctRegion>::iterator it = parts.begin(); it != parts.end(); ++it ) {
				if ( ! ( *it & t ).empty() ) {
					contRgn[i] |= *it;
					changed = true;
				} else {
					wholeRgn[i] |= *it;
				}
			}
			if ( changed ) {
				if ( ! targetSlice.search( i ) ) targetSlice.push_back( i );
			}
			if ( progress ) statusBar->loop();
		}
	}
	if ( progress ) statusBar->reset();
}
//�ۑ�
bool ctSlice::save( const imageInterface< pixelLuminance<int16> > & ctimg, const char * filename )
{
	retainer<file> f = file::initialize( filename, true, true );
	if ( ! f ) return false;
	for ( int y = 0; y < ctimg.height; ++y ) {
		for ( int x = 0; x < ctimg.width; ++x ) {
			f->write( ctimg.getInternal( x, y ).y );
		}
	}
	f->write( int32( ctimg.width ) );
	f->write( int32( ctimg.height ) );
	f->write( int32( imageInterface< pixelLuminance<int16> >::dot ) );
	return true;
}
//�ǂݍ���
bool ctSlice::load( imageInterface< pixelLuminance<int16> > & ctimg, const char * filename )
{
	retainer<file> f = file::initialize( filename, false, true );
	if ( ! f ) return false;
	f->seek( sizeof( int32 ) * 3, SEEK_END );
	int32 w, h, b;
	f->read( w );
	f->read( h );
	f->read( b );
	if ( b != imageInterface< pixelLuminance<int16> >::dot || 
		w * h * imageInterface< pixelLuminance<int16> >::dot + 
		sizeof( int32 ) * 3 != f->getSize() ) return false;
	f->seek( 0, SEEK_SET );
	ctimg.topdown = true;
	ctimg.create( w, h );
	pixelLuminance<int16> p;
	for ( int y = 0; y < ctimg.height; ++y ) {
		for ( int x = 0; x < ctimg.width; ++x ) {
			f->read( p.y );
			ctimg.setInternal( x, y, p );
		}
	}
	return true;
}

//��w�I�m���Ɋ�Â��l�̗̈撊�o
//�E�܋C�̈�ȊO�̍��z���̈�Ől�̗̈悪���Ă���
//�E���ɂ͐l�̗̈�͂Ȃ�
void ctSlice::searchHuman()
{
	//�̈�����Z�b�g
	ctRegion box( ctRegion::rectangle::size( imageSize.x, imageSize.y ) );
	changeRegion( regionHuman, box );
	//�L���ȑ���͈͂𒲂ׂ�
	ctRegion validRgn = ct2region( -2047, 2048 );
	// bodyLungBound�`�@�l�̂Ɣx��̋��E����̗̈��������
	ctRegion body = ct2region( bodyLungBound, 2048 );
	//�����̈悪����Ƃ��ɂ́A������x�̖ʐς����������̂��̗p����
    std::list<ctRegion> multiple;
	body.divideMultiRegion( multiple );
	body.release();
	double limitarea = ( imageSize.x / 5.0 ) * ( imageSize.y / 5.0 );
    for ( std::list<ctRegion>::iterator it = multiple.begin(); it != multiple.end(); ++it ) {
		if ( it->area() > limitarea ) body |= *it;
	}
    multiple.clear();
	// ���̗̈�̊O���i��C�̈�j����菜���B
	ctRegion others = validRgn - body;//�x������̗̈�
	ctRegion top, bottom;//����̗̈�
	ctRegion::rectangle toprect = validRgn.first();
	ctRegion::rectangle bottomrect = validRgn.last();
	others.select( top, toprect.left, toprect.top );
	others.select( bottom, bottomrect.right - 1, bottomrect.bottom - 1 );

	body = ( validRgn ) - ( top | bottom );
	if ( ! body.empty() ) {
		changeRegion( regionHuman, body );
	} else {
		//������Ȃ����ɂ́A����͈͂Ƃ��Ă���
		changeRegion( regionHuman, validRgn );
	}
}
//�x��̈撊�o
void ctSlices::searchLung()
{
	//臒l����x����̈�𒊏o
	statusBar->set( "�x����̈撊�o��" );
	search( regionLung, -2048, ctSlice::bodyLungBound, regionOperationModeChange );
	//�擪�̂�����x�̑傫�����������̈���C�ǎx�̃g�b�v�Ƃ���
	int tracheaslice = bottomFirst ? slices.size - 1 : 0;
	int tracheax, tracheay;
	ctRegion trachea;
	statusBar->set( "�C�ǎx������" );
	statusBar->reset();
	statusBar->expand( tracheaslice );
	
	for ( ; ( bottomFirst ? 0 <= tracheaslice : tracheaslice < slices.size ); bottomFirst ? --tracheaslice : ++tracheaslice ) {
		ctSlice & s = slices[tracheaslice];
		const ctRegion & r = s.regions[regionLung];
		if ( r.empty() ) continue;
		if ( r.area() < 100 ) continue;
        std::list<ctRegion> parts;
		r.divideMultiRegion( parts );
		checkMaximum<double,std::list<ctRegion>::iterator> mx;
		for ( std::list<ctRegion>::iterator it = parts.begin(); it != parts.end(); ++it ) {
			ctRegion::point p = it->center();
			double area = it->area();
			if ( area < 100 ) continue;
            if ( ! between<int16>( s.imageSize.x * 2 / 5, p.x, s.imageSize.x * 3 / 5 ) ) continue;
			mx( area, it );
		}
		if ( ! mx ) continue;
		trachea = *(mx.sub);
		ctRegion::point p = trachea.center();
		tracheax = trachea.first().left;
		tracheay = trachea.first().top;
		statusBar->loop();
		break;
	}
	statusBar->reset();
	if ( ! between( 0, tracheaslice, slices.size - 1 ) ) return;
	//�x��̈撊�o
	statusBar->set( "�x��̈挟����" );
	array<ctRegion> lungs;
	continuousRegion( tracheax, tracheay, tracheaslice, regionLung, lungs );
	//�C�ǎx�̕��ʂ��폜
	statusBar->reset();
	statusBar->expand( tracheaslice );
	for ( int i = tracheaslice; bottomFirst ? 0 <= i : i < slices.size; bottomFirst ? --i : ++i ) {
		ctRegion & l = lungs[i];
        std::list<ctRegion> parts;
		l.divideMultiRegion( parts );
		if ( parts.empty() ) break;
		ctRegion now;
		int count = 0;
		for ( std::list<ctRegion>::iterator it = parts.begin(); it != parts.end(); ++it ) {
			ctRegion & r = *it;
			if ( ( r & trachea ).empty() ) continue;
			if ( r.area() > trachea.area() * 2 ) continue;//�x����Ɍq�������̂ł����܂�
			now |= r;
			++count;
		}
		lungs[i] -= now;
		if ( 1 != count ) {
			numberCenter = bottomFirst ? minimum<int>( i + 1, tracheaslice ) : maximum<int>( i - 1, tracheaslice );
            ctRegion::point & p = trachea.center();
			centerposition = point2<int>( p.x, p.y );
			break;
		}
		trachea = now;
		statusBar->loop();
	}
	statusBar->reset();
	//�x��̈�ݒ�
	statusBar->expand( slices.size );
	statusBar->set( "�x��̈�ݒ蒆" );
	for ( int i = 0; i < slices.size; ++i ) {
		slices[i].changeRegion( regionLung, lungs[i] );
		statusBar->progress();
	}
	statusBar->reset();
	miDocument * doc = miDocument::get();
	if ( doc ) doc->change();
}

void ctSlices::calcVolume()
{
	int numberRegion = miDocument::get()->numberRegion();
	for ( int i = 0; i < numberRegion; ++i ) {
		for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
			double area = square<double>( fieldOfViewMM / double( it().imageSize.x ) );//[mm^2]
			it->clusterArea[i] = it->regions[i].area() * area / 100.0;// [cm^2]
			it->clusterAreaLeft[i] = it->left[i].area() * area / 100.0;// [cm^2]
			it->clusterAreaRight[i] = it->right[i].area() * area / 100.0;// [cm^2]
		}
	}
	for ( int i = 0; i < numberRegion; ++i ) {
		clusterVolume[i] = 0;
		clusterVolumeLeft[i] = 0;
		clusterVolumeRight[i] = 0;
		if ( slices.size <= 1 ) {
			for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
				clusterVolume[i] += it->clusterArea[i];
				clusterVolumeLeft[i] += it->clusterAreaLeft[i];
				clusterVolumeRight[i] += it->clusterAreaRight[i];
			}
		} else {
			clusterVolume[i] += fabs( slices.first().position - slices.second().position ) * slices.first().clusterArea[i] / 10.0;
			clusterVolume[i] += fabs( slices.last().position - slices.prelast().position ) * slices.last().clusterArea[i] / 10.0;
			clusterVolumeLeft[i] += fabs( slices.first().position - slices.second().position ) * slices.first().clusterAreaLeft[i] / 10.0;
			clusterVolumeLeft[i] += fabs( slices.last().position - slices.prelast().position ) * slices.last().clusterAreaLeft[i] / 10.0;
			clusterVolumeRight[i] += fabs( slices.first().position - slices.second().position ) * slices.first().clusterAreaRight[i] / 10.0;
			clusterVolumeRight[i] += fabs( slices.last().position - slices.prelast().position ) * slices.last().clusterAreaRight[i] / 10.0;
			array<ctSlice>::iterator it( slices );
			array<ctSlice>::iterator prev( it ); ++it;
			array<ctSlice>::iterator next( it ); ++next;
			for ( ; next; ++prev, ++it, ++next ) {
				clusterVolume[i] += 
					( fabs( it->position - prev->position ) + 
						fabs( it->position - next->position ) ) / 2.0 *
						it->clusterArea[i] / 10.0; //[cc]
				clusterVolumeLeft[i] += 
					( fabs( it->position - prev->position ) + 
						fabs( it->position - next->position ) ) / 2.0 *
						it->clusterAreaLeft[i] / 10.0; //[cc]
				clusterVolumeRight[i] += 
					( fabs( it->position - prev->position ) + 
						fabs( it->position - next->position ) ) / 2.0 *
						it->clusterAreaRight[i] / 10.0; //[cc]
			}
		}
	}
}
bool ctSliceView::initialize( ctSlice * slice )
{
	ctSliceViewManager.initialize( this );
	this->slice = slice;
	valid = false;
	finalize();
	return true;
}
bool ctSliceView::finalize()
{
#ifdef VIEW3D
	//texture->texture.finalize();//�V�[���O���t�p�e�N�X�`��
#else
	surface.finalize();
#endif
	valid = false;
	ctSliceViewManager.finalize( this );
	return true;
}

//�\���p���쐬
void ctSliceView::mode( int16 windowSize, int16 windowLevel, int viewmode, int viewside, int regionindex )
{
	if ( this->windowSize == windowSize && 
		this->windowLevel == windowLevel &&
		this->viewmode == viewmode &&
		this->viewside == viewside &&
		this->regionindex == regionindex ) return;
	this->valid = false;
	this->windowSize = windowSize;
	this->windowLevel = windowLevel;
	this->viewmode = viewmode;
	this->viewside = viewside;
	this->regionindex = regionindex;
}
void ctSliceView::update()
{
	miDocument * doc = miDocument::get();
	if ( ! doc ) return;
	//�ύX�����邩�ǂ����𒲂ׂ�
	if ( ! slice->regionChange && this->active && this->valid ) {
		return;
	}
	slice->regionChange = false;
	valid = true;
#ifdef VIEW3D
	image & img = texture->texture;
	texture->dirty = true;
#else
	image & img = surface;
#endif
#ifndef VIEW3D
	ctSliceViewManager.activate( this );
#endif
	active = true;
	pixel p( 0, 0, 0, 0 );
	img.create( slice->imageSize.x, slice->imageSize.y, p );
	img.fill( p );
	decimal rate = 255.0 / windowSize;
	decimal offset = windowLevel - windowSize / 2;
	miFrame * frame = miFrame::get();
	const ctRegion * regions = slice->regions;
	switch ( viewside ) {
	case viewSideLeft:
		regions = slice->left;
		break;
	case viewSideRight:
		regions = slice->right;
		break;
	}
	const int numberRegion = doc->numberRegion();
#ifdef VIEW3D
	texture->valid( true );
#endif
	rectangle2<int> srcRect = rectangle2<int>::size( slice->imageSize.x, slice->imageSize.y );
	imageRect = srcRect;
	switch ( viewmode ) {
	case viewModeMix://����
		{
			ctImage & ctimg = slice->getImage();
			ctRegion::rectangle ir = ctimg.shape.get();
			imageRect = rectangle2<int>( ir.left, ir.top, ir.right, ir.bottom );
			for ( ctImage::iterator it( ctimg ); it; ++it ) {
				p.r = p.g = p.b = static_cast<int8>( clamp<decimal>( 0, ( ctimg.getInternal( it() ).y - offset ) * rate, 255.0 ) );
				img.set( it.x(), it.y(), p );
			}
			for ( int i = 0; i < numberRegion; ++i ) {
				regionBlend( img, regions[i], doc->regionColor( i ) );
			}
			if ( doc->regionindex == regionLung ) {
				setAlpha( img, regions[regionLung], 64 );
			} else {
				setAlpha( img, regions[regionLung], 6 );
				setAlpha( img, regions[regionindex], 64 );
			}
		}
		break;
	case viewModeOriginal://���摜
		{
			ctImage & ctimg = slice->getImage();
			ctRegion::rectangle ir = ctimg.shape.get();
			imageRect = rectangle2<int>( ir.left, ir.top, ir.right, ir.bottom );
			for ( ctImage::iterator it( ctimg ); it; ++it ) {
				p.r = p.g = p.b = static_cast<int8>( clamp<decimal>( 0, ( ctimg.getInternal( it() ).y - offset ) * rate, 255.0 ) );
				img.set( it.x(), it.y(), p );
			}
			setAlpha( img, regions[regionLung] );
#ifdef VIEW3D
			texture->valid( ! regions[regionLung].empty() );
#endif
		}
		break;
	case viewModeLabel://���x��
		ctRegion::rectangle ir = regions[regionHuman].get();
		imageRect = rectangle2<int>( ir.left, ir.top, ir.right, ir.bottom );
		for ( int i = 0; i < numberRegion; ++i ) {
			regionBlend( img, regions[i], doc->regionColor( i ).opacity() );
		}
		setAlpha( img, regions[regionHuman], 64 );
		break;
	}
#ifdef VIEW3D
	frame->gr.dispose( texture );
#else
	if ( imageRect != srcRect ) {
		image temp = img;
		img.create( imageRect.width(), imageRect.height() );
		img.copy( temp, rectangle2<int>::size( imageRect.width(), imageRect.height() ), imageRect );
	}
#endif
}
const image & ctSliceView::getImage()
{
	update();
#ifdef VIEW3D
	return texture->texture;//�V�[���O���t�p�e�N�X�`��
#else
	rectangle2<int> srcRect = rectangle2<int>::size( slice->imageSize.x, slice->imageSize.y );
	if ( imageRect != srcRect ) {
		static image temp;
		temp.create( srcRect.width(), srcRect.height(), pixel( 0, 0, 0, 0 ) );
		temp.fill( pixel( 0, 0, 0, 0 ) );
		temp.copy( surface, imageRect, rectangle2<int>::size( imageRect.width(), imageRect.height() ) );
		return temp;//��ʕ\���p�摜
	} else {
		return surface;//��ʕ\���p�摜
	}
#endif
}


