#include "main.h"
#include <time.h>
#include "parallel_for.h"
extern int thread_count;

miDocument * miDocument::document = NULL;
miDocument * miDocument::get(){ return document; }
void miDocument::set( miDocument * doc ){ document = doc; }
void miDocument::release(){ delete document; document = NULL; }

ctSlice & miDocument::slice() { return slices.slices[sliceindex]; }

void dicomExport( const char * filename );

static int compairDecimal( const void * v1, const void * v2 )
{
	return *( reinterpret_cast< const decimal* >( v1 ) ) - 
		*( reinterpret_cast< const decimal* >( v2 ) );
}
void miDocument::registerRegion( const string & name )
{
	if ( ! name.length() ) return;
	for ( regionIterator rit( *this ); rit; ++rit ) {
		if ( rit->name == name ) {
			return;
		}
	}
	regionTypes.push_back( regionType( name ) );
}

void miDocument::open( const char * indexed_path )
{
	//現在開いてるドキュメントを閉じる
	close();

	//slice.data, slice.hdrファイルのパスを取得。slice.hdrは同ディレクトリにない場合にはダイアログを表示する
	char slice_data_filename[MAX_PATH];
    if ( indexed_path && strlen( indexed_path ) < MAX_PATH )
    {
        strcpy( slice_data_filename, indexed_path );
        int len = strlen( indexed_path );
        if ( len > 2 && indexed_path[0] == '"' && indexed_path[len-1] == '"' )
        {
            strcpy( slice_data_filename, indexed_path + 1 );
            slice_data_filename[len-2] = '\0';
        }
    } else {
	    if ( ! windows::fileOpen( slice_data_filename, "", "DICOM Files(*.dcm)\0*.dcm\0Slice Data(slice.data)\0slice.data\0DICOM(DICOMDIR)\0DICOMDIR\0", "Open CT File", mainWindow->hwnd ) ) return;
    }
	if ( ! file::exist( slice_data_filename ) ) return;
	char drive[MAX_PATH], path[MAX_PATH], filename[MAX_PATH], extension[MAX_PATH];
	_splitpath( slice_data_filename, drive, path, filename, extension );
	char slice_header_filename[MAX_PATH];
	
	bool dicommode = false;
	bool dicom_directory = false;
	if ( _stricmp( extension, ".data" ) == 0 ) {
		dicommode = false;
		// slice.data
		_makepath( slice_header_filename, drive, path, filename, ".hdr" );
		if ( ! file::exist( slice_header_filename ) ) {
			if ( ! windows::fileOpen( slice_header_filename, "slice.hdr", "Slice Header(slice.hdr)\0slice.hdr\0", "Open CT Header", mainWindow->hwnd ) ) return;
		}
	} else if ( _stricmp( filename, "DICOMDIR" ) == 0 ) {
		dicommode = true;
	} else {
		strcpy( slice_header_filename, drive );
		strcat( slice_header_filename, path );
		dicom_directory = true;
	}

	statusBar->set( "ドキュメントを開いています" );

	//ドキュメントを作成
	miDocument * doc = new miDocument;
	doc->drive = drive;
	doc->path = path;

	statusBar->set( "レジストリから情報を取得しています" );
	//教師の種類をレジストリから取得
	list< pair<string> > teacherslist;
	regsGet( "teachers", teacherslist );//teachersディレクトリ内に，キー名は連番若しくは任意で，データに領域名が入っている
	int index = 0;
	while ( teacherslist.size && index < 100 ) {
		string string_index( index );
		for ( list< pair<string> >::iterator it( teacherslist ); it; ++it ) {
			if ( it->left == string_index ) {
				doc->registerRegion( it->right );
				teacherslist.pop( it() );
				break;
			}
		}
		++index;
	}
	//連番以外のデータがあった場合は，無順序に追加
	for ( list< pair<string> >::iterator it( teacherslist ); it; ++it ) {
		doc->registerRegion( it->right );
	}
	//初期起動時には，デフォルトを追加
	if ( doc->regionTypes.size == 3 ) {
		//doc->registerRegion( "alveolus" );//昔の正常だが，読み込み用
		doc->registerRegion( "air" );
		doc->registerRegion( "groundglass" );
		doc->registerRegion( "consolidation" );
		doc->registerRegion( "honeycomblung" );
		doc->registerRegion( "grain" );
		doc->registerRegion( "line" );
		doc->registerRegion( "net" );
	}
	//色取得
    int color_index = 1;
	for ( regionIterator rit( *doc ); rit; ++rit, ++color_index ) {
		if ( ! regGet( rit->name, rit->color ) )
        {
            rit->color.a = 127;
            rit->color.r = ( color_index * 51358 ) % 127 + 127;
            rit->color.g = ( color_index * 3818 ) % 127 + 127;
            rit->color.b = ( color_index * 20325 ) % 127 + 127;
        }
	}

	//グローバルにドキュメントを呼び出せるようにセット
	miDocument::set( doc );

	statusBar->set( "画像ファイルを検査中" );
	if ( ! dicommode && ! dicom_directory ) {
		//slice.data, slice.hdrから、CT画像を読み込む
		if ( ! doc->slices.initialize( slice_data_filename, slice_header_filename ) ) {
			miDocument::set( NULL );
			delete doc;
			return;
		}
	} else if ( dicommode ) {
		//DICOMDIRファイルから読み込む
		if ( ! doc->slices.initialize_dicom_dir( slice_data_filename ) ) {
			miDocument::set( NULL );
			delete doc;
			return;
		}
	} else {
		//dicomファイルのあるディレクトリ全体を読み込む
		if ( ! doc->slices.initialize_directory( slice_header_filename ) ) {
			miDocument::set( NULL );
			delete doc;
			return;
		}
	}
	//同じディレクトリに処理中のデータがあれば読み込む
	statusBar->set( "形状情報を読み込んでいます" );
	if ( ! doc->load() ) {
		statusBar->set( "human領域を検出しています" );
		doc->slices.searchHuman();
	}

	//教師情報をディレクトリから取得
	statusBar->set( "教師情報を読み込んでいます" );
	loadPattern( doc->drive, doc->path, true, false );

	//表示の初期化を行う
	statusBar->set( "表示用データを準備しています" );
	miView * view = new miView();
	view->initialize( doc );
	miView::set( view );

	//表示用モード設定
	view->mode( view->windowSize, view->windowLevel, controlWindow->viewmode.index, controlWindow->viewside.index );

	//ウィンドウ・コントロールの更新をかける
	controlWindow->updateClusters( true );
	controlWindow->validateControl();
	view->changeTimes( 1 );
	doc->change( true );
	imageWindow->invalidate();

	char wk[1024], title[1024];
	_makepath( wk, doc->drive, doc->path, "", "" );
	sprintf( title, "肺野CT画像処理システム：ファイル[%s]", wk );
	mainWindow->setText( title );
}

