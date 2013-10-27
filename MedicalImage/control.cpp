#include "main.h"
#include "patternAttribute.h"
#include "ctAttribute.h"
#include "colorpicker.h"

colorPicker * colorPicker::_this = NULL;

//常時更新時に現在の状態をundoして、実行するための、初期のundo用のfake用コマンド
class miCommandFake : public miCommand
{
public:
	miCommandFake()
	{
		m.initialize();
		m.change( false );
	}
	virtual bool execute()
	{
		return true;
	}
};
//閾値処理コマンド
class miCommandSliceSearch : public miCommand
{
public:
	bool allSlices;
	int16 lower, upper;
	int mode;
	miCommandSliceSearch( bool allSlices, int16 lower, int16 upper, int mode )
	{
		this->allSlices = allSlices;
		this->lower = lower;
		this->upper = upper;
		this->mode = mode;
		m.initialize();
		m.change( allSlices );
	}
	virtual bool execute()
	{
		miDocument * doc = miDocument::get();
		if ( doc ) {
			if ( allSlices ) {
				doc->slices.search( doc->regionindex, lower, upper, mode );
			} else {
				doc->slice().search( doc->regionindex, lower, upper, mode );
			}
		}
		doc->change();
		return true;
	}
};
//形状修正コマンド
class miCommandSliceModify : public miCommand
{
public:
	bool allSlices;
	int closing, opening;
	miCommandSliceModify( bool allSlices, int closing, int opening )
	{
		this->allSlices = allSlices;
		this->closing = closing;
		this->opening = opening;
		m.initialize();
		m.change( allSlices );
	}
	virtual bool execute()
	{
		miDocument * doc = miDocument::get();
		if ( doc ) {
			if ( allSlices ) {
				doc->slices.modify( doc->regionindex, closing, opening );
			} else {
				doc->slice().modify( doc->regionindex, closing, opening );
			}
		}
		doc->change();
		return true;
	}
};


