#include "main.h"

miView * miView::instance = NULL;

void miView::initialize( miDocument * doc )
{
	slices.allocate( doc->slices.slices.size );
	array<ctSlice>::iterator its( doc->slices.slices );
	for ( array<ctSliceView>::iterator it( slices ); it; ++it, ++its ) {
		it->initialize( & its() );
	}
	miFrame * frame = miFrame::get();
#ifdef VIEW3D
	//3D表示用のグラフ作成
	graph = new sgGraph();
	camera = new sgCamera( *graph );
	camera->name = "default$camera";
	camera->position( vector3( 0, 300, 0 ) );
	camera->upper( vector3( 0, 0, 1 ) );
	perspective = new sgPerspective( *graph );
	perspective->name = "default$perspective";
	perspective->nearZ( 1.0 );
	perspective->farZ( 10000.0 );
	ortho = new sgOrtho( *graph );
	ortho->name = "default$ortho";
	ortho->nearZ( 1.0 );
	ortho->farZ( 10000.0 );

	double minimumPosition = doc->slicesize ? doc->slices.slices[0].position : 0;
	double maximumPosition = doc->slicesize ? doc->slices.slices[0].position : 0;
	for ( int z = 1; z < doc->slices.slices.size; ++z ) {
		minimumPosition = minimum( minimumPosition, doc->slices.slices[z].position );
		maximumPosition = maximum( maximumPosition, doc->slices.slices[z].position );
	}
	sgTransformMatrix * transform = new sgTransformMatrix( *graph );
	transform->name = "$world";
	sgDagNode * dagtransform = graph->root.insert( *transform );
	bool forward = true;
	if ( doc->slices.slices.size > 2 ) {
		forward = doc->slices.slices[0].position < doc->slices.slices[1].position;
	}
	sgPolygon * polygon = new sgPolygon( *graph );
	polygon->name = "slices$polygon";
	sgDagNode * dagpolygon = dagtransform->insert( *polygon );
	for ( int z = 0; z < doc->slices.slices.size * 2; ++z ) {
		int i = ( z < doc->slices.slices.size ? z : doc->slices.slices.size - 1 - ( z - doc->slices.slices.size ) );
		bool f = ( forward && ( i == z ) ) || ( ( ! forward ) && ( i != z ) );
		sgMaterial * material = new sgMaterial( *graph );//テクスチャが異なるため、複数必要
		material->name = string( z ) + "$material";
		material->ambient( vector3::white );
		material->diffuse( vector3::white );
		material->specular( vector3::black );
		material->emissive( vector3::white );
		material->transparency( decimal( 1.0 / doc->slices.slices.size ) );
		//スライス単位に四角形ポリゴンを作成する
		sgVertices * vertices = new sgVertices( *graph );
		sgIndices * indices = new sgIndices( *graph );
		vertices->name = string( z ) + "$vertices";
		indices->name = string( z ) + "$indices";
		const int numIndices = 3 * 4;
		array<int> & index = indices->indices();
		index.allocate( numIndices );
		int square[numIndices] = { 0, 1, 2, 2, 1, 3, 5, 4, 6, 5, 6, 7 };
		int squareReverse[numIndices] = { 1, 0, 2, 1, 2, 3, 4, 5, 6, 6, 5, 7 };
		memcpy( index.data, f ? square : squareReverse, numIndices * sizeof( int ) );
		array< vector3 > & positions = vertices->positions();
		array< vector3 > & normals = vertices->normals();
		array< vector2 > & textureCoordinates = vertices->textureCoordinates();
		const int numVertices = 8;
		positions.allocate( numVertices );
		normals.allocate( numVertices );
		textureCoordinates.allocate( numVertices );
		const double s = doc->slices.fieldOfViewMM / 2;
		const double posz = doc->slices.slices[i].position - ( maximumPosition + minimumPosition ) / 2;
		positions[0] = positions[4] = vector3( -s, -s, posz );
		positions[1] = positions[5] = vector3(  s, -s, posz );
		positions[2] = positions[6] = vector3( -s,  s, posz );
		positions[3] = positions[7] = vector3(  s,  s, posz );
		normals[0] = normals[1] = normals[2] = normals[3] = f ? vector3( 0, 0, 1 ) : vector3( 0, 0, -1 );
		normals[0] = normals[1] = normals[2] = normals[3] = vector3( 0, 0, 1 );
		normals[4] = normals[5] = normals[6] = normals[7] = - normals[0];
		textureCoordinates[0] = textureCoordinates[4] = vector2( 1, 0 );
		textureCoordinates[1] = textureCoordinates[5] = vector2( 0, 0 );
		textureCoordinates[2] = textureCoordinates[6] = vector2( 1, 1 );
		textureCoordinates[3] = textureCoordinates[7] = vector2( 0, 1 );
		positions.allocate( numVertices / 2 );
		normals.allocate( numVertices / 2 );
		textureCoordinates.allocate( numVertices / 2 );
		index.allocate( numIndices / 2 );
		sgTexture * texture = NULL;
		if ( i == z ) {
			texture = new sgTexture( *graph );
			texture->name = string( z ) + "$texture";
			texture->url( string( z ) );
			texture->texture.create( 512, 512 );
			slices[i].texture = texture;
		} else {
			texture = slices[i].texture;
		}
		new sgConnection( * vertices, *( sgVertices::attributes ), * polygon, sgPolygon::attributes->vertices );
		new sgConnection( * indices, *( sgIndices::attributes ), * polygon, sgPolygon::attributes->indices );
		new sgConnection( * material, *( sgMaterial::attributes ), * polygon, sgPolygon::attributes->materials );
		new sgConnection( * texture, *( sgTexture::attributes ), * material, sgMaterial::attributes->textures );
	}
	graph->alignment();
#endif
	//update();//全体更新
}
void miView::finalize( miDocument * doc )
{
#ifdef VIEW3D
	delete graph;
	lights.release();
	graph = NULL;
#endif
	for ( array<ctSliceView>::iterator it( slices ); it; ++it ) {
		it->finalize();
	}
	slices.release();
	patterns.finalize();
}
//tの区切りの中で最小のxを返す
int quantumMinimum( int x, int t )
{
	int r = x % t;
	if ( r == 0 ) return x;
	return x - r;
}
//tの区切りの中で最大のxを返す
int quantumMaximum( int x, int t )
{
	int r = x % t;
	if ( r == 0 ) return x;
	return x - r + t;
}

