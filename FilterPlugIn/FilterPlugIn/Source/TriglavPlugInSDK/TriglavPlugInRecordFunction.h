//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINRECORDFUNCTION_H_
#define	_TRIGLAVPLUGINRECORDFUNCTION_H_

#include "TriglavPlugInRecord.h"

TRIGLAV_PLUGIN_EXTERN_C_START
#pragma TRIGLAV_PLUGIN_PACK

//	フィルタ初期化レコードのAPI
#if defined(TRIGLAV_PLUGIN_ACTIVATION)
#define	TriglavPlugInGetFilterInitializeRecord(record)																					((record)->filterActivationInitializeRecord)
#define	TriglavPlugInFilterInitializeSetFilterCategoryName(record,hostObject,filterCategoryName,accessKey)								((record)->filterActivationInitializeRecord->setFilterCategoryNameProc((hostObject),(record)->filterActivationInitializeRecord->getHostPermissionProc((hostObject)),(filterCategoryName),(accessKey)))
#define	TriglavPlugInFilterInitializeSetFilterName(record,hostObject,filterName,accessKey)												((record)->filterActivationInitializeRecord->setFilterNameProc((hostObject),(record)->filterActivationInitializeRecord->getHostPermissionProc((hostObject)),(filterName),(accessKey)))
#define	TriglavPlugInFilterInitializeSetCanPreview(record,hostObject,canPreview)														((record)->filterActivationInitializeRecord->setCanPreviewProc((hostObject),(record)->filterActivationInitializeRecord->getHostPermissionProc((hostObject)),(canPreview)))
#define	TriglavPlugInFilterInitializeSetUseBlankImage(record,hostObject,useBlankImage)													((record)->filterActivationInitializeRecord->setUseBlankImageProc((hostObject),(record)->filterActivationInitializeRecord->getHostPermissionProc((hostObject)),(useBlankImage)))
#define	TriglavPlugInFilterInitializeSetTargetKinds(record,hostObject,targetKinds,targetKindCount)										((record)->filterActivationInitializeRecord->setTargetKindsProc((hostObject),(record)->filterActivationInitializeRecord->getHostPermissionProc((hostObject)),(targetKinds),(targetKindCount)))
#define	TriglavPlugInFilterInitializeSetProperty(record,hostObject,propertyObject)														((record)->filterActivationInitializeRecord->setPropertyProc((hostObject),(record)->filterActivationInitializeRecord->getHostPermissionProc((hostObject)),(propertyObject)))
#define	TriglavPlugInFilterInitializeSetPropertyCallBack(record,hostObject,propertyCallBackProc,data)									((record)->filterActivationInitializeRecord->setPropertyCallBackProc((hostObject),(record)->filterActivationInitializeRecord->getHostPermissionProc((hostObject)),(propertyCallBackProc),(data)))
#else
#define	TriglavPlugInGetFilterInitializeRecord(record)																					((record)->filterInitializeRecord)
#define	TriglavPlugInFilterInitializeSetFilterCategoryName(record,hostObject,filterCategoryName,accessKey)								((record)->filterInitializeRecord->setFilterCategoryNameProc((hostObject),(filterCategoryName),(accessKey)))
#define	TriglavPlugInFilterInitializeSetFilterName(record,hostObject,filterName,accessKey)												((record)->filterInitializeRecord->setFilterNameProc((hostObject),(filterName),(accessKey)))
#define	TriglavPlugInFilterInitializeSetCanPreview(record,hostObject,canPreview)														((record)->filterInitializeRecord->setCanPreviewProc((hostObject),(canPreview)))
#define	TriglavPlugInFilterInitializeSetUseBlankImage(record,hostObject,useBlankImage)													((record)->filterInitializeRecord->setUseBlankImageProc((hostObject),(useBlankImage)))
#define	TriglavPlugInFilterInitializeSetTargetKinds(record,hostObject,targetKinds,targetKindCount)										((record)->filterInitializeRecord->setTargetKindsProc((hostObject),(targetKinds),(targetKindCount)))
#define	TriglavPlugInFilterInitializeSetProperty(record,hostObject,propertyObject)														((record)->filterInitializeRecord->setPropertyProc((hostObject),(propertyObject)))
#define	TriglavPlugInFilterInitializeSetPropertyCallBack(record,hostObject,propertyCallBackProc,data)									((record)->filterInitializeRecord->setPropertyCallBackProc((hostObject),(propertyCallBackProc),(data)))
#endif

