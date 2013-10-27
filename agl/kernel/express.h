////////////////////////////////////////////////////////////////////////////////
// ウィンドウ・インタフェース・シーングラフ表示のMediator

//シーングラフのビューワーや、使い方のサンプルソースとして
class express
{
public:
	HWND hwnd;
	windows *window;
	renderInterface * render;
	int width, height;
	graphRender gr;
	float currentTime;
	bool laspe, loopScene;
	DWORD lastTime, oldTime;
	sgCamera * camera;
	sgOrtho * ortho;
	sgPerspective * perspective;
	sgFrustum * frustum;
	retainer<sgGraph> defaultGraph;
	vector3 initialCameraPosition, initialCameraInterest, initialCameraUpper;

	class sgGraphEx
	{
	public:
		sgGraph * graph;
		sgTime * time;
		list<sgCamera*> cameras;
		sgCamera * camera;
		sgOrtho * ortho;
		sgPerspective * perspective;
		sgFrustum * frustum;
		list<sgNode*> lights;
		list<sgMayaIkHandle*> ikHandles;
		sgGraphEx( sgGraph * _graph ) : graph( _graph ), time( NULL ), camera( NULL ), ortho( NULL ), perspective( NULL ), frustum( NULL )
		{
			list<sgNode*> result;
			graph->enumrate( result, sgTime::descriptor );
			if ( result.size ) time = static_cast<sgTime*>( result.first() );
			graph->enumrate( result, sgCamera::descriptor );
			if ( 0 && result.size ) {
				camera = static_cast<sgCamera*>( result.first() );
				for ( list<sgNode*>::iterator it( result ); it; ++it ) cameras.push_back( static_cast<sgCamera *>( it() ) );
				ortho = static_cast<sgOrtho*>( camera->getInput( sgCamera::attributes->ortho, sgOrtho::descriptor ) );
				perspective = static_cast<sgPerspective*>( camera->getInput( sgCamera::attributes->perspective, sgPerspective::descriptor ) );
				frustum = static_cast<sgFrustum*>( camera->getInput( sgCamera::attributes->frustum, sgFrustum::descriptor ) );
			}
			graph->enumrate( result, sgAmbientLight::descriptor );
			for ( list<sgNode*>::iterator ita( result ); ita; ++ita ) lights.push_back( ita() );
			graph->enumrate( result, sgPositionLight::descriptor );
			for ( list<sgNode*>::iterator itp( result ); itp; ++itp ) lights.push_back( itp() );
			graph->enumrate( result, sgDirectionLight::descriptor );
			for ( list<sgNode*>::iterator itd( result ); itd; ++itd ) lights.push_back( itd() );
			graph->enumrate( result, sgSpotLight::descriptor );
			for ( list<sgNode*>::iterator its( result ); its; ++its ) lights.push_back( its() );
			graph->enumrate( result, sgMayaIkHandle::descriptor );
			for ( list<sgNode*>::iterator its( result ); its; ++its ) ikHandles.push_back( static_cast<sgMayaIkHandle*>( its() ) );
		}
		~sgGraphEx()
		{
			delete graph;
		}
	};
	list< retainer<sgGraphEx> > graphs;
	express( HINSTANCE instance = NULL, sgGraph * g = NULL ) : hwnd( NULL ), window( NULL ), width( 0 ), height( 0 ), render( NULL ), currentTime( 0 ), laspe( false ), loopScene( true ), lastTime( 0 ), oldTime( timeGetTime() ), 
		camera( NULL ), ortho( NULL ), perspective( NULL ), frustum( NULL )
	{
		sgGraph::initialize();
		defaultGraph = new sgGraph;
		camera = new sgCamera( *defaultGraph );
		initialCameraPosition = camera->position();
		initialCameraInterest = camera->interest();
		initialCameraUpper = camera->upper();
		if ( g && set( g ) && createWindow( instance ) && createRender() && initializeRender() ) {
			loop();
		} else if ( instance && load() && createWindow( instance ) && createRender() && initializeRender() ) {
			loop();
		}
	}
	~express()
	{
		delete window;
		graphs.release();
		sgGraph::finalize();
	}
	bool createRender( bool directX8 = true )
	{
		//if ( directX8 ) {
		//	gr.render = render = renderInterface::createDirectX8Render();
		//} else {
			gr.render = render = renderInterface::createOpenGLRender();
		//}
		return render ? true : false;
	}
	bool createWindow( HINSTANCE instance, WNDPROC wndproc = defaultWndProc, int width = 640, int height = 480, int menu = 0, int accel = 0, bool fullscreen = false, const char * classname = "express", bool regist = true )
	{
		window = new windows( instance, classname, wndproc );
		window->width = width;
		window->height = height;
		window->fullscreen = fullscreen;
		if ( regist ) if ( ! window->registerClass() ) return false;
		if ( ! window->windows::createWindow( this, LoadMenu( instance, MAKEINTRESOURCE( menu ) ), LoadAccelerators( instance, MAKEINTRESOURCE( accel ) ) ) ) return false;

		this->hwnd = window->hwnd;
		this->width = window->width;
		this->height = window->height;
		return true;
	}
	bool setWindow( HINSTANCE instance, const char * classname, HWND hwnd, int accel = 0 )
	{
		window = new windows( instance, classname, NULL );
		window->setWindow( NULL, hwnd, accel ? LoadAccelerators( instance, MAKEINTRESOURCE( accel ) ) : NULL );
		this->hwnd = window->hwnd;
		this->width = window->width;
		this->height = window->height;
		return true;
	}
	bool initializeRender()
	{
		if ( ! window ) return false;
		if ( ! render ) return false;
		return render->initialize( hwnd, ! window->fullscreen, width, height );
	}
	void loop()
	{
		if ( window ) window->loop();
	}
	bool set( sgGraph * g )
	{
		if ( ! g ) return false;

		sgGraphEx * gex = new sgGraphEx( g );
		graphs.push_back( gex );
		if ( gex->camera ) {//シーンのカメラは無視
			camera = gex->camera;
			ortho = gex->ortho;
			perspective = gex->perspective;
			frustum = gex->frustum;
			//初期カメラ情報の保持
			initialCameraPosition = camera->position();
			initialCameraInterest = camera->interest();
			initialCameraUpper = camera->upper();
		} else {
			//カメラ情報がない場合には、オブジェクトの中心を見るようにカメラを設定
			graphInformation gi;
			vector3 mnmm, mxmm;
			if ( gi.boundaryBox( *g, mnmm, mxmm ) ) {
				vector3 center = ( mnmm + mxmm ) / 2;
				//あまりにも小さいものはある程度はなして見る。
				if ( fabs( mnmm.z - center.z ) < 1 ) {
					initialCameraPosition = vector3( 0, 0, mnmm.z - 1 );
				} else {
					initialCameraPosition = vector3( 0, 0, mnmm.z + ( mnmm.z - center.z ) );
				}
				initialCameraInterest = center;
				initialCameraUpper = vector3( 0, 1, 0 );
				camera->position() = initialCameraPosition;
				camera->interest() = initialCameraInterest;
				camera->upper() = initialCameraUpper;
				camera->dirty = true;
			}
		}
		return true;
	}
	bool load( const char * _filename = "" )
	{
		char filename[MAX_PATH] = "";
		if ( strlen( _filename ) ) {
			strcpy( filename, _filename );
		} else {
			if ( ! windows::fileOpen( filename, "", "AGL File(*.agl)\0*.agl\0", "Open AGL File", NULL ) ) return false;
		}
		return set( sgGraph::load( filename ) );
	}
	void draw()
	{
		if ( ! render || ! render->initialized() ) return;

		DWORD nowTime = timeGetTime();
		if ( laspe && lastTime ) currentTime += float( ( double ) nowTime - ( double ) lastTime ) / 1000.0f;
		lastTime = timeGetTime();

		bool first = true;
		render->activate( true );
		render->cull( false, true );
		gr.initialize();
		gr.defaultTest();
		for ( list< retainer<sgGraphEx> >::iterator it( graphs ); it; ++it ) {
			sgGraphEx * g = & it()();
			if ( g->time && g->time->current() != currentTime ) {
				g->time->dirty = true;
				decimal ct = g->time->fps() * currentTime / 10.0;
				if ( loopScene ) ct = fmod( ct, g->time->final() - g->time->first() );
				g->time->current( ct );
			}
			g->graph->update( true );//情報の更新
			gr.prepare( g->graph );
			render->clearCache();
			if ( first ) {
				if ( g->camera ) {
					if ( g->perspective ) {
						g->perspective->aspect( ( float ) ( width ) / ( float ) ( height ) );
						gr.execute( g->perspective );
					} else if ( g->ortho ) {
						g->ortho->sizeY( ( float ) ( height ) );
						g->ortho->aspect( ( float ) ( width ) / ( float ) ( height ) );
						gr.execute( g->ortho );
					} else if ( g->frustum ) {
						gr.execute( g->frustum );
					} else {
						render->setPerspective( 30, ( float ) ( width ) / ( float ) ( height ), 0.1f, 1000.0f );
					}
				} else if ( camera ) {
					if ( perspective ) {
						perspective->aspect( ( float ) ( width ) / ( float ) ( height ) );
						gr.execute( perspective );
					} else if ( ortho ) {
						ortho->sizeY( ( float ) ( height ) );
						ortho->aspect( ( float ) ( width ) / ( float ) ( height ) );
						gr.execute( ortho );
					} else if ( frustum ) {
						gr.execute( frustum );
					} else {
						render->setPerspective( 30, ( float ) ( width ) / ( float ) ( height ), 0.1f, 1000.0f );
					}
				} else {
					render->setPerspective( 30, ( float ) ( width ) / ( float ) ( height ), 0.1f, 1000.0f );
				}
				render->setViewport( 0, 0, width, height );
				render->startScene( 0xFF777777 );
			}
			if ( g->camera ) {
				gr.execute( g->camera );
			} else if ( camera ) {
				gr.execute( camera );
			} else {
				gr.defaultCamera();
			}
			gr.defaultLight();
			for ( list<sgNode*>::iterator itl( g->lights ); itl; ++itl ) {
				sgNode * light = itl();
				if ( light->thisDescriptor == sgAmbientLight::descriptor ) gr.execute( static_cast<sgAmbientLight*>( light ) );
				else if ( light->thisDescriptor == sgPositionLight::descriptor ) gr.execute( static_cast<sgPositionLight*>( light ) );
				else if ( light->thisDescriptor == sgDirectionLight::descriptor ) gr.execute( static_cast<sgDirectionLight*>( light ) );
				else if ( light->thisDescriptor == sgSpotLight::descriptor ) gr.execute( static_cast<sgSpotLight*>( light ) );
			}
			gr.execute( g->graph->root );
			first = false;
		}
		render->swapBuffer();
		render->activate( false );
		ValidateRect( hwnd, NULL );
	}
	void pan( decimal x ) { if ( camera ) camera->pan( x ); }
	void tilt( decimal y ) { if ( camera ) camera->tilt( y ); }
	void pan( decimal x, decimal y ) { if ( camera ) camera->pan( x, y ); }
	void follow( decimal x, decimal y, decimal z ) { if ( camera ) camera->follow( x, y, z ); }
	void zoom( decimal z ) { if ( camera ) camera->zoom( z ); }
	void track( decimal x ) { if ( camera ) camera->track( x ); }
	void crane( decimal y ) { if ( camera ) camera->crane( y ); }
	void dolly( decimal z ) { if ( camera ) camera->dolly( z ); }
	void move( decimal x, decimal y, decimal z ) { if ( camera ) camera->move( x, y, z ); }