void miView::paint()
{
	miFrame * frame = miFrame::get(); if ( ! frame ) return;
	miDocument * doc = miDocument::get(); if ( ! doc ) return;
	RECT cr = imageWindow->client();
	int width = cr.right - cr.left;
	int height = cr.bottom - cr.top;
#ifdef VIEW3D
	if ( controlWindow->viewing3d && frame->render ) {
		frame->render->activate( true );
		frame->gr.initialize();
		if ( graph->update( true ) ) frame->gr.prepare( graph );
		frame->render->clearCache();
		ortho->sizeY( height );
		ortho->aspect( ( float ) ( width ) / ( float ) ( height ) );
		perspective->aspect( ( float ) ( width ) / ( float ) ( height ) );
		frame->gr.execute( perspective );
		//gr.execute( ortho );
		frame->render->setViewport( 0, 0, width, height );
		frame->render->startScene( 0xFF777777 );
		frame->render->alphaTest( true, 0, renderInterface::testFuncGreater );
		frame->render->depthTest( true, true, renderInterface::testFuncLessEqual );
		frame->render->cull( true, true );
		vector3 look = vector3::normalize3( camera->interest() - camera->position() );
		frame->render->blend( renderInterface::blendInverseSourceAlpha, renderInterface::blendSourceAlpha );
		//frame->render->blend( renderInterface::blendZero, renderInterface::blendOne );
		frame->gr.execute( camera );
		frame->gr.defaultLight();
		for ( list<sgNode*>::iterator itl( lights ); itl; ++itl ) {
			sgNode * light = itl();
			if ( light->thisDescriptor == sgAmbientLight::descriptor ) frame->gr.execute( static_cast<sgAmbientLight*>( light ) );
			else if ( light->thisDescriptor == sgPositionLight::descriptor ) frame->gr.execute( static_cast<sgPositionLight*>( light ) );
			else if ( light->thisDescriptor == sgDirectionLight::descriptor ) frame->gr.execute( static_cast<sgDirectionLight*>( light ) );
			else if ( light->thisDescriptor == sgSpotLight::descriptor ) frame->gr.execute( static_cast<sgSpotLight*>( light ) );
		}
		//frame->gr.execute( graph->root, look.z < 0 ? true : false );
		frame->gr.execute( graph->root );
		frame->render->swapBuffer();
		frame->render->activate( false );
	} else 
#endif
	{
		HDC hdc = imageWindow->getDC();
		windowSurfaceBitmap * surface = NULL;
		static windowSurfaceBitmap wk;
		if ( controlWindow->tab.index == attributeSheetIndex ) {
			wk = patterns.getImage();
		} else {
			wk = slices[doc->sliceindex].getImage();
		}
		surface = & wk;
		if ( surface && surface->object ) {
			RECT srcRect;//拡大された画像座標系での転送元矩形
			srcRect.left = imageWindow->scrollPosition.x;
			srcRect.top = imageWindow->scrollPosition.y;
			srcRect.right = minimum<LONG>( imageWindow->scrollPosition.x + width, surface->width * times );
			srcRect.bottom = minimum<LONG>( imageWindow->scrollPosition.y + height, surface->height * times );
			RECT srcRectAdjust;//量子化
			srcRectAdjust.left = quantumMinimum( srcRect.left, times ) / times;
			srcRectAdjust.top = quantumMinimum( srcRect.top, times ) / times;
			srcRectAdjust.right = quantumMaximum( srcRect.right, times ) / times;
			srcRectAdjust.bottom = quantumMaximum( srcRect.bottom, times ) / times;
			RECT dstRect;//画面座標系での転送先矩形
			dstRect.left = cr.left;
			dstRect.top = cr.top;
			dstRect.right = minimum<int>( cr.right, surface->width * times );
			dstRect.bottom = minimum<int>( cr.bottom, surface->height * times );
			RECT dstRectAdjust;
			dstRectAdjust.left = dstRect.left - ( srcRect.left - srcRectAdjust.left * times );
			dstRectAdjust.top = dstRect.top - ( srcRect.top - srcRectAdjust.top * times );
			dstRectAdjust.right = dstRect.right + ( srcRectAdjust.right * times - srcRect.right );
			dstRectAdjust.bottom = dstRect.bottom + ( srcRectAdjust.bottom * times - srcRect.bottom );
			StretchBlt( hdc, 
				dstRectAdjust.left, dstRectAdjust.top, dstRectAdjust.right - dstRectAdjust.left, dstRectAdjust.bottom - dstRectAdjust.top, 
				surface->hdc, 
				srcRectAdjust.left, srcRectAdjust.top, srcRectAdjust.right - srcRectAdjust.left, srcRectAdjust.bottom - srcRectAdjust.top, 
				SRCCOPY );
			if ( dstRectAdjust.right < cr.right ) {
				BitBlt( hdc, dstRectAdjust.right, cr.top, cr.right - dstRectAdjust.right, height, NULL, 0, 0, BLACKNESS );
			}
			if ( dstRectAdjust.bottom < cr.bottom ) {
				BitBlt( hdc, cr.left, dstRectAdjust.bottom, width, cr.bottom - dstRectAdjust.bottom, NULL, 0, 0, BLACKNESS );
			}
		} else {
			BitBlt( hdc, cr.left, cr.top, width, height, NULL, 0, 0, BLACKNESS );
		}
		imageWindow->release( hdc );
	}
}

