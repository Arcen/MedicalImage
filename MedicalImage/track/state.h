
class trackStateBase
{
public:
	virtual void begin()
	{
	}
	virtual void end()
	{
	}
	virtual void paint()
	{
	}
	virtual point2<int> imageSize()
	{
		return point2<int>( 0, 0 );
	}
	virtual void mouseLeftPush( point2<int> p )
	{
	}
	virtual void mouseLeftPop( point2<int> p )
	{
	}
	virtual void mouseLeftMove( point2<int> p )
	{
	}
	virtual void mouseRightPush( point2<int> p )
	{
	}
	virtual void mouseRightPop( point2<int> p )
	{
	}
	virtual void mouseRightMove( point2<int> p )
	{
	}
	virtual void mouseMove( point2<int> p )
	{
	}
};

class trackState
{
public:
	static trackState * get()
	{
		static trackState instance;
		return & instance;
	}
	trackStateBase * now;
	void change( trackStateBase * after )
	{
		if ( now ) {
			now->end();
			now = NULL;
		}
		now = after;
		if ( now ) {
			now->begin();
		}
	}
	void paint()
	{
		if ( now ) now->paint();
	}
	point2<int> imageSize()
	{
		if ( now ) return now->imageSize();
		return point2<int>( 0, 0 );
	}
	bool mousePoint( point2<int> & p, LPARAM lParam )
	{
		p = point2<int>( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
		p.x += imageWindow->scrollPosition.x;
		p.y += imageWindow->scrollPosition.y;
		trackView * view = trackView::get();
		p.x /= view->imageScale;
		p.y /= view->imageScale;
		point2<int> s = imageSize();
		if ( ! s.x || ! s.y ) return false;
		p.x = clamp( 0, p.x, s.x - 1 );
		p.y = clamp( 0, p.y, s.y - 1 );
		return true;
	}
	void mouseLeftPush( LPARAM lParam )
	{
		point2<int> p; if ( ! mousePoint( p, lParam ) ) return;
		if ( now ) now->mouseLeftPush( p );
	}
	void mouseLeftPop( LPARAM lParam )
	{
		point2<int> p; if ( ! mousePoint( p, lParam ) ) return;
		if ( now ) now->mouseLeftPop( p );
	}
	void mouseLeftMove( LPARAM lParam )
	{
		point2<int> p; if ( ! mousePoint( p, lParam ) ) return;
		if ( now ) now->mouseLeftMove( p );
	}
	void mouseRightPush( LPARAM lParam )
	{
		point2<int> p; if ( ! mousePoint( p, lParam ) ) return;
		if ( now ) now->mouseRightPush( p );
	}
	void mouseRightPop( LPARAM lParam )
	{
		point2<int> p; if ( ! mousePoint( p, lParam ) ) return;
		if ( now ) now->mouseRightPop( p );
	}
	void mouseRightMove( LPARAM lParam )
	{
		point2<int> p; if ( ! mousePoint( p, lParam ) ) return;
		if ( now ) now->mouseRightMove( p );
	}
	void mouseMove( LPARAM lParam )
	{
		point2<int> p; if ( ! mousePoint( p, lParam ) ) return;
		if ( now ) now->mouseMove( p );
	}
};

class stateNoDocument : public trackStateBase
{
public:
	static stateNoDocument * get()
	{
		static stateNoDocument instance;
		return & instance;
	}
	virtual void begin()
	{
		sliderBar->changed = NULL;
		sliderBar->set( 0, 0 );
		sliderBar->set( 0 );
		sliderBar->enable( false );
		trackView::get()->updateImageScale();

		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->enabled( IDM_PLAY, false );
		toolBar->enabled( IDM_SETBASEWAVE, false );
		toolBar->enabled( IDM_EDITBASEWAVE, false );
		toolBar->enabled( IDM_WRITECURVE, false );
		toolBar->enabled( IDM_EDITCURVE, false );
		toolBar->enabled( IDM_DIVIDECURVE, false );

		mainMenu->enable( IDM_LOAD, false );
		mainMenu->enable( IDM_LOAD_AS, false );
		mainMenu->enable( IDM_IMPORT_FROM, false );
		mainMenu->enable( IDM_SAVE, false );
		mainMenu->enable( IDM_SAVE_AS, false );
		mainMenu->enable( IDM_EXPORT_INFO, false );
		mainMenu->enable( IDM_EXPORT_SELECTED_INFO, false );

		mainMenu->enable( IDM_PLAY, false );
		mainMenu->enable( IDM_NEXT, false );
		mainMenu->enable( IDM_PREV, false );
		mainMenu->enable( IDM_FIT, false );
		mainMenu->enable( IDM_VS_1, false );
		mainMenu->enable( IDM_VS_2, false );
		mainMenu->enable( IDM_VS_4, false );
		mainMenu->enable( IDM_VM_ORIGINAL, false );
		mainMenu->enable( IDM_VM_VERTEDGE, false );
		mainMenu->enable( IDM_VM_HORZEDGE, false );
		mainMenu->enable( IDM_VM_NONE, false );
		mainMenu->enable( IDM_VM_V, false );
		mainMenu->enable( IDM_VM_H, false );
		mainMenu->enable( IDM_VM_A, false );
		mainMenu->enable( IDM_VM_D, false );

		mainMenu->enable( IDM_NORMALMODE, false );
		mainMenu->enable( IDM_SETBASETIME, false );
		mainMenu->enable( IDM_GETBASETIME, false );
		mainMenu->enable( IDM_SETINSPIRATIONFRAME, false );
		mainMenu->enable( IDM_GETINSPIRATIONFRAME, false );
		mainMenu->enable( IDM_SETEXPIRATIONFRAME, false );
		mainMenu->enable( IDM_GETEXPIRATIONFRAME, false );
		mainMenu->enable( IDM_SETBASEWAVE, false );
		mainMenu->enable( IDM_EDITBASEWAVE, false );
		mainMenu->enable( IDM_WRITECURVE, false );
		mainMenu->enable( IDM_EDITCURVE, false );
		mainMenu->enable( IDM_DIVIDECURVE, false );
		mainMenu->enable( IDM_CLEARCURVES, false );
		mainMenu->enable( IDM_AGAIN_ANALYSIS, false );
		
		mainMenu->enable( IDM_A_ALL, false );
		mainMenu->enable( IDM_A_ALLS, false );
		mainMenu->enable( IDM_CLEAR, false );
		mainMenu->enable( IDM_A_V, false );
		mainMenu->enable( IDM_A_H, false );
		mainMenu->enable( IDM_A_A, false );
		mainMenu->enable( IDM_A_D, false );
		mainMenu->enable( IDM_A_VS, false );
		mainMenu->enable( IDM_A_HS, false );
		mainMenu->enable( IDM_A_AS, false );
		mainMenu->enable( IDM_A_DS, false );
		mainMenu->enable( IDM_A_BLOOD_ALL, false );
		mainMenu->enable( IDM_ANALYSIS_DOC, false );
		mainMenu->enable( ID_SHOW_CURVE, false );
	}
	virtual void end()
	{
		sliderBar->enable( true );
	}
	virtual void paint()
	{
		RECT cr = imageWindow->client();
		HDC hdc = imageWindow->getDC();
		BitBlt( hdc, cr.left, cr.top, cr.right - cr.left, cr.bottom - cr.top, NULL, 0, 0, BLACKNESS );
		imageWindow->release( hdc );
		imageWindow->validate();
		mainWindow->validate();
	}
};
class stateDocument : public trackStateBase
{
	boundaryArea * movingArea;
	int movingCurveIndex;
	int movingPointIndex;
public:
	trackDocument * doc;
	stateDocument()
	{
		movingArea = NULL;
	}
	static stateDocument * get()
	{
		static stateDocument instance;
		return & instance;
	}
	static void sliderMoved( fc::window * value )
	{
		if ( value == sliderBar ) {
			trackDocument * doc = trackDocument::get();
			if ( doc ) {
				doc->currentViewImageIndex = sliderBar->index;
				statusBar->set( string( "File : " ) + doc->imageFilenames[sliderBar->index] );
			}
			imageWindow->invalidate();
		}
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		movingArea = NULL;
		sliderBar->changed = stateDocument::sliderMoved;
		sliderBar->set( 0, doc->sizeTime() - 1 );
		sliderBar->enable( true );
		sliderBar->set( doc->currentViewImageIndex );
		trackView::get()->updateImageScale();

		toolBar->enabled( IDM_NORMALMODE, true );
		toolBar->enabled( IDM_PLAY, true );
		toolBar->enabled( IDM_SETBASEWAVE, true );
		toolBar->enabled( IDM_EDITBASEWAVE, ! doc->wave.empty() );
		toolBar->enabled( IDM_WRITECURVE, ! doc->wave.empty() );
		toolBar->enabled( IDM_EDITCURVE, ! doc->areas.empty() );
		toolBar->enabled( IDM_DIVIDECURVE, ! doc->areas.empty() );

		toolBar->checked( IDM_NORMALMODE, true );

		mainMenu->enable( IDM_LOAD, true );
		mainMenu->enable( IDM_LOAD_AS, true );
		mainMenu->enable( IDM_IMPORT_FROM, true );
		mainMenu->enable( IDM_SAVE, true );
		mainMenu->enable( IDM_SAVE_AS, true );
		mainMenu->enable( IDM_EXPORT_INFO, true );
		mainMenu->enable( IDM_EXPORT_SELECTED_INFO, true );

		mainMenu->enable( IDM_PLAY, true );
		mainMenu->enable( IDM_NEXT, true );
		mainMenu->enable( IDM_PREV, true );
		mainMenu->enable( IDM_FIT, true );
		mainMenu->enable( IDM_VS_1, true );
		mainMenu->enable( IDM_VS_2, true );
		mainMenu->enable( IDM_VS_4, true );
		mainMenu->enable( IDM_VM_ORIGINAL, true );
		mainMenu->enable( IDM_VM_VERTEDGE, true );
		mainMenu->enable( IDM_VM_HORZEDGE, true );
		mainMenu->enable( IDM_VM_NONE, true );
		mainMenu->enable( IDM_VM_V, true );
		mainMenu->enable( IDM_VM_H, true );
		mainMenu->enable( IDM_VM_A, true );
		mainMenu->enable( IDM_VM_D, true );

		mainMenu->enable( IDM_NORMALMODE, true );
		mainMenu->enable( IDM_SETBASETIME, true );
		mainMenu->enable( IDM_GETBASETIME, true );
		mainMenu->enable( IDM_SETINSPIRATIONFRAME, true );
		mainMenu->enable( IDM_GETINSPIRATIONFRAME, true );
		mainMenu->enable( IDM_SETEXPIRATIONFRAME, true );
		mainMenu->enable( IDM_GETEXPIRATIONFRAME, true );
		mainMenu->enable( IDM_SETBASEWAVE, true );
		mainMenu->enable( IDM_EDITBASEWAVE, ! doc->wave.empty() );
		mainMenu->enable( IDM_WRITECURVE, ! doc->wave.empty() );
		mainMenu->enable( IDM_EDITCURVE, ! doc->areas.empty() );
		mainMenu->enable( IDM_DIVIDECURVE, ! doc->areas.empty() );
		mainMenu->enable( IDM_CLEARCURVES, ! doc->areas.empty() );
		mainMenu->enable( IDM_AGAIN_ANALYSIS, ! doc->areas.empty() );
		
		mainMenu->enable( IDM_A_ALL, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_ALLS, ! doc->wave.empty() );
		mainMenu->enable( IDM_CLEAR, true );
		mainMenu->enable( IDM_A_V, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_H, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_A, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_D, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_VS, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_HS, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_AS, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_DS, ! doc->wave.empty() );
		mainMenu->enable( IDM_A_BLOOD_ALL, true );
		mainMenu->enable( IDM_ANALYSIS_DOC, ! doc->areas.empty() );
		mainMenu->enable( ID_SHOW_CURVE, true );
	}
	virtual void end()
	{
		sliderBar->changed = NULL;
		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->enabled( IDM_PLAY, false );
		toolBar->enabled( IDM_SETBASEWAVE, false );
		toolBar->enabled( IDM_EDITBASEWAVE, false );
		mainMenu->enable( IDM_SETINSPIRATIONFRAME, false );
		mainMenu->enable( IDM_GETINSPIRATIONFRAME, false );
		mainMenu->enable( IDM_SETEXPIRATIONFRAME, false );
		mainMenu->enable( IDM_GETEXPIRATIONFRAME, false );
		toolBar->enabled( IDM_WRITECURVE, false );
		toolBar->enabled( IDM_DIVIDECURVE, false );
		toolBar->checked( IDM_NORMALMODE, false );

		mainMenu->enable( IDM_LOAD, false );
		mainMenu->enable( IDM_LOAD_AS, false );
		mainMenu->enable( IDM_IMPORT_FROM, false );
		mainMenu->enable( IDM_SAVE, false );
		mainMenu->enable( IDM_SAVE_AS, false );
		mainMenu->enable( IDM_EXPORT_INFO, false );
		mainMenu->enable( IDM_EXPORT_SELECTED_INFO, false );

		mainMenu->enable( IDM_PLAY, false );
		mainMenu->enable( IDM_NEXT, false );
		mainMenu->enable( IDM_PREV, false );
		mainMenu->enable( IDM_FIT, false );
		mainMenu->enable( IDM_VS_1, false );
		mainMenu->enable( IDM_VS_2, false );
		mainMenu->enable( IDM_VS_4, false );
		mainMenu->enable( IDM_VM_ORIGINAL, false );
		mainMenu->enable( IDM_VM_VERTEDGE, false );
		mainMenu->enable( IDM_VM_HORZEDGE, false );
		mainMenu->enable( IDM_VM_NONE, false );
		mainMenu->enable( IDM_VM_V, false );
		mainMenu->enable( IDM_VM_H, false );
		mainMenu->enable( IDM_VM_A, false );
		mainMenu->enable( IDM_VM_D, false );

		mainMenu->enable( IDM_NORMALMODE, false );
		mainMenu->enable( IDM_SETBASETIME, false );
		mainMenu->enable( IDM_GETBASETIME, false );
		mainMenu->enable( IDM_SETBASEWAVE, false );
		mainMenu->enable( IDM_EDITBASEWAVE, false );
		mainMenu->enable( IDM_WRITECURVE, false );
		mainMenu->enable( IDM_EDITCURVE, false );
		mainMenu->enable( IDM_DIVIDECURVE, false );
		mainMenu->enable( IDM_CLEARCURVES, false );
		mainMenu->enable( IDM_AGAIN_ANALYSIS, false );
		
		mainMenu->enable( IDM_A_ALL, false );
		mainMenu->enable( IDM_A_ALLS, false );
		mainMenu->enable( IDM_CLEAR, false );
		mainMenu->enable( IDM_A_V, false );
		mainMenu->enable( IDM_A_H, false );
		mainMenu->enable( IDM_A_A, false );
		mainMenu->enable( IDM_A_D, false );
		mainMenu->enable( IDM_A_VS, false );
		mainMenu->enable( IDM_A_HS, false );
		mainMenu->enable( IDM_A_AS, false );
		mainMenu->enable( IDM_A_DS, false );
		mainMenu->enable( IDM_A_BLOOD_ALL, false );
		mainMenu->enable( IDM_ANALYSIS_DOC, false );
		mainMenu->enable( ID_SHOW_CURVE, false );
	}
	virtual void paint();
	void paint( image & viewImage );
	virtual point2<int> imageSize()
	{
		return point2<int>( doc->width, doc->height );
	}
	virtual void mouseLeftPush( point2<int> p );
	virtual void mouseLeftMove( point2<int> p );
	virtual void mouseLeftPop( point2<int> p );
	virtual void mouseRightPush( point2<int> p );
	virtual void mouseRightMove( point2<int> p );
	virtual void mouseRightPop( point2<int> p );
};

class stateIndex1stPointOfBaseWave : public stateDocument
{
public:
	point2<int> firstwave;
	static stateIndex1stPointOfBaseWave * get()
	{
		static stateIndex1stPointOfBaseWave instance;
		return & instance;
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		statusBar->set( "Point first point of segment for base wave." );
		toolBar->enabled( IDM_NORMALMODE, true );
		toolBar->checked( IDM_SETBASEWAVE, true );
		mainMenu->enable( IDM_NORMALMODE, true );
	}
	virtual void end()
	{
		statusBar->set( "" );
		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->checked( IDM_SETBASEWAVE, false );
		mainMenu->enable( IDM_NORMALMODE, false );
	}
	virtual void mouseLeftPush( point2<int> p );
	virtual void mouseRightPush( point2<int> p ){};//ìÆçÏÇ≥ÇπÇ»Ç¢ÇÊÇ§Ç…Ç∑ÇÈÇΩÇﬂ
	virtual void mouseRightPop( point2<int> p )
	{
		trackState::get()->change( stateDocument::get() );
	}
};

class stateIndex2ndPointOfBaseWave : public stateDocument
{
public:
	static stateIndex2ndPointOfBaseWave * get()
	{
		static stateIndex2ndPointOfBaseWave instance;
		return & instance;
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		statusBar->set( "Point last point of segment for base wave. If shift was pushed, adjust x position." );
		toolBar->enabled( IDM_NORMALMODE, true );
		toolBar->checked( IDM_SETBASEWAVE, true );
		mainMenu->enable( IDM_NORMALMODE, true );
	}
	virtual void end()
	{
		statusBar->set( "" );
		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->checked( IDM_SETBASEWAVE, false );
		mainMenu->enable( IDM_NORMALMODE, false );
	}
	virtual void mouseLeftPush( point2<int> p );
	virtual void mouseRightPush( point2<int> p ){};//ìÆçÏÇ≥ÇπÇ»Ç¢ÇÊÇ§Ç…Ç∑ÇÈÇΩÇﬂ
	virtual void mouseRightPop( point2<int> p )
	{
		trackState::get()->change( stateDocument::get() );
	}
};

class stateWriteCurve : public stateDocument
{
public:
	static stateWriteCurve * get()
	{
		static stateWriteCurve instance;
		return & instance;
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		if ( doc ) {
			doc->currentViewImageIndex = doc->baseIndex;
			sliderBar->set( doc->currentViewImageIndex );
		}
		imageWindow->invalidate();
		sliderBar->enable( false );
		sliderBar->changed = NULL;
		statusBar->set( "Write curve. The left mouse click adds point to this curve. The right mouse click let this mode end." );
		if ( doc ) {
			doc->areas.push_back( boundaryArea::factory() );
			boundaryArea & area = doc->areas.last();
			area.enable = false;
			area.touch( -1 );
		}
		toolBar->enabled( IDM_NORMALMODE, true );
		toolBar->checked( IDM_WRITECURVE, true );
		mainMenu->enable( IDM_NORMALMODE, true );
	}
	virtual void end()
	{
		if ( doc ) {
			if ( ! doc->areas.empty() ) {
				boundaryArea & area = doc->areas.last();
				if ( area.divisions.last().points.empty() ) {
					doc->areas.pop_back();
				} else {
					area.enable = true;
					statusBar->set( "Analysing" );
					area.updatePoint();
				}
			}
		}
		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->checked( IDM_WRITECURVE, false );
		mainMenu->enable( IDM_NORMALMODE, false );
		imageWindow->invalidate();
		statusBar->set( "" );
	}
	virtual void mouseLeftPush( point2<int> p )
	{
		if ( ! doc ) return;
		if ( doc->areas.empty() ) return;
		boundaryArea & area = doc->areas.last();
		area.appendCircumferencePoint( p );
		imageWindow->invalidate();
	}
	virtual void mouseRightPush( point2<int> p ){};//ìÆçÏÇ≥ÇπÇ»Ç¢ÇÊÇ§Ç…Ç∑ÇÈÇΩÇﬂ
	virtual void mouseRightPop( point2<int> p )
	{
		trackState::get()->change( stateDocument::get() );
	}
};
class stateEditCurve : public stateDocument
{
public:
	HCURSOR normalCursor;
	HCURSOR appendCursor;
	HCURSOR releaseCursor;
	stateEditCurve()
	{
		normalCursor = LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) );
		appendCursor = LoadCursor( fc::window::instance, MAKEINTRESOURCE( IDC_APPEND ) );
		releaseCursor = LoadCursor( fc::window::instance, MAKEINTRESOURCE( IDC_RELEASE ) );
	}
	static stateEditCurve * get()
	{
		static stateEditCurve instance;
		return & instance;
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		sliderBar->enable( false );
		sliderBar->changed = NULL;
		statusBar->set( "Edit curve. The left mouse click adds point on the curve or removes point on the control point. The right mouse click let this mode end." );
		toolBar->enabled( IDM_NORMALMODE, true );
		toolBar->checked( IDM_EDITCURVE, true );
		mainMenu->enable( IDM_NORMALMODE, true );
	}
	virtual void end()
	{
		SetCursor( normalCursor );
		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->checked( IDM_EDITCURVE, false );
		mainMenu->enable( IDM_NORMALMODE, false );
		imageWindow->invalidate();
		statusBar->set( "" );
	}
	virtual void mouseMove( point2<int> p );
	virtual void mouseLeftPush( point2<int> p );
	virtual void mouseRightPush( point2<int> p ){};//ìÆçÏÇ≥ÇπÇ»Ç¢ÇÊÇ§Ç…Ç∑ÇÈÇΩÇﬂ
	virtual void mouseRightPop( point2<int> p )
	{
		trackState::get()->change( stateDocument::get() );
	}
};

