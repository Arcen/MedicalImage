#include "main.h"
#include "adom.h"
#include "document.h"
#include "view.h"
#include "resource.h"
#include "state.h"
#include "dicom.h"
#include "dicomexport.h"
#include "dicomseries.h"
#include <vector>
#include <algorithm>

void pointsToRegion( list< point2<int> > & points, region<int> & rgn, int width, int height );

bool strless( const char * e1, const char * e2 )
{
	return stricmp( ( const char * ) e1, ( const char * ) e2 ) < 0;
}
void sort( array<string> & data )
{
	std::vector<char *> chars;
	chars.reserve( data.size );
	for ( int i = 0; i < data.size; ++i ) chars.push_back( data[i].chars() );
	std::sort( chars.begin(), chars.end(), strless );
	array<string> new_data;
	new_data.reserve( data.size );
	for ( int i = 0; i < data.size; ++i ) {
		new_data.push_back( string( chars[i] ) );
	}
	data = new_data;
}

bool pairless( const std::pair<int,char*> & e1, const std::pair<int,char*> & e2 )
{
	return e1.first < e2.first;
}

void sort_by( array<string> & data, array<int> & comp_data )
{
	std::vector< std::pair<int,char*> > chars;
	chars.reserve( data.size );
	for ( int i = 0; i < data.size; ++i ) chars.push_back( std::pair<int,char*>(comp_data[i], data[i].chars()) );
	std::sort( chars.begin(), chars.end(), pairless );
	array<string> new_data;
	new_data.reserve( data.size );
	for ( int i = 0; i < data.size; ++i ) {
		new_data.push_back( string( chars[i].second ) );
	}
	data = new_data;
}

