////////////////////////////////////////////////////////////////////////////////
//�p�^�[��
#include "main.h"

//������
bool miPattern::initialize( const char * drive, const char * path, const char * name, const char * postfix )
{
	regionChange = true;//�ύX�t���O

	//�摜������
	ctimg.create( 512, 512 );
	ctimg.fill( pixelLuminance<int16>( 0 ) );

	//���t����XML�̃p�X
	char ct_filename[MAX_PATH], tea_filename[MAX_PATH];
	_makepath( tea_filename, drive, path, name, NULL );

	//�摜�̃p�X
	char _drive[MAX_PATH], _path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( tea_filename, _drive, _path, filename, extension );
	_makepath( ct_filename, drive, path, filename, ".2di" );

	//����ꍇ�ɂ̂ݓǂݍ���
	if ( ! file::exist( ct_filename ) || ! file::exist( tea_filename ) ) 
		return false;

	imageInterface< pixelLuminance<int16> > wk;
	wk.topdown = true;
	if ( ! ctSlice::load( wk, ct_filename ) ) return false;
	ctimg = wk;

	if ( ! regionIO::load( *this, tea_filename ) ) return false;
	if ( postfix && strlen( postfix ) ) this->postfix = postfix;
	//�p�����[�^�̏�����
	initializeParameter();
	return true;
}
//�I������
bool miPattern::finalize( const char * drive, const char * path, const char * name )
{
	//��̏ꍇ�ɂ͕ۑ����Ȃ�
	if ( shape.empty() || ctimg.empty() ) return false;

	//�ۑ��p�p�X�쐬
	char ct_filename[MAX_PATH], tea_filename[MAX_PATH];
	_makepath( ct_filename, drive, path, name, ".2di" );
	_makepath( tea_filename, drive, path, name, ".tea" );

	//�ۑ�
	imageInterface< pixelLuminance<int16> > wk;
    wk.create( ctimg.width, ctimg.height );
	wk.fill( pixelLuminance<int16>( 0 ) );
	for ( ctImage::iterator it( ctimg ); it; ++it ) {
		wk.setInternal( it.x(), it.y(), ctimg.getInternal( it() ) );
	}
	ctSlice::save( wk, ct_filename );
    regionIO::save( *this, tea_filename );
	return true;
}
bool miPattern::remove( const char * drive, const char * path, const char * name )
{
	//�ۑ��p�p�X�쐬
	char ct_filename[MAX_PATH], tea_filename[MAX_PATH], rgn_filename[MAX_PATH];
	_makepath( ct_filename, drive, path, name, ".2di" );
	_makepath( tea_filename, drive, path, name, ".tea" );
	_makepath( rgn_filename, drive, path, name, ".rgn" );
	if ( file::exist( rgn_filename ) ||
		file::exist( ct_filename ) || file::exist( tea_filename ) ) {
		DeleteFile( rgn_filename );//�Â��`��������΍폜
		DeleteFile( ct_filename );
		DeleteFile( tea_filename );
		return true;
	} else {
		return false;
	}
}

//�w�肵���̈�ł̏�����
bool miPattern::initialize( ctSlice & slice, const ctRegion & rgn )
{
	if ( rgn.empty() ) return false;
	regionChange = true;
	slice.getImage( ctimg );
	shape = rgn;
	initializeParameter();
	return true;
}
static int compairInt16( const void * v1, const void * v2 )
{
	return *( reinterpret_cast< const int16* >( v1 ) ) - 
		*( reinterpret_cast< const int16* >( v2 ) );
}