BOOL CALLBACK controlInterface::dlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static controlInterface * _this = NULL;
	if ( _this ) fc::window::processAll( hDlg, uMsg, wParam, lParam );
	if ( ! _this || hDlg == _this->hwnd ) {
		switch( uMsg ) {
		case WM_INITDIALOG:
			{
				if ( _this ) break;
				miDocument * doc = miDocument::get();
				_this = static_cast<controlInterface*>( reinterpret_cast<fc::dialog*>( lParam ) );
				_this->hwnd = hDlg;
				_this->tab.initialize( hDlg, IDC_TAB );
				_this->tab.changed = controlInterface::sheetChange;
				_this->view.initialize( IDD_SHEET_VIEW, "表示", controlInterface::dlgProc );
				_this->regions.initialize( IDD_SHEET_REGION, "領域", controlInterface::dlgProc );
				_this->attribute.initialize( IDD_SHEET_ATTRIBUTE, "属性", controlInterface::dlgProc );

				_this->viewslice.value = 0;
				_this->viewslice.initialize( _this->view.hwnd, IDC_EDIT_SLICE, sliceSpinID, IDC_SLIDER_SLICE, 0, doc ? doc->slicesize - 1 : -1 );
				_this->viewsize.initialize( _this->view.hwnd, IDC_EDIT_SIZE, sizeSpinID, IDC_SLIDER_SIZE, 1, 4096 );
				_this->viewlevel.initialize( _this->view.hwnd, IDC_EDIT_LEVEL, levelSpinID, IDC_SLIDER_LEVEL, -2048, 2048 );
				_this->viewmode.initialize( _this->view.hwnd, IDC_COMBO_VIEWMODE );
				_this->viewside.initialize( _this->view.hwnd, IDC_COMBO_SIDE );
				_this->viewmode.push_back( "CT画像＋形状" );
				_this->viewmode.push_back( "CT画像" );
				_this->viewmode.push_back( "形状" );
				_this->viewside.push_back( "左右両方" );
				_this->viewside.push_back( "左側" );
				_this->viewside.push_back( "右側" );
				_this->viewmode.changed = controlInterface::changeMode;
				_this->viewside.changed = controlInterface::changeMode;
				_this->viewupdate.initialize( _this->view.hwnd, IDC_UPDATE );
				_this->view3d.initialize( _this->view.hwnd, IDC_CHECK_3D );
				_this->view3d.clicked = controlInterface::clicked;
				_this->editctattribute.initialize( _this->view.hwnd, IDC_EDIT_CTATTRIBUTE );
				_this->changectattribute.initialize( _this->view.hwnd, IDC_CHANGE_CTATTRIBUTE );
				_this->changectattribute.clicked = controlInterface::clicked;

				_this->target.initialize( _this->regions.hwnd, IDC_COMBO_TARGET );
				_this->target.push_back( "human" );
				_this->target.push_back( "lung" );
				_this->target.push_back( "normal" );
				_this->target.select( regionLung );
				
				_this->target.changed = controlInterface::changeMode;
				_this->targeted = regionLung;
				string s;
				int extractLimitDown = -2048, extractLimitUp = -200;
				if ( regGet( "extractLimitDown", s ) ) extractLimitDown = s.toi();
				if ( regGet( "extractLimitUp", s ) ) extractLimitUp = s.toi();
				_this->extractLimit.down( extractLimitDown );
				_this->extractLimit.up( extractLimitUp );
				_this->extractLimit.initialize( _this->regions.hwnd, IDC_EDIT_EXTRACT_UPLIMIT, extractUpLimitSpinID, IDC_SLIDER_EXTRACT_UPLIMIT, IDC_EDIT_EXTRACT_DOWNLIMIT, extractDownLimitSpinID, IDC_SLIDER_EXTRACT_DOWNLIMIT, -2048, 2048 );
				_this->extract.initialize( _this->regions.hwnd, IDC_BUTTON_EXTRACT );
				_this->extract.clicked = controlInterface::clicked;
				_this->extractStop.initialize( _this->regions.hwnd, IDC_BUTTON_EXTRACT_STOP );
				_this->extractStop.clicked = controlInterface::clicked;

				int modifyClosing = 5, modifyOpening = 0;
				if ( regGet( "modifyClosing", s ) ) modifyClosing = s.toi();
				if ( regGet( "modifyOpening", s ) ) modifyOpening = s.toi();
				_this->modifyClosing( modifyClosing );
				_this->modifyClosing.initialize( _this->regions.hwnd, IDC_EDIT_MODIFY_CLOSING, modifyClosingSpinID, 0, 256 );
				_this->modifyOpening( modifyOpening );
				_this->modifyOpening.initialize( _this->regions.hwnd, IDC_EDIT_MODIFY_OPENING, modifyOpeningSpinID, 0, 256 );
				_this->modify.initialize( _this->regions.hwnd, IDC_BUTTON_MODIFY );
				_this->modify.clicked = controlInterface::clicked;
				_this->modifyStop.initialize( _this->regions.hwnd, IDC_BUTTON_MODIFY_STOP );
				_this->modifyStop.clicked = controlInterface::clicked;
				_this->brushes.value = 0;
				_this->brushes.initialize( _this->regions.hwnd, IDC_EDIT_BRUSH, brushSpinID, IDC_SLIDER_BRUSH, 0, 15 );
				_this->brushEnable.initialize( _this->regions.hwnd, IDC_BRUSH_ENABLE );

				_this->regionPreview.initialize( _this->regions.hwnd, IDC_PREVIEW );
				_this->regionAll.initialize( _this->regions.hwnd, IDC_ALL );
				_this->regionOperationMode.initialize( _this->regions.hwnd, IDC_COMBO_OPERATION_MODE );
				_this->regionOperationMode.push_back( "追加" );
				_this->regionOperationMode.push_back( "削除" );
				_this->regionOperationMode.push_back( "変更" );
				_this->regionOperationMode.select( regionOperationModeAppend );

				_this->cluster.initialize( _this->attribute.hwnd, IDC_TREE_REGION );
				_this->clusters.allocate( regionNormal + 1 );
				_this->clusters[regionHuman] = _this->cluster.push( fc::treeview::item( "human" ) );
				_this->clusters[regionLung] = _this->cluster.push( fc::treeview::item( "lung" ), _this->clusters[regionHuman] );
				_this->clusters[regionNormal] = _this->cluster.push( fc::treeview::item( "normal" ), _this->clusters[regionLung] );
				_this->cluster.selected = controlInterface::clicked;
				_this->cluster.select( _this->clusters[regionLung] );
				_this->clusterColor.initialize( _this->attribute.hwnd, IDC_REGION_COLOR );
				_this->clusterAlpha.initialize( _this->attribute.hwnd, IDC_REGION_ALPHA );
				_this->clusterColor.paint = controlInterface::painter;
				_this->clusterAlpha.paint = controlInterface::painter;
				_this->clusterColor.clicked = controlInterface::clicked;
				_this->clusterAlpha.clicked = controlInterface::clicked;
				_this->clusterArea.initialize( _this->attribute.hwnd, IDC_EDIT_AREA );
				_this->clusterVolume.initialize( _this->attribute.hwnd, IDC_EDIT_VOLUME );
				_this->clusterAverage.initialize( _this->attribute.hwnd, IDC_EDIT_AVERAGE );
				_this->clusterStandardDeviation.initialize( _this->attribute.hwnd, IDC_EDIT_STANDARD_DEVIATION );
				_this->clusterArea.set( 0 );
				_this->clusterVolume.set( 0 );
				_this->patternDelete.initialize( _this->attribute.hwnd, IDC_BUTTON_DELETE_PATTERN );
				_this->patternDelete.clicked = controlInterface::clicked;
				_this->patternAllDelete.initialize( _this->attribute.hwnd, IDC_BUTTON_DELETE_ALL_PATTERN );
				_this->patternAllDelete.clicked = controlInterface::clicked;
				_this->patternChange.initialize( _this->attribute.hwnd, IDC_BUTTON_CHANGE_PATTERN );
				_this->patternChange.clicked = controlInterface::clicked;
				_this->patternComment.initialize( _this->attribute.hwnd, IDC_EDIT_COMMENT );
				

//				_this->progress.initialize( hDlg, IDC_PROGRESS );
//				_this->information.initialize( hDlg, IDC_EDIT_INFORMATION );

				_this->tab.reflesh();

				_this->validateControl();

				ShowWindow( hDlg, SW_SHOW );
				BringWindowToTop( hDlg );
			}
			return 1;
		case WM_DESTROY:
			{
				//このウィンドウは最後まで閉じないので、終了処理は省略（解放はfc側で行う）
				_this = NULL;
			}
			return 1;
		case WM_COMMAND:
			{
 				switch(LOWORD(wParam))
				{
				case IDOK:
				case IDCANCEL:
					break;
				}
			}
			break;
		}
	}
	if ( singleThread.check() ) {
		miDocument * doc = miDocument::get();
		if ( _this && doc ) {
			switch ( _this->tab.index ) {
			case viewSheetIndex: //表示タブ
				//スライス変更
				if ( _this->viewslice.process() ) {
					doc->change();
					controlWindow->clusterArea.set( doc->slice().clusterArea[doc->regionindex] );
					controlWindow->clusterVolume.set( doc->slices.clusterVolume[doc->regionindex] );
				}
				//ウィンドウ値随時更新 三次元表示中は更新せず
				if ( _this->viewupdate.get() && ! controlWindow->viewing3d ) {
					bool changedsize = _this->viewsize.process();
					bool changedlevel = _this->viewlevel.process();
					if ( changedsize || changedlevel ) {
						doc->change();
					}
				}
				break;
			case regionSheetIndex: //肺野タブ
				_this->extractLimit.process();
				_this->modifyClosing.process();
				_this->modifyOpening.process();
				_this->brushes.process();
				//ここで随時修正が行われる。
				if ( _this->extracting ) {
					if ( _this->regionPreview.get() ) {
						if ( _this->lastExtractDown != _this->extractLimit.down() ||
							_this->lastExtractUp != _this->extractLimit.up() ) {
							_this->lastExtractDown = _this->extractLimit.down();
							_this->lastExtractUp = _this->extractLimit.up();
							miCommands::get().undo();//変更前に戻す
							miCommands::get().regist( new miCommandSliceSearch( false, _this->lastExtractDown, _this->lastExtractUp, _this->regionOperationMode.index ) );
							doc->change();
						}
					}
				} else if ( _this->modifing ) {
					if ( _this->regionPreview.get() ) {
						if ( _this->lastModifyClosing != _this->modifyClosing() ||
							_this->lastModifyOpening != _this->modifyOpening() ) {
							_this->lastModifyClosing = _this->modifyClosing();
							_this->lastModifyOpening = _this->modifyOpening();
							miCommands::get().undo();//変更前に戻す
							miCommands::get().regist( new miCommandSliceModify( false, _this->lastModifyClosing, _this->lastModifyOpening ) );
							doc->change();
						}
					}
				}
				break;
			}
		}
		singleThread.signal();
	}
	return 0;
}
void controlInterface::doExtract( bool execute )
{
	miDocument * doc = miDocument::get();
	if ( execute ) {
		if ( extracting ) {
			//処理実行
			extracting = false;
			//抽出
			lastExtractDown = extractLimit.down();
			lastExtractUp = extractLimit.up();
			miCommands::get().undo();//変更前に戻す
			miCommands::get().regist( new miCommandSliceSearch( regionAll.get() ? true : false, lastExtractDown, lastExtractUp, controlWindow->regionOperationMode.index ) );
			regSet( "extractLimitDown", string( lastExtractDown ) );
			regSet( "extractLimitUp", string( lastExtractUp ) );
		} else {
			//処理開始
			extracting = true;
			lastExtractDown = lastExtractUp = 0x7fff;
			miCommands::get().regist( new miCommandFake() );
		}
		doc->change();
		controlWindow->validateControl();
	} else {
		//キャンセル
		if ( extracting ) {
			extracting = false;
			miCommands::get().undo();
			miCommands::get().cancelRedo();
			doc->change();
			validateControl();
		}
	}
}
void controlInterface::doModify( bool execute )
{
	miDocument * doc = miDocument::get();
	if ( execute ) {
		if ( modifing ) {
			modifing = false;
			//変更
			lastModifyClosing = modifyClosing();
			lastModifyOpening = modifyOpening();
			miCommands::get().undo();//変更前に戻す
			miCommands::get().regist( new miCommandSliceModify( regionAll.get() ? true : false, lastModifyClosing, lastModifyOpening ) );
			regSet( "modifyClosing", string( lastModifyClosing ) );
			regSet( "modifyOpening", string( lastModifyOpening ) );
		} else {
			modifing = true;
			lastModifyClosing = lastModifyOpening = 0;
			miCommands::get().regist( new miCommandFake() );
		}
		doc->change();
		controlWindow->validateControl();
	} else {
		//キャンセル
		if ( modifing ) {
			modifing = false;
			miCommands::get().undo();
			miCommands::get().cancelRedo();
			doc->change();
			validateControl();
		}
	}
}

