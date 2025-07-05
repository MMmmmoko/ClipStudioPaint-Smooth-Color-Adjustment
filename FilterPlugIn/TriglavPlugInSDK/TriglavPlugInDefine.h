//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINDEFINE_H_
#define	_TRIGLAVPLUGINDEFINE_H_

//	プラグインの真理値
#define	kTriglavPlugInBoolTrue												(1)
#define	kTriglavPlugInBoolFalse												(0)

//	プラグインコールの戻り値
#define	kTriglavPlugInCallResultSuccess										(0)
#define	kTriglavPlugInCallResultFailed										(-1)

//	プラグインAPIの戻り値
#define	kTriglavPlugInAPIResultSuccess										(0)
#define	kTriglavPlugInAPIResultFailed										(-1)

//	プラグインモジュールの種類
#define	kTriglavPlugInModuleKindFilter										(0x4380)
#define	kTriglavPlugInModuleKindFilterActivation							(0x5530)

#if defined(TRIGLAV_PLUGIN_ACTIVATION)
#define	kTriglavPlugInNeedHostVersion										(3)
#else
#define	kTriglavPlugInNeedHostVersion										(1)
#endif

//	プラグインモジュールの種類　切り替え用
#if defined(TRIGLAV_PLUGIN_ACTIVATION)
#define	kTriglavPlugInModuleSwitchKindFilter								kTriglavPlugInModuleKindFilterActivation
#else
#define	kTriglavPlugInModuleSwitchKindFilter								kTriglavPlugInModuleKindFilter
#endif

//	プラグインの処理
#define	kTriglavPlugInSelectorModuleInitialize								(0x0101)
#define	kTriglavPlugInSelectorModuleTerminate								(0x0102)
#define	kTriglavPlugInSelectorFilterInitialize								(0x0201)
#define	kTriglavPlugInSelectorFilterRun										(0x0202)
#define	kTriglavPlugInSelectorFilterTerminate								(0x0203)

//	プラグインフィルタのターゲットの種類
#define	kTriglavPlugInFilterTargetKindRasterLayerGrayAlpha					(0x0101)
#define	kTriglavPlugInFilterTargetKindRasterLayerRGBAlpha					(0x0102)
#define	kTriglavPlugInFilterTargetKindRasterLayerCMYKAlpha					(0x0103)
#define	kTriglavPlugInFilterTargetKindRasterLayerAlpha						(0x0104)
#define	kTriglavPlugInFilterTargetKindRasterLayerBinarizationAlpha			(0x0105)
#define	kTriglavPlugInFilterTargetKindRasterLayerBinarizationGrayAlpha		(0x0106)

//	プラグインフィルタ実行の処理の戻り値
#define	kTriglavPlugInFilterRunProcessResultContinue						(0x0101)
#define	kTriglavPlugInFilterRunProcessResultRestart							(0x0102)
#define	kTriglavPlugInFilterRunProcessResultExit							(0x0103)

//	プラグインフィルタ実行の処理の状態
#define	kTriglavPlugInFilterRunProcessStateStart							(0x0101)
#define	kTriglavPlugInFilterRunProcessStateContinue							(0x0102)
#define	kTriglavPlugInFilterRunProcessStateEnd								(0x0103)
#define	kTriglavPlugInFilterRunProcessStateAbort							(0x0104)

//	ビットマップのスキャンライン
#define	kTriglavPlugInBitmapScanlineHorizontalLeftTop						(0x10)
#define	kTriglavPlugInBitmapScanlineHorizontalRightTop						(0x11)
#define	kTriglavPlugInBitmapScanlineHorizontalLeftBottom					(0x12)
#define	kTriglavPlugInBitmapScanlineHorizontalRightBottom					(0x13)
#define	kTriglavPlugInBitmapScanlineVerticalLeftTop							(0x14)
#define	kTriglavPlugInBitmapScanlineVerticalRightTop						(0x15)
#define	kTriglavPlugInBitmapScanlineVerticalLeftBottom						(0x16)
#define	kTriglavPlugInBitmapScanlineVerticalRightBottom						(0x17)

//	オフスクリーンのチャンネルオーダー
#define	kTriglavPlugInOffscreenChannelOrderAlpha							(0x01)
#define	kTriglavPlugInOffscreenChannelOrderGrayAlpha						(0x02)
#define	kTriglavPlugInOffscreenChannelOrderRGBAlpha							(0x03)
#define	kTriglavPlugInOffscreenChannelOrderCMYKAlpha						(0x04)
#define	kTriglavPlugInOffscreenChannelOrderBinarizationAlpha				(0x05)
#define	kTriglavPlugInOffscreenChannelOrderBinarizationGrayAlpha			(0x06)
#define	kTriglavPlugInOffscreenChannelOrderSelectArea						(0x10)
#define	kTriglavPlugInOffscreenChannelOrderPlane							(0x20)