void miPattern::initializeParameter()
{
	//�ʐς̌v�Z
	area = shape.area();
	average = variance = 0;
	//�������̊m�ۂƏ�����
	sample.reserve( int( area ) );
	for ( int i = 0; i < 3; ++i ) {
		moment[i].allocate( maximumValue - minimumValue + 1 );
		memset( moment[i].data, 0, moment[i].size * sizeof( decimal ) );
		cumulativeMoment[i].allocate( maximumValue - minimumValue + 1 );
		memset( cumulativeMoment[i].data, 0, cumulativeMoment[i].size * sizeof( decimal ) );
	}

	//�����̍ő�ŏ��̏�����
	minimumCT = maximumValue;
	maximumCT = minimumValue;
	int16 ct;
	for ( ctRegion::point_enumrator it( shape ); it; ++it ) {
		ct = ctimg.getInternal( it->x, it->y ).y;
		if ( between<int32>( minimumValue, int32( ct ), maximumValue ) ) {
			sample.push_back( ct );
			average += ct;
			//getHistogram( ct ) += d;
			minimumCT = minimum( minimumCT, ct );
			maximumCT = maximum( maximumCT, ct );
		}
	}
    if ( sample.size )
    {
        area = sample.size;
        const decimal d = 1 / area;
	    qsort( sample.data, sample.size, sizeof( int16 ), compairInt16 );
	    average /= area;
	    for ( array<int16>::iterator its( sample ); its; ++its ) {
		    ct = its();
		    getHistogram( ct ) += d;
		    variance += ( ct - average ) * ( ct - average );
	    }
	    variance /= area;
    }
	getCumulativeHistogram( minimumValue ) = getHistogram( minimumValue );
	getAverages( minimumValue ) = getCumulativeAverages( minimumValue ) = getHistogram( minimumValue ) * decimal( minimumValue );
	get2Moment( minimumValue ) = getCumulative2Moment( minimumValue ) = getHistogram( minimumValue ) * square( decimal( minimumValue ) );
	for ( int16 i = minimumValue + 1; i <= maximumValue; ++i ) {
		getAverages(i) = getHistogram(i) * decimal( i );
		get2Moment(i) = getHistogram(i) * square( decimal( i ) );
		getCumulativeHistogram(i) = getCumulativeHistogram(i-1) + getHistogram(i);
		getCumulativeAverages(i) = getCumulativeAverages(i-1) + getAverages(i);
		getCumulative2Moment(i) = getCumulative2Moment(i-1) + get2Moment(i);
	}
    standardDeviation = ::sqrt( variance );
	ge->buildup( sample, histogram_table );
	histogram_table_normalized = histogram_table;
	ge->normalized( sample, histogram_table_normalized );
	info.release();//�ĕύX���C��قǍX�V�K�{
	regionChange = true;
}

//���ʕ��͖@��臒l���Z�o
decimal miPattern::discriminantAnalysis()
{
	checkMaximum<decimal,int16> threshold;//臒l�ȉ���臒l���ł킯��
	for ( int16 t = minimumCT + 1; t < maximumCT - 1; ++t ) {
		const decimal percentageLower = getCumulativeHistogram( t );
		const decimal percentageHigher = getCumulativeHistogram( maximumValue ) - percentageLower;
		if ( percentageLower == 0 || percentageHigher == 0 ) continue;
		const decimal averageLower = getCumulativeAverages( t ) / percentageLower;
		const decimal averageHigher = ( getCumulativeAverages( maximumValue ) - getCumulativeAverages( t ) ) / percentageHigher;
		const decimal momentLower = getCumulative2Moment( t ) / percentageLower;
		const decimal momentHigher = ( getCumulative2Moment( maximumValue ) - getCumulative2Moment( t ) ) / percentageHigher;
		const decimal varianceLower = momentLower - square( averageLower );
		const decimal varianceHigher = momentHigher - square( averageHigher );

		//�N���X�����U
		const decimal varianceInner = percentageLower * varianceLower + percentageHigher * varianceHigher;
		//�N���X�ԕ��U
		const decimal varianceOuter = percentageLower * square( averageLower - average ) + percentageHigher * square( averageHigher - average );
		if ( varianceInner == 0 ) continue;
		threshold( varianceOuter / varianceInner, t );
	}
	//�G���[
	if ( ! threshold ) 
		return ( maximumCT + minimumCT ) / 2;
	return threshold.sub;
}
static int compairDecimal( const void * v1, const void * v2 )
{
	return *( reinterpret_cast< const decimal* >( v1 ) ) - 
		*( reinterpret_cast< const decimal* >( v2 ) );
}
decimal miPattern::regionAverage( const ctRegion & rgn )
{
	decimal area = rgn.area();
	decimal average = 0;
	for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
		average += static_cast<decimal>( ctimg.get( it->x, it->y ).y ) / area;
	}
	return average;
}
decimal miPattern::regionMedian( const ctRegion & rgn )
{
	decimal area = rgn.area();
	//�֊s��CT�l�����X�g�A�b�v
	array<decimal> cts; cts.reserve( area );
	for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
		cts.push_back( static_cast<decimal>( ctimg.get( it->x, it->y ).y ) );
	}
	//�����l�����߂�
	qsort( cts.data, cts.size, sizeof( decimal ), compairDecimal );
	return cts[cts.size/2];
}
decimal miPattern::regionVariance( const ctRegion & rgn, decimal average )
{
	decimal area = rgn.area();
	decimal variance = 0;
	for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
		variance += square( ctimg.get( it->x, it->y ).y - average ) / area;
	}
	return variance;
}