void controlInterface::changeViewMode( int src )
{
	miDocument * doc = miDocument::get();
	if ( doc && controlWindow && controlWindow->viewmode ) {
		controlWindow->viewmode.select( src );
		doc->change();
	}
}

void controlInterface::changeViewSide( int src )
{
	miDocument * doc = miDocument::get();
	if ( doc && controlWindow && controlWindow->viewside ) {
		controlWindow->viewside.select( src );
		doc->change();
		imageWindow->invalidate();
	}
}

void controlInterface::changeMode( fc::window * w )
{
	miDocument * doc = miDocument::get();
	if ( doc && controlWindow && controlWindow->viewmode && controlWindow->viewside ) {
		//表示モード変更
		if ( w == & controlWindow->viewmode || w == & controlWindow->viewside ) {
			doc->change();
		} else if ( w == & controlWindow->target ) {
			//ここでターゲットが代わった場合には修正作業はキャンセル
			controlWindow->doExtract( false );
			controlWindow->doModify( false );
			controlWindow->targeted = controlWindow->target.index;
			controlWindow->clusterColor.invalidate();
			controlWindow->clusterAlpha.invalidate();
			miPattern * p = doc->pattern();
			controlWindow->patternComment.set( p ? p->comment : "" );
			if ( p ) {
				fc::treeview::node * n = controlWindow->clusters[controlWindow->targeted]->head;
				for ( int i = 0; i < doc->patternindex && n; ++i ) {
					n = n->next;
				}
				if ( n ) {
					if ( n != controlWindow->cluster.current ) {
						controlWindow->cluster.select( n );
					}
				} else {
					if ( controlWindow->cluster.current != controlWindow->clusters[controlWindow->targeted] ) {
						controlWindow->cluster.select( controlWindow->clusters[controlWindow->targeted] );
					}
				}
			} else {
				if ( controlWindow->cluster.current != controlWindow->clusters[controlWindow->targeted] ) {
					controlWindow->cluster.select( controlWindow->clusters[controlWindow->targeted] );
				}
			}
			imageWindow->invalidate();
		}
		controlWindow->clusterArea.set( doc->slice().clusterArea[doc->regionindex] );
		controlWindow->clusterVolume.set( doc->slices.clusterVolume[doc->regionindex] );
		controlWindow->patternComment.enable( doc->pattern() ? true : false );
		controlWindow->patternComment.set( doc->pattern() ? doc->pattern()->comment : "" );
		controlWindow->patternDelete.enable( doc->pattern() ? true : false );
		controlWindow->patternAllDelete.enable( doc->pattern() ? true : false );
		controlWindow->patternChange.enable( doc->pattern() ? true : false );
	}
}
//シートが変更されたときもキャンセル
void controlInterface::sheetChange( fc::window * )
{
	miDocument * doc = miDocument::get();
	if ( controlWindow->extracting ) {
		controlWindow->doExtract( false );
	} else if ( controlWindow->modifing ) {
		controlWindow->doModify( false );
	}
	if ( controlWindow->viewing3d ) {
		controlWindow->viewing3d = false;
		controlWindow->validateControl();
	}
	if ( doc ) doc->change();
	imageWindow->invalidate();
	controlWindow->validateControl();
}