	static LRESULT CALLBACK defaultWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		static express * e = NULL;
		if ( ! e ) e = ( express * ) GetWindowLong( hWnd, GWL_USERDATA );
		DWORD mouseX = 0xffffffff, mouseY = 0xffffffff;
		static DWORD mouseOldX = 0xffffffff, mouseOldY = 0xffffffff;
		bool mouseRPushed = false;
		bool mouseMPushed = false;
		bool mouseLPushed = false;
		switch( message ) 
		{
		case WM_KEYDOWN:
			if ( wParam == VK_ESCAPE ) DestroyWindow( hWnd );
			if ( wParam == VK_SPACE ) {
				if ( e->camera ) {
					e->camera->position() = e->initialCameraPosition;
					e->camera->interest() = e->initialCameraInterest;
					e->camera->upper() = e->initialCameraUpper;
					e->camera->dirty = true;
				}
			}
			if ( wParam == VK_NUMPAD1 ) {
				e->laspe = ! e->laspe;
				SetTimer( hWnd, 1, 10, NULL );
			}
			if ( wParam == VK_NUMPAD2 ) {
				KillTimer( hWnd, 1 );
				e->currentTime = 0; e->lastTime = 0;
			}
			InvalidateRect( hWnd, NULL, FALSE );
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			mouseOldX = GET_X_LPARAM(lParam);
			mouseOldY = GET_Y_LPARAM(lParam);
			break;
		case WM_MOUSEMOVE:
			mouseLPushed = ( wParam & MK_LBUTTON ) ? true : false;
			mouseMPushed = ( wParam & MK_MBUTTON ) ? true : false;
			mouseRPushed = ( wParam & MK_RBUTTON ) ? true : false;
			mouseX = GET_X_LPARAM(lParam);
			mouseY = GET_Y_LPARAM(lParam);
			if ( mouseOldX == 0xffffffff || mouseOldY == 0xffffffff ) {
				mouseOldX = mouseX;
				mouseOldY = mouseY;
				break;
			}
			if ( mouseLPushed && ! mouseMPushed && ! mouseRPushed ) {
				e->follow( ( float( mouseX ) - float( mouseOldX ) ) / e->width / 2, -( float( mouseY ) - float( mouseOldY ) ) / e->height / 2, 0 );
			} else if ( ! mouseLPushed && mouseMPushed && ! mouseRPushed ) {
				e->move( ( float( mouseX ) - float( mouseOldX ) ) / e->width / 2, -( float( mouseY ) - float( mouseOldY ) ) / e->height / 2, 0 );
			} else if ( mouseLPushed && mouseMPushed && ! mouseRPushed ) {
				e->zoom( ( float( mouseX ) - float( mouseOldX ) ) / e->width );
			} else if ( ! mouseLPushed && ! mouseMPushed && mouseRPushed ) {
				e->move( -( float( mouseX ) - float( mouseOldX ) ) / e->width / 2, 0, -( float( mouseY ) - float( mouseOldY ) ) / e->height / 2 );
			} else if ( mouseLPushed && ! mouseMPushed && mouseRPushed ) {
				e->pan( ( float( mouseX ) - float( mouseOldX ) ) / e->width / 2, -( float( mouseY ) - float( mouseOldY ) ) / e->height / 2 );
			}
			mouseOldX = mouseX;
			mouseOldY = mouseY;
			InvalidateRect( hWnd, NULL, FALSE );
			break;
			/*
#if (_WIN32_WINNT >= 0x0400) || (_WIN32_WINDOWS > 0x0400)
		case WM_MOUSEWHEEL:
			{
#ifdef GET_WHEEL_DELTA_WPARAM
				short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
#else
				short zDelta = HIWORD(wParam);
#endif
				if ( zDelta <= -120 || 120 <= zDelta ) {
					e->zoom( float( zDelta ) / 120.0 / 4 );
				}
			}
			break;
#endif
			*/
		case WM_PAINT:
			e->draw();
			break;
		case WM_TIMER:
			InvalidateRect( hWnd, NULL, FALSE );
			break;
		case WM_QUIT:
			break;
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		return 0;
	}
};