void miDocument::close()
{
	miDocument * doc = get();
	if ( doc ) {
    	statusBar->set( "ドキュメントを閉じています" );
		miCommands::get().clear();//アンドゥ不可能にする
		statusBar->set( "レジストリに情報を保存しています" );
		{
            //削除
			list< pair<string> > teacherslist;
			regsGet( "teachers", teacherslist );
			regsKill( "teachers", teacherslist );
		}
        //リスト作製
		list< pair<string> > teacherslist;
		for ( regionIterator rit( *doc ); rit; ++rit ) {
			teacherslist.push_back( pair<string>( string( rit.index() ), rit->name ) );
		}
        //名前の保存
		regsSet( "teachers", teacherslist );//teachersディレクトリ内に，キー名は連番若しくは任意で，データに領域名が入っている
		//色の保存
		for ( regionIterator rit( *doc ); rit; ++rit ) {
			regSet( rit->name, rit->color );
		}
		//教師画像の保存
        if ( ::MessageBox( mainWindow->hwnd, "教師情報を保存せずに終了しますか？", "確認", MB_YESNO ) == IDNO )
        {
		    statusBar->set( "教師情報を保存しています" );
		    savePattern( doc->drive, doc->path, true );
        }
		statusBar->set( "データを開放しています" );
		doc->regionTypes.release();
		//表示の初期化を行う
		miView * view = miView::get();
		view->finalize( doc );
		delete view;
		miView::set( NULL );
        //ドキュメントの開放
	    release();
	}
	controlWindow->viewslice( 0 );
	controlWindow->validateControl();
	controlWindow->releaseClusters( true );
	imageWindow->invalidate();
    //タイトルの設定
	mainWindow->setText( "肺野CT画像処理システム" );
    //ステータスの設定
	statusBar->set( "" );
}
//ドキュメントが変更されたときのアップデート用
void miDocument::change( bool force )
{
	miView * view = miView::get();
	if ( ! view ) {
		//属性を空にしておく
		string ctattribute;
		string old = controlWindow->editctattribute.get();
		if ( old != ctattribute ) controlWindow->editctattribute.set( ctattribute );
		return;
	}
	slices.index = clamp( 0, controlWindow->viewslice(), slicesize - 1 );
	for ( array<ctSlice>::iterator it( slices.slices ); it; ++it ) {
		it->regionChange = it->regionChange || force;
	}
	//パターン情報の初期化
	view->patterns.initialize( pattern(), & regionColor() );
	view->mode( controlWindow->viewsize.value, controlWindow->viewlevel.value, 
		controlWindow->viewmode.index, controlWindow->viewside.index );
	if ( controlWindow->viewing3d ) {
		//テクスチャとして、全体が必要になるので、全体の更新を行う
		view->update();
	} else {
		//全体の設定を変えるが、アップデートするのは現在の画像のみ
		ctSliceView & sv = view->slices[sliceindex];
		sv.update();
	}
	//情報の計算
	slices.calcVolume();
	switch ( view->viewside ) {
	case viewSideBoth:
		controlWindow->clusterArea.set( slice().clusterArea[regionindex] );
		controlWindow->clusterVolume.set( slices.clusterVolume[regionindex] );
		break;
	case viewSideLeft:
		controlWindow->clusterArea.set( slice().clusterAreaLeft[regionindex] );
		controlWindow->clusterVolume.set( slices.clusterVolumeLeft[regionindex] );
		break;
	case viewSideRight:
		controlWindow->clusterArea.set( slice().clusterAreaRight[regionindex] );
		controlWindow->clusterVolume.set( slices.clusterVolumeRight[regionindex] );
		break;
	}
	controlWindow->clusterArea.set( string( controlWindow->clusterArea.get() ) + "[㎝2]" );
	controlWindow->clusterVolume.set( string( controlWindow->clusterVolume.get() ) + "[cc]" );

	if ( pattern() ) {
		controlWindow->clusterAverage.set( pattern()->average );
		controlWindow->clusterStandardDeviation.set( pattern()->standardDeviation );
	} else {
		controlWindow->clusterAverage.set( "" );
		controlWindow->clusterStandardDeviation.set( "" );
	}
	{
		string ctattribute;
		ctattribute.print( "肺尖部[%d%s]\r\n気管分岐部[%d(%d,%d)]\r\n肺底部[%d%s]\r\n%s",
			slices.numberTop, slices.bottomFirst ? "以上" : "以下",
			slices.numberCenter, slices.centerposition.x, slices.centerposition.y, 
			slices.numberBottom, slices.bottomFirst ? "以下" : "以上",
			slices.prone ? "腹臥位" : "背臥位" );
		string old = controlWindow->editctattribute.get();
		if ( old != ctattribute ) controlWindow->editctattribute.set( ctattribute );
	}
	view->statusUpdate();
	imageWindow->invalidate();
	imageWindow->update();
}
//CT画像などの保存と読み込み
void miDocument::save()
{
	miDocument * doc = get();
	if ( ! doc ) return;
	char filename[MAX_PATH];
	_makepath( filename, doc->drive, doc->path, "regions", ".rgn" );
    regionIO::save( doc->slices, filename );
}

bool miDocument::load()
{
	char filename[MAX_PATH];
	_makepath( filename, drive, path, "regions", ".rgn" );
	if ( ! file::exist( filename ) ) return false;
	return regionIO::load( this->slices, filename );
}
//領域形状のみの保存と読み込み
void miDocument::loadAsRgn()
{
	miDocument * doc = get();
	if ( ! doc ) return;
	//rgnファイルを指定する
	char rgn_filename[MAX_PATH];
	if ( ! windows::fileOpen( rgn_filename, "regions.rgn", "Regions Data(*.rgn)\0*.rgn\0Label Data(*.3di)\0*.3di\0", "Open Regions File", mainWindow->hwnd ) ) return;
    char rgn_drive[_MAX_DRIVE] = "", rgn_dir[_MAX_DIR] = "", rgn_name[_MAX_FNAME] = "", rgn_ext[_MAX_EXT] = "";
    ::_splitpath( rgn_filename, rgn_drive, rgn_dir, rgn_name, rgn_ext );
    miCommands::get().clear();//アンドゥ不可能にする
    if ( stricmp( rgn_ext, ".rgn" ) == 0 ) {
    	regionIO::load( doc->slices, rgn_filename );
    } else if ( stricmp( rgn_ext, ".3di" ) == 0 ) {
    	regionIO::loadLabel( doc->slices, rgn_filename );
    } else {
        return;
    }
	for ( array<ctSlice>::iterator it( doc->slices.slices ); it; ++it ) {
		it->regionChange = true;
	}
	controlWindow->validateControl();
	imageWindow->invalidate();
}

void miDocument::saveAsRgn( const char * filename )
{
	miDocument * doc = get();
	if ( ! doc ) return;
	char rgn_filename[MAX_PATH];
	if ( ! filename ) {
		if ( ! windows::fileSave( rgn_filename, "regions.rgn", "Regions Data(*.rgn)\0*.rgn\0", "Save Regions File", mainWindow->hwnd, ".rgn" ) ) return;
	} else {
		strcpy( rgn_filename, filename );
	}
	regionIO::save( doc->slices, rgn_filename );
}

inline void loadRegionFromLabel( ctRegion & rgn, retainer<file> & f, int width, int height )
{
	rgn.reserve( width * height );
	unsigned short wk;
	for ( int y = 0; y < height; ++y ) {
		for ( int x = 0; x < width; ++x ) {
			f->read( wk );
			if ( wk ) {
				rgn.push_back( x, y );
			}
		}
	}
	rgn.connect();
}

