#include "activate.h"
////////////////////////////////////////////////////////////////////////////////
//CT�摜

typedef regionImageInterface< pixelLuminance<int16>, int16 > ctImage;
//�X���C�X���
class ctSlice
{
	array<ctRegion> _regions;//���E�����̗̈�
	array<ctRegion> _left;//�����̗̈�
	array<ctRegion> _right;//�E���̗̈�
public:
	enum {
		bodyBoneBound = 200,//�̂ƍ��̋��E
		bodyLungBound = -200,//�̂Ɣx�̋��E
		imageSourceNone = 0,
		imageSourceSliceData,
		imageSourceDicom,
	};
	double position;//�B�e�ʒu
    critical_section cs;//�X���C�X�摜�p�̃N���e�B�J���Z�N�V����.
private:
	ctImage rimg;//�X���C�X�摜(������������Ȃ���ΊJ�������)
public:
	bool active;//�������}�l�[�W���p�t���O.
	int imageSource;//�ǂݍ��݈ʒu���
	string imageSourcePath;
	int imageSourceOffset;
	point2<int16> imageSize;
	bool airCalibrate, waterCalibrate;
	int16 air, water;
	bool regionChange;//�ύX�m�F�t���O
	//���J�p
	const ctRegion * const & regions;//���E�����̗̈�
	const ctRegion * const & left;//�����̗̈�
	const ctRegion * const & right;//�E���̗̈�
	//�ʐϏ��
	array<decimal> clusterArea;
	array<decimal> clusterAreaLeft;
	array<decimal> clusterAreaRight;

