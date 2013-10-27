#include "main.h"

miFrame * miFrame::instance = NULL;

void miFrame::initialize()
{
//	render = renderInterface::createDirectX8Render();
	render = renderInterface::createOpenGLRender();
	render->rightHand = false;
	render->cw = false;
	render->culling = false;
	if ( ! render->initialize( imageWindow->hwnd, true, 512, 512 ) ) {
		delete render;
		render = NULL;
	}
	gr.render = render;
}
void miFrame::finalize()
{
	delete render; render = NULL;
}

void miFrame::paint()
{
	if ( singleThread.check() ) {
		imageWindow->validate();
		miView * view = miView::get();
		miDocument * doc = miDocument::get();
		RECT cr = imageWindow->client();
		int width = cr.right - cr.left;
		int height = cr.bottom - cr.top;
		if ( view ) {
			view->paint();
		} else {
			HDC hdc = imageWindow->getDC();
			BitBlt( hdc, cr.left, cr.top, width, height, NULL, 0, 0, BLACKNESS );
			imageWindow->release( hdc );
		}
		singleThread.signal();
	}
}