//	フィルタ実行レコードのAPI
#if defined(TRIGLAV_PLUGIN_ACTIVATION)
#define	TriglavPlugInGetFilterRunRecord(record)																							((record)->filterActivationRunRecord)
#define	TriglavPlugInFilterRunGetProperty(record,propertyObject,hostObject)																((record)->filterActivationRunRecord->getPropertyProc((propertyObject),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetCanvasWidth(record,width,hostObject)																	((record)->filterActivationRunRecord->getCanvasWidthProc((width),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetCanvasHeight(record,height,hostObject)																	((record)->filterActivationRunRecord->getCanvasHeightProc((height),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetCanvasResolution(record,resolution,hostObject)															((record)->filterActivationRunRecord->getCanvasResolutionProc((resolution),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetLayerOrigin(record,layerOrigin,hostObject)																((record)->filterActivationRunRecord->getLayerOriginProc((layerOrigin),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunIsLayerMaskSelected(record,selected,hostObject)															((record)->filterActivationRunRecord->isLayerMaskSelectedProc((selected),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunIsAlphaLocked(record,locked,hostObject)																	((record)->filterActivationRunRecord->isAlphaLockedProc((locked),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetSourceOffscreen(record,offscreenObject,hostObject)														((record)->filterActivationRunRecord->getSourceOffscreenProc((offscreenObject),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetDestinationOffscreen(record,offscreenObject,hostObject)												((record)->filterActivationRunRecord->getDestinationOffscreenProc((offscreenObject),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetSelectAreaRect(record,seclectAreaRect,hostObject)														((record)->filterActivationRunRecord->getSelectAreaRectProc((seclectAreaRect),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunHasSelectAreaOffscreen(record,hasSelectAreaOffscreen,hostObject)											((record)->filterActivationRunRecord->hasSelectAreaOffscreenProc((hasSelectAreaOffscreen),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetSelectAreaOffscreen(record,offscreenObject,hostObject)													((record)->filterActivationRunRecord->getSelectAreaOffscreenProc((offscreenObject),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunUpdateDestinationOffscreenRect(record,hostObject,updateRect)												((record)->filterActivationRunRecord->updateDestinationOffscreenRectProc((hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject)),(updateRect)))
#define	TriglavPlugInFilterRunGetMainColor(record,mainColor,mainAlpha,hostObject)														((record)->filterActivationRunRecord->getMainColorProc((mainColor),(mainAlpha),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetSubColor(record,subColor,subAlpha,hostObject)															((record)->filterActivationRunRecord->getSubColorProc((subColor),(subAlpha),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunGetDrawColor(record,drawColor,drawAlpha,hostObject)														((record)->filterActivationRunRecord->getDrawColorProc((drawColor),(drawAlpha),(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject))))
#define	TriglavPlugInFilterRunProcess(record,result,hostObject,processState)															((record)->filterActivationRunRecord->processProc(result,(hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject)),(processState)))
#define	TriglavPlugInFilterRunSetProgressTotal(record,hostObject,progressTotal)															((record)->filterActivationRunRecord->setProgressTotalProc((hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject)),(progressTotal)))
#define	TriglavPlugInFilterRunSetProgressDone(record,hostObject,progressDone)															((record)->filterActivationRunRecord->setProgressDoneProc((hostObject),(record)->filterActivationRunRecord->getHostPermissionProc((hostObject)),(progressDone)))
#else
#define	TriglavPlugInGetFilterRunRecord(record)																							((record)->filterRunRecord)
#define	TriglavPlugInFilterRunGetProperty(record,propertyObject,hostObject)																((record)->filterRunRecord->getPropertyProc((propertyObject),(hostObject)))
#define	TriglavPlugInFilterRunGetCanvasWidth(record,width,hostObject)																	((record)->filterRunRecord->getCanvasWidthProc((width),(hostObject)))
#define	TriglavPlugInFilterRunGetCanvasHeight(record,height,hostObject)																	((record)->filterRunRecord->getCanvasHeightProc((height),(hostObject)))
#define	TriglavPlugInFilterRunGetCanvasResolution(record,resolution,hostObject)															((record)->filterRunRecord->getCanvasResolutionProc((resolution),(hostObject)))
#define	TriglavPlugInFilterRunGetLayerOrigin(record,layerOrigin,hostObject)																((record)->filterRunRecord->getLayerOriginProc((layerOrigin),(hostObject)))
#define	TriglavPlugInFilterRunIsLayerMaskSelected(record,selected,hostObject)															((record)->filterRunRecord->isLayerMaskSelectedProc((selected),(hostObject)))
#define	TriglavPlugInFilterRunIsAlphaLocked(record,locked,hostObject)																	((record)->filterRunRecord->isAlphaLockedProc((locked),(hostObject)))
#define	TriglavPlugInFilterRunGetSourceOffscreen(record,offscreenObject,hostObject)														((record)->filterRunRecord->getSourceOffscreenProc((offscreenObject),(hostObject)))
#define	TriglavPlugInFilterRunGetDestinationOffscreen(record,offscreenObject,hostObject)												((record)->filterRunRecord->getDestinationOffscreenProc((offscreenObject),(hostObject)))
#define	TriglavPlugInFilterRunGetSelectAreaRect(record,seclectAreaRect,hostObject)														((record)->filterRunRecord->getSelectAreaRectProc((seclectAreaRect),(hostObject)))
#define	TriglavPlugInFilterRunHasSelectAreaOffscreen(record,hasSelectAreaOffscreen,hostObject)											((record)->filterRunRecord->hasSelectAreaOffscreenProc((hasSelectAreaOffscreen),(hostObject)))
#define	TriglavPlugInFilterRunGetSelectAreaOffscreen(record,offscreenObject,hostObject)													((record)->filterRunRecord->getSelectAreaOffscreenProc((offscreenObject),(hostObject)))
#define	TriglavPlugInFilterRunUpdateDestinationOffscreenRect(record,hostObject,updateRect)												((record)->filterRunRecord->updateDestinationOffscreenRectProc((hostObject),(updateRect)))
#define	TriglavPlugInFilterRunGetMainColor(record,mainColor,mainAlpha,hostObject)														((record)->filterRunRecord->getMainColorProc((mainColor),(mainAlpha),(hostObject)))
#define	TriglavPlugInFilterRunGetSubColor(record,subColor,subAlpha,hostObject)															((record)->filterRunRecord->getSubColorProc((subColor),(subAlpha),(hostObject)))
#define	TriglavPlugInFilterRunGetDrawColor(record,drawColor,drawAlpha,hostObject)														((record)->filterRunRecord->getDrawColorProc((drawColor),(drawAlpha),(hostObject)))
#define	TriglavPlugInFilterRunProcess(record,result,hostObject,processState)															((record)->filterRunRecord->processProc(result,(hostObject),(processState)))
#define	TriglavPlugInFilterRunSetProgressTotal(record,hostObject,progressTotal)															((record)->filterRunRecord->setProgressTotalProc((hostObject),(progressTotal)))
#define	TriglavPlugInFilterRunSetProgressDone(record,hostObject,progressDone)															((record)->filterRunRecord->setProgressDoneProc((hostObject),(progressDone)))
#endif

#pragma TRIGLAV_PLUGIN_RESETPACK
TRIGLAV_PLUGIN_EXTERN_C_END

#endif
