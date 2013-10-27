#include "main.h"
#include "document.h"
#include "view.h"
#include "resource.h"
#include "state.h"
#include "setattrdialog.h"
#include "bmptool.inl"

void stateIndex1stPointOfBaseWave::mouseLeftPush( point2<int> p )
{
	firstwave = p;
	trackState::get()->change( stateIndex2ndPointOfBaseWave::get() );
}

void stateIndex2ndPointOfBaseWave::mouseLeftPush( point2<int> p )
{
	point2<int> firstwave = stateIndex1stPointOfBaseWave::get()->firstwave;
	point2<int> lastwave = p;
	if ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) {
		lastwave.x = firstwave.x;
	}
	doc->firstwave = firstwave;
	doc->lastwave = lastwave;
	statusBar->set( "updating" );
	doc->setBaseWave();
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		it->updatePoint( false, false, true );
	}
	trackState::get()->change( stateDocument::get() );
}
void stateDocument::mouseLeftPush( point2<int> p )
{
	movingArea = NULL;
	movingCurveIndex = -1;
	movingPointIndex = 0;
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		if ( it->find( movingCurveIndex, movingPointIndex, doc->currentViewImageIndex, p ) ) {
			movingArea = & it();
			movingArea->enable = false;
			return;
		}
	}
}
void stateDocument::mouseLeftMove( point2<int> p )
{
	if ( ! movingArea ) return;
	movingArea->movePoint( movingCurveIndex, movingPointIndex, doc->currentViewImageIndex, p );
	imageWindow->invalidate();
}
void stateDocument::mouseRightMove( point2<int> p )
{
	mouseLeftMove( p );
}
void stateDocument::mouseLeftPop( point2<int> p )
{
	if ( ! movingArea ) return;
	mouseLeftMove( p );
	movingArea->enable = true;
	movingArea->updatePoint( true, true );
	movingArea = NULL;
	imageWindow->invalidate();
}
void stateDocument::mouseRightPop( point2<int> p )
{
	if ( ! movingArea ) return;
	mouseLeftMove( p );
	movingArea->enable = true;
	movingArea->updatePoint( false, true );
	movingArea = NULL;
	imageWindow->invalidate();
}

