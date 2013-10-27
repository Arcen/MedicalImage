//���C������

//#define VIEW3D	//�O�����\���������Ȃ����ǂ���

#include <agl/kernel/agl.h>
using namespace agl;
#include <fc/fc.h>
#include "region.h"

enum {
	//control ID
	statusBarID = 31000,
	sizeSpinID,
	levelSpinID,
	sliceSpinID,
	extractUpLimitSpinID,
	extractDownLimitSpinID,
	modifyClosingSpinID,
	modifyOpeningSpinID,
	brushSpinID,
	calibrationAirSpinID,
	calibrationWaterSpinID,
	gaussCorrelationParamStartSpinID,
	gaussCorrelationParamEndSpinID,
	gaussCorrelationParamStepSpinID,
	//popup
	rclickAppendRegionID,
	rclickDeleteID,
	rclickDefineCenterID,
	rclickDeleteRegionID,
	rclickContinouseDeleteID,
	rclickSamplingRegionID,
	rclickAverageRegionID,
	rclickDeleteMinorRegionID,
	rclickMergeMinorRegionID,

	//�^�u�V�[�g�C���f�b�N�X
	viewSheetIndex = 0,
	regionSheetIndex,
	attributeSheetIndex,

	//�̈�C���f�b�N�X
	regionHuman = 0,
	regionLung,
	regionNormal,

	//�\�����[�h�C���f�b�N�X
	viewModeMix = 0,
	viewModeOriginal,
	viewModeLabel,

	viewSideBoth = 0,
	viewSideLeft,
	viewSideRight,

	regionOperationModeAppend = 0,
	regionOperationModeDelete,
	regionOperationModeChange,

	lungTop = 0,
	lungMiddle,
	lungBottom,
	numberLungTopBottom,
	lungFront = 0,
	//lungMiddle = 1,//�Q�d��`
	lungRear = 2,
	numberLungFrontRear,
};

typedef region<int16>	ctRegion;

extern HINSTANCE hInstance;
extern fc::window * mainWindow;
extern fc::window * imageWindow;
extern fc::statusbar * statusBar;

#include "resource.h"
#include "reg.h"
#include "semaphore.h"
#include "regionfile.h"
#include "freehand.h"
#include "brush.h"
#include "step.h"

extern semaphore singleThread;
extern semaphore onTrackMenu;
extern freehand freehandinterface;
extern brush brushinterface;

#include "regionimage.h"

#include "interface.h"
#include "control.h"
#include "estimate.h"
#include "ctimage.h"
#include "pixelinfo.h"
#include "pattern.h"
#include "document.h"
#include "command.h"
#include "frame.h"
#include "view.h"