bool trackDocument::openDicom( const string & documentPath )
{
	sourceDicom = true;
	// DICOM情報取得
	retainer<file> f = file::initialize( documentPath + "DICOMDIR", false, true );
	array<dicomSeriesInformation> series;
	series.reserve( 100 );
	dicom & dcm = dicom::get();
	if ( f ) {
		MSXML2::IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
		if ( ! dcm.convert( f(), domDocument ) ) return false;

		//ディレクトリ取得
		int firstChildOffset = 0;
		MSXML2::IXMLDOMElementPtr elementDirecotry = NULL;
		MSXML2::IXMLDOMElementPtr element = domDocument->documentElement;
		for ( element = element->firstChild; element != NULL; element = element->nextSibling ) {
			string name = static_cast<const char*>( static_cast<_bstr_t>( element->nodeName ) );
			MSXML2::IXMLDOMAttributePtr attr = element->getAttributeNode( _bstr_t( "name" ) );
			if ( attr ) {} else continue;
			name = static_cast<const char*>( static_cast<_bstr_t>( attr->value ) );
			if ( name == "ルートディレクトリエンティティの最初のディレクトリレコードのオフセット" ) {
				attr = element->getAttributeNode( _bstr_t( "value" ) );
				if ( attr ) {} else continue;
				firstChildOffset = atoi( static_cast<const char*>( static_cast<_bstr_t>( attr->value ) ) );
			} else if ( name == "ディレクトリレコードシーケンス" ) {
				elementDirecotry = element;
			}
		}
		dicomListup( elementDirecotry, firstChildOffset, documentPath, series );
	} else {
		series.push_back( dicomSeriesInformation() );
		dicomSeriesInformation & dsi = series.first();
		dsi.targetOffset = 0;
		WIN32_FIND_DATA wfd;
		HANDLE fff = FindFirstFile( documentPath + "*", & wfd );
		if ( fff != INVALID_HANDLE_VALUE ) {
			do {
				if ( 
					stricmp( ".", wfd.cFileName ) != 0 &&
					stricmp( "..", wfd.cFileName ) != 0  ) {
					dsi.files.push_back( documentPath + string( wfd.cFileName ) ); 
				}
			} while( FindNextFile( fff, & wfd ) );
		}
		FindClose( fff );
		sort( dsi.files );
		dsi.thickness = 1;
		dsi.fieldOfViewMM = 1;
	}
	if ( series.size == 0 ) return false;
	dicomSeriesSelect dss( mainWindow->hwnd );
	dss.series = & series;
	dss.index = 0;
	if ( ! dss.call() ) return false;
	if ( dss.index == -1 ) return false;

	dicomSeriesInformation & dsi = series[dss.index];
	if ( ! dsi.image_ids.empty() ) {
		sort_by( dsi.files, dsi.image_ids );
	}

	array<string> & filenames = dsi.files;
	int readoffset = dss.start;//読み込み開始位置
	int readstep = dss.space;//読み込み時のスライス間の間隔
	int readsize = maximum<int>( 0, filenames.size - readoffset );//読み込みを行うスライス数
	if ( readsize ) {
		readsize = ( readsize - 1 ) / readstep + 1;//飛ばすために読み込むスライス数が減る
		readsize = minimum<int>( readsize, dss.maximumNumber );//限度を超えないように設定
	}
	if ( ! readsize ) return false;

	originalImages.reserve( readsize );
	maxValues.reserve( readsize );

	array<string>::iterator itn( filenames );
	for ( int i = 0; i < readoffset; ++i ) ++itn;//skip
	bool first = true;
	checkMinimum<int> minValueCheck;
	checkMaximum<int> maxValueCheck;
	width = height = 0;
	for ( ; itn; ++itn ) {
		imageInterface< pixelLuminance<int16> > img;
		img.topdown = true;

		retainer<file> f = file::initialize( itn(), false, true );
		if ( f ) {
			MSXML2::IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
			if ( dcm.convert( f(), domDocument ) ) {
				if ( dcm.read( f(), domDocument, img ) ) {
					if ( first ) {
						first = false;
						string value;
						if ( dcm.getAttribute( domDocument, "画素間隔", value ) ) {
							pixelSize = value.tof();
						}
					}
					width = maximum( width, img.width );
					height = maximum( height, img.height );
					originalImages.push_back( img );
					imageFilenames.push_back( itn() );
					checkMaximum<int> mx;
					for ( int y = 0; y < img.height; ++y ) {
						for ( int x = 0; x < img.width; ++x ) {
							int16 v = img.getInternal( x, y ).y;
							mx( v );
							minValueCheck( v );
						}
					}
					maxValueCheck( mx() );
					maxValues.push_back( mx() );
				}
			}
			--readsize;
		}
		if ( readsize <= 0 ) break;
		for ( int i = 1; i < readstep; ++i ) ++itn;//スライスを読み飛ばす
	}
	minValue = minValueCheck();
	if ( minValue < 0 ) {
		for ( int i = 0; i < originalImages.size; ++i ) {
			maxValues[i] -= minValue;
			imageInterface< pixelLuminance<int16> > & img = originalImages[i];
			pixelLuminance<int16> p;
			for ( int y = 0; y < img.height; ++y ) {
				for ( int x = 0; x < img.width; ++x ) {
					p.y = img.getInternal( x, y ).y - minValue;
					img.setInternal( x, y, p );
				}
			}
		}
		maxValueCheck.value -= minValue;
	}
	maxValue = maxValueCheck();
	windowLevelBar->set( 0, maxValue );
	windowSizeBar->set( 1, maxValue );
	windowLevelBar->set( maxValue / 2 );
	windowSizeBar->set( maxValue );
	if ( ! sizeTime() ) return false;
	initializeAfterOpen();
	return true;
}
bool trackDocument::openRaw( const string & documentPath )
{
	sourceDicom = false;
	this->documentPath = documentPath;
	int forcast = 0;
	for ( int i = 3; ; ++i ) {
		if ( ! file::exist( documentPath + string( i ) ) ) {
			break;
		}
		++forcast;
	}
	originalImages.reserve( forcast );
	maxValues.reserve( forcast );
	checkMaximum<int> maxValueCheck;
	bool first = true;
	for ( int i = 3; ; ++i ) {
		imageInterface< pixelLuminance<int16> > img;
		retainer<file> f = file::initialize( documentPath + string( i ), false, true );
		if ( ! f ) break;
		double fsize = f->getSize();
		int edge = sqrt( fsize / 2 );
		img.topdown = true;
		array<char> wk( edge * edge * 2 );
		img.create( edge, edge );
		f->seek( 0, SEEK_SET );
		if ( fsize == wk.size && f->read( wk.data, wk.size ) == wk.size ) {
			if ( first ) {
				width = height = edge;
				first = false;
			} else {
				if ( width != edge ) continue;
			}
			_swab( reinterpret_cast<char*>( wk.data ), reinterpret_cast<char*>( img.data ), wk.size );
			checkMaximum<int> mx;
			for ( int y = 0; y < img.height; ++y ) {
				for ( int x = 0; x < img.width; ++x ) {
					mx( img.getInternal( x, y ).y );
				}
			}
			maxValueCheck( mx() );
			maxValues.push_back( mx() );
			originalImages.push_back( img );
			imageFilenames.push_back( documentPath + string( i ) );
		}
	}
	maxValue = maxValueCheck();
	windowLevelBar->set( 0, maximum( 512, maxValue ) );
	windowSizeBar->set( 1, maximum( 512, maxValue ) );
	windowLevelBar->set( maxValue / 2 );
	windowSizeBar->set( maxValue );
	if ( ! sizeTime() ) return false;
	initializeAfterOpen();
	return true;
}
void trackDocument::initializeAfterOpen()
{
	//エッジ画像作成
	horizontalEdgeImages.allocate( sizeTime() );
	verticalEdgeImages.allocate( sizeTime() );
	for ( int i = 0; i < sizeTime(); ++i ) {
		imageInterface< pixelLuminance<int16> > & original = originalImages[i];
		imageInterface< pixelLuminance<int16> > & horz = horizontalEdgeImages[i];
		imageInterface< pixelLuminance<int16> > & vert = verticalEdgeImages[i];
		horz.create( original.width, original.height );
		vert.create( original.width, original.height );
		pixelLuminance<int16> p0( 0 );
		horz.fill( p0 );
		vert.fill( p0 );
		const int window = 1;
		const int count = square( 2 * window + 1 );
		for ( int y = window; y < original.height - window - 1; ++y ) {
			for ( int x = window; x < original.width - window - 1; ++x ) {
				double ym = 0, yp = 0, xm = 0, xp = 0;
				for ( int a = 1; a <= window; ++a ) {
					for ( int b = -window; b <= window; ++b ) {
						ym += original.getInternal( x + b, y - a ).y;
						yp += original.getInternal( x + b, y + a ).y;
						xm += original.getInternal( x - a, y + b ).y;
						xp += original.getInternal( x + a, y + b ).y;
					}
				}
				xm /= count;
				xp /= count;
				ym /= count;
				yp /= count;
				horz.setInternal( x, y, pixelLuminance<int16>( xp - xm ) );
				vert.setInternal( x, y, pixelLuminance<int16>( yp - ym ) );
			}
		}
	}
	verticalLinks.allocate( width );
	horizontalLinks.allocate( height );
	ascentLinks.allocate( width + height - 1 );
	descentLinks.allocate( width + height - 1 );
}

