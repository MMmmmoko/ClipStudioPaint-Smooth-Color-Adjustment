//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINRECORD_H_
#define	_TRIGLAVPLUGINRECORD_H_

#include "TriglavPlugInType.h"

TRIGLAV_PLUGIN_EXTERN_C_START
#pragma TRIGLAV_PLUGIN_PACK

//	プロパティコールバック
typedef void				(TRIGLAV_PLUGIN_CALLBACK *TriglavPlugInPropertyCallBackProc)( TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data );

//	モジュール初期化レコードのAPI
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInModuleInitializeGetHostVersionProc)( TriglavPlugInInt* hostVersion, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInModuleInitializeSetModuleIDProc)( TriglavPlugInHostObject hostObject, TriglavPlugInStringObject moduleID );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInModuleInitializeSetModuleKindProc)( TriglavPlugInHostObject hostObject, const TriglavPlugInInt moduleKind );

//	ホスト権限の取得
typedef TriglavPlugInHostPermissionObject	(TRIGLAV_PLUGIN_API *TriglavPlugInActivationGetHostPermissionProc)( TriglavPlugInHostObject hostObject );

//	フィルタ初期化レコードのAPI
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterInitializeSetFilterCategoryNameProc)( TriglavPlugInHostObject hostObject, TriglavPlugInStringObject filterCategoryName, const TriglavPlugInChar accessKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterInitializeSetFilterNameProc)( TriglavPlugInHostObject hostObject, TriglavPlugInStringObject filterName, const TriglavPlugInChar accessKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterInitializeSetCanPreviewProc)( TriglavPlugInHostObject hostObject, const TriglavPlugInBool canPreview );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterInitializeSetUseBlankImageProc)( TriglavPlugInHostObject hostObject, const TriglavPlugInBool useBlankImage );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterInitializeSetTargetKindsProc)( TriglavPlugInHostObject hostObject, const TriglavPlugInInt* targetKinds, const TriglavPlugInInt targetKindCount );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterInitializeSetPropertyProc)( TriglavPlugInHostObject hostObject, TriglavPlugInPropertyObject propertyObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterInitializeSetPropertyCallBackProc)( TriglavPlugInHostObject hostObject, TriglavPlugInPropertyCallBackProc propertyCallBackProc, TriglavPlugInPtr data );

typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationInitializeSetFilterCategoryNameProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, TriglavPlugInStringObject filterCategoryName, const TriglavPlugInChar accessKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationInitializeSetFilterNameProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, TriglavPlugInStringObject filterName, const TriglavPlugInChar accessKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationInitializeSetCanPreviewProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, const TriglavPlugInBool canPreview );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationInitializeSetUseBlankImageProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, const TriglavPlugInBool useBlankImage );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationInitializeSetTargetKindsProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, const TriglavPlugInInt* targetKinds, const TriglavPlugInInt targetKindCount );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationInitializeSetPropertyProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, TriglavPlugInPropertyObject propertyObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationInitializeSetPropertyCallBackProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, TriglavPlugInPropertyCallBackProc propertyCallBackProc, TriglavPlugInPtr data );

//	フィルタ実行レコードのAPI
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetPropertyProc)( TriglavPlugInPropertyObject* propertyObject, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetCanvasWidthProc)( TriglavPlugInInt* width, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetCanvasHeightProc)( TriglavPlugInInt* height, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetCanvasResolutionProc)( TriglavPlugInDouble* resolution, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetLayerOriginProc)( TriglavPlugInPoint* layerOrigin, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunIsLayerMaskSelectedProc)( TriglavPlugInBool* selected, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunIsAlphaLockedProc)( TriglavPlugInBool* locked, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetSourceOffscreenProc)( TriglavPlugInOffscreenObject* offscreenObject, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetDestinationOffscreenProc)( TriglavPlugInOffscreenObject* offscreenObject, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetSelectAreaRectProc)( TriglavPlugInRect* seclectAreaRect, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunHasSelectAreaOffscreenProc)( TriglavPlugInBool* hasSelectAreaOffscreen, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetSelectAreaOffscreenProc)( TriglavPlugInOffscreenObject* offscreenObject, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunUpdateDestinationOffscreenRectProc)( TriglavPlugInHostObject hostObject, const TriglavPlugInRect* updateRect );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetMainColorProc)( TriglavPlugInRGBColor* mainColor, TriglavPlugInUInt8* mainAlpha, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetSubColorProc)( TriglavPlugInRGBColor* subColor, TriglavPlugInUInt8* subAlpha, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunGetDrawColorProc)( TriglavPlugInRGBColor* drawColor, TriglavPlugInUInt8* drawAlpha, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunProcessProc)( TriglavPlugInInt* result, TriglavPlugInHostObject hostObject, const TriglavPlugInInt processState );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunSetProgressTotalProc)( TriglavPlugInHostObject hostObject, const TriglavPlugInInt progressTotal );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterRunSetProgressDoneProc)( TriglavPlugInHostObject hostObject, const TriglavPlugInInt progressDone );

typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetPropertyProc)( TriglavPlugInPropertyObject* propertyObject, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetCanvasWidthProc)( TriglavPlugInInt* width, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetCanvasHeightProc)( TriglavPlugInInt* height, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetCanvasResolutionProc)( TriglavPlugInDouble* resolution, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetLayerOriginProc)( TriglavPlugInPoint* layerOrigin, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunIsLayerMaskSelectedProc)( TriglavPlugInBool* selected, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunIsAlphaLockedProc)( TriglavPlugInBool* locked, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetSourceOffscreenProc)( TriglavPlugInOffscreenObject* offscreenObject, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetDestinationOffscreenProc)( TriglavPlugInOffscreenObject* offscreenObject, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetSelectAreaRectProc)( TriglavPlugInRect* seclectAreaRect, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunHasSelectAreaOffscreenProc)( TriglavPlugInBool* hasSelectAreaOffscreen, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetSelectAreaOffscreenProc)( TriglavPlugInOffscreenObject* offscreenObject, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunUpdateDestinationOffscreenRectProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, const TriglavPlugInRect* updateRect );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetMainColorProc)( TriglavPlugInRGBColor* mainColor, TriglavPlugInUInt8* mainAlpha, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetSubColorProc)( TriglavPlugInRGBColor* subColor, TriglavPlugInUInt8* subAlpha, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunGetDrawColorProc)( TriglavPlugInRGBColor* drawColor, TriglavPlugInUInt8* drawAlpha, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunProcessProc)( TriglavPlugInInt* result, TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, const TriglavPlugInInt processState );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunSetProgressTotalProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, const TriglavPlugInInt progressTotal );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInFilterActivationRunSetProgressDoneProc)( TriglavPlugInHostObject hostObject, TriglavPlugInHostPermissionObject hostPermissionObject, const TriglavPlugInInt progressDone );

//	モジュール初期化レコード
typedef	struct	_TriglavPlugInModuleInitializeRecord
{
	TriglavPlugInModuleInitializeGetHostVersionProc							getHostVersionProc;
	TriglavPlugInModuleInitializeSetModuleIDProc							setModuleIDProc;
	TriglavPlugInModuleInitializeSetModuleKindProc							setModuleKindProc;

}	TriglavPlugInModuleInitializeRecord;

//	フィルタ初期化レコード
typedef	struct	_TriglavPlugInFilterInitializeRecord
{
	TriglavPlugInFilterInitializeSetFilterCategoryNameProc					setFilterCategoryNameProc;
	TriglavPlugInFilterInitializeSetFilterNameProc							setFilterNameProc;
	TriglavPlugInFilterInitializeSetCanPreviewProc							setCanPreviewProc;
	TriglavPlugInFilterInitializeSetUseBlankImageProc						setUseBlankImageProc;
	TriglavPlugInFilterInitializeSetTargetKindsProc							setTargetKindsProc;
	TriglavPlugInFilterInitializeSetPropertyProc							setPropertyProc;
	TriglavPlugInFilterInitializeSetPropertyCallBackProc					setPropertyCallBackProc;

}	TriglavPlugInFilterInitializeRecord;

typedef	struct	_TriglavPlugInFilterActivationInitializeRecord
{
	TriglavPlugInActivationGetHostPermissionProc							getHostPermissionProc;
	TriglavPlugInFilterActivationInitializeSetFilterCategoryNameProc		setFilterCategoryNameProc;
	TriglavPlugInFilterActivationInitializeSetFilterNameProc				setFilterNameProc;
	TriglavPlugInFilterActivationInitializeSetCanPreviewProc				setCanPreviewProc;
	TriglavPlugInFilterActivationInitializeSetUseBlankImageProc				setUseBlankImageProc;
	TriglavPlugInFilterActivationInitializeSetTargetKindsProc				setTargetKindsProc;
	TriglavPlugInFilterActivationInitializeSetPropertyProc					setPropertyProc;
	TriglavPlugInFilterActivationInitializeSetPropertyCallBackProc			setPropertyCallBackProc;

}	TriglavPlugInFilterActivationInitializeRecord;