//	オフスクリーンのコピーモード
#define	kTriglavPlugInOffscreenCopyModeNormal								(0x01)
#define	kTriglavPlugInOffscreenCopyModeImage								(0x02)
#define	kTriglavPlugInOffscreenCopyModeGray									(0x03)
#define	kTriglavPlugInOffscreenCopyModeRed									(0x04)
#define	kTriglavPlugInOffscreenCopyModeGreen								(0x05)
#define	kTriglavPlugInOffscreenCopyModeBlue									(0x06)
#define	kTriglavPlugInOffscreenCopyModeCyan									(0x07)
#define	kTriglavPlugInOffscreenCopyModeMagenta								(0x08)
#define	kTriglavPlugInOffscreenCopyModeYellow								(0x09)
#define	kTriglavPlugInOffscreenCopyModeKeyPlate								(0x10)
#define	kTriglavPlugInOffscreenCopyModeAlpha								(0x11)

//	プロパティの値の型
#define	kTriglavPlugInPropertyValueTypeVoid									(0x00)
#define	kTriglavPlugInPropertyValueTypeBoolean								(0x01)
#define	kTriglavPlugInPropertyValueTypeEnumeration							(0x02)
#define	kTriglavPlugInPropertyValueTypeInteger								(0x11)
#define	kTriglavPlugInPropertyValueTypeDecimal								(0x12)
#define	kTriglavPlugInPropertyValueTypePoint								(0x21)
#define	kTriglavPlugInPropertyValueTypeString								(0x31)

//	プロパティの値の種類
#define	kTriglavPlugInPropertyValueKindDefault								(0x11)
#define	kTriglavPlugInPropertyValueKindPixel								(0x21)

//	プロパティの入力の種類
//属性输入类型，hide意义是啥？
#define	kTriglavPlugInPropertyInputKindHide									(0x10)
#define	kTriglavPlugInPropertyInputKindDefault								(0x11)
#define	kTriglavPlugInPropertyInputKindPushButton							(0x21)
#define	kTriglavPlugInPropertyInputKindCanvas								(0x31)

//	プロパティの座標値のデフォルト値の種類
#define	kTriglavPlugInPropertyPointDefaultValueKindDefault					(0x11)
#define	kTriglavPlugInPropertyPointDefaultValueKindCanvasLeftTop			(0x21)
#define	kTriglavPlugInPropertyPointDefaultValueKindCanvasRightTop			(0x22)
#define	kTriglavPlugInPropertyPointDefaultValueKindCanvasLeftBottom			(0x23)
#define	kTriglavPlugInPropertyPointDefaultValueKindCanvasRightBottom		(0x24)
#define	kTriglavPlugInPropertyPointDefaultValueKindCanvasCenter				(0x25)
#define	kTriglavPlugInPropertyPointDefaultValueKindSelectAreaLeftTop		(0x31)
#define	kTriglavPlugInPropertyPointDefaultValueKindSelectAreaRightTop		(0x32)
#define	kTriglavPlugInPropertyPointDefaultValueKindSelectAreaLeftBottom		(0x33)
#define	kTriglavPlugInPropertyPointDefaultValueKindSelectAreaRightBottom	(0x34)
#define	kTriglavPlugInPropertyPointDefaultValueKindSelectAreaCenter			(0x35)

//	プロパティの座標値の最小最大値の種類
#define	kTriglavPlugInPropertyPointMinMaxValueKindDefault					(0x21)
#define	kTriglavPlugInPropertyPointMinMaxValueKindNo						(0x22)

//	プロパティコールバックの通知
#define	kTriglavPlugInPropertyCallBackNotifyValueChanged					(0x11)
#define	kTriglavPlugInPropertyCallBackNotifyButtonPushed					(0x21)
#define	kTriglavPlugInPropertyCallBackNotifyValueCheck						(0x31)

//	プロパティコールバックの通知の戻り値
#define	kTriglavPlugInPropertyCallBackResultNoModify						(0x01)
#define	kTriglavPlugInPropertyCallBackResultModify							(0x02)
#define	kTriglavPlugInPropertyCallBackResultInvalid							(0x03)

#endif