void controlInterface::clicked( fc::window * w )
{
	miDocument * doc = miDocument::get();
	miView * view = miView::get();
	if ( w == & controlWindow->view3d ) {
		controlWindow->viewing3d = controlWindow->view3d.get() ? true : false;
		controlWindow->validateControl();
		if ( view && controlWindow->viewing3d ) {
			view->mode( view->windowSize, view->windowLevel, controlWindow->viewmode.index, controlWindow->viewside.index );
			view->update();
			//元画像の分をメモリから削除
			ctSlice::inactivate();
		}
		imageWindow->invalidate();
		//変更作業を行うか中止する
	} else if ( w == & controlWindow->extract ) {
		controlWindow->doExtract( true );
	} else if ( w == & controlWindow->extractStop ) {
		controlWindow->doExtract( false );
	} else if ( w == & controlWindow->modify ) {
		controlWindow->doModify( true );
	} else if ( w == & controlWindow->modifyStop ) {
		controlWindow->doModify( false );
	} else if ( w == & controlWindow->clusterColor || w == & controlWindow->clusterAlpha ) {
		if ( doc ) {
			colorPicker cp( w->hwnd );
			doc->regionColor( doc->regionindex ) = cp.call( doc->regionColor( doc->regionindex ) );
			doc->change( true );
			controlWindow->clusterColor.invalidate();
			controlWindow->clusterAlpha.invalidate();
		}
	} else if ( w == & controlWindow->cluster ) {
		if ( doc ) {
			for ( int i = 0; i < controlWindow->clusters.size; ++i ) {
				if ( controlWindow->clusters[i] == controlWindow->cluster.current ) {
					if ( doc->regionindex != i || doc->patternindex != 0 ) {
						doc->patternindex = 0;
						controlWindow->changeRegion( i );
					}
					break;
				} else if ( between<int>( regionNormal, i, doc->numberRegion() - 1 ) &&
					controlWindow->clusters[i] == controlWindow->cluster.current->parent ) {
					int newpatternindex = controlWindow->cluster.current->data.name.toi();//名前の前半にはインデックスがついている
					if ( doc->patternindex != newpatternindex || doc->regionindex != i ) {
						doc->patternindex = newpatternindex;
						controlWindow->changeRegion( i );
					}
					break;
				}
			}
		}
	} else if ( w == & controlWindow->patternChange ) {
		if ( doc && doc->pattern() ) {
			miPattern * p = doc->pattern();
			patternAttribute pa( mainWindow->hwnd );
			if ( pa.call( doc->regionName(), p->postfix, p->comment, p->topbottom, p->frontrear ) ) {
				p->postfix = "";
				//接尾辞が重複しないようにチェック
				if ( pa.postfix.length() ) {
					pa.postfix.tolower();
					if ( pa.postfix.isinteger() ) {
						pa.postfix = "";
					} else {
						for ( array< retainer<miPattern> >::iterator itc( doc->patterns() ); itc; ++itc ) {
							if ( ! itc()().postfix.length() ) continue;
							if ( stricmp( itc()().postfix, pa.postfix ) == 0 ) {
								pa.postfix = "";
								break;
							}
						}
					}
				}

				p->postfix = pa.postfix;
				p->comment = pa.comment;
				p->topbottom = pa.topbottom;
				p->frontrear = pa.frontrear;
				controlWindow->updateClusters();
				doc->change();
			}
		}
	} else if ( w == & controlWindow->patternDelete ) {
		if ( doc && doc->pattern() ) {
			array< retainer<miPattern> > & pattern = doc->patterns();
			array< retainer<miPattern> >::iterator it( pattern, doc->patternindex );
			pattern.pop( it() );
			controlWindow->updateClusters();
			doc->change();
		}
	} else if ( w == & controlWindow->patternAllDelete ) {
		miDocument::clearPattern();
	} else if ( w == & controlWindow->changectattribute ) {
		if ( doc ) {
			ctAttribute ca( mainWindow->hwnd );
			if ( ca.call( doc->slices.numberTop, doc->slices.numberBottom, doc->slices.bottomFirst, doc->slices.prone ) ) {
				doc->slices.numberTop = ca.numberTop;
				doc->slices.numberBottom = ca.numberBottom;
				doc->slices.bottomFirst = ca.bottomFirst;
				doc->slices.prone = ca.prone;
				doc->change();
			}
		}
	}
}