inline void save( MSXML2::IXMLDOMElementPtr & linknode, link & lnk )
{
	MSXML2::IXMLDOMElementPtr node = adom::createChildElement( linknode, "l" );
	adom::setAttribute( node, "bx", string( lnk.bx ) );
	adom::setAttribute( node, "by", string( lnk.by ) );
	string wk;
	if ( lnk.cx ) {
		wk.print( "%.1f", lnk.cx );
		adom::setAttribute( node, "cx", wk );
	}
	if ( lnk.cy ) {
		wk.print( "%.1f", lnk.cy );
		adom::setAttribute( node, "cy", wk );
	}
	if ( lnk.reliability != 1 ) {
		wk.print( "%.2f", lnk.reliability );
		adom::setAttribute( node, "r", wk );
	}
	if ( lnk.intensity != 1 ) {
		wk.print( "%.2f", lnk.intensity );
		adom::setAttribute( node, "i", wk );
	}
	switch ( lnk.type ) {
	case link::typeVertical: adom::setAttribute( node, "d", string( "vert" ) ); break;
	case link::typeHorizontal: adom::setAttribute( node, "d", string( "horz" ) ); break;
	case link::typeAscent: adom::setAttribute( node, "d", string( "asce" ) ); break;
	case link::typeDescent: adom::setAttribute( node, "d", string( "desc" ) ); break;
	}
}
inline void load( MSXML2::IXMLDOMElementPtr & node, link & lnk )
{
	lnk.bx = adom::getAttribute( node, "bx" ).tof();
	lnk.by = adom::getAttribute( node, "by" ).tof();
	lnk.cx = adom::getAttribute( node, "cx" ).tof();
	lnk.cy = adom::getAttribute( node, "cy" ).tof();
	if ( adom::existAttribute( node, "r" ) ) {
		lnk.reliability = adom::getAttribute( node, "r" ).tof();
	} else {
		lnk.reliability = 1;
	}
	if ( adom::existAttribute( node, "i" ) ) {
		lnk.intensity = adom::getAttribute( node, "i" ).tof();
	} else {
		lnk.intensity = 1;
	}
	string dir = adom::getAttribute( node, "d" );
	if ( dir == "d" ) { lnk.type = link::typeVertical; }
	else if ( dir == "u" ) { lnk.type = link::typeVertical; }
	else if ( dir == "r" ) { lnk.type = link::typeHorizontal; }
	else if ( dir == "l" ) { lnk.type = link::typeHorizontal; }
	else if ( dir == "ur" ) { lnk.type = link::typeAscent; }
	else if ( dir == "dl" ) { lnk.type = link::typeAscent; }
	else if ( dir == "ul" ) { lnk.type = link::typeDescent; }
	else if ( dir == "dr" ) { lnk.type = link::typeDescent; }
	else if ( dir == "vert" ) { lnk.type = link::typeVertical; }
	else if ( dir == "horz" ) { lnk.type = link::typeHorizontal; }
	else if ( dir == "asce" ) { lnk.type = link::typeAscent; }
	else if ( dir == "desc" ) { lnk.type = link::typeDescent; }
}
void trackDocument::saveXML( const string & filename )
{
	//ＸＭＬとして保存
	MSXML2::IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );
	MSXML2::IXMLDOMElementPtr root = adom::createChildElement( domDocument, "root" );
	MSXML2::IXMLDOMElementPtr bwnode = adom::createChildElement( root, "basewave" );
	adom::setAttribute( bwnode, "b", string( baseIndex ) );
	adom::setAttribute( bwnode, "i", string( inspirationFrame ) );
	adom::setAttribute( bwnode, "e", string( expirationFrame ) );
	adom::setAttribute( bwnode, "fx", string( firstwave.x ) );
	adom::setAttribute( bwnode, "fy", string( firstwave.y ) );
	adom::setAttribute( bwnode, "lx", string( lastwave.x ) );
	adom::setAttribute( bwnode, "ly", string( lastwave.y ) );
	adom::setAttribute( bwnode, "w0", string( waveBias ) );
	for ( int x = 0; x < wave.size; ++x ) {
		MSXML2::IXMLDOMElementPtr node = adom::createChildElement( bwnode, "p" );
		adom::setAttribute( node, "value", string( wave[x] ) );
	}
	MSXML2::IXMLDOMElementPtr linknode = adom::createChildElement( root, "links" );
	for ( array< list<link> >::iterator it( verticalLinks ); it; ++it ) {
		for ( list<link>::iterator itl( it() ); itl; ++itl ) {
			save( linknode, itl() );
		}
	}
	for ( array< list<link> >::iterator it( horizontalLinks ); it; ++it ) {
		for ( list<link>::iterator itl( it() ); itl; ++itl ) {
			save( linknode, itl() );
		}
	}
	for ( array< list<link> >::iterator it( ascentLinks ); it; ++it ) {
		for ( list<link>::iterator itl( it() ); itl; ++itl ) {
			save( linknode, itl() );
		}
	}
	for ( array< list<link> >::iterator it( descentLinks ); it; ++it ) {
		for ( list<link>::iterator itl( it() ); itl; ++itl ) {
			save( linknode, itl() );
		}
	}
	MSXML2::IXMLDOMElementPtr areasNode = adom::createChildElement( root, "areas" );
	for ( list<boundaryArea>::iterator ita( areas ); ita; ++ita ) {
		MSXML2::IXMLDOMElementPtr areaNode = adom::createChildElement( areasNode, "area" );
		boundaryArea & area = ita();
		area.updatePoint();
		area.updateConnection();
		area.update( -1 );
		for ( list<boundaryCurve>::iterator itc( area.divisions ); itc; ++itc ) {
			MSXML2::IXMLDOMElementPtr curveNode = adom::createChildElement( areaNode, "curve" );
			boundaryCurve & curve = itc();
			adom::setAttribute( curveNode, "cyclic", string( curve.cyclic ? 1 : 0 ) );
			for ( list<boundaryPoint>::iterator itp( curve.points ); itp; ++itp ) {
				MSXML2::IXMLDOMElementPtr pointNode = adom::createChildElement( curveNode, "point" );
				boundaryPoint & pnt = itp();
				if ( pnt.referencing() ) {
					adom::setAttribute( pointNode, "r", "1" );
					int curveIndex, pointIndex;
					if ( area.findOriginal( curveIndex, pointIndex, & pnt ) ) {
						adom::setAttribute( pointNode, "ci", string( curveIndex ) );
						adom::setAttribute( pointNode, "pi", string( pointIndex ) );
					}
				} else {
					for ( array<boundaryPointFrame>::iterator itf( pnt.points ); itf; ++itf ) {
						MSXML2::IXMLDOMElementPtr frameNode = adom::createChildElement( pointNode, "frame" );
						boundaryPointFrame & frame = itf();
						adom::setAttribute( frameNode, "x", string( frame.position.x ) );
						adom::setAttribute( frameNode, "y", string( frame.position.y ) );
						adom::setAttribute( frameNode, "type", string( frame.type ) );
					}
				}
			}
		}
		for ( list<boundaryPart>::iterator itp( area.parts ); itp; ++itp ) {
			MSXML2::IXMLDOMElementPtr partNode = adom::createChildElement( areaNode, "part" );
			boundaryPart & part = itp();
			adom::setAttribute( partNode, "type", string( part.type ) );
			for ( array<boundaryCurveIndex>::iterator itc( part.curves ); itc; ++itc ) {
				MSXML2::IXMLDOMElementPtr indexNode = adom::createChildElement( partNode, "index" );
				boundaryCurveIndex & index = itc();
				adom::setAttribute( indexNode, "curve", string( index.curve ) );
				adom::setAttribute( indexNode, "start", string( index.start ) );
				adom::setAttribute( indexNode, "end", string( index.end ) );
				adom::setAttribute( indexNode, "size", string( index.size ) );
			}
		}
	}
	adom::save( domDocument, filename );
}
void trackDocument::loadXML( const string & filename, bool replace )
{
	//ＸＭＬとして読み込み
	MSXML2::IXMLDOMDocumentPtr domDocument( "MSXML.DOMDocument" );

	if ( ! adom::load( domDocument, filename ) ) return;
	MSXML2::IXMLDOMElementPtr root = domDocument->firstChild;
	if ( adom::getNodeName( root ) != "root" ) return;

	bool clearedWave = false;
	bool clearedLinks = false;
	bool clearedAreas = false;
	if ( replace ) {
		if ( ! clearedWave ) { clearedWave = true; clearWave(); }
		if ( ! clearedLinks ) { clearedLinks = true; clearLinks(); }
		if ( ! clearedAreas ) { clearedAreas = true; clearAreas(); }
	}
	for ( MSXML2::IXMLDOMElementPtr element = root->firstChild; element; element = element->nextSibling ) {
		string nodename = adom::getNodeName( element );
		if ( nodename == "basewave" ) {
			if ( ! clearedWave ) { clearedWave = true; clearWave(); }
			
			wave.reserve( 100 );
			baseIndex = adom::getAttribute( element, "b" ).toi();
			if ( adom::existAttribute( element, "i" ) ) {
				inspirationFrame = adom::getAttribute( element, "i" ).toi();
			}
			if ( adom::existAttribute( element, "e" ) ) {
				expirationFrame = adom::getAttribute( element, "e" ).toi();
			}
			firstwave.x = adom::getAttribute( element, "fx" ).toi();
			firstwave.y = adom::getAttribute( element, "fy" ).toi();
			lastwave.x = adom::getAttribute( element, "lx" ).toi();
			lastwave.y = adom::getAttribute( element, "ly" ).toi();
			waveBias = adom::getAttribute( element, "w0" ).toi();
			for ( MSXML2::IXMLDOMElementPtr node = element->firstChild; node; node = node->nextSibling ) {
				int v = adom::getAttribute( node, "value" ).toi();
				wave.push_back( v );
			}
			while ( wave.size < this->sizeTime() ) {//読み込む内容が足りていない
				int v = 0;
				wave.push_back( v );
			}
			waveParameterUpdate( false );
		} else if ( nodename == "links" ) {
			if ( ! clearedLinks ) { clearedLinks = true; clearLinks(); }
			for ( MSXML2::IXMLDOMElementPtr node = element->firstChild; node; node = node->nextSibling ) {
				link lnk;
				load( node, lnk );
				if ( lnk.type == link::typeVertical ) {
					verticalLinks[lnk.base( height )].push_back( lnk );
				} else if ( lnk.type == link::typeHorizontal ) {
					horizontalLinks[lnk.base( height )].push_back( lnk );
				} else if ( lnk.type == link::typeAscent ) {
					ascentLinks[lnk.base( height )].push_back( lnk );
				} else if ( lnk.type == link::typeDescent ) {
					descentLinks[lnk.base( height )].push_back( lnk );
				}
			}
			for ( array< list<link> >::iterator it( verticalLinks ); it; ++it ) {
				linksort( it() );
			}
			for ( array< list<link> >::iterator it( horizontalLinks ); it; ++it ) {
				linksort( it() );
			}
			for ( array< list<link> >::iterator it( ascentLinks ); it; ++it ) {
				linksort( it() );
			}
			for ( array< list<link> >::iterator it( descentLinks ); it; ++it ) {
				linksort( it() );
			}
			flickerSetup( flickers[0], link::typeVertical, verticalLinks );
			flickerSetup( flickers[1], link::typeHorizontal, horizontalLinks );
			flickerSetup( flickers[2], link::typeAscent, ascentLinks );
			flickerSetup( flickers[3], link::typeDescent, descentLinks );
		} else if ( nodename == "areas" ) {
			for ( MSXML2::IXMLDOMElementPtr areasNode = element->firstChild; areasNode; areasNode = areasNode->nextSibling ) {
				boundaryArea & area = areas.push_back( boundaryArea::factory() );
				area.reset();
				for ( MSXML2::IXMLDOMElementPtr childNode = areasNode->firstChild; childNode; childNode = childNode->nextSibling ) {
					string nodename = adom::getNodeName( childNode );
					if ( nodename == "curve" ) {
						boundaryCurve & curve = area.divisions.push_back( boundaryCurve() );
						curve.cyclic = adom::getAttribute( childNode, "cyclic" ).toi() ? true : false;
						for ( MSXML2::IXMLDOMElementPtr pointNode = childNode->firstChild; pointNode; pointNode = pointNode->nextSibling ) {
							bool referencing = adom::getAttribute( pointNode, "r" ).toi() ? true : false;
							if ( referencing ) {
								int curveIndex = adom::getAttribute( pointNode, "ci" ).toi();
								int pointIndex = adom::getAttribute( pointNode, "pi" ).toi();
								boundaryPoint & point = curve.points.push_back( boundaryPoint::factory() );
								boundaryPoint & src = area.divisions[curveIndex].points[pointIndex];
								point.setReference( & src );
							} else {
								boundaryPoint & point = curve.points.push_back( boundaryPoint::factory( point2<int>( 0, 0 ) ) );
								MSXML2::IXMLDOMElementPtr frameNode = pointNode->firstChild;
								for ( array<boundaryPointFrame>::iterator itf( point.points ); itf; ++itf ) {
									if ( frameNode ) {
										itf->position.x = adom::getAttribute( frameNode, "x" ).toi();
										itf->position.y = adom::getAttribute( frameNode, "y" ).toi();
										itf->type = adom::getAttribute( frameNode, "type" ).toi();
										itf->adjusted = true;
										frameNode = frameNode->nextSibling;
									}
								}
							}
						}
					} else if ( nodename == "part" ) {
						boundaryPart & part = area.parts.push_back( boundaryPart::factory() );
						part.resetConnection();
						part.type = adom::getAttribute( childNode, "type" ).toi();
						for ( MSXML2::IXMLDOMElementPtr indexNode = childNode->firstChild; indexNode; indexNode = indexNode->nextSibling ) {
							boundaryCurveIndex & index = part.curves.push_back( boundaryCurveIndex() );
							index.curve = adom::getAttribute( indexNode, "curve" ).toi();
							index.start = adom::getAttribute( indexNode, "start" ).toi();
							index.end = adom::getAttribute( indexNode, "end" ).toi();
							index.size = adom::getAttribute( indexNode, "size" ).toi();
						}
					}
				}
				area.updateConnection();
				area.update( -1 );
			}
		}
	}
	updateBaseWaveImage();
}
void trackDocument::exportInformation( const string & filename )
{
	retainer<file> f = file::initialize( filename, true, false );
	//面積
	if ( this->pixelSize == 1 ) {
		f->printf( "Area[pixels]\n" );
	} else {
		f->printf( "Area[mm^2]\n" );
	}
	double coefficientArea = pixelSize * pixelSize;
	f->printf( "File" );
	for ( list<boundaryArea>::iterator ita( areas ); ita; ++ita ) {
		boundaryArea & area = ita();
		for ( list<boundaryPart>::iterator itp( area.parts ); itp; ++itp ) {
			boundaryPart & part = itp();
			static char * name[] = { "右上葉", "右中葉", "右下葉", "左上葉", "左下葉" };
			f->printf( ",%s(%d:%d),plus,minus", name[part.type%5], areas.index( area ), area.parts.index( part ) );//No.
		}
	}
	f->printf( "\n" );
	for ( int t = 0; t < sizeTime(); ++t ) {
		f->printf( "%s", imageFilenames[t].chars() );
		for ( list<boundaryArea>::iterator ita( areas ); ita; ++ita ) {
			boundaryArea & area = ita();
			area.update( t );
			for ( list<boundaryPart>::iterator itp( area.parts ); itp; ++itp ) {
				boundaryPart & part = itp();
				f->printf( ",%f", part.shapes[t].area() );
				if ( t ) {
					f->printf( ",%f", ( part.shapes[t] - part.shapes[t-1] ).area() * coefficientArea );
					f->printf( ",%f", ( part.shapes[t-1] - part.shapes[t] ).area() * coefficientArea );
				} else {
					f->printf( ",0,0" );
				}
			}
		}
		f->printf( "\n" );
	}
	f->printf( "Histogram[pixels],\n" );
	if ( this->pixelSize != 1 ) {
		f->printf( "1 pixel == %f mm^2 \n", pixelSize * pixelSize );
	}
	for ( list<boundaryArea>::iterator ita( areas ); ita; ++ita ) {
		boundaryArea & area = ita();
		for ( list<boundaryPart>::iterator itp( area.parts ); itp; ++itp ) {
			boundaryPart & part = itp();
			static char * name[] = { "右上葉", "右中葉", "右下葉", "左上葉", "左下葉" };
			f->printf( "%s\n", name[part.type%5] );
			f->printf( "Intencity:Time" );
			array<double> moment1;
			array<double> moment2;
			array< array<int> > histogram;
			moment1.allocate( sizeTime() );
			moment2.allocate( sizeTime() );
			histogram.allocate( sizeTime() );
			for ( int t = 0; t < sizeTime(); ++t ) {
				f->printf( ",%s", imageFilenames[t].chars() );
				double & m1 = moment1[t];
				double & m2 = moment2[t];
				m1 = 0; m2 = 0;
				array<int> & hist = histogram[t];
				hist.allocate( 512 );
				memset( hist.data, 0, sizeof( int ) * hist.size );
				region<int> & shape = part.shapes[t];
				imageInterface< pixelLuminance<int16> > & img = originalImages[t];
				for ( region<int>::iterator itp( shape ); itp; ++itp ) {
					const int y = img.get( itp->x, itp->y ).y;
					m1 += y;
					m2 += y * y;
					hist[clamp( 0, y, 512 - 1 )] += 1;
				}
			}
			f->printf( "\n" );
			f->printf( "Average" );
			double sum_average = 0;
			for ( int t = 0; t < sizeTime(); ++t ) {
				double a = part.shapes[t].area();
				double ave = zerodivide( moment1[t], a );
				sum_average += ave;
				f->printf( ",%f", ave );
			}
			sum_average /= sizeTime();
			f->printf( ",%f", sum_average );
			f->printf( "\n" );
			f->printf( "Standard Deviation" );
			double sum_m2 = 0;
			for ( int t = 0; t < sizeTime(); ++t ) {
				double a = part.shapes[t].area();
				double ave = zerodivide( moment1[t], a );
				double sd = ::sqrt( zerodivide( moment2[t], a ) - ave * ave );
				sum_m2 += zerodivide( moment2[t], a );
				f->printf( ",%f", sd );
			}
			f->printf( ",%f", ::sqrt( sum_m2 / sizeTime() - sum_average * sum_average ) );
			f->printf( "\n" );
			for ( int i = 0; i < 512; ++i ) {
				f->printf( "%d", i );//Intensity
				for ( int t = 0; t < sizeTime(); ++t ) {
					f->printf( ",%d", histogram[t][i] );
				}
				f->printf( "\n" );
			}
		}
	}
}
void stateSelectPointForExport::output()
{
	retainer<file> f = file::initialize( filename, true, false );
	boundaryArea & area = doc->areas[areaIndex];
	boundaryCurve & curve = area.divisions.first();
	boundaryCurveIndex indices[3];
	boundaryCurveIndex & innerIndex = indices[0];
	boundaryCurveIndex & outerIndex = indices[1];
	boundaryCurveIndex & bottomIndex = indices[2];
	innerIndex.curve = outerIndex.curve = bottomIndex.curve = 0;
	innerIndex.size = outerIndex.size = bottomIndex.size = curve.points.size;
	innerIndex.start = topPoint;
	innerIndex.end = innerPoint;
	bottomIndex.start = innerPoint;
	bottomIndex.end = outerPoint;
	outerIndex.start = outerPoint;
	outerIndex.end = topPoint;
	if ( innerIndex.include( outerPoint ) ) {
		swap( innerIndex.start, innerIndex.end );
		swap( outerIndex.start, outerIndex.end );
		swap( bottomIndex.start, bottomIndex.end );
	}
	const double & pixelSize = doc->pixelSize;
	double coefficientArea = pixelSize * pixelSize;
	if ( pixelSize == 1 ) {
		f->printf( "Difference[pixels]\n" );
	} else {
		f->printf( "Difference[mm^2]\n" );
	}
//	f->printf( "time,inner plus,inner minus,outer plus,outer minus,bottom plus,bottom minus\n" );
	f->printf( "time,all,plus,minus,inner,plus,minus,outer,plus,minus,bottom,plus,minus\n" );
	for ( int time = 1; time < doc->sizeTime(); ++time ) {
		area.update( time );
		f->printf( "%s", doc->imageFilenames[time].chars() );
		region<int> prev = area.shapes[time-1];
		region<int> now = area.shapes[time];
		const region<int> & or = prev | now;
		const region<int> & and = prev & now;
		const region<int> & difference = or - and;

		f->printf( ",%f,%f,%f", now.area() * coefficientArea, ( now - prev ).area() * coefficientArea, ( prev - now ).area() * coefficientArea );
		prev = prev - and;
		now = now - and;

		for ( int i = 0; i < 3; ++i ) {
			boundaryCurveIndex & index = indices[i];
			int si = index.start, ei = index.end;
			region<int> boundary;
			list< point2<int> > segments;
			for ( boundaryCurveIndex::iterator iti( index ); iti; ++iti ) {
				curve.segment( segments, iti(), time - 1 );
				curve.segment( segments, iti(), time );
			}
			pointsToRegion( segments, boundary, doc->width, doc->height );
			{
				region<int> line;
				point2<int> p1 = curve.points[si]( time - 1 );
				point2<int> p2 = curve.points[si]( time );
				line.line( p1.x, p1.y, p2.x, p2.y );
				boundary |= line;
			}
			{
				region<int> line;
				point2<int> p1 = curve.points[ei]( time - 1 );
				point2<int> p2 = curve.points[ei]( time );
				line.line( p1.x, p1.y, p2.x, p2.y );
				boundary |= line;
			}
			region<int> part;
			boundary.fill( part );
			part = part & difference;
			const region<int> & plus = part & now;
			const region<int> & minus = part & prev;
			f->printf( ",%f,%f,%f", part.area() * coefficientArea, plus.area() * coefficientArea, minus.area() * coefficientArea );
			now = now - plus;
			prev = prev - minus;
		}
		f->printf( "\n" );
	}
}