//ラベル画像からの読み込み
void miDocument::loadLabel()
{
	//rgnファイルを指定する
	char label_filename[MAX_PATH];
	if ( ! windows::fileOpen( label_filename, "label.img", "2Byte Label Data(*.img)\0*.img\0", "Open 2Byte Label File", mainWindow->hwnd ) ) return;
	retainer<file> f = file::initialize( label_filename, false, true );
	if ( ! f ) return;
	miCommands::get().clear();//アンドゥ不可能にする
	ctRegion wk;

	const bool forward = ( 1 < slices.slices.size ) && ( slices.slices.first().position < slices.slices.last().position );
	for ( array<ctSlice>::iterator it( slices.slices, forward ); it; forward ? ( ++it ) : ( --it ) ) {
		ctSlice & slice = it();
		loadRegionFromLabel( wk, f, slice.imageSize.x, slice.imageSize.y );
		slice.changeRegion( regionLung, wk );
	}
	controlWindow->validateControl();
	imageWindow->invalidate();
}

//アンドゥ処理のための形見の保存と読み込み
void miDocument::load( miDocument::memento & m )
{
	array<ctSlice>::iterator dst( slices.slices );
	array<ctSlice>::iterator src( m.slices.slices );
	array<bool>::iterator f( m.flags );
	for ( ; dst; ++src, ++dst ) {
		if ( f() ) {
			for ( int i = 0; i < numberRegion(); ++i ) {
				dst->setRegion( i, src->regions[i] );
			}
			dst->adjustRegion();
		}
		++f;
	}
	slices.calcVolume();
	change();
}
void miDocument::save( miDocument::memento & m )
{
	m.slices.initialize( slicesize );
	array<ctSlice>::iterator src( slices.slices );
	array<ctSlice>::iterator dst( m.slices.slices );
	array<bool>::iterator f( m.flags );
	for ( ; src; ++src, ++dst ) {
		if ( f() ) {
			for ( int i = 0; i < numberRegion(); ++i ) {
				dst->setRegion( i, src->regions[i] );
			}
		}
		++f;
	}
}
void miDocument::exchange( miDocument::memento & m )
{
	array<ctSlice>::iterator dst( slices.slices );
	array<ctSlice>::iterator src( m.slices.slices );
	array<bool>::iterator f( m.flags );
	for ( ; dst; ++src, ++dst ) {
		if ( f() ) {
			for ( int i = 0; i < numberRegion(); ++i ) {
				ctRegion wk = dst->regions[i];
				dst->setRegion( i, src->regions[i] );
				src->setRegion( i, wk );
			}
			dst->adjustRegion();
		}
		++f;
	}
	slices.calcVolume();
	change();
}
//情報のレポート
void miDocument::saveSampleHistogram()
{
	miDocument * doc = get();
	if ( ! doc ) return;
	char filename[1024];
	if ( windows::fileSave( filename, "sample.csv", "CSV file ( *.csv )\0*.csv\0", "標本ヒストグラム保存", mainWindow->hwnd, ".csv" ) ) {
		retainer<file> f = file::initialize( filename, true, false );
        if ( f )
        {
		    f->printf( "CT値" );
		    for ( regionIterator rit( *doc, true ); rit; ++rit ) {
			    if ( ! rit->patterns.size ) continue;
			    f->printf( ",%s", rit->name );
		    }
		    f->printf( "\n" );
		    for ( int i = -2048; i <= 2048; ++i ) {
			    f->printf( "%d", i );
			    for ( regionIterator rit( *doc, true ); rit; ++rit ) {
				    if ( ! rit->patterns.size ) continue;
				    f->printf( ",%f", rit->patterns.first()->moment[0][i+2048] );
			    }
			    f->printf( "\n" );
		    }
        }
	}
}
//情報のレポート
void miDocument::saveHistogram()
{
	ctSlice & s = slice();
	ctImage * ctimg = & s.getImage();
	const ctRegion * r = & s.regions[this->regionindex];
	if ( controlWindow->tab.index == attributeSheetIndex ) {
		miPattern * p = this->pattern();
		if ( ! p ) return;
		ctimg = & p->ctimg;
		r = & p->shape;
	}
	char filename[1024];
	if ( windows::fileSave( filename, "sample.csv", "CSV file ( *.csv )\0*.csv\0", "指定ヒストグラム保存", mainWindow->hwnd, ".csv" ) ) {
		array2<pixelInfo> info;
		analyzePixelInformation( *ctimg, info );

		retainer<file> f = file::initialize( filename, true, false );
		if ( f )
        {
		    f->printf( "CT値,分布,微分値,ガウス分布\n" );

		    array<int> histogram;
		    array<int> difference_histogram;
		    //ヒストグラムを計算
		    histogram.allocate( 4097 );
		    memset( histogram.data, 0, histogram.size * sizeof( int ) );
		    difference_histogram.allocate( 4097 );
		    memset( difference_histogram.data, 0, difference_histogram.size * sizeof( int ) );
		    double area = 0;
		    double average = 0;
		    for ( ctRegion::point_enumrator it( *r ); it; ++it ) {
			    int address = ctimg->address( it->x, it->y );
			    if ( 0 <= address ) {
				    int16 ct = ctimg->getInternal( address ).y;
		    //		if ( ! between<int16>( -100, ct, 100 ) ) continue;
				    ++histogram[ clamp( 0, ct + 2048, 4096 ) ];
				    ++difference_histogram[ clamp<int>( 0, ( int ) info( it->x, it->y ).gnorm, 2048 ) ];
				    average += ct;
				    area += 1;
			    }
		    }
		    average /= area;
		    double variance = 0;
		    for ( int i = -2048; i <= 2048; ++i ) {
			    variance += histogram[i+2048] * pow( i - average, 2 );
		    }
		    variance /= area;
            const double coefficient = 1 / ::sqrt( 2 * pi * variance );
		    for ( int i = -2048; i <= 2048; ++i ) {
			    f->printf( "%d,%lf,%lf,%lf\n", i, histogram[i+2048] / area, 
				    0 <= i ? difference_histogram[i] / area : 0,
				    coefficient * exp( - pow( ( i - average ), 2 ) / 2 / variance ) );
		    }
        }
	}
}
//情報のレポート
void miDocument::saveDistoribution()
{
	if ( ! slicesize ) return;
	const int rectsize = 4;
	char filename[1024];
	statusBar->reset();
	if ( ! windows::fileSave( filename, "sample.csv", "CSV file ( *.csv )\0*.csv\0", "指定分布保存", mainWindow->hwnd, ".csv" ) ) return;
	int width = 0;
	int height = 0;
    {
		ctSlice & s = this->slices.slices[0];
		ctImage & ctimg = s.getImage();
		width = ctimg.width;
		height = ctimg.height;
    }
	statusBar->expand( static_cast<float>( width ) / rectsize * slicesize );
	retainer<file> f = file::initialize( filename, true, false );
	if ( ! f ) return;
	f->printf( "Y/Z" );
	for ( int y = 0; y < height; y += rectsize ) {
		f->printf( ",%d", y );
	}
	f->printf( "\n" );
	for ( int i = 0; i < slicesize; ++i ) {
		ctSlice & s = this->slices.slices[i];
		ctImage & ctimg = s.getImage();
		const int width = ctimg.width;
		const int height = ctimg.height;
		const ctRegion & r = s.regions[this->regionindex];
		f->printf( "%d", i );
		for ( int y = 0; y < height; y += rectsize ) {
			int count = 0;
			float sum = 0;
			for ( int b = 0; b < rectsize; ++b ) {
				for ( int x = 0; x < width; ++x ) {
					if ( ! r.in( x, y + b ) ) continue;
					if ( ! ctimg.in( x, y + b ) ) continue;
					const int16 ct = ctimg.getInternal( x, y + b ).y;
					if ( between<int16>( -1000, ct, -750 ) ) {
						sum += float( ct );
						++count;
					}
				}
			}
			if ( count > 5 ) {
				f->printf( ",%f", sum / count );
			} else {
				f->printf( ",-2048" );
			}
			statusBar->progress();
		}
		f->printf( "\n" );
	}
	statusBar->reset();
}