void controlInterface::validateControl()
{
	miDocument * doc = miDocument::get();
	bool existDocument = doc ? true : false;
	viewmode.enable( existDocument && ! viewing3d );
	viewside.enable( existDocument && ! viewing3d );
	viewupdate.enable( existDocument && ! viewing3d );
	viewsize.enable( existDocument && ! viewing3d );
	viewlevel.enable( existDocument && ! viewing3d );
	viewslice.enable( existDocument && ! viewing3d );
#if defined(VIEW3D)
	view3d.enable( existDocument );
#else
	view3d.enable( false );
#endif
	view3d.set( viewing3d ? 1 : 0 );
	changectattribute.enable( existDocument );
	target.enable( existDocument && ( ! extracting && ! modifing ) );
	extract.enable( existDocument );
	extractLimit.enable( extracting );
	extractStop.enable( extracting );
	modify.enable( existDocument );
	modifyClosing.enable( modifing );
	modifyOpening.enable( modifing );
	modifyStop.enable( modifing );
	cluster.enable( existDocument );
	clusterColor.enable( existDocument );
	clusterAlpha.enable( existDocument );
	clusterArea.enable( existDocument );
	clusterVolume.enable( existDocument );
	clusterAverage.enable( existDocument );
	clusterStandardDeviation.enable( existDocument );
	brushes.enable( existDocument );
	brushEnable.enable( existDocument );
	regionPreview.enable( existDocument );
	regionAll.enable( existDocument );
	regionOperationMode.enable( existDocument );
	patternDelete.enable( existDocument );
	patternAllDelete.enable( existDocument );
	patternChange.enable( existDocument );
	if ( existDocument ) {
		viewslice( doc->sliceindex );
		miView * view = miView::get();
		viewsize( view->windowSize );
		viewlevel( view->windowLevel );
		viewslice.initialize( 0, doc->slicesize - 1 );
		if ( ! doc->pattern() ) {
			patternDelete.enable( false );
			patternAllDelete.enable( false );
			patternChange.enable( false );
		}
	}
}
//カスタムドロー
void controlInterface::painter( fc::window * w )
{
	miDocument * doc = miDocument::get();
	if ( w == ( fc::window * ) & controlWindow->clusterColor ) {
		pixel c = doc ? doc->regionColor( controlWindow->targeted ) : pixel( 0, 0, 0 );
		RECT cr = w->client();
		HDC hdc = w->getDC();
		HBRUSH brush = CreateSolidBrush( RGB( c.r, c.g, c.b ) );
		FillRect( hdc, & cr, brush );
		DeleteObject( brush );
		w->release( hdc );
	} else if ( w == ( fc::window * ) & controlWindow->clusterAlpha ) {
		pixel c = doc ? doc->regionColor( controlWindow->targeted ) : pixel( 0, 0, 0 );
		RECT cr = w->client();
		HDC hdc = w->getDC();
		HBRUSH brush = CreateSolidBrush( RGB( c.a, c.a, c.a ) );
		FillRect( hdc, & cr, brush );
		DeleteObject( brush );
		w->release( hdc );
	}
}
void controlInterface::prevSlice()
{
	miDocument * doc = miDocument::get();
	miView * view = miView::get();
	if ( ! doc || ! view ) return;
	controlInterface::sheetChange( NULL );
	viewslice.slider.set( maximum( 0, viewslice.value - 1 ) );
	viewslice.process();
	doc->change();
}
void controlInterface::nextSlice()
{
	miDocument * doc = miDocument::get();
	if ( ! doc ) return;
	controlInterface::sheetChange( NULL );
	viewslice.slider.set( minimum( doc->slicesize - 1, viewslice.value + 1 ) );
	viewslice.process();
	doc->change();
}
void controlInterface::changeRegion( int index )
{
	miDocument * doc = miDocument::get();
	miView * view = miView::get();
	if ( ! doc || ! view ) return;
	target.select( index );
	changeMode( & target );
	miPattern * p = doc->pattern();
	doc->change();
}