	ctSlice() : regionChange( true ), position( 0 ), 
		regions( _regions.data ), left( _left.data ), right( _right.data ), 
		imageSource( imageSourceNone ), active( false ),
		airCalibrate( false ), waterCalibrate( false ), air( -1000 ), water( 0 )
	{
	}
	~ctSlice()
	{
		finalize();
	}
	//����E�R�s�[�R���X�g���N�^�̋֎~�i���s���j
	ctSlice & operator=( const ctSlice & value )
	{
		assertInstance( "ctSlice::operator=", __FILE__, __LINE__ );
		return *this;
	}
	//������
	bool initialize();
	bool initialize( const char * filename );//DICOM
	bool initialize( const char * filename, int offset, int width, int height, double position );//SLICE.DATA
private:
	bool reload();
public:
	//�I����
	bool finalize();
	ctImage & getImage()
	{
		if ( ! active ) reload();
		return rimg;
	}
	void getImage( imageInterface< pixelLuminance<int16> > & img )
	{
		ctImage & ctimg = getImage();
		convert( ctimg, img );
	}
	void getImage( ctImage & img )
	{
		ctImage & ctimg = getImage();
		img.create( ctimg.width, ctimg.height );
		img.fill( pixelLuminance<int16>( 0 ) );
		for ( ctImage::iterator it( ctimg ); it; ++it ) {
			img.setInternal( it.x(), it.y(), ctimg.getInternal( it() ) );
		}
	}
	static void inactivate();
	static void convert( const ctImage & rimg, imageInterface< pixelLuminance<int16> > & img )
	{
		img.create( rimg.width, rimg.height );
		img.fill( pixelLuminance<int16>( -2048 ) );
		for ( ctImage::iterator it( rimg ); it; ++it ) {
			img.setInternal( it.x(), it.y(), rimg.getInternal( it() ) );
		}
	}
	//�Q�������摜�ݒ�
	static void set2differentiation( imageInterface< pixelLuminance<int16> > & laplacian, const imageInterface< pixelLuminance<int16> > & ctimg )
	{
		pixelLuminance<int16> p;
		p.y = 0;
		laplacian.create( ctimg.width, ctimg.height );
		for ( int y = 0; y < ctimg.height; ++y ) {
			for ( int x = 0; x < ctimg.width; x += ctimg.width - 1 ) {
				laplacian.setInternal( x, y, p );
			}
		}
		for ( int y = 0; y < ctimg.height; y += ctimg.height - 1 ) {
			for ( int x = 1; x < ctimg.width - 1; ++x ) {
				laplacian.setInternal( x, y, p );
			}
		}
		for ( int y = 1; y < ctimg.height - 1; ++y ) {
			for ( int x = 1; x < ctimg.width - 1; ++x ) {
				p.y = clamp<int16>( -4096, ctimg.getInternal( x, y ).y - ( 
					ctimg.getInternal( x - 1, y ).y + ctimg.getInternal( x + 1, y ).y + 
					ctimg.getInternal( x, y - 1 ).y + ctimg.getInternal( x, y + 1 ).y + 
					ctimg.getInternal( x - 1, y - 1 ).y + ctimg.getInternal( x + 1, y - 1 ).y + 
					ctimg.getInternal( x - 1, y + 1 ).y + ctimg.getInternal( x + 1, y + 1 ).y ) / 8, 4096 );
				laplacian.setInternal( x, y, p );
			}
		}
	}
	//�㉺��臒l����̈���l��
	const ctRegion ct2region( int minimum, int maximum )
	{
		ctImage & ctimg = getImage();
		ctRegion wk;
		wk.reserve( ctimg.shape.area() );
		for ( ctRegion::point_enumrator it( ctimg.shape ); it; ++it ) {
			int16 x = it->x, y = it->y;
			if ( between<int16>( minimum, ctimg.getInternal( x, y ).y, maximum ) ) {
				wk.push_back( ctRegion::rectangle::pixel( x, y ) );
			}
		}
		wk.connect();
		return wk;
	}
	//��w�I�m���Ɋ�Â��l�̗̈撊�o
	//�E�܋C�̈�ȊO�̍��z���̈�Ől�̗̈悪���Ă���
	//�E���ɂ͐l�̗̈�͂Ȃ�
	void searchHuman();
	//臒l�ɂ��ύX
	void search( int index, int minimum, int maximum, int mode )
	{
		if ( index == regionHuman ) return;//�l�͖̂���
		// down �` up�̗̈��������
		ctRegion rgn = ct2region( minimum, maximum );
		// ���̗̈���ɂ�����݂̂̂ɂ���
		if ( index == regionLung ) {
			//���̌��͖��߂�
			( regions[regionHuman] & rgn ).fill( rgn );
			changeRegion( regionLung, rgn );
		} else {
			switch ( mode ) {
			case regionOperationModeAppend: appendRegion( index, rgn ); break;
			case regionOperationModeDelete: removeRegion( index, rgn ); break;
			case regionOperationModeChange: changeRegion( index, rgn ); break;
			}
		}
	}
	//�}�X�}�e�B�J�������t�H���W�����ŕύX
	void modify( int index, int closing, int opening )
	{
		if ( index == regionHuman ) return;
		ctRegion wk, rgn;
		regions[index].closing( wk, closing );
		wk.opening( rgn, opening );
		changeRegion( index, rgn );
	}
	//�w��̈���̃q�X�g�O�������擾
	void getHistogram( const ctRegion & rgn, array<decimal> & histogram )
	{
		histogram.allocate( 4097 );
		memset( histogram.data, 0, histogram.size * sizeof( decimal ) );
		decimal d = 1 / decimal( rgn.area() );
		ctImage & ctimg = getImage();
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			int address = ctimg.address( it->x, it->y );
			if ( address < 0 ) continue;
			histogram[ctimg.getInternal( address ).y+2048] += d;
		}
	}
	//�w��̈���̉�f�Q���擾
	void getSample( const ctRegion & rgn, array<int16> & sample )
	{
		ctImage & ctimg = getImage();
		getSample( ctimg, rgn, sample );
	}
	//�w��摜�Ŏw��̈���̉�f�Q���擾
	static void getSample( const ctImage & ctimg, const ctRegion & rgn, array<int16> & sample )
	{
		int area = int( rgn.area() );
		sample.reserve( area );
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			int address = ctimg.address( it->x, it->y );
			if ( address < 0 ) continue;
			sample.push_back( ctimg.getInternal( address ).y );
		}
	}
private:
	//���E�̈撲���i�f�t�H���g�͑S�̈�j
	void adjustSide( int index = -1 );