void miView::mode( int16 windowSize, int16 windowLevel, int viewmode, int viewside )
{
	miDocument * doc = miDocument::get(); if ( ! doc ) return;
	this->windowSize = windowSize;
	this->windowLevel = windowLevel;
	this->viewmode = viewmode;
	this->viewside = viewside;
	for ( array<ctSliceView>::iterator it( slices ); it; ++it ) {
		it->mode( windowSize, windowLevel, viewmode, viewside, doc->regionindex );
	}
	patterns.mode( windowSize, windowLevel, viewmode );
}
void miView::notify()
{
	mode( windowSize, windowLevel, viewmode, viewside );
}

void miView::update()
{
	miDocument * doc = miDocument::get(); if ( ! doc ) return;
	statusBar->reset();
	statusBar->expand( slices.size + 1 );
	for ( array<ctSliceView>::iterator it( slices ); it; ++it ) {
		it->update();
		statusBar->progress();
	}
	patterns.update();
	statusBar->progress();
	statusBar->reset();
	statusUpdate();
	imageWindow->invalidate();
}
void miView::statusUpdate()
{
	miDocument * doc = miDocument::get();
	miView * view = miView::get();
	string info;
	if ( doc && view ) {
		char * modeString[] = { "CT画像＋形状", "CT画像", "形状" };
		char * sideString[] = { "左右両方", "左側", "右側" };
		char * mode = "", * side = "";
		if ( between<int>( 0, view->viewmode, 2 ) ) mode = modeString[view->viewmode];
		if ( between<int>( 0, view->viewside, 2 ) ) side = sideString[view->viewside];
		info.print( "スライス[%d]：領域[%s]：モード[%s]：左右[%s]：CT値[%d]", 
			doc->sliceindex, doc->regionName().chars(), mode, side, 
			view->currentCTValue );
	}
	statusBar->set( info );
}
void miView::patternUpdate()
{
	miDocument * doc = miDocument::get();
	if ( doc ) {
		patterns.initialize( doc->pattern(), & doc->regionColor() );
		patterns.mode( windowSize, windowLevel, viewmode );
		patterns.update();
		imageWindow->invalidate();
	}
}

void miView::changeTimes( int t )
{
	miView * view = miView::get();
	if ( ! view ) return;
	miDocument * doc = miDocument::get();
	if ( ! doc ) return;
	view->times = t;
	if ( controlWindow->tab.index == attributeSheetIndex ) {
		image & img = view->patterns.getImage();
		imageWindow->scrollDocumentSize.x = t * img.width;
		imageWindow->scrollDocumentSize.y = t * img.height;
	} else {
		ctSlice & s = doc->slice();
		imageWindow->scrollDocumentSize.x = t * s.imageSize.x;
		imageWindow->scrollDocumentSize.y = t * s.imageSize.y;
	}
	imageWindow->scrollPosition.x = 0;
	imageWindow->scrollPosition.y = 0;
	imageWindow->updateScrollRange();
	imageWindow->invalidate();
}
