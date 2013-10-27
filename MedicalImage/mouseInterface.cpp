#include "main.h"
#include "operation.h"
#include "popup.h"
#include "mouse.h"

class miCommandMouseBrush : public miCommand
{
public:
	//コマンドを実行する。内部でmemoryを呼んで保存することが必須。
	ctRegion rgn;
	int mode;
	miCommandMouseBrush( const ctRegion & rgn, int mode )
	{
		this->rgn = rgn;
		this->mode = mode;
		m.initialize();
		m.change( false );//現在のスライスの現在のターゲットのみ変更
	}
	virtual bool execute()
	{
		miDocument * doc = miDocument::get();
		if ( ! doc ) return false;
		switch ( mode ) {
			case regionOperationModeAppend: doc->slice().appendRegion( doc->regionindex, rgn ); break;
			case regionOperationModeDelete: doc->slice().removeRegion( doc->regionindex, rgn ); break;
			case regionOperationModeChange: doc->slice().changeRegion( doc->regionindex, rgn ); break;
		}
		doc->change();
		return true;
	}
};
class miCommandRegionChange : public miCommand
{
public:
	int x, y, z, r;
	miCommandRegionChange( int _x, int _y, int _z, int _r ) : x( _x ), y( _y ), z( _z ), r( _r )
	{
		m.initialize();
		m.change( true );
	}
	virtual bool execute()
	{
		bool done = false;
		miDocument * doc = miDocument::get();
		if ( doc ) {
			done = true;
			doc->deleteContinuousRegion( x, y, z, r );
		}
		return done;
	}
};