class inputBox
{
	string title;
	string msg;
	string editbox;
	static BOOL CALLBACK dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		static inputBox * _this = NULL;
		static HWND okbottun = NULL;
		static HWND cancelbottun = NULL;
		if ( uMsg != WM_INITDIALOG && _this == NULL ) return 0;
		switch( uMsg ) {
		case WM_INITDIALOG:
			{
				fc::dialog * dlg = reinterpret_cast<fc::dialog*>( lParam );
				_this = reinterpret_cast<inputBox*>( dlg->externalData );
				dlg->hwnd = hDlg;
				RECT wr;
				int width = 320, height = 200;
				GetWindowRect( dlg->parent, & wr );
				//��\���C�Z���^�����O�C��Ԏ�O�C�T�C�Y�ݒ�
				SetWindowPos( hDlg, HWND_TOPMOST, 
					( wr.right - wr.left - width ) / 2, 
					( wr.bottom - wr.top - height ) / 2, 
					width, height,
					SWP_HIDEWINDOW );
				//�N���C�A���g�ʒu�擾
				RECT cr;
				GetClientRect( hDlg, & cr );
				//�{�^���쐬
				int w = 80, h = 20;
				okbottun = CreateWindow( "BUTTON", "OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER, 
					20, 100, w, h, hDlg, reinterpret_cast<HMENU>( IDOK ), dlg->instance, NULL );
				cancelbottun = CreateWindow( "BUTTON", "OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_CENTER | BS_VCENTER, 
					120, 100, w, h, hDlg, reinterpret_cast<HMENU>( IDCANCEL ), dlg->instance, NULL );
				//�\��
				ShowWindow( hDlg, SW_SHOW );
			}
			return 1;
		case WM_COMMAND:
			{
 				switch(LOWORD(wParam))
				{
				case IDOK:
					EndDialog( hDlg, IDOK );
					break;
				case IDCANCEL:
					EndDialog( hDlg, IDCANCEL );
					break;
				}
			}
			break;
		default:
			return 0;
		}
		return 1;
	}
public:
	static const string call( HWND parent, const char * _title = "input box", const char * _msg = "info" )
	{
		inputBox ib;
		ib.title = _title;
		ib.msg = _msg;
		fc::dialog dlg( parent, 0, dlgProc );
		dlg.externalData = & ib;
		list< array<char> > tmpls;
		dlg.setTemplate( tmpls.push_back( array<char>() ), 0, 0, 0, 320, 240, _title );
		bool r = dlg.call( tmpls );
		if ( ! r ) return "";
		return ib.editbox;
	}
};
//�p�����[�^�Z�o
void miPattern::parameterAnalysis()
{
	//���ʕ��͖@��臒l���o ���m�ɋ�ʂł��Ȃ�����
	decimal th = 750;//discriminantAnalysis();

	inputBox::call( mainWindow->hwnd );

	//臒l�ɂ��`����킯��
	ctRegion lower;
	int area = shape.area();
	lower.reserve( area );
	for ( ctRegion::point_enumrator it( shape ); it; ++it ) {
		if ( ctimg.getInternal( it->x, it->y ).y <= th ) {
			lower.push_back( it->x, it->y );
		}
	}
	lower.connect();
	//����ł��C�������Ă������opening�����ŗ���
	lower.opening( lower, 1 );
	//�E���Ƃɂ킯��
    std::list<ctRegion> lowers;
	lower.divideMultiRegion( lowers );

	ctRegion outline;
	checkCT.initialize();
	checkArea.initialize();
	checkCircular.initialize();
	checkMargin.initialize();
	checkOutlineWidth.initialize();
	char wk[1024];
	sprintf( wk, "lowers.size(%d)", lowers.size() );
	MessageBox( mainWindow->hwnd, wk, "analysis", MB_OK );
    for ( std::list<ctRegion>::iterator it = lowers.begin(); it != lowers.end(); ++it ) {
		ctRegion & hole = *it;
		//�W���΍����̗֊s�����͓����Ɋ܂܂��l�Ɋg������
		bool expanced = false;
		decimal average, sd;
		int area = 0;
		do {
			expanced = false;
			area = hole.area();
			average = regionAverage( hole );
			decimal variance = regionVariance( hole, average );
            sd = ::sqrt( variance );
			hole.dilation( outline, 1 );
			outline -= hole;
			ctRegion add;
			add.reserve( outline.area() );
			for ( ctRegion::point_enumrator ito( outline ); ito; ++ito ) {
				decimal ct = ctimg.get( ito->x, ito->y ).y;
				if ( absolute( ct - average ) < sd  ) {
					add.push_back( ito->x, ito->y );
				}
			}
			add.connect();
			if ( ! add.empty() ) {
				expanced = true;
				hole = hole | add;
			}
		} while ( expanced );
		//�O���̕ǂ����ς��������Ȃ�܂Ŋg������
		int width = 1;
		/*
		decimal prevOutlineAverage = 0;
		for ( width = 1; width < 1; ++width ) {
			hole.dilation( outline, width );
			outline -= hole;
			decimal outlineAverage = regionAverage( outline );
			if ( width == 1 ) {
				prevOutlineAverage = outlineAverage;
				continue;
			}
			if ( prevOutlineAverage > outlineAverage ) {
				--width; break;
			}
			prevOutlineAverage = outlineAverage;
		}
		*/
		hole.dilation( outline, width );
		outline -= hole;
		int outlinearea = outline.area();
        decimal circular = ::sqrt( area / pi ) / ( outlinearea / pi / 2.0 / width );
		if ( circular < 0.3 ) continue;
		//�����l�����߂�
		decimal outlinemedian = regionMedian( outline );
		decimal margin = outlinemedian - average;

		//�L�^
		for ( ctRegion::point_enumrator ito( hole ); ito; ++ito ) {
			decimal ct = ctimg.get( ito->x, ito->y ).y;
			checkCT( ct );
		}
		checkArea( area );
		checkCircular( circular );
		checkOutlineWidth( width );
		checkMargin( margin );
	}
	if ( checkArea ) {
		sprintf( wk, "ct(%d,%d), area(%d,%d), circular(%f,%f), outline width(%f,%f), margin(%f,%f)", 
			checkCT.minimumValue, checkCT.maximumValue, 
			checkArea.minimumValue, checkArea.maximumValue, 
			checkCircular.minimumValue, checkCircular.maximumValue, 
			checkOutlineWidth.minimumValue, checkOutlineWidth.maximumValue, 
			checkMargin.minimumValue, checkMargin.maximumValue );
		MessageBox( mainWindow->hwnd, wk, "analysis", MB_OK );
	}
}