//	フィルタ実行レコード
typedef	struct	_TriglavPlugInFilterRunRecord
{
	TriglavPlugInFilterRunGetPropertyProc									getPropertyProc;
	TriglavPlugInFilterRunGetCanvasWidthProc								getCanvasWidthProc;
	TriglavPlugInFilterRunGetCanvasHeightProc								getCanvasHeightProc;
	TriglavPlugInFilterRunGetCanvasResolutionProc							getCanvasResolutionProc;
	TriglavPlugInFilterRunGetLayerOriginProc								getLayerOriginProc;
	TriglavPlugInFilterRunIsLayerMaskSelectedProc							isLayerMaskSelectedProc;
	TriglavPlugInFilterRunIsAlphaLockedProc									isAlphaLockedProc;
	TriglavPlugInFilterRunGetSourceOffscreenProc							getSourceOffscreenProc;
	TriglavPlugInFilterRunGetDestinationOffscreenProc						getDestinationOffscreenProc;
	TriglavPlugInFilterRunGetSelectAreaRectProc								getSelectAreaRectProc;
	TriglavPlugInFilterRunHasSelectAreaOffscreenProc						hasSelectAreaOffscreenProc;
	TriglavPlugInFilterRunGetSelectAreaOffscreenProc						getSelectAreaOffscreenProc;
	TriglavPlugInFilterRunUpdateDestinationOffscreenRectProc				updateDestinationOffscreenRectProc;
	TriglavPlugInFilterRunGetMainColorProc									getMainColorProc;
	TriglavPlugInFilterRunGetSubColorProc									getSubColorProc;
	TriglavPlugInFilterRunGetDrawColorProc									getDrawColorProc;
	TriglavPlugInFilterRunProcessProc										processProc;
	TriglavPlugInFilterRunSetProgressTotalProc								setProgressTotalProc;
	TriglavPlugInFilterRunSetProgressDoneProc								setProgressDoneProc;

}	TriglavPlugInFilterRunRecord;

typedef	struct	_TriglavPlugInFilterActivationRunRecord
{
	TriglavPlugInActivationGetHostPermissionProc							getHostPermissionProc;
	TriglavPlugInFilterActivationRunGetPropertyProc							getPropertyProc;
	TriglavPlugInFilterActivationRunGetCanvasWidthProc						getCanvasWidthProc;
	TriglavPlugInFilterActivationRunGetCanvasHeightProc						getCanvasHeightProc;
	TriglavPlugInFilterActivationRunGetCanvasResolutionProc					getCanvasResolutionProc;
	TriglavPlugInFilterActivationRunGetLayerOriginProc						getLayerOriginProc;
	TriglavPlugInFilterActivationRunIsLayerMaskSelectedProc					isLayerMaskSelectedProc;
	TriglavPlugInFilterActivationRunIsAlphaLockedProc						isAlphaLockedProc;
	TriglavPlugInFilterActivationRunGetSourceOffscreenProc					getSourceOffscreenProc;
	TriglavPlugInFilterActivationRunGetDestinationOffscreenProc				getDestinationOffscreenProc;
	TriglavPlugInFilterActivationRunGetSelectAreaRectProc					getSelectAreaRectProc;
	TriglavPlugInFilterActivationRunHasSelectAreaOffscreenProc				hasSelectAreaOffscreenProc;
	TriglavPlugInFilterActivationRunGetSelectAreaOffscreenProc				getSelectAreaOffscreenProc;
	TriglavPlugInFilterActivationRunUpdateDestinationOffscreenRectProc		updateDestinationOffscreenRectProc;
	TriglavPlugInFilterActivationRunGetMainColorProc						getMainColorProc;
	TriglavPlugInFilterActivationRunGetSubColorProc							getSubColorProc;
	TriglavPlugInFilterActivationRunGetDrawColorProc						getDrawColorProc;
	TriglavPlugInFilterActivationRunProcessProc								processProc;
	TriglavPlugInFilterActivationRunSetProgressTotalProc					setProgressTotalProc;
	TriglavPlugInFilterActivationRunSetProgressDoneProc						setProgressDoneProc;

}	TriglavPlugInFilterActivationRunRecord;

//	プラグインレコードスイート
typedef	struct	_TriglavPlugInRecordSuite
{
	TriglavPlugInModuleInitializeRecord*									moduleInitializeRecord;
	void*																	reserved1;
	void*																	reserved2;
	void*																	reserved3;
	void*																	reserved4;
	TriglavPlugInFilterInitializeRecord*									filterInitializeRecord;
	TriglavPlugInFilterRunRecord*											filterRunRecord;
	void*																	reserved5;
	void*																	reserved6;
	void*																	reserved7;
	void*																	reserved8;
	void*																	reserved9;
	void*																	reserved10;
	void*																	reserved11;
	void*																	reserved12;
	void*																	reserved13;
	void*																	reserved14;
	void*																	reserved15;
	void*																	reserved16;
	void*																	reserved17;
	void*																	reserved18;
	void*																	reserved19;	
	TriglavPlugInFilterActivationInitializeRecord*							filterActivationInitializeRecord;
	TriglavPlugInFilterActivationRunRecord*									filterActivationRunRecord;

	void*	reserved[256-24];

}	TriglavPlugInRecordSuite;

#pragma TRIGLAV_PLUGIN_RESETPACK
TRIGLAV_PLUGIN_EXTERN_C_END

#endif