public:
	//�̈撲��
	void adjustRegion( int priority = -1 );
	//�폜����
	void removeRegion( int index, const ctRegion & rgn );
	//�ǉ�����
	void appendRegion( int index, const ctRegion & rgn );
	//�ύX����
	void changeRegion( int index, const ctRegion & rgn );
	//���������ɐݒ肷��
	void setRegion( int index, const ctRegion & rgn );
	//�L�����u���[�V�����̎�����
	void uncalibrate();
	//�L�����u���[�V�����̒l�ݒ�
	void calibrate( int16 air, int16 water, bool airCalibrate, bool waterCalibrate );
	//�L�����u���[�V�����̎��s
	void calibrate();
	static bool save( const imageInterface< pixelLuminance<int16> > & ctimg, const char * filename );
	static bool load( imageInterface< pixelLuminance<int16> > & ctimg, const char * filename );
	double getAverage( int index )
	{
		ctImage & img = getImage();
		const ctRegion & rgn = _regions[index];
		double area = rgn.area();
		if ( ! area ) return 0;
		double average = 0;
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			if ( ! img.in( it->x, it->y ) ) continue;
			const double ctv = double( img.get( it->x, it->y ).y );
			average += ctv;
		}
		return average / area;
	}
	double getAverageSquare( int index )
	{
		ctImage & img = getImage();
		const ctRegion & rgn = _regions[index];
		double area = rgn.area();
		if ( ! area ) return 0;
		double average2 = 0;
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			if ( ! img.in( it->x, it->y ) ) continue;
			const double ctv = double( img.get( it->x, it->y ).y );
			average2 += ctv * ctv;
		}
		return average2 / area;
	}
	double getStandardDeviation( int index )
	{
		ctImage & img = getImage();
		const ctRegion & rgn = _regions[index];
		double area = rgn.area();
		if ( ! area ) return 0;
		double average = getAverage(index);
		double average2 = getAverageSquare(index);
		double variance = fabs( average2 - average * average );
        return ::sqrt( variance );
	}
};