void stateDocument::mouseRightPush( point2<int> p )
{
	movingArea = NULL;
	movingCurveIndex = -1;
	movingPointIndex = 0;
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		if ( it->find( movingCurveIndex, movingPointIndex, doc->currentViewImageIndex, p ) ) {
			movingArea = & it();
			movingArea->enable = false;
			return;
		}
	}
	if ( movingArea ) return;//左クリックで作業中のためキャンセル

	//論文用画像生成
	if ( false ) {
		p.x = 87;
		p.y = 133;
		int time = doc->sizeTime();
		int width = doc->width;
		int height = doc->height;
		array2<double> vertical, horizontal;
		vertical.allocate( time, height );
		horizontal.allocate( width, time );
		array2<double> descent, ascent;
		descent.allocate( maximum( width, height ), time );
		ascent.allocate( maximum( width, height ), time );
		for ( int t = 0; t < time; ++t ) {
			for ( int y = 0; y < height; ++y ) {
				int16 v = doc->originalImages[t].getInternal( p.x, y ).y;
				vertical( t, y ) = v;
			}
			for ( int x = 0; x < height; ++x ) {
				int16 v = doc->originalImages[t].getInternal( x, p.y ).y;
				horizontal( x, t ) = v;
			}
			int za = 0;
			int zd = 0;
			for ( int z = - maximum( width, height ); z < maximum( width, height ); ++z ) {
				{
					int x = p.x + z, y = p.y + z;
					if ( between( 0, x, width - 1 ) && between( 0, y, height - 1 ) ) {
						int16 v = doc->originalImages[t].getInternal( x, y ).y;
						descent( zd++, t ) = v;
					}
				}
				{
					int x = p.x + z, y = p.y - z;
					if ( between( 0, x, width - 1 ) && between( 0, y, height - 1 ) ) {
						int16 v = doc->originalImages[t].getInternal( x, y ).y;
						ascent( za++, t ) = v;
					}
				}
			}
		}
		string fn;
		fn.print( "D:\\temp\\(%d %d)vert.bmp", p.x, p.y );
		saveBmp( vertical, fn );
		fn.print( "D:\\temp\\(%d %d)horz.bmp", p.x, p.y );
		saveBmp( horizontal, fn );
		fn.print( "D:\\temp\\(%d %d)asce.bmp", p.x, p.y );
		saveBmp( ascent, fn );
		fn.print( "D:\\temp\\(%d %d)desc.bmp", p.x, p.y );
		saveBmp( descent, fn );
		memset( vertical.data, 0, vertical.size * sizeof( double ) );
		memset( horizontal.data, 0, horizontal.size * sizeof( double ) );

		for ( list<link>::iterator it( doc->verticalLinks[p.x] ); it; ++it ) {
			double v = it->intensity;
			for ( int t = 0; t < time; ++t ) {
				int y = it->cy * doc->wave[t] + it->by;
				if ( between( 0, y, height - 1 ) ) {
					vertical( t, y ) = v;
				}
			}
		}
		for ( list<link>::iterator it( doc->horizontalLinks[p.y] ); it; ++it ) {
			double v = it->intensity;
			for ( int t = 0; t < time; ++t ) {
				int x = it->cx * doc->wave[t] + it->bx;
				if ( between( 0, x, width - 1 ) ) {
					horizontal( x, t ) = v;
				}
			}
		}
		fn.print( "D:\\temp\\(%d %d)vert_link.bmp", p.x, p.y );
		saveBmp( vertical, fn );
		fn.print( "D:\\temp\\(%d %d)horz_link.bmp", p.x, p.y );
		saveBmp( horizontal, fn );
	}

	//領域を右クリックで属性を設定する
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		if ( ! it->shapes[doc->currentViewImageIndex].in( p.x, p.y ) ) continue;
		for ( list<boundaryPart>::iterator itp( it->parts ); itp; ++itp ) {
			if ( ! itp->shapes[doc->currentViewImageIndex].in( p.x, p.y ) ) continue;
			int areaIndex = doc->areas.index( it() );
			int partIndex = it->parts.index( itp() );
			setPartAttributeDialog dlg( areaIndex, partIndex, itp->type, mainWindow->hwnd );
			if ( dlg.call() ) {
				itp->type = dlg.type;
			}
		}
	}
}

void stateEditCurve::mouseMove( point2<int> p )
{
	int curveIndex, pointIndex;
	//点を右クリックなら削除
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		if ( it->find( curveIndex, pointIndex, doc->currentViewImageIndex, p ) ) {
			//点を削除できる
			SetCursor( releaseCursor );
			return;
		}
	}
	//線上を右クリックなら追加
	if ( doc->currentViewImageIndex == doc->baseIndex ) {
		for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
			if ( it->findOnLine( curveIndex, pointIndex, doc->currentViewImageIndex, p ) ) {
				SetCursor( appendCursor );
				return;
			}
		}
	}
	SetCursor( normalCursor );
}
void stateEditCurve::mouseLeftPush( point2<int> p )
{
	int curveIndex, pointIndex;
	//点を右クリックなら削除
	for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
		if ( it->find( curveIndex, pointIndex, doc->currentViewImageIndex, p ) ) {
			//点を削除する
			it->removePoint( curveIndex, pointIndex );
			mouseMove( p );
			imageWindow->invalidate();
			return;
		}
	}
	//線上を右クリックなら追加
	if ( doc->currentViewImageIndex == doc->baseIndex ) {
		for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
			if ( it->findOnLine( curveIndex, pointIndex, doc->currentViewImageIndex, p ) ) {
				boundaryCurve & curve = it->divisions[curveIndex];

				//セグメントの位置としてくるので，終点に移動
				pointIndex = pointIndex + 1;
				//if ( curve.cyclic ) pointIndex %= curve.points.size;
				//点を追加する
				bool validPoint = true;
				if ( curveIndex == 0 ) {
				} else {
					if ( pointIndex == 0 ) {
						validPoint = false;
					}
				}
				if ( validPoint ) {
					it->appendPoint( curveIndex, pointIndex, p );
				}
				mouseMove( p );
				imageWindow->invalidate();
				return;
			}
		}
	}
}