class stateDivideCurve : public stateDocument
{
public:
	int state;
	boundaryArea * targetArea;
	int firstCurveIndex;
	int firstPointIndex;
	int targetPartIndex;
	int lastCurveIndex;
	int lastPointIndex;
	static stateDivideCurve * get()
	{
		static stateDivideCurve instance;
		return & instance;
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		state = 0;
		targetArea = NULL;
		if ( doc ) {
			doc->currentViewImageIndex = doc->baseIndex;
			sliderBar->set( doc->currentViewImageIndex );
		}
		sliderBar->enable( false );
		sliderBar->changed = NULL;
		statusBar->set( "Divide curve. Indicate first point of circumference curve. The right mouse click let this mode end." );
		toolBar->enabled( IDM_NORMALMODE, true );
		toolBar->checked( IDM_DIVIDECURVE, true );
		mainMenu->enable( IDM_NORMALMODE, true );
	}
	virtual void end()
	{
		if ( targetArea ) {
			statusBar->set( "Analysing" );
			targetArea->enable = true;
			targetArea->touch( -1 );
			targetArea->update( -1 );
		}
		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->checked( IDM_DIVIDECURVE, false );
		mainMenu->enable( IDM_NORMALMODE, false );
		imageWindow->invalidate();
		statusBar->set( "" );
	}
	virtual void mouseLeftPush( point2<int> p )
	{
		if ( ! doc ) return;
		if ( state == 0 ) {
			targetArea = NULL;
			for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
				if ( it->find( firstCurveIndex, firstPointIndex, doc->currentViewImageIndex, p ) ) {
					if ( firstCurveIndex == 0 ) {
						if ( it->numberPart( firstCurveIndex, firstPointIndex ) == 1 ) {
							targetArea = & it();
							targetPartIndex = it->partIndex( firstCurveIndex, firstPointIndex );
							state = 1;
							statusBar->set( "Divide curve. Indicate last point of same part. The right mouse click let this mode end." );
							return;
						} else {
							statusBar->set( "Need to indicate the point which is not included multiple part. : Divide curve. Indicate first point of circumference curve. The right mouse click let this mode end." );
						}
					}
				}
			}
		} else if ( state == 1 ) {
			lastPointIndex = 0;
			boundaryPart & part = targetArea->parts[targetPartIndex];
			if ( targetArea->find( lastCurveIndex, lastPointIndex, doc->currentViewImageIndex, p ) ) {
				if ( part.including( lastCurveIndex, lastPointIndex ) ) {
					if ( firstCurveIndex == lastCurveIndex && 
						firstPointIndex == lastPointIndex ) {
						statusBar->set( "Need to indicate other point. : Divide curve. Indicate first point of circumference curve. The right mouse click let this mode end." );
						return;
					}

					state = 2;
					targetArea->enable = false;
					targetArea->divide( targetPartIndex, firstCurveIndex, firstPointIndex, lastCurveIndex, lastPointIndex );
					imageWindow->invalidate();
					statusBar->set( "Divide curve. Write intermidiate point. The right mouse click let this mode end." );
					return;
				}
			}
		} else if ( state == 2 ) {
			targetArea->appendDividePoint( p );
			imageWindow->invalidate();
		}
	}
	virtual void mouseRightPush( point2<int> p ){};//ìÆçÏÇ≥ÇπÇ»Ç¢ÇÊÇ§Ç…Ç∑ÇÈÇΩÇﬂ
	virtual void mouseRightPop( point2<int> p )
	{
		trackState::get()->change( stateDocument::get() );
	}
};