//�����̃X���C�X
class ctSlices
{
public:
	array<ctSlice> slices;//�e�X���C�X�f�[�^
	double fieldOfViewMM;//�摜�̑傫������mm��
	//�e�̈�̑̐�
	array<decimal> clusterVolume;
	array<decimal> clusterVolumeLeft;
	array<decimal> clusterVolumeRight;
	//���݂̃X���C�X
	int index;
	//�Ō�ɐݒ肵���L�����u���[�V�������
	bool airValid, waterValid;
	int16 air, water;
	int numberTop, numberBottom, numberCenter;
	point2<int> centerposition;
	bool prone;//true�̏ꍇ������
	bool bottomFirst;//true�̏ꍇ�A�������ŏ��̉摜�ɂȂ�.
	ctSlices() : fieldOfViewMM( 0 ), index( 0 ), 
		airValid( false ), waterValid( false ), air( -1000 ), water( 0 ),
		numberTop( 0 ), numberBottom( 0 ), numberCenter( 0 ), prone( false ), bottomFirst( true ),
		centerposition( 256, 256 )
	{
	}
	~ctSlices()
	{
	}
	//����E�R�s�[�R���X�g���N�^�̋֎~�i���s���j
	ctSlices & operator=( const ctSlices & value )
	{
		assertInstance( "ctSlices::operator=", __FILE__, __LINE__ );
		return *this;
	}
	bool initialize( int slicesize );
	bool initialize( const char * slice, const char * header );
	bool initialize_dicom_dir( const char * dicomdir );
	bool initialize_directory( const char * dicompath );
	bool finalize()
	{
		slices.release();
		return true;
	}
	void search( int index, int down, int up, int mode, bool progress = true );
	void modify( int index, int closing, int opening, bool progress = true );
	void searchHuman( bool progress = true );
	//�e�X���C�X�̖ʐρE�̐όv�Z
	void calcVolume();
	void continuousRegion( int x, int y, int zslice, int rtype, array<ctRegion> & contRgn, bool progress = true );
	void searchLung();
	//�L�����u���[�V����
	void calibration( int16 _air, int16 _water, bool _airCalibrate, bool _waterCalibrate )
	{
		air = _air; water = _water;
		airValid = _airCalibrate; waterValid = _waterCalibrate;
		for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
			it->calibrate( air, water, airValid, waterValid );
		}
	}
	void uncalibrate()
	{
		air = -1000; water = 0;
		airValid = false; waterValid = false;
		for ( array<ctSlice>::iterator it( slices ); it; ++it ) {
			it->uncalibrate();
		}
	}
	class iterator
	{
	public:
		array<ctSlice>::iterator it;
		bool all;
		////////////////////////////////////////////////////////////////////////////////
		//�����E����
		iterator( array<ctSlice> & src, int offset, bool _all = false ) 
			: it( array<ctSlice>::iterator( src, offset ) ), all( _all ){}
		iterator( const iterator & v ) 
			: it( v.it ), all( v.all ){}
		iterator & operator=( const iterator & src )
		{
            if ( this != & src )
            {
			    it = src.it;
			    all = src.all;
            }
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//��������p�^�ϊ�
		operator ctSlice * () { return it; }
		operator const ctSlice * () const { return it; }
		////////////////////////////////////////////////////////////////////////////////
		//�^�ϊ�
		ctSlice & operator()()				{ return it(); }
		const ctSlice & operator()() const	{ return it(); }
		////////////////////////////////////////////////////////////////////////////////
		//�O��
		iterator & operator--()
		{
			if ( ! all ) {
				while ( it ) --it;
			} else {
				--it;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//����
		iterator & operator++()
		{
			if ( ! all ) {
				while ( it ) ++it;
			} else {
				++it;
			}
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////////
		//���̑��̃C���^�t�F�[�X
		ctSlice & operator*() { return ( * this )(); } const ctSlice & operator*() const { return ( * this )(); }
		ctSlice * operator->() { return & ( * this )(); } const ctSlice * operator->() const { return & ( * this )(); }
		iterator operator++(int) { iterator wk(*this); ++(*this); return wk; }
		iterator operator--(int) { iterator wk(*this); --(*this); return wk; }
	};
	iterator createIterator( bool all )
	{
		return iterator( slices, all ? 0 : index, all );
	}
	int indexIterator( const iterator & it ) const
	{
		if ( ! it ) return -1;
		return slices.index( it.it );
	}
    int indexSlice( const ctSlice & slice ) const
    {
		return slices.index( slice );
    }
};

//�X���C�X�̕\���p�f�[�^
class ctSliceView
{
#ifdef VIEW3D
	friend class miView;
	sgTexture * texture;//�V�[���O���t�p�e�N�X�`��
#else
	image surface;//��ʕ\���p�摜
#endif
	rectangle2<int> imageRect;
public:
	//windowSurfaceBitmap surface;//GDC�p�r�b�g�}�b�v�T�[�t�F�[�X
	int16 windowSize, windowLevel;//�\���pCT�E�B���h�E�l
	int viewmode, viewside;//�\�����[�h�ƍ��E���[�h
	int regionindex;//����ύX���邽�߂ɁC�\�����̗̈�̎��
	ctSlice * slice;//���݂̃X���C�X
	bool active;
	bool valid;
	ctSliceView() : viewmode( viewModeOriginal ), viewside( viewSideBoth ), regionindex( -1 ), 
		windowSize( 0 ), windowLevel( 0 ), slice( NULL ), active( false ), valid( false )
#ifdef VIEW3D
		, texture( NULL )
#endif
	{
	}
	~ctSliceView()
	{
		finalize();
	}
	//����E�R�s�[�R���X�g���N�^�̋֎~�i���s���j
	ctSliceView & operator=( const ctSliceView & value )
	{
		assertInstance( "ctSliceView::operator=", __FILE__, __LINE__ );
		return *this;
	}
	bool initialize( ctSlice * slice );
	bool finalize();
	void mode( int16 windowSize, int16 windowLevel, int viewmode, int viewside, int regionindex );
	void update();
	const image & getImage();
	//�Ώۉ摜�Ɏw��̈�̌`����w��F�œh��B���u�����f�B���O�L��
	static void regionBlend( image & img, const ctRegion & rgn, const pixel & color )
	{
		if ( color.a == 255 ) {
			for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
				img.set( it->x, it->y, color );
			}
			return;
		}
		decimal da = ( 255 - color.a ) / 255.0;
		decimal sa = color.a / 255.0;
		pixel sc( color.r * sa, color.g * sa, color.b * sa );
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			pixel p = img.getInternal( it->x, it->y );
			p.r = ( uint8 ) clamp<int32>( 0, p.r * da + sc.r, 255 );
			p.g = ( uint8 ) clamp<int32>( 0, p.g * da + sc.g, 255 );
			p.b = ( uint8 ) clamp<int32>( 0, p.b * da + sc.b, 255 );
			img.set( it->x, it->y, p );
		}
	}
	//�Ώۉ摜�Ɏw��̈�̌`����w�胿�ɐݒ�B
	static void setAlpha( image & img, const ctRegion & rgn, const uint8 alpha )
	{
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			pixel p = img.getInternal( it->x, it->y );
			p.a = alpha;
			img.setInternal( it->x, it->y, p );
		}
	}
	//������蓧�߂�����
	static void setAlpha( image & img, const ctRegion & rgn )
	{
		for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
			pixel p = img.getInternal( it->x, it->y );
			p.a = static_cast<uint8>( ( static_cast<uint16>( p.r ) + p.g + p.a ) / 3.0 );
			img.setInternal( it->x, it->y, p );
		}
	}
};