void mouseInterface::rightPush( HWND hWnd, 
	bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
	int mouseX, int mouseY, int mouseOldX, int mouseOldY )
{
	miDocument * doc = miDocument::get();
	if ( ! doc ) return;
	if ( controlWindow->tab.index != regionSheetIndex ) return;//領域シートでない場合は動作しない
	if ( controlWindow->extracting || controlWindow->modifing ) return;//別の操作中は動作しない
	if ( brushinterface.started() || freehandinterface.started() ) return;//すでに動作している場合は無視
	miView * view = miView::get();
	freehandinterface.documentTimes = view ? view->times : 1;
	freehandinterface.windowPos = point2<int>( imageWindow->scrollPosition.x, imageWindow->scrollPosition.y );
	freehandinterface.documentSize = point2<int>( doc->slice().imageSize.x, doc->slice().imageSize.y );
	freehandinterface.start( imageWindow->hwnd, point2<int>( mouseX, mouseY ) );
}
void mouseInterface::leftPush( HWND hWnd, 
	bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
	int mouseX, int mouseY, int mouseOldX, int mouseOldY )
{
	miDocument * doc = miDocument::get();
	if ( ! doc ) return;
	if ( controlWindow->tab.index != regionSheetIndex ) return;//領域シートでない場合は動作しない
	if ( controlWindow->extracting || controlWindow->modifing ) return;//別の操作中は動作しない
	if ( brushinterface.started() || freehandinterface.started() ) return;//すでに動作している場合は無視
	if ( ! controlWindow->brushEnable.get() ) return;//ブラシが有効でない場合は動作しない
	miView * view = miView::get();
	brushinterface.documentTimes = view ? view->times : 1;
	brushinterface.windowPos = point2<int>( imageWindow->scrollPosition.x, imageWindow->scrollPosition.y );
	brushinterface.documentSize = point2<int>( doc->slice().imageSize.x, doc->slice().imageSize.y );
	brushinterface.start( imageWindow->hwnd, point2<int>( mouseX, mouseY ), controlWindow->brushes.value );
}
void mouseInterface::leftPop( HWND hWnd, 
	bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
	int mouseX, int mouseY, int mouseOldX, int mouseOldY )
{
	if ( brushinterface.started() && brushinterface.finish() ) {
		miCommands::get().regist( new miCommandMouseBrush( brushinterface.rgn, controlWindow->regionOperationMode.index ) );
		imageWindow->invalidate();
	}
}
void mouseInterface::rightPop( HWND hWnd, 
	bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
	int mouseX, int mouseY, int mouseOldX, int mouseOldY )
{
	miDocument * doc = miDocument::get();
	if ( ! doc ) return;
	miView * view = miView::get();
	if ( ! view ) return;
	if ( controlWindow->tab.index != regionSheetIndex ) return;
	if ( controlWindow->extracting || controlWindow->modifing ) return;

	ctSlice & slice = doc->slice();
	bool existRegion = false;

	//マウス座標をスクリーン座標にする
	POINT sp = { LONG( mouseX ), LONG( mouseY ) };
	ClientToScreen( hWnd, & sp );
	//画像座標系を計算
	int docX = ( mouseX + imageWindow->scrollPosition.x ) / view->times;
	int docY = ( mouseY + imageWindow->scrollPosition.y ) / view->times;

	//すでに領域指定をしていた場合
	if ( freehandinterface.started() ) {
		if ( freehandinterface.finish() ) {//有効領域な場合
			if ( freehandinterface.rgn.area() <= 1 ) {
				//領域指定無しのメニュー
                if ( clickMenu )
                {
				    onTrackMenu.wait();
				    popupmenuitem * r = clickMenu->call( sp.x, sp.y );
				    onTrackMenu.signal();
				    if ( r == clickMenu->deleteRegion ) {//指定領域削除
					    ctRegion rgn;
					    slice.regions[doc->regionindex].select( rgn, docX, docY );
					    miCommands::get().regist( new miCommandMouseBrush( rgn, regionOperationModeDelete ) );
				    } else if ( r == clickMenu->deleteContinuousRegion ) {//連続領域削除
					    miCommands::get().regist( new miCommandRegionChange( docX, docY, doc->sliceindex, doc->regionindex ) );
				    } else if ( r == clickMenu->defineCenter ) {
					    doc->slices.numberCenter = doc->sliceindex;
					    doc->slices.centerposition.x = docX;
					    doc->slices.centerposition.y = docY;
					    doc->change();
				    }
                }
			} else {
				//領域ありのメニュー
                if ( regionMenu )
                {
				    onTrackMenu.wait();
				    popupmenuitem * r = regionMenu->call( sp.x, sp.y );
				    onTrackMenu.signal();
				    if ( r == regionMenu->deleteRegion ) {//指定領域削除
					    miCommands::get().regist( new miCommandMouseBrush( freehandinterface.rgn, regionOperationModeDelete ) );
				    } else if ( r == regionMenu->appendRegion ) {//指定領域追加
					    miCommands::get().regist( new miCommandMouseBrush( freehandinterface.rgn, regionOperationModeAppend ) );
				    } else if ( r == regionMenu->samplingRegion ) {
					    if ( doc->regionindex != regionHuman && doc->regionindex != regionLung ) {
						    retainer<miPattern> tp( new miPattern );
						    if ( tp->initialize( slice, freehandinterface.rgn & slice.regions[regionLung] ) ) {
							    doc->patterns().push_back( tp );
							    controlWindow->updateClusters();
						    }
					    }
				    } else if ( r == regionMenu->averageRegion ) {
					    doc->displayAverageRegion( freehandinterface.rgn );
				    } else if ( r == regionMenu->deleteMinorRegion ) {
					    doc->deleteMinorRegion( freehandinterface.rgn, false );
				    } else if ( r == regionMenu->mergeMinorRegion ) {
					    doc->deleteMinorRegion( freehandinterface.rgn, true );
				    }
                }
			}
		}
		freehandinterface.done();
		imageWindow->invalidate();
	}
}
void mouseInterface::move( HWND hWnd, 
	bool mouseLPushed, bool mouseMPushed, bool mouseRPushed, 
	int mouseX, int mouseY, int mouseOldX, int mouseOldY )
{
	miDocument * doc = miDocument::get();
	miView * view = miView::get();
	if ( ! doc ) return;
	if ( ! view ) return;
	//３Ｄでの移動
#ifdef VIEW3D
	if ( controlWindow->viewing3d && ( mouseX != mouseOldX || mouseY != mouseOldY ) ) {
		int width = 512, height = 512;
		if ( mouseLPushed && ! mouseMPushed && ! mouseRPushed ) {
			view->camera->follow( ( float( mouseX ) - float( mouseOldX ) ) / width / 2, ( float( mouseY ) - float( mouseOldY ) ) / height / 2, 0 );
		} else if ( ! mouseLPushed && mouseMPushed && ! mouseRPushed ) {
			view->camera->move( ( float( mouseX ) - float( mouseOldX ) ) / width / 2, - ( float( mouseY ) - float( mouseOldY ) ) / height / 2, 0 );
		} else if ( mouseLPushed && mouseMPushed && ! mouseRPushed ) {
			view->camera->zoom( ( float( mouseX ) - float( mouseOldX ) ) / width );
		} else if ( ! mouseLPushed && ! mouseMPushed && mouseRPushed ) {
			view->camera->move( -( float( mouseX ) - float( mouseOldX ) ) / width / 2, 0, -( float( mouseY ) - float( mouseOldY ) ) / height / 2 );
		} else if ( mouseLPushed && ! mouseMPushed && mouseRPushed ) {
			view->camera->pan( ( float( mouseX ) - float( mouseOldX ) ) / width / 2, ( float( mouseY ) - float( mouseOldY ) ) / height / 2 );
		}
		if ( mouseLPushed || mouseMPushed || mouseRPushed ) {
			vector3 upper = vector3::normalize3( view->camera->upper() );
			if ( upper.z ) {
				vector3 recomandupper( 0, 0, upper.z > 0 ? 1 : -1 );
				vector3 direction = vector3::normalize3( view->camera->interest() - view->camera->position() );
				vector3 other = vector3::normalize3( vector3::cross( recomandupper, direction ) );
				view->camera->upper( vector3::normalize3( vector3::cross( direction, other ) ) );
			}
			view->camera->update();
			imageWindow->invalidate();
		}
        return;
	}
#endif
	//情報表示
	RECT cr = imageWindow->client();
	if ( between<LONG>( 0, mouseX, cr.right - 1 ) && 
		between<LONG>( 0, mouseY, cr.bottom - 1 ) ) {
		//画像座標系を計算
		int docX = ( mouseX + imageWindow->scrollPosition.x ) / view->times;
		int docY = ( mouseY + imageWindow->scrollPosition.y ) / view->times;
		if ( controlWindow->tab.index == viewSheetIndex && ! controlWindow->viewing3d ||
			controlWindow->tab.index == regionSheetIndex ) {
			ctImage & ctimg = doc->slice().getImage();
			if ( ctimg.in( docX, docY ) ) {
				view->currentCTValue = ctimg.get( docX, docY ).y;
			} else {
				view->currentCTValue = -2048;
			}
		} else if ( controlWindow->tab.index == attributeSheetIndex ) {
			if ( doc->pattern() ) {
				view->currentCTValue = doc->pattern()->ctimg.get( docX, docY ).y;
			}
		}
		view->statusUpdate();
	}
	//領域編集
	if ( controlWindow->tab.index == regionSheetIndex &&
		! controlWindow->extracting && ! controlWindow->modifing ) {
		//ポインタをトラッキングして領域を作る
		if ( mouseRPushed && freehandinterface.started() ) {
			freehandinterface.track( point2<int>( mouseOldX, mouseOldY ), point2<int>( mouseX, mouseY ) );
		}
		//ブラシで塗りつぶす
		if ( mouseLPushed && brushinterface.started() ) {
			brushinterface.track( point2<int>( mouseX, mouseY ) );
		}
	}
}