void miDocument::deleteContinuousRegion( int x, int y, int zslice, int rtype )
{
	array<ctRegion> contRgn;
	statusBar->set( "連続領域探索中" );
	slices.continuousRegion( x, y, zslice, rtype, contRgn );
	statusBar->reset();
	statusBar->expand( contRgn.size );
	for ( int i = 0; i < contRgn.size; ++i ) {
		if ( contRgn[i].empty() ) continue;
		slices.slices[i].removeRegion( rtype, contRgn[i] );
		statusBar->loop();
	}
	statusBar->reset();
}

//結果領域を読み込んで、現在の領域との合致度をエクセルに出力
void miDocument::analysis( const char * in_filename, const char * out_filename, bool enableAbbrivation )
{
	bool checkControl = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) ? true : false;
	miDocument * doc = get();
	if ( ! doc ) return;
	//rgnファイルを指定する
	char rgn_filename[MAX_PATH];
	if ( ! in_filename ) {
		if ( checkControl ) enableAbbrivation = true;
		if ( ! windows::fileOpen( rgn_filename, "regions.rgn", "Regions Data(*.rgn)\0*.rgn\0", "Open Regions File", mainWindow->hwnd ) ) return;
	} else {
		strcpy( rgn_filename, in_filename );
		if ( ! file::exist( rgn_filename ) ) return;
	}
	ctSlices srcslices;
	srcslices.slices.allocate( doc->slicesize );
	regionIO::load( srcslices, rgn_filename );

	analysis( srcslices, out_filename, enableAbbrivation );
}
void miDocument::analysis( ctSlices & srcslices, const char * out_filename, bool enableAbbrivation )
{
	statusBar->reset();
	miDocument * doc = get();
	if ( ! doc ) return;

	array<ctSlice> & srces = srcslices.slices;

	if ( srces.size != doc->slicesize ) return;//スライス数が異なる場合.

	char excel_filename[MAX_PATH];
	if ( ! out_filename ) {
		if ( ! windows::fileSave( excel_filename, "output.csv", "CSV File(*.csv)\0*.csv\0", "解析結果の保存", mainWindow->hwnd, ".csv" ) ) return;
	} else {
		strcpy( excel_filename, out_filename );
	}
    const int numRegion = doc->numberRegion();
	const int numSlice = doc->slicesize;
    double sumT = 0, sumP = 0, sumTP = 0;
    array<double> Ts( numRegion ), Fs( numRegion ), Ps( numRegion ), Ns( numRegion ), TPs( numRegion ), TNs( numRegion );
    Ts = 0.0; Fs = 0.0; Ps = 0.0; Ns = 0.0; TPs = 0.0; TNs = 0.0;
    {
	    retainer<file> f = file::initialize( excel_filename, true, false );
	    if ( ! f ) return;
        struct analysisStatus {
            miDocument * doc;
            int numRegion, numSlice;
            array<ctSlice> * correct;
            array<ctSlice> * test;
            LONG abbrevated;
            array2<double> T, F, P, N, TP, TN;
            array<bool> abbreviation;
            analysisStatus( miDocument * doc_, int numRegion_, int numSlice_, array<ctSlice> * correct_, array<ctSlice> * test_ ) 
                : doc( doc_ )
                , numRegion( numRegion_ ), numSlice( numSlice_ )
                , correct( correct_ ), test( test_ )
                , abbrevated( 0 )
                , T( numSlice, numRegion ), F( numSlice, numRegion )
                , P( numSlice, numRegion ), N( numSlice, numRegion )
                , TP( numSlice, numRegion ), TN( numSlice, numRegion )
                , abbreviation( numSlice )
            {
	            T = 0.0; F = 0.0; P = 0.0; N = 0.0; TP = 0.0; TN = 0.0;
                abbreviation = false;
            }
            static void process( void * data, int index )
            {
                analysisStatus * self = ( analysisStatus * ) data;
                ctSlice & correct = (*(self->correct))[index];
                ctSlice & test = (*(self->test))[index];
		        const ctRegion & andlung = correct.regions[regionLung] & test.regions[regionLung];
		        //肺野が空で無く全部が正常な場合には全体評価から省く
		        self->abbreviation[index] = ( ( ! correct.regions[regionLung].empty() ) &&
			        correct.regions[regionLung].area() == correct.regions[regionNormal].area() );
		        if ( self->abbreviation[index] ) (ULONG)InterlockedIncrement(&self->abbrevated);
		        for ( int i = regionNormal; i < self->numRegion; ++i ) {
			        const ctRegion & truergn = correct.regions[i] & andlung;
			        const ctRegion & falsergn = andlung - truergn;
			        const ctRegion & positivergn = test.regions[i] & andlung;
			        const ctRegion & negativergn = andlung - positivergn;
			        self->T( index, i ) = truergn.area();
			        self->F( index, i ) = falsergn.area();
			        self->P( index, i ) = positivergn.area();
			        self->N( index, i ) = negativergn.area();
			        self->TP( index, i ) = ( truergn & positivergn ).area();
			        self->TN( index, i ) = ( falsergn & negativergn ).area();//真陰性.
                }
                self->doc->progress();
            }
        } status( doc, numRegion, numSlice, & srces, & doc->slices.slices );

        {//計算
	        statusBar->reset();
	        statusBar->expand( doc->slicesize * ( numRegion - regionNormal + 1 ) );
            parallel_for pf( mainWindow, thread_count, 0, numSlice, analysisStatus::process, & status );
            pf.begin();
            pf.join();
        }
	    statusBar->reset();
	    statusBar->expand( doc->slicesize * ( numRegion - regionNormal + 1 ) );

	    //領域名
	    f->printf( "," );
	    for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		    f->printf( "%s,,,,,,,,", rit->name.chars() );
	    }
	    f->printf( "\n" );
	    //項目名
	    f->printf( "No.," );
	    for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		    f->printf( "T[画素],F[画素],P[画素],N[画素],TP[画素],TN[画素],sen.[％],spe.[％]," );
	    }
	    f->printf( "\n" );
	    //各項目
	    array<double> vTs( numRegion ), vFs( numRegion ), vPs( numRegion ), vNs( numRegion ), vTPs( numRegion ), vTNs( numRegion );
	    vTs = 0.0; vFs = 0.0; vPs = 0.0; vNs = 0.0; vTPs = 0.0; vTNs = 0.0;
        const bool abbrevated = ( status.abbrevated ? true : false );
	    for ( int z = 0; z < doc->slicesize; z++ ) {
		    //ctSlice & srcslice = srces[z];
		    //ctSlice & slice = doc->slices.slices[z];
		    //const ctRegion andlung = srcslice.regions[regionLung] & slice.regions[regionLung];
		    f->printf( "%d,", z );
		    //肺野が空で無く全部が正常な場合には全体評価から省く
		    //const bool abbreviation = ( ( ! srcslice.regions[regionLung].empty() ) &&
			///    srcslice.regions[regionLung].area() == srcslice.regions[regionNormal].area() );
		    //if ( abbreviation ) abbrevated = true;
		    for ( int i = regionNormal; i < numRegion; ++i ) {
			    //const ctRegion truergn = srcslice.regions[i] & andlung;
			    //const ctRegion falsergn = andlung - truergn;
			    //const ctRegion positivergn = slice.regions[i] & andlung;
			    //const ctRegion negativergn = andlung - positivergn;
			    const int T = status.T( z, i );//truergn.area();
			    const int F = status.F( z, i );//falsergn.area();
			    const int P = status.P( z, i );//positivergn.area();
			    const int N = status.N( z, i );//negativergn.area();
			    const int TP = status.TP( z, i );//( truergn & positivergn ).area();
			    const int TN = status.TN( z, i );//( falsergn & negativergn ).area();
			    Ts[i] += T; Fs[i] += F; Ps[i] += P; Ns[i] += N; TPs[i] += TP; TNs[i] += TN;
			    if ( ! status.abbreviation[z] ) {
				    vTs[i] += T; vFs[i] += F; vPs[i] += P; vNs[i] += N; vTPs[i] += TP; vTNs[i] += TN;
			    }
			    f->printf( "%d,%d,%d,%d,%d,%d,", T, F, P, N, TP, TN );
			    if ( T ) {
				    f->printf( "%.4f,", 100.0 * TP / T );
			    } else {
				    f->printf( "-," );
			    }
			    if ( F ) {
				    f->printf( "%.4f,", 100.0 * TN / F );
			    } else {
				    f->printf( "-," );
			    }
			    statusBar->progress();
		    }
		    f->printf( "\n" );
	    }
	    if ( abbrevated && enableAbbrivation ) {
		    Ts = vTs; Fs = vFs; Ps = vPs; Ns = vNs; TPs = vTPs; TNs = vTNs;
		    f->printf( "以下は正解領域が正常領域のみのスライスを無視した合計である（特殊）\n" );
	    }
	    f->printf( "小計," );
	    for ( int i = regionNormal; i < numRegion; ++i ) {
		    sumT += Ts[i];
		    sumP += Ps[i];
		    sumTP += TPs[i];
		    f->printf( "%f,%f,%f,%f,%f,%f,", Ts[i], Fs[i], Ps[i], Ns[i], TPs[i], TNs[i] );
		    if ( Ts[i] ) {
			    f->printf( "%.4f,", 100.0 * TPs[i] / Ts[i] );
		    } else {
			    f->printf( "-," );
		    }
		    if ( Fs[i] ) {
			    f->printf( "%.4f,", 100.0 * TNs[i] / Fs[i] );
		    } else {
			    f->printf( "-," );
		    }
	    }
	    f->printf( "\n" );
	    f->printf( "まとめ\n" );
	    f->printf( "全てのカテゴリ合計," );
	    f->printf( "%f,-,%f,-,%f,-\n", sumT, sumP, sumTP );
	    if ( sumT ) {
		    f->printf( "全体の正確度[％],%f\n", 100.0 * sumTP / sumT );
	    }
	    f->printf( "合計," );
	    for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		    f->printf( "%s,", rit->name.chars() );
	    }
	    f->printf( "\n感度[％]," );
	    for ( int i = regionNormal; i < numRegion; ++i ) {
		    if ( Ts[i] ) {
			    f->printf( "%.4f,", 100.0 * TPs[i] / Ts[i] );
		    } else {
			    f->printf( "-," );
		    }
	    }
	    f->printf( "\n特異度[％]," );
	    for ( int i = regionNormal; i < numRegion; ++i ) {
		    if ( Fs[i] ) {
			    f->printf( "%.4f,", 100.0 * TNs[i] / Fs[i] );
		    } else {
			    f->printf( "-," );
		    }
	    }
	    f->printf( "\n割合[％]," );
	    for ( int i = regionNormal; i < numRegion; ++i ) {
		    if ( Fs[i] ) {
			    f->printf( "%.4f,", 100.0 * Ts[i] / sumT );
		    } else {
			    f->printf( "-," );
		    }
	    }
	    f->printf( "\n" );
	    f->printf( "説明\n" );
	    f->printf( "T：読み込んだ真（正解）の領域の面積[画素]\n" );
	    f->printf( "F：読み込んだ真以外（偽）の領域の面積[画素]\n" );
	    f->printf( "P：現在のそのクラス（陽性）の領域の面積[画素]\n" );
	    f->printf( "N：現在のそのクラス以外（陰性）の領域の面積[画素]\n" );
	    f->printf( "TP：真陽性：正解(T)で陽性(P)の面積[画素]\n" );
	    f->printf( "TN：真陰性：正解以外(F)で陰性(N)の面積[画素]\n" );
	    f->printf( "FP：偽陽性：正解以外(F)で陽性(P)の面積[画素]\n" );
	    f->printf( "FN：偽陰性：正解(T)で陰性(N)の面積[画素]\n" );
	    f->printf( "sen.：感度（TP/(TP+FN)）==(TP/T)[％]\n" );
	    f->printf( "spe.：特異度（TN/(TN+FP)）==(TN/F)[％]\n" );
	    f->printf( "割合：真の値の他のカテゴリの合計に対する割合[％]\n" );
	    f->printf( "注：肺の領域が異なる場合には両方の論理積の肺領域を全体領域とする\n" );
    }
	statusBar->reset();
	//正解率のみを重複して出しておく
    {
	    char sum_filename[MAX_PATH];
	    if ( windows::fileSave( sum_filename, "result.csv", "CSV File(*.csv)\0*.csv\0", "正答率のみの追加保存", mainWindow->hwnd, ".csv" ) )
        {
	        retainer<file> ft = file::initialize( sum_filename, true, false, true );
	        if ( ft ) {
		        ft->printf( "%s,", excel_filename );
		        if ( sumT ) {
			        ft->printf( "%f,", 100.0 * sumTP / sumT );
		        } else {
			        ft->printf( "-," );
		        }
		        for ( int i = regionNormal; i < numRegion; ++i ) {
			        if ( Ts[i] ) {
				        ft->printf( "%.4f,", 100.0 * TPs[i] / Ts[i] );
			        } else {
				        ft->printf( "-," );
			        }
		        }
		        for ( int i = regionNormal; i < numRegion; ++i ) {
			        if ( Ts[i] ) {
				        ft->printf( "%.4f,", 100.0 * TNs[i] / Fs[i] );
			        } else {
				        ft->printf( "-," );
			        }
		        }
		        ft->printf( "\n" );
            }
	    } 
    }
	statusBar->reset();
}
void miDocument::saveInformation()
{
	miDocument * doc = get();
	if ( ! doc ) return;

	//出力ファイル名 
	char excel_filename[MAX_PATH];
	if ( ! windows::fileSave( excel_filename, "output.csv", "CSV File(*.csv)\0*.csv\0", "Save Result", mainWindow->hwnd, ".csv" ) ) return;

	retainer<file> f = file::initialize( excel_filename, true, false );
	if ( ! f ) return;
	statusBar->reset();
	statusBar->expand( doc->slicesize * ( doc->numberRegion() - regionNormal + 1 ) );

	//領域名
	f->printf( "No" );
	for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
		f->printf( ",%s Average,%s Standard Deviation,%S area", doc->regionTypes[i].name.chars(), doc->regionTypes[i].name.chars(), doc->regionTypes[i].name.chars() );
	}
	f->printf( ",Total Average,Total Standard Deviation,Total Area\n" );
	//各項目
	array<double> averages( numberRegion() ), averages2( numberRegion() ), areas( numberRegion() );
	for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
		averages[i] = 0.0; averages2[i] = 0.0; areas[i] = 0.0;
	}
	for ( int z = 0; z < doc->slicesize; z++ ) {
		ctSlice & slice = doc->slices.slices[z];
		f->printf( "%d,", z );
		double slice_area = 0;
		double slice_average = 0;
		double slice_average2 = 0;
		for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
			double area = slice.regions[i].area();
			if ( area )
			{
				double average = slice.getAverage( i );
				double average2 = slice.getAverageSquare( i );
				averages[i] += average * area;
				averages2[i] += average2 * area;
				areas[i] += area;
				slice_average += average * area;
				slice_average2 += average2 * area;
				slice_area += area;
                f->printf( "%lf,%lf,%lf,", average, ::sqrt( fabs( average2 - average * average ) ), area );
			} else {
				f->printf( ",,%lf,", area );
			}
			statusBar->progress();
		}
		if ( slice_area )
		{
			slice_average /= slice_area;
			slice_average2 /= slice_area;
            f->printf( "%lf,%lf,%lf\n", slice_average, ::sqrt( fabs( slice_average2 - slice_average * slice_average ) ), slice_area );
		} else {
			f->printf( ",,%lf\n", slice_area );
		}
	}
	f->printf( "All Slices," );
	double total_area = 0;
	double total_average = 0;
	double total_average2 = 0;
	for ( int i = regionNormal; i < doc->numberRegion(); ++i ) {
		total_area += areas[i];
		if ( areas[i] )
		{
			total_average += averages[i];
			total_average2 += averages2[i];
            f->printf( "%lf,%lf,%lf,", averages[i] / areas[i], ::sqrt( fabs( averages2[i] / areas[i] - averages[i] / areas[i] * averages[i] / areas[i] ) ), areas[i] );
		} else {
			f->printf( ",,%lf,", areas[i] );
		}
	}
	if ( total_area )
	{
		total_average /= total_area;
		total_average2 /= total_area;
        f->printf( "%lf,%lf,%lf\n", total_average, ::sqrt( fabs( total_average2 - total_average * total_average ) ), total_area );
	} else {
		f->printf( ",,%lf\n", total_area );
	}
	statusBar->reset();
}