class stateBaseWave : public stateDocument
{
public:
	point2<int> movingPoint;
	bool changed;
	static stateBaseWave * get()
	{
		static stateBaseWave instance;
		return & instance;
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		sliderBar->enable( false );
		sliderBar->changed = NULL;
		trackView::get()->updateImageScale();
		toolBar->enabled( IDM_NORMALMODE, true );
		toolBar->checked( IDM_EDITBASEWAVE, true );
		mainMenu->enable( IDM_NORMALMODE, true );
		mainMenu->enable( IDM_FIT, true );
		trackView::get()->changeImageScale( trackView::get()->imageScale * 2 );
		changed = false;
	}
	virtual void end()
	{
		if ( changed ) {
			statusBar->set( "updating" );
			doc->waveParameterUpdate( true );
			volatile int bi = doc->inspirationFrame;
			doc->changeBaseIndex( bi );
			for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
				it->updatePoint( false, false, true );
			}
		}
		toolBar->enabled( IDM_NORMALMODE, false );
		toolBar->checked( IDM_EDITBASEWAVE, false );
		mainMenu->enable( IDM_NORMALMODE, false );
		mainMenu->enable( IDM_FIT, false );
		trackView::get()->changeImageScale( trackView::get()->imageScale / 2 );
		statusBar->set( "" );
	}
	virtual void paint();
	virtual point2<int> imageSize()
	{
		return point2<int>( doc->baseWave.w, doc->baseWave.h );
	}
	virtual void mouseLeftPush( point2<int> p )
	{
		movingPoint = p;
	}
	virtual void mouseLeftMove( point2<int> p )
	{
		if ( movingPoint == point2<int>( -1, -1 ) ) return;
		if ( movingPoint.y == p.y ) return;
		if ( movingPoint.x != doc->baseIndex ) {
			doc->wave[movingPoint.x] += p.y - movingPoint.y;
			changed = true;
		} else {
			int delta = p.y - movingPoint.y;
			doc->waveBias += delta;
			for ( int x = 0; x < doc->wave.size; ++x ) {
				if ( doc->baseIndex == x ) continue;
				doc->wave[x] -= delta;
			}
			changed = true;
		}
		movingPoint.y = p.y;
		imageWindow->invalidate();
	}
	virtual void mouseLeftPop( point2<int> p )
	{
		mouseLeftMove( p );
		movingPoint = point2<int>( -1, -1 );
	}
	virtual void mouseRightPush( point2<int> p ){};//ìÆçÏÇ≥ÇπÇ»Ç¢ÇÊÇ§Ç…Ç∑ÇÈÇΩÇﬂ
	virtual void mouseRightPop( point2<int> p )
	{
		trackState::get()->change( stateDocument::get() );
	}
};
class stateSelectPointForExport : public stateDocument
{
public:
	int state;
	int areaIndex;
	int topPoint, innerPoint, outerPoint;
	string filename;
	static stateSelectPointForExport * get()
	{
		static stateSelectPointForExport instance;
		return & instance;
	}
	virtual void begin()
	{
		doc = trackDocument::get();
		state = 0;
		statusBar->set( "Export Select information. Indicate top point of curve. The right mouse click let this mode end." );
		toolBar->enabled( IDM_NORMALMODE, true );
		mainMenu->enable( IDM_NORMALMODE, true );
	}
	virtual void end()
	{
		toolBar->enabled( IDM_NORMALMODE, false );
		mainMenu->enable( IDM_NORMALMODE, false );
		statusBar->set( "" );
	}
	void output();
	virtual void mouseLeftPush( point2<int> p )
	{
		if ( ! doc ) return;
		if ( state == 0 ) {
			for ( list< boundaryArea >::iterator it( doc->areas ); it; ++it ) {
				int curveIndex, pointIndex;
				if ( it->find( curveIndex, pointIndex, doc->currentViewImageIndex, p ) ) {
					if ( curveIndex == 0 ) {
						state = 1;
						areaIndex = doc->areas.index( it() );
						topPoint = pointIndex;
						statusBar->set( "Export Select information. Indicate inner point of curve. The right mouse click let this mode end." );
					} else {
						statusBar->set( "Need to indicate circumference. Export Select information. Indicate top point of curve. The right mouse click let this mode end." );
					}
				}
			}
		} else if ( state == 1 ) {
			boundaryArea & area = doc->areas[areaIndex];
			int curveIndex, pointIndex;
			if ( area.find( curveIndex, pointIndex, doc->currentViewImageIndex, p ) ) {
				if ( curveIndex == 0 && topPoint != pointIndex ) {
					state = 2;
					innerPoint = pointIndex;
					statusBar->set( "Export Select information. Indicate outer point of curve. The right mouse click let this mode end." );
				} else {
					statusBar->set( "Need to indicate circumference. Export Select information. Indicate inner point of curve. The right mouse click let this mode end." );
				}
			}
		} else if ( state == 2 ) {
			boundaryArea & area = doc->areas[areaIndex];
			int curveIndex, pointIndex;
			if ( area.find( curveIndex, pointIndex, doc->currentViewImageIndex, p ) ) {
				if ( curveIndex == 0 && topPoint != pointIndex && innerPoint != pointIndex ) {
					outerPoint = pointIndex;
					statusBar->set( "Exporting" );
					output();
					trackState::get()->change( stateDocument::get() );
				} else {
					statusBar->set( "Need to indicate circumference. Export Select information. Indicate outer point of curve. The right mouse click let this mode end." );
				}
			}
		}
	}
	virtual void mouseRightPush( point2<int> p ){};//ìÆçÏÇ≥ÇπÇ»Ç¢ÇÊÇ§Ç…Ç∑ÇÈÇΩÇﬂ
	virtual void mouseRightPop( point2<int> p )
	{
		trackState::get()->change( stateDocument::get() );
	}
};