void controlInterface::releaseClusters( bool changeClusterSet )
{
	//教師情報を削除
	for ( int i = regionNormal; i < clusters.size; ++i ) {
		while( clusters[i]->head ) {
			cluster.pop( clusters[i]->head );
		}
	}
	if ( changeClusterSet ) {
		target.nodes.allocate( regionNormal + 1 );
		target.refresh();
		//正常以外のカテゴリも削除
		for ( int i = regionNormal + 1; i < clusters.size; ++i ) {
			cluster.pop( clusters[i] );
		}
		clusters.allocate( regionNormal + 1 );
	}
}
void controlInterface::updateClusters( bool changeClusterSet )
{
	releaseClusters( changeClusterSet );
	miDocument * doc = miDocument::get();
	miView * view = miView::get();
	if ( ! doc || ! view ) return;
	if ( changeClusterSet ) {
		//正常以外のカテゴリを追加
		target.nodes.release();
		target.nodes.reserve( doc->numberRegion() );
		for ( int i = 0; i < doc->numberRegion(); ++i ) {
			target.nodes.push_back( doc->regionName( i ) );
		}
		target.refresh();
		for ( int i = regionNormal + 1; i < doc->numberRegion(); ++i ) {
			clusters.push_back( 
				cluster.push( fc::treeview::item( doc->regionName( i ) ), clusters[regionLung] ) );
		}
		target.select( targeted );
	}
	const char * lungTopBottom[] = { "[肺尖部]", "[肺中部]", "[肺底部]" };
	const char * lungFrontRear[] = { "[腹側]", "[中間]", "[背側]" };
	for ( miDocument::regionIterator rit( *doc, true ); rit; ++rit ) {
		int j = 0;
		for ( array< retainer<miPattern> >::iterator it( rit->patterns ); it; ++it, ++j ) {
			//教師画像の子供を追加していく
			cluster.push( 
				fc::treeview::item( string( j ) + 
					( it()->postfix.length() ? string( ":" ) + it()->postfix : ":" ) +
					+ lungTopBottom[clamp<int>( lungTop, it()->topbottom, lungBottom )] + 
					lungFrontRear[clamp<int>( lungFront, it()->frontrear, lungRear )] ), 
				clusters[rit.index()] );
		}
	}
}