//�\���p�N���X

void teacherPatternView::initialize( miPattern * pattern, pixel * color )
{
	this->pattern = pattern;
	this->color = color;
	valid = false;
}
bool teacherPatternView::finalize()
{
	pattern = NULL;
	color = NULL;
	valid = false;
	return true;
}
void teacherPatternView::mode( int16 windowSize, int16 windowLevel, int viewmode )
{
	if ( this->windowSize == windowSize && this->windowLevel == windowLevel && this->viewmode == viewmode ) return;
	this->windowSize = windowSize;
	this->windowLevel = windowLevel;
	this->viewmode = viewmode;
	valid = false;
}
image & teacherPatternView::getImage()
{
	if ( ( ! valid ) || ( pattern && pattern->regionChange ) ) update();
	return img;
}

void teacherPatternView::update()
{
	valid = true;
	if ( ! color ) {
		static pixel black( 0, 0, 0, 255 );
		color = & black;
	}
	if ( pattern ) {
		pattern->regionChange = false;
		//�\���p�摜�쐬
		img.create( 512, 512 );
		decimal rate = 255.0 / windowSize;
		decimal offset = windowLevel - windowSize / 2;
		int x, y;
		pixel p( 0, 0, 0, 255 );
		if ( ! pattern->shape.empty() && 
			pattern->ctimg.height == img.height && 
			pattern->ctimg.width == img.width ) {
			for ( y = 0; y < img.height; y++ ) for ( x = 0; x < img.width; x++ ) {
				p.r = p.g = p.b = static_cast<int8>( clamp<decimal>( 0, ( pattern->ctimg.getInternal( x, y ).y - offset ) * rate, 255.0 ) );
				img.set( x, y, p );
			}
		}
		//�O��������
		ctRegion outer;
		switch ( viewmode ) {
		case viewModeOriginal:
			break;
		case viewModeMix:
			pattern->shape.dilation( outer, 2 );
			outer = ( ctRegion( ctRegion::rectangle::size( img.width, img.height ) ) & outer ) - pattern->shape;
			break;
		case viewModeLabel:
			outer = ctRegion( ctRegion::rectangle::size( img.width, img.height ) ) - pattern->shape;
			break;
		}
		ctSliceView::regionBlend( img, outer, pixel( color->r, color->g, color->b, 255 ) );
		ctSliceView::setAlpha( img, outer, 0 );
	} else {//�p�^�[�����Ȃ��ꍇ.
		//�\���p�摜�쐬
		if ( img.width == 512 && img.height == 512 ) {
			img.fill( pixel( color->r, color->g, color->b, 255 ) );
		} else {
			img.create( 512, 512, pixel( color->r, color->g, color->b, 255 ) );
		}
	}
}