void miDocument::displayAverageVariance()
{
	ctSlice & s = slice();
	ctImage * ctimg = & s.getImage();
	const ctRegion * r = & s.regions[this->regionindex];
	if ( controlWindow->tab.index == attributeSheetIndex ) {
		miPattern * p = this->pattern();
		if ( ! p ) return;
		ctimg = & p->ctimg;
		r = & p->shape;
	}
	int area = 0;
	double average = 0;
	for ( ctRegion::point_enumrator it( *r ); it; ++it ) {
		if ( ! ctimg->in( it->x, it->y ) ) continue;
		const double ctv = double( ctimg->getInternal( it->x, it->y ).y );
//		if ( ! between<int>( -80, ctv, 80 ) ) continue;
		average += ctv;
		++area;
	}
	average /= area;
	double variance = 0;
	for ( ctRegion::point_enumrator it( *r ); it; ++it ) {
		if ( ! ctimg->in( it->x, it->y ) ) continue;
		const double ctv = double( ctimg->getInternal( it->x, it->y ).y );
//		if ( ! between<int>( -80, ctv, 80 ) ) continue;
		const double v = ( ctv - average );
		variance += v * v;
	}
	variance /= area;
    double standardDeviation = ::sqrt( variance );
	char wk[1024];
	sprintf( wk, "average:%f, variance:%f, standard deviation:%f", average, variance, standardDeviation );
	MessageBox( mainWindow->hwnd, wk, "analysis result", MB_OK );
}
void miDocument::displayAverageRegion( ctRegion & rgn )
{
	ctSlice & s = slice();
	ctImage & ctimg = s.getImage();
	int area = 0;
	double average = 0;
	for ( ctRegion::point_enumrator it( rgn ); it; ++it ) {
		if ( ! ctimg.in( it->x, it->y ) ) continue;
		int16 ct = ctimg.get( it->x, it->y ).y;
		if ( ct == -2048 ) continue;
		average += ct;
		++area;
	}
	if ( ! area ) return;
	average /= area;
	char wk[1024];
	sprintf( wk, "average:%.1f", average );
	MessageBox( mainWindow->hwnd, wk, "平均値", MB_OK );
}

