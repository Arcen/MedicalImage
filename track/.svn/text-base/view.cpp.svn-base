#include "main.h"
#include "document.h"
#include "view.h"
#include "resource.h"
#include "state.h"

inline pixel blend( const pixel & dst, const pixel & src )
{
	const double sc = src.a / 255.0;
	const double dc = 1.0 - sc;
	return pixel( 
		clamp<uint16>( 0, dc * dst.r + sc * src.r, 255 ),
		clamp<uint16>( 0, dc * dst.g + sc * src.g, 255 ),
		clamp<uint16>( 0, dc * dst.b + sc * src.b, 255 ), 255 );
}

inline pixel blendMax( const pixel & dst, const pixel & src )
{
	return pixel( maximum( dst.r, src.r ), maximum( dst.g, src.g ), maximum( dst.b, src.b ), 255 );
}
/*
region<int> getDiffer( trackDocument * doc, int areaIndex, int start, int end, int time )
{
	boundaryArea & area = doc->areas[areaIndex];
	boundaryCurve & curve = area.divisions.first();
	boundaryCurveIndex index;
	index.curve = 0;
	index.size = curve.points.size;
	index.start = start;
	index.end = end;
	static array2<bool> flag;
	flag.allocate( doc->width, doc->height );
	region<int> result;
	if ( ! time ) return result;
	area.update( time - 1 );
	area.update( time );
	region<int> prevShape, nowShape;
	{
		list< point2<int> > segments;
		bool checkStart = true;
		for ( boundaryCurveIndex::iterator iti( index ); iti; ++iti ) {
			curve.segment( segments, iti(), time - 1 );
		}
		memset( flag.data, 0, sizeof( bool ) * flag.size );
		for ( list< point2<int> >::iterator itp( segments ); itp; ++itp ) {
			flag( clamp( 0, itp->x, doc->width - 1 ), clamp( 0, itp->y, doc->height - 1 ) ) = true;
		}
		prevShape.set( flag, 0, 0 );
	}
	{
		list< point2<int> > segments;
		bool checkStart = true;
		for ( boundaryCurveIndex::iterator iti( index ); iti; ++iti ) {
			curve.segment( segments, iti(), time );
		}
		memset( flag.data, 0, sizeof( bool ) * flag.size );
		for ( list< point2<int> >::iterator itp( segments ); itp; ++itp ) {
			flag( clamp( 0, itp->x, doc->width - 1 ), clamp( 0, itp->y, doc->height - 1 ) ) = true;
		}
		nowShape.set( flag, 0, 0 );
	}
	region<int> round;
	round = nowShape | prevShape;
	{
		region<int> line;
		point2<int> p1 = curve.points[start]( time - 1 );
		point2<int> p2 = curve.points[start]( time );
		line.line( p1.x, p1.y, p2.x, p2.y );
		round |= line;
	}
	{
		region<int> line;
		point2<int> p1 = curve.points[end]( time - 1 );
		point2<int> p2 = curve.points[end]( time );
		line.line( p1.x, p1.y, p2.x, p2.y );
		round |= line;
	}
	round.fill( result );
	return result;
}
*/
void stateDocument::paint()
{
	trackDocument * doc = trackDocument::get();
	trackView * view = trackView::get();
	if ( ! doc ) return;
	if ( ! view ) return;
	static image viewImage;//画面表示用画像
	viewImage.topdown = false;
	double maxValue = doc->maxValues[doc->currentViewImageIndex];
	double windowLevel = windowLevelBar->get();
	double windowSize = windowSizeBar->get();
	decimal rate = 255.0 / windowSize;
	decimal offset = windowLevel - windowSize / 2;

	imageInterface< pixelLuminance<int16> > * img = & doc->originalImages[doc->currentViewImageIndex];
	switch ( view->mode & trackView::baseImageMask ) {
	case trackView::original:
		break;
	case trackView::vertEdge: 
		img = & doc->verticalEdgeImages[doc->currentViewImageIndex]; 
		break;
	case trackView::horzEdge: 
		img = & doc->horizontalEdgeImages[doc->currentViewImageIndex]; 
		break;
	default:
		offset = 0;
		rate = 0;
		break;
	}
	viewImage.create( img->width, img->height );
	pixel p( 0, 0, 0, 255 );
	if ( rate ) {
		for ( int y = 0; y < img->height; ++y ) {
			for ( int x = 0; x < img->width; ++x ) {
				p.r = p.g = p.b = static_cast<int8>( clamp<decimal>( 0, 
					( img->getInternal( x, y ).y - offset ) * rate, 255.0 ) );
				viewImage.setInternal( x, y, p );
			}
		}
	} else {
		for ( int y = 0; y < img->height; ++y ) {
			for ( int x = 0; x < img->width; ++x ) {
				viewImage.setInternal( x, y, p );
			}
		}
	}
	//波の表示用の色
	pixel colorV( 0, 0, 255, 255 );
	pixel colorH( 0, 255, 0, 255 );
	pixel colorA( 255, 255, 0, 255 );
	pixel colorD( 255, 0, 255, 255 );
	//論文投稿用の表示
	if ( false && ( ( view->mode & trackView::baseImageMask ) == trackView::none ) ) {
		p.r = p.g = p.b = 0;
		for ( int y = 0; y < img->height; ++y ) {
			for ( int x = 0; x < img->width; ++x ) {
				viewImage.setInternal( x, y, p );
			}
		}
		for ( int t = 0; t < doc->sizeTime(); ++t ) {
			pixel c;
			if ( view->mode & trackView::vertical ) {
				for ( array< list<link> >::iterator ita( doc->verticalLinks ); ita; ++ita ) {
					for ( list<link>::iterator it( ita() ); it; ++it ) {
						c.r = c.g = c.b = clamp<int>( 0, it->reliability * 255, 255 );
						int x = it->cx * doc->wave[t] + it->bx;
						int y = it->cy * doc->wave[t] + it->by;
						viewImage.set( x, y, blendMax( viewImage.get( x, y ), c ) );
					}
				}
			}
			if ( view->mode & trackView::horizontal ) {
				for ( array< list<link> >::iterator ita( doc->horizontalLinks ); ita; ++ita ) {
					for ( list<link>::iterator it( ita() ); it; ++it ) {
						c.r = c.g = c.b = clamp<int>( 0, it->reliability * 255, 255 );
						int x = it->cx * doc->wave[t] + it->bx;
						int y = it->cy * doc->wave[t] + it->by;
						viewImage.set( x, y, blendMax( viewImage.get( x, y ), c ) );
					}
				}
			}
			if ( view->mode & trackView::ascent ) {
				for ( array< list<link> >::iterator ita( doc->ascentLinks ); ita; ++ita ) {
					for ( list<link>::iterator it( ita() ); it; ++it ) {
						c.r = c.g = c.b = clamp<int>( 0, it->reliability * 255, 255 );
						int x = it->cx * doc->wave[t] + it->bx;
						int y = it->cy * doc->wave[t] + it->by;
						viewImage.set( x, y, blendMax( viewImage.get( x, y ), c ) );
					}
				}
			}
			if ( view->mode & trackView::descent ) {
				for ( array< list<link> >::iterator ita( doc->descentLinks ); ita; ++ita ) {
					for ( list<link>::iterator it( ita() ); it; ++it ) {
						c.r = c.g = c.b = clamp<int>( 0, it->reliability * 255, 255 );
						int x = it->cx * doc->wave[t] + it->bx;
						int y = it->cy * doc->wave[t] + it->by;
						viewImage.set( x, y, blendMax( viewImage.get( x, y ), c ) );
					}
				}
			}
		}
	} else
	if ( view->mode != trackView::original ) {
/*
		if ( view->mode & trackView::vertical ) {
			checkMaximum<double> mx;
			for ( array< list<link> >::iterator ita( doc->verticalLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					mx( it->intensity );
				}
			}
			for ( array< list<link> >::iterator ita( doc->verticalLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorV;
					c.a = clamp<int>( 0, c.a * it->intensity / mx(), 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
		if ( view->mode & trackView::horizontal ) {
			checkMaximum<double> mx;
			for ( array< list<link> >::iterator ita( doc->horizontalLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					mx( it->intensity );
				}
			}
			for ( array< list<link> >::iterator ita( doc->horizontalLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorH;
					c.a = clamp<int>( 0, c.a * it->intensity / mx(), 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
		if ( view->mode & trackView::ascent ) {
			checkMaximum<double> mx;
			for ( array< list<link> >::iterator ita( doc->ascentLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					mx( it->intensity );
				}
			}
			for ( array< list<link> >::iterator ita( doc->ascentLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorA;
					c.a = clamp<int>( 0, c.a * it->intensity / mx(), 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
		if ( view->mode & trackView::descent ) {
			checkMaximum<double> mx;
			for ( array< list<link> >::iterator ita( doc->descentLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					mx( it->intensity );
				}
			}
			for ( array< list<link> >::iterator ita( doc->descentLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorD;
					c.a = clamp<int>( 0, c.a * it->intensity / mx(), 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}

		*/
		if ( view->mode & trackView::vertical ) {
			for ( array< list<link> >::iterator ita( doc->verticalLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorV;
					c.a = clamp<int>( 0, c.a * it->reliability, 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
		if ( view->mode & trackView::horizontal ) {
			for ( array< list<link> >::iterator ita( doc->horizontalLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorH;
					c.a = clamp<int>( 0, c.a * it->reliability, 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
		if ( view->mode & trackView::ascent ) {
			for ( array< list<link> >::iterator ita( doc->ascentLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorA;
					c.a = clamp<int>( 0, c.a * it->reliability, 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
		if ( view->mode & trackView::descent ) {
			for ( array< list<link> >::iterator ita( doc->descentLinks ); ita; ++ita ) {
				for ( list<link>::iterator it( ita() ); it; ++it ) {
					pixel c = colorD;
					c.a = clamp<int>( 0, c.a * it->reliability, 255 );
					int x = it->cx * doc->wave[doc->currentViewImageIndex] + it->bx;
					int y = it->cy * doc->wave[doc->currentViewImageIndex] + it->by;
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
	}
	/*
	for ( int i = 0; i < 4; ++i ) {
		for ( array2<flicker>::iterator it( doc->flickers[i] ); it; ++it ) {
			if ( it->value < 0.25 ) continue;
			pixel c( 0, 255, 0, 255 );
			c.a = clamp<int>( 0, c.a * it->value, 128 );
			const point2<int> & pos = it->lnk.position( doc->currentViewImageIndex, doc->wave );
			int x = pos.x, y = pos.y;
			viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
		}
	}
	*/
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		it->update( doc->currentViewImageIndex );
	}
	list< point2<int> > points;
	pixel linecolor( 255, 255, 0, 128 );
	pixel pointcolor( 0, 255, 0, 255 );
	pixel itnitialpointcolor( 0, 0, 255, 255 );
	pixel editedpointcolor( 255, 0, 0, 255 );
	/*
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		boundaryArea & area = it();
		if ( ! area.enable ) continue;
		pixel areacolor( 0, 0, 0, 64 );
		for ( list< boundaryPart >::iterator ita( area.parts ); ita; ++ita ) {
			areacolor.r = rand() % 256;
			areacolor.g = rand() % 256;
			areacolor.b = rand() % 256;
			areacolor.a = 128;
			boundaryPart & part = ita();
			for ( region<int>::iterator itp( part.shapes[doc->currentViewImageIndex] ); itp; ++itp ) {
				int x = itp->x;
				int y = itp->y;
				viewImage.set( x, y, blend( viewImage.get( x, y ), areacolor ) );
			}
		}
	}
	*/
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		//曲線の表示
		if ( doc->show_curve )
		{
			for ( list< point2<int> >::iterator itp( it->boundaryCurves[doc->currentViewImageIndex] ); itp; ++itp ) {
				const point2<int> & p = itp();
				int x = p.x;
				int y = p.y;
				viewImage.set( x, y, blend( viewImage.get( x, y ), linecolor ) );
			}
		}
		//制御点の表示
		for ( list<boundaryPoint*>::iterator itc( it->controlPoints ); itc; ++itc ) {
			boundaryPoint & bp = *itc();
			const point2<int> & p = bp( doc->currentViewImageIndex );
			pixel c;
			switch ( bp.type( doc->currentViewImageIndex ) ) {
			case boundaryPointFrame::typeInitial: c = itnitialpointcolor; break;
			case boundaryPointFrame::typeEdited: c = editedpointcolor; break;
			case boundaryPointFrame::typeInterpolated: c = pointcolor; break;
			default: c = pointcolor; break;
			}
			for ( int y = p.y - 1; y <= p.y + 1; ++y ) {
				for ( int x = p.x - 1; x <= p.x + 1; ++x ) {
					viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
				}
			}
		}
	}
	paint( viewImage );
}

//tの区切りの中で最小のxを返す
inline int quantumMinimum( int x, int t )
{
	int r = x % t;
	if ( r == 0 ) return x;
	return x - r;
}
//tの区切りの中で最大のxを返す
inline int quantumMaximum( int x, int t )
{
	int r = x % t;
	if ( r == 0 ) return x;
	return x - r + t;
}

void stateDocument::paint( image & viewImage )
{
	HDC hdc = imageWindow->getDC();
	RECT cr = imageWindow->client();
	static windowSurfaceBitmap surface;
	surface = viewImage;
	int width = cr.right - cr.left;
	int height = cr.bottom - cr.top;
	trackView * view = trackView::get();
	if ( surface.object ) {
		RECT srcRect;//拡大された画像座標系での転送元矩形
		srcRect.left = imageWindow->scrollPosition.x;
		srcRect.top = imageWindow->scrollPosition.y;
		srcRect.right = minimum<LONG>( imageWindow->scrollPosition.x + width, surface.width * view->imageScale );
		srcRect.bottom = minimum<LONG>( imageWindow->scrollPosition.y + height, surface.height * view->imageScale );
		RECT srcRectAdjust;//量子化
		srcRectAdjust.left = quantumMinimum( srcRect.left, view->imageScale ) / view->imageScale;
		srcRectAdjust.top = quantumMinimum( srcRect.top, view->imageScale ) / view->imageScale;
		srcRectAdjust.right = quantumMaximum( srcRect.right, view->imageScale ) / view->imageScale;
		srcRectAdjust.bottom = quantumMaximum( srcRect.bottom, view->imageScale ) / view->imageScale;
		RECT dstRect;//画面座標系での転送先矩形
		dstRect.left = cr.left;
		dstRect.top = cr.top;
		dstRect.right = minimum<int>( cr.right, surface.width * view->imageScale );
		dstRect.bottom = minimum<int>( cr.bottom, surface.height * view->imageScale );
		RECT dstRectAdjust;
		dstRectAdjust.left = dstRect.left - ( srcRect.left - srcRectAdjust.left * view->imageScale );
		dstRectAdjust.top = dstRect.top - ( srcRect.top - srcRectAdjust.top * view->imageScale );
		dstRectAdjust.right = dstRect.right + ( srcRectAdjust.right * view->imageScale - srcRect.right );
		dstRectAdjust.bottom = dstRect.bottom + ( srcRectAdjust.bottom * view->imageScale - srcRect.bottom );
		StretchBlt( hdc, 
			dstRectAdjust.left, dstRectAdjust.top, dstRectAdjust.right - dstRectAdjust.left, dstRectAdjust.bottom - dstRectAdjust.top, 
			surface.hdc, 
			srcRectAdjust.left, srcRectAdjust.top, srcRectAdjust.right - srcRectAdjust.left, srcRectAdjust.bottom - srcRectAdjust.top, 
			SRCCOPY );
		if ( dstRectAdjust.right < cr.right ) {
			BitBlt( hdc, dstRectAdjust.right, cr.top, cr.right - dstRectAdjust.right, height, NULL, 0, 0, BLACKNESS );
		}
		if ( dstRectAdjust.bottom < cr.bottom ) {
			BitBlt( hdc, cr.left, dstRectAdjust.bottom, width, cr.bottom - dstRectAdjust.bottom, NULL, 0, 0, BLACKNESS );
		}
	}
	imageWindow->release( hdc );
	imageWindow->validate();
	mainWindow->validate();
}

void stateBaseWave::paint()
{
	trackDocument * doc = trackDocument::get();
	static image viewImage;//画面表示用画像
	viewImage.topdown = false;
	viewImage.create( doc->baseWave.w, doc->baseWave.h );
	pixel p( 0, 0, 0, 255 );
	for ( int x = 0; x < viewImage.width; ++x ) {
		int maxValue = doc->maxValues[x];
		for ( int y = 0; y < viewImage.height; ++y ) {
			p.r = p.g = p.b = static_cast<unsigned char>( clamp<int>( 0, double( doc->baseWave( x, y ) ) / maxValue * 255, 255 ) );
			viewImage.setInternal( x, y, p );
		}
	}
	array<int>::iterator it( doc->wave );
	pixel c( 0, 255, 255, 255 );
	pixel baseColor( 255, 0, 255, 255 );
	for ( int x = 0; x < viewImage.width && it; ++x, ++it ) {
		int y = doc->waveBias + it();
		if ( x == doc->baseIndex ) {
			viewImage.set( x, y, blend( viewImage.get( x, y ), baseColor ) );
		} else {
			viewImage.set( x, y, blend( viewImage.get( x, y ), c ) );
		}
	}
	stateDocument::paint( viewImage );
}
void trackView::changeImageScale( int t )
{
	if ( ! imageWindow ) return;
	RECT r = imageWindow->client();
	int w = r.right - r.left;
	int h = r.bottom - r.top;
	int x = ( imageWindow->scrollPosition.x + ( w / 2 ) ) / imageScale;
	int y = ( imageWindow->scrollPosition.y + ( h / 2 ) ) / imageScale;
	imageScale = t;

	point2<int> imageSize = trackState::get()->imageSize();
	imageWindow->scrollDocumentSize.x = imageSize.x * imageScale;
	imageWindow->scrollDocumentSize.y = imageSize.y * imageScale;
	imageWindow->scrollPosition.x = minimum<int>( maximum<int>( 0, x * imageScale - ( w / 2 ) ), maximum<int>( 0, imageWindow->scrollDocumentSize.x - 1 - w ) );
	imageWindow->scrollPosition.y = minimum<int>( maximum<int>( 0, y * imageScale - ( h / 2 ) ), maximum<int>( 0, imageWindow->scrollDocumentSize.y - 1 - h ) );
	imageWindow->updateScrollRange();
	imageWindow->invalidate();
}