void miDocument::exportPatterns()
{
	miDocument * doc = get();
	if ( doc ) {
		return;
		_tzset();
		time_t nowtime;
		time( & nowtime );
		tm * nowtm = localtime( &nowtime );
		string newpath = doc->path + "\\" + 
			string( nowtm->tm_year + 1900 ) + "年" + 
			string( nowtm->tm_mon + 1 ) + "月" + 
			string( nowtm->tm_mday ) + "日" + 
			string( nowtm->tm_hour ) + "時" + 
			string( nowtm->tm_min ) + "分" + 
			string( nowtm->tm_sec ) + "秒";
		CreateDirectory( newpath, NULL );
		savePattern( doc->drive, newpath, true );
	}
}

BOOL SHGetIDListFromPath( const char * path, ITEMIDLIST * iidl )
{
	LPSHELLFOLDER ish;
	if ( SHGetDesktopFolder(&ish) != NOERROR ) return FALSE;

	OLECHAR opath[MAX_PATH];
	ULONG osize;
	ULONG attr;	//属性を受け取ります。

	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, path, -1, opath, MAX_PATH );

	//　実際にITEMIDLISTを取得します。
	if ( ish->ParseDisplayName( NULL, NULL, opath, &osize, &iidl, &attr) != NOERROR )
		iidl = NULL;

	ish->Release();

	return iidl ? TRUE : FALSE;
}
void SHFreeIDList( ITEMIDLIST * iidl )
{
	if( ! iidl ) return;
	LPMALLOC pMalloc;
	if( ::SHGetMalloc( &pMalloc ) != NOERROR ) return;
	pMalloc->Free( iidl );
	pMalloc->Release();
}	

int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    if ( uMsg == BFFM_INITIALIZED ){
        SendMessage( hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData );
    }
    return 0;
}
void miDocument::clearPattern()
{
	miDocument * doc = get();
	if ( ! doc ) return;

	for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		rit->patterns.release();
	}
	statusBar->reset();
	controlWindow->updateClusters();
	doc->change();
}

void miDocument::loadPattern( string drive, string path, bool clear, bool update )
{
	miDocument * doc = get();
	if ( ! doc ) return;

	if ( ! drive.length() ) {
		char folder[MAX_PATH];
		_makepath( folder, doc->drive, doc->path, "", "" );
		BROWSEINFO bi = { NULL };
		bi.hwndOwner = mainWindow->hwnd;
		bi.pszDisplayName = folder;
		bi.lpszTitle = "パターンを読み込むフォルダを指定してください";
		bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM) folder;
		ITEMIDLIST * iidl = SHBrowseForFolder(&bi);
		if ( ! iidl ) return;
		SHGetPathFromIDList( iidl, folder );
		SHFreeIDList( iidl );
		char _drive[MAX_PATH], _path[MAX_PATH], _name[MAX_PATH], _ext[MAX_PATH];
		_splitpath( folder, _drive, _path, _name, _ext );
		drive = _drive;
		path = string( _path ) + _name + _ext;
	}
	
	//教師情報をディレクトリから取得
	statusBar->reset();
	statusBar->expand( 16 );
	for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		if ( clear ) rit->patterns.release();
		//{
		//	retainer<miPattern> tp( new miPattern );
		//	if ( tp->initialize( drive, path, rit->name + ".rgn", NULL ) ) {
		//		rit->patterns.push_back( tp );
		//	}
		//}
		//古いパターンの読み込み
		//for ( int j = 0; ; ++j ) {
		//	//複数の教師情報（連番）を読み込み
		//	retainer<miPattern> tp( new miPattern );
		//	statusBar->loop();
		//	if ( tp->initialize( drive, path, rit->name + ( 0 <= j ? string( j ) : "" ) + ".rgn", NULL ) ) {
		//		rit->patterns.push_back( tp );
		//	} else {
		//		break;
		//	}
		//}
		//if ( rit->name == "normal" ) {
		//	{
		//		retainer<miPattern> tp( new miPattern );
		//		if ( tp->initialize( drive, path, string( "alveolus" ) + ".rgn", NULL ) ) {
		//			rit->patterns.push_back( tp );
		//		}
		//	}
		//	//古いパターンの読み込み
		//	for ( int j = 0; ; ++j ) {
		//		//複数の教師情報（連番）を読み込み
		//		retainer<miPattern> tp( new miPattern );
		//		statusBar->loop();
		//		if ( tp->initialize( drive, path, string( "alveolus" ) + ( 0 <= j ? string( j ) : "" ) + ".rgn", NULL ) ) {
		//			rit->patterns.push_back( tp );
		//		} else {
		//			break;
		//		}
		//	}
		//}
		//新規教師画像群読み込み
		char search_path[MAX_PATH];
		_makepath( search_path, drive, path, rit().name + "_*", ".tea" );
		WIN32_FIND_DATA wfd;
		HANDLE fff = FindFirstFile( search_path, & wfd );
		if ( fff != INVALID_HANDLE_VALUE ) {
			do {
				retainer<miPattern> tp( new miPattern );
				//教師情報を読み込む
				string postfix;
				int prevlength = rit->name.length() + 1;
				if ( strlen( wfd.cFileName ) > prevlength ) {
					postfix = wfd.cFileName + prevlength;
					postfix.tolower();
					if ( postfix.length() >= 4 ) {
						postfix -= ".tea";
					}
					if ( postfix.isinteger() ) {
						postfix = "";
					}
				}
				//接尾辞が重複しないようにチェック
				if ( postfix.length() ) {
					for ( array< retainer<miPattern> >::iterator itc( rit->patterns ); itc; ++itc ) {
						if ( itc()().postfix == postfix ) {
							postfix = "";
							break;
						}
					}
				}
				statusBar->loop();
				if ( tp->initialize( drive, path, wfd.cFileName, postfix ) ) {
					rit->patterns.push_back( tp );
				}
			} while( FindNextFile( fff, & wfd ) );
		}
		FindClose( fff );
		statusBar->loop();
	}
	statusBar->reset();
	if ( update ) {
		controlWindow->updateClusters();
		doc->change();
	}
}
void miDocument::deletePattern( const string & drive, const string & path )
{
	miDocument * doc = get();
	if ( ! doc ) return;
	if ( ! drive.length() ) return;
	for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		//今ある教師データを削除
		////単一の教師情報を削除
		//miPattern::remove( drive, path, rit->name );
		//for ( int j = 0; ; ++j ) {
		//	//複数の教師情報（連番）を削除
		//	if ( ! miPattern::remove( drive, path, rit->name + string( j ) ) ) 
		//		break;
		//}
		//if ( rit->name == "normal" ) {
		//	miPattern::remove( drive, path, "alveolus" );
		//	for ( int j = 0; ; ++j ) {
		//		//複数の教師情報（連番）を削除
		//		if ( ! miPattern::remove( drive, path, string( "alveolus" ) + string( j ) ) ) 
		//			break;
		//	}
		//}
		//複数の教師情報（接尾辞）を削除 接尾辞は数字のみは不可
		char search_path[MAX_PATH];
		_makepath( search_path, drive, path, rit->name + "_*", ".tea" );
		WIN32_FIND_DATA wfd;
		HANDLE fff = FindFirstFile( search_path, & wfd );
		if ( fff != INVALID_HANDLE_VALUE ) {
			char _drive[MAX_PATH], _path[MAX_PATH], _filename[MAX_PATH], _extension[MAX_PATH];
			do {
				//教師情報を削除
				_splitpath( wfd.cFileName, _drive, _path, _filename, _extension );
				miPattern::remove( drive, path, _filename );
			} while( FindNextFile( fff, & wfd ) );
		}
		FindClose( fff );
	}
}
void miDocument::savePattern( string drive, string path, bool clear )
{
	miDocument * doc = get();
	if ( ! doc ) return;
	if ( ! drive.length() ) {
		char folder[MAX_PATH];
		_makepath( folder, doc->drive, doc->path, "", "" );
		BROWSEINFO bi = { NULL };
		bi.hwndOwner = mainWindow->hwnd;
		bi.pszDisplayName = folder;
		bi.lpszTitle = "パターンを保存するフォルダを指定してください";
		bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM) folder;
		ITEMIDLIST * iidl = SHBrowseForFolder(&bi);
		if ( ! iidl ) return;
		SHGetPathFromIDList( iidl, folder );
		SHFreeIDList( iidl );
		char _drive[MAX_PATH], _path[MAX_PATH], _name[MAX_PATH], _ext[MAX_PATH];
		_splitpath( folder, _drive, _path, _name, _ext );
		drive = _drive;
		path = string( _path ) + _name + _ext;
	}
	//今ある教師データを削除
	if ( clear ) deletePattern( drive, path );
	for ( regionIterator rit( *doc, true ); rit; ++rit ) {
		//教師データを保存
		int j = 0;
		for ( array< retainer<miPattern> >::iterator it( rit->patterns ); it; ++it, ++j ) {
			string wk = rit->name + "_" + string( j );//デフォルトでは連番の接尾辞をつける
			const string & postfix = it()().postfix;
			if ( postfix.length() && ! postfix.isinteger() ) {
				//接尾辞形式
				wk = rit->name + "_" + postfix;
			}
			
			it()->finalize( drive, path, wk );
		}
	}
}
//教師のパターン設定の情報出力.
void miDocument::savePatternSetting()
{
	char filename[MAX_PATH];
	if ( ! windows::fileSave( filename, "教師パターン設定", "レジストリデータ(*.reg)\0*.reg\0", "Save Registry File", mainWindow->hwnd, ".reg" ) ) return;
	retainer<file> f = file::initialize( filename, true, false );
    if ( ! f ) return;//失敗.
	f->printf( "Windows Registry Editor Version 5.00\n" );

    //データ取得.
	list< pair<string> > teacherslist;
	regsGet( "teachers", teacherslist );//teachersディレクトリ内に，キー名は連番若しくは任意で，データに領域名が入っている
    list<string> names;
    names.push_back( "human" );
    names.push_back( "lung" );
    names.push_back( "normal" );
	int index = 0;
	while ( teacherslist.size && index < 100 ) {
		string string_index( index );
		for ( list< pair<string> >::iterator it( teacherslist ); it; ++it ) {
			if ( it->left == string_index ) {
                bool found = false;
                for ( list<string>::iterator rit( names ); rit; ++rit ) {
                    if ( *rit == it->right )
                    {
                        found = true;
                        break;
                    }
                }
                if ( ! found ) names.push_back( it->right );
				teacherslist.pop( it() );
				break;
			}
		}
		++index;
	}
	//連番以外のデータがあった場合は，無順序に追加
	for ( list< pair<string> >::iterator it( teacherslist ); it; ++it ) {
        bool found = false;
        for ( list<string>::iterator rit( names ); rit; ++rit ) {
            if ( *rit == it->right )
            {
                found = true;
                break;
            }
        }
        if ( ! found ) names.push_back( it->right );
	}
    f->printf( "\n[HKEY_CURRENT_USER\\SOFTWARE\\Akira\\MedicalImage]\n" );
    for ( list<string>::iterator it( names ); it; ++it ) {
        string name = *it;
        pixel color;
		if ( regGet( name, color ) )
        {
            f->printf( "\"%sR\"=\"%d\"\n", name.chars(), color.r );
            f->printf( "\"%sG\"=\"%d\"\n", name.chars(), color.g );
            f->printf( "\"%sB\"=\"%d\"\n", name.chars(), color.b );
            f->printf( "\"%sA\"=\"%d\"\n", name.chars(), color.a );
        }
	}
    f->printf( "\n[HKEY_CURRENT_USER\\SOFTWARE\\Akira\\MedicalImage\\teachers]\n" );
    index = 0;
    for ( list<string>::iterator it( names ); it; ++it, ++index ) {
        string name = *it;
        f->printf( "\"%d\"=\"%s\"\n", index, name.chars() );
	}
}

void miDocument::recognitionNetlike()
{
	bool allSlices = ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) ? false : true;
	if ( this->regionindex <= regionNormal ) {
		MessageBox( mainWindow->hwnd, "網状の個所を検出し，現在選択されている領域に追加するため，患部領域を選択してから実行してください．またCTRLキーを押しながら行うと現スライスのみを処理します．", "確認", MB_OK );
		return;
	}
	if ( MessageBox( mainWindow->hwnd, "網状の個所を検出し，現在選択されている領域に追加します．またCTRLキーを押しながら行うと現スライスのみを処理します．", "確認", MB_OKCANCEL ) == IDCANCEL ) return;
	//const int thresholds[] = { -650, -700, -750, -800, -850, -900, -950 };
	const int thresholds[] = { -750 };
	if ( allSlices ) {
		statusBar->expand( slices.slices.size * sizeof( thresholds ) / sizeof( int ) );
	} else {
		statusBar->expand( sizeof( thresholds ) / sizeof( int ) );
	}
	for ( ctSlices::iterator its( slices.createIterator( allSlices ) ); its; ++its ) {
		ctSlice & slice = its();
		ctImage & ctimg = slice.getImage();

		const ctRegion & lung = slice.regions[regionLung];
		ctRegion netlike;
		for ( int i = 0; i < sizeof( thresholds ) / sizeof( int ); ++i ) {
			ctRegion allholes = slice.ct2region( -2048, thresholds[i] ) - netlike;
            std::list<ctRegion> holes;
			allholes.divideMultiRegion( holes );
			ctRegion outline;
			array<ctRegion*> holepointer;
			array<int> holeareas;
            const size_t holes_size = holes.size();
			holepointer.reserve( holes_size );
			holeareas.reserve( holes_size );
            for ( std::list<ctRegion>::iterator it = holes.begin(); it != holes.end(); ++it ) {
				ctRegion & hole = *it;
				int area = hole.area();
				hole.opening( hole, 1 );
				if ( 1000 < area ) continue;//大きい孔は削除
				//輪郭抽出
				hole.dilation( outline, 1 );
				outline -= hole;
				int outlinearea = outline.area();
				if ( 20 < area && //ある程度の大きさがあるときに円度判定
                    ::sqrt( area / pi ) / ( outlinearea / pi / 2.0 ) < 0.6 ) continue;//円度が低い
				//太目の輪郭抽出
				hole.dilation( outline, 2 );
				outline = ( outline - hole ) & ctRegion( ctRegion::rectangle::size( ctimg.width, ctimg.height ) );
				outlinearea = outline.area();
				if ( outlinearea < 1 ) continue;//ありえないと思うが，落ちないように
				//輪郭のCT値をリストアップ
				array<decimal> outlineCTs; outlineCTs.reserve( outlinearea );
				for ( ctRegion::point_enumrator ito( outline ); ito; ++ito ) {
					outlineCTs.push_back( static_cast<decimal>( ctimg.getInternal( ito->x, ito->y ).y ) );
				}
				//中央値を求める
				qsort( outlineCTs.data, outlineCTs.size, sizeof( decimal ), compairDecimal );
				decimal outlineCT = outlineCTs[outlineCTs.size/2];
				//内部の平均CT値を求める
				decimal averageCT = 0;
				for ( ctRegion::point_enumrator ito( hole ); ito; ++ito ) {
					averageCT += static_cast<decimal>( ctimg.getInternal( ito->x, ito->y ).y ) / area;
				}
				if ( outlineCT - averageCT < 75 ) continue;//周りの壁のCT値が低い
				
				holepointer.push_back( & hole );
				holeareas.push_back( area );
			}
			/*
			//繋がっているリストを作成
			array< array<int> > links( holepointer.size );
			ctRegion holeexpand;
			for ( int i = 0; i < links.size; ++i ) {
				( *holepointer[i] ).dilation( holeexpand, 4 );
				for ( int j = i + 1; j < links.size; ++j ) {
					if ( ! ( holeexpand & ( *holepointer[j] ) ).empty() ) {
						if ( ! between<decimal>( decimal( 0.2 ), holeareas[i] / decimal( holeareas[j] ), 5.0 ) ) continue;
						links[i].push_back( j );
						links[j].push_back( i );
					} else if ( ! ( holeexpand & netlike ).empty() ) {
						links[i].push_back( i );
					}
				}
			}
			//２個以上繋がっている物を合成
			allholes.initialize();
			for ( int i = 0; i < links.size; ++i ) {
				//リンクが無い時に削除（これは必須か？）
				if ( links[i].size == 0 ) continue;
				//リンクが２個の時に削除（これは必須か？）
				if ( links[i].size == 1 && links[links[i].first()].size < 2 ) continue;
				allholes |= *holepointer[i];
			}
			*/
			allholes.initialize();
			for ( int i = 0; i < holepointer.size; ++i ) {
				allholes |= *holepointer[i];
			}
			ctRegion expanded;
			allholes.dilation( expanded, 3 );
			netlike |= expanded;
			statusBar->progress();
		}
		slice.changeRegion( regionindex, netlike );
	}
	change();
	statusBar->reset();
	imageWindow->invalidate();
}

//AGLファイルとして，シーングラフを保存
void miDocument::saveAsGraph()
{
#ifdef VIEW3D
	char filename[MAX_PATH];
	if ( ! windows::fileSave( filename, "slices.agl", "Scene Graph File(*.agl)\0*.agl\0", "Save Scene Graph", mainWindow->hwnd ) ) return;

	miView * view = miView::get();
	//現在の表示状態で全体を更新
	view->mode( view->windowSize, view->windowLevel, controlWindow->viewmode.index, controlWindow->viewside.index );
	view->update();

	view->graph->save( filename );
#endif
}
void miDocument::progress()
{
    check_critical_section ccs( cs_statusbar );
    statusBar->progress();
}
