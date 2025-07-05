//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#ifndef	_TRIGLAVPLUGINSERVICE_H_
#define	_TRIGLAVPLUGINSERVICE_H_

#include "TriglavPlugInType.h"

TRIGLAV_PLUGIN_EXTERN_C_START
#pragma TRIGLAV_PLUGIN_PACK

//	文字列サービスAPI
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringCreateWithAsciiStringProc)( TriglavPlugInStringObject* stringObject, const TriglavPlugInChar* ascii, const TriglavPlugInInt length );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringCreateWithUnicodeStringProc)( TriglavPlugInStringObject* stringObject, const TriglavPlugInUniChar* unicode, const TriglavPlugInInt length );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringCreateWithLocalCodeStringProc)( TriglavPlugInStringObject* stringObject, const TriglavPlugInChar* localcode, const TriglavPlugInInt length );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringCreateWithStringIDProc)( TriglavPlugInStringObject* stringObject, const TriglavPlugInInt stringID, TriglavPlugInHostObject hostObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringRetainProc)( TriglavPlugInStringObject stringObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringReleaseProc)( TriglavPlugInStringObject stringObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringGetUnicodeCharsProc)( const TriglavPlugInUniChar** unicode, TriglavPlugInStringObject stringObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringGetUnicodeLengthProc)( TriglavPlugInInt* length, TriglavPlugInStringObject stringObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringGetLocalCodeCharsProc)( const TriglavPlugInChar** localcode, TriglavPlugInStringObject stringObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInStringGetLocalCodeLengthProc)( TriglavPlugInInt* length, TriglavPlugInStringObject stringObject );

//	ビットマップサービスAPI
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapCreateProc)( TriglavPlugInBitmapObject* bitmapObject, const TriglavPlugInInt width, const TriglavPlugInInt height, const TriglavPlugInInt depth, const TriglavPlugInInt scanline );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapRetainProc)( TriglavPlugInBitmapObject bitmapObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapReleaseProc)( TriglavPlugInBitmapObject bitmapObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapGetWidthProc)( TriglavPlugInInt* width, TriglavPlugInBitmapObject bitmapObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapGetHeightProc)( TriglavPlugInInt* height, TriglavPlugInBitmapObject bitmapObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapGetDepthProc)( TriglavPlugInInt* depth, TriglavPlugInBitmapObject bitmapObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapGetScanlineProc)( TriglavPlugInInt* scanline, TriglavPlugInBitmapObject bitmapObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapGetAddressProc)( TriglavPlugInPtr* address, TriglavPlugInBitmapObject bitmapObject, const TriglavPlugInPoint* pos );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapGetRowBytesProc)( TriglavPlugInInt* rowBytes, TriglavPlugInBitmapObject bitmapObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInBitmapGetPixelBytesProc)( TriglavPlugInInt* pixelBytes, TriglavPlugInBitmapObject bitmapObject );

//	オフスクリーンサービスAPI
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenCreatePlaneProc)( TriglavPlugInOffscreenObject* offscreenObject, const TriglavPlugInInt width, const TriglavPlugInInt height, const TriglavPlugInInt depth );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenRetainProc)( TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenReleaseProc)( TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetWidthProc)( TriglavPlugInInt* width, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetHeightProc)( TriglavPlugInInt* height, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetRectProc)( TriglavPlugInRect* rect, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetExtentRectProc)( TriglavPlugInRect* rect, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetChannelOrderProc)( TriglavPlugInInt* channelOrder, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetRGBChannelIndexProc)( TriglavPlugInInt* redChannelIndex, TriglavPlugInInt* greenChannelIndex, TriglavPlugInInt* blueChannelIndex, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetCMYKChannelIndexProc)( TriglavPlugInInt* cyanChannelIndex, TriglavPlugInInt* magentaChannelIndex, TriglavPlugInInt* yellowChannelIndex, TriglavPlugInInt* keytoneChannelIndex, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBlockRectCountProc)( TriglavPlugInInt* blockRectCount, TriglavPlugInOffscreenObject offscreenObject, TriglavPlugInRect* bounds );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBlockRectProc)( TriglavPlugInRect* blockRect, TriglavPlugInInt blockIndex, TriglavPlugInOffscreenObject offscreenObject, TriglavPlugInRect* bounds );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBlockImageProc)( TriglavPlugInPtr* address, TriglavPlugInInt* rowBytes, TriglavPlugInInt* pixelBytes, TriglavPlugInRect* blockRect, TriglavPlugInOffscreenObject offscreenObject, TriglavPlugInPoint* pos );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBlockAlphaProc)( TriglavPlugInPtr* address, TriglavPlugInInt* rowBytes, TriglavPlugInInt* pixelBytes, TriglavPlugInRect* blockRect, TriglavPlugInOffscreenObject offscreenObject, TriglavPlugInPoint* pos );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBlockSelectAreaProc)( TriglavPlugInPtr* address, TriglavPlugInInt* rowBytes, TriglavPlugInInt* pixelBytes, TriglavPlugInRect* blockRect, TriglavPlugInOffscreenObject offscreenObject, TriglavPlugInPoint* pos );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBlockPlaneProc)( TriglavPlugInPtr* address, TriglavPlugInInt* rowBytes, TriglavPlugInInt* pixelBytes, TriglavPlugInRect* blockRect, TriglavPlugInOffscreenObject offscreenObject, TriglavPlugInPoint* pos );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetTileWidthProc)( TriglavPlugInInt* tileWidth, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetTileHeightProc)( TriglavPlugInInt* tileHeight, TriglavPlugInOffscreenObject offscreenObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBitmapProc)( TriglavPlugInBitmapObject bitmapObject, const TriglavPlugInPoint* bitmapPos, TriglavPlugInOffscreenObject offscreenObject, const TriglavPlugInPoint* offscreenPos, const TriglavPlugInInt copyWidth, const TriglavPlugInInt copyHeight, const TriglavPlugInInt copyMode );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenSetBitmapProc)( TriglavPlugInOffscreenObject offscreenObject, const TriglavPlugInPoint* offscreenPos, TriglavPlugInBitmapObject bitmapObject, const TriglavPlugInPoint* bitmapPos, const TriglavPlugInInt copyWidth, const TriglavPlugInInt copyHeight, const TriglavPlugInInt copyMode );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInOffscreenGetBitmapNormalAlphaChannelIndexProc)( TriglavPlugInInt* alphaChannelIndex, TriglavPlugInOffscreenObject offscreenObject );

//	プロパティサービスAPI
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyCreateProc)( TriglavPlugInPropertyObject* propertyObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyRetainProc)( TriglavPlugInPropertyObject propertyObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyReleaseProc)( TriglavPlugInPropertyObject propertyObject );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyAddItemProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt valueType, const TriglavPlugInInt valueKind, const TriglavPlugInInt inputKind, TriglavPlugInStringObject caption, const TriglavPlugInChar accessKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetItemStoreValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInBool storeValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetBooleanValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInBool value );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetBooleanValueProc)( TriglavPlugInBool* value, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetBooleanDefaultValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInBool defaultValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetBooleanDefaultValueProc)( TriglavPlugInBool* defaultValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetIntegerValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt value );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetIntegerValueProc)( TriglavPlugInInt* value, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetIntegerDefaultValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt defaultValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetIntegerDefaultValueProc)( TriglavPlugInInt* defaultValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetIntegerMinValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt minValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetIntegerMinValueProc)( TriglavPlugInInt* minValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetIntegerMaxValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt maxValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetIntegerMaxValueProc)( TriglavPlugInInt* maxValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetDecimalValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInDouble value );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetDecimalValueProc)( TriglavPlugInDouble* value, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetDecimalDefaultValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInDouble defaultValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetDecimalDefaultValueProc)( TriglavPlugInDouble* defaultValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetDecimalMinValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInDouble minValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetDecimalMinValueProc)( TriglavPlugInDouble* minValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetDecimalMaxValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInDouble maxValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetDecimalMaxValueProc)( TriglavPlugInDouble* maxValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetPointValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInPoint* value );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetPointValueProc)( TriglavPlugInPoint* value, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetPointDefaultValueKindProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt defaultValueKind );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetPointDefaultValueKindProc)( TriglavPlugInInt* defaultValueKind, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetPointDefaultValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInPoint* defaultValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetPointDefaultValueProc)( TriglavPlugInPoint* defaultValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetPointMinMaxValueKindProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt minMaxValueKind );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetPointMinMaxValueKindProc)( TriglavPlugInInt* minMaxValueKind, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetPointMinValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInPoint* minValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetPointMinValueProc)( TriglavPlugInPoint* minValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetPointMaxValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInPoint* maxValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetPointMaxValueProc)( TriglavPlugInPoint* maxValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetEnumerationValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt value );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetEnumerationValueProc)( TriglavPlugInInt* value, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetEnumerationDefaultValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt value );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetEnumerationDefaultValueProc)( TriglavPlugInInt* value, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyAddEnumerationItemProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt value, TriglavPlugInStringObject caption, const TriglavPlugInChar accessKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetStringValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, TriglavPlugInStringObject value );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetStringValueProc)( TriglavPlugInStringObject* value, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetStringDefaultValueProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, TriglavPlugInStringObject defaultValue );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetStringDefaultValueProc)( TriglavPlugInStringObject* defaultValue, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertySetStringMaxLengthProc)( TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt maxLength );
typedef TriglavPlugInInt	(TRIGLAV_PLUGIN_API *TriglavPlugInPropertyGetStringMaxLengthProc)( TriglavPlugInInt* maxLength, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey );

//	文字列サービス
typedef	struct	_TriglavPlugInStringService
{
	TriglavPlugInStringCreateWithAsciiStringProc					createWithAsciiStringProc;
	TriglavPlugInStringCreateWithUnicodeStringProc					createWithUnicodeStringProc;
	TriglavPlugInStringCreateWithLocalCodeStringProc				createWithLocalCodeStringProc;
	TriglavPlugInStringCreateWithStringIDProc						createWithStringIDProc;
	TriglavPlugInStringRetainProc									retainProc;
	TriglavPlugInStringReleaseProc									releaseProc;
	TriglavPlugInStringGetUnicodeCharsProc							getUnicodeCharsProc;
	TriglavPlugInStringGetUnicodeLengthProc							getUnicodeLengthProc;
	TriglavPlugInStringGetLocalCodeCharsProc						getLocalCodeCharsProc;
	TriglavPlugInStringGetLocalCodeLengthProc						getLocalCodeLengthProc;

}	TriglavPlugInStringService;

//	ビットマップサービス
typedef	struct	_TriglavPlugInBitmapService
{
	TriglavPlugInBitmapCreateProc									createProc;
	TriglavPlugInBitmapRetainProc									retainProc;
	TriglavPlugInBitmapReleaseProc									releaseProc;
	TriglavPlugInBitmapGetWidthProc									getWidthProc;
	TriglavPlugInBitmapGetHeightProc								getHeightProc;
	TriglavPlugInBitmapGetDepthProc									getDepthProc;
	TriglavPlugInBitmapGetScanlineProc								getScanlineProc;
	TriglavPlugInBitmapGetAddressProc								getAddressProc;
	TriglavPlugInBitmapGetRowBytesProc								getRowBytesProc;
	TriglavPlugInBitmapGetPixelBytesProc							getPixelBytesProc;

}	TriglavPlugInBitmapService;

//	オフスクリーンサービス
typedef	struct	_TriglavPlugInOffscreenService
{
	TriglavPlugInOffscreenCreatePlaneProc							createPlaneProc;
	TriglavPlugInOffscreenRetainProc								retainProc;
	TriglavPlugInOffscreenReleaseProc								releaseProc;
	TriglavPlugInOffscreenGetWidthProc								getWidthProc;
	TriglavPlugInOffscreenGetHeightProc								getHeightProc;
	TriglavPlugInOffscreenGetRectProc								getRectProc;
	TriglavPlugInOffscreenGetExtentRectProc							getExtentRectProc;
	TriglavPlugInOffscreenGetChannelOrderProc						getChannelOrderProc;
	TriglavPlugInOffscreenGetRGBChannelIndexProc					getRGBChannelIndexProc;
	TriglavPlugInOffscreenGetCMYKChannelIndexProc					getCMYKChannelIndexProc;
	TriglavPlugInOffscreenGetBlockRectCountProc						getBlockRectCountProc;
	TriglavPlugInOffscreenGetBlockRectProc							getBlockRectProc;
	TriglavPlugInOffscreenGetBlockImageProc							getBlockImageProc;
	TriglavPlugInOffscreenGetBlockAlphaProc							getBlockAlphaProc;
	TriglavPlugInOffscreenGetBlockSelectAreaProc					getBlockSelectAreaProc;
	TriglavPlugInOffscreenGetBlockPlaneProc							getBlockPlaneProc;
	TriglavPlugInOffscreenGetTileWidthProc							getTileWidthProc;
	TriglavPlugInOffscreenGetTileHeightProc							getTileHeightProc;
	TriglavPlugInOffscreenGetBitmapProc								getBitmapProc;
	TriglavPlugInOffscreenSetBitmapProc								setBitmapProc;

}	TriglavPlugInOffscreenService;

//	オフスクリーンサービス2
typedef	struct	_TriglavPlugInOffscreenService2
{
	TriglavPlugInOffscreenGetBitmapNormalAlphaChannelIndexProc		getBitmapNormalAlphaChannelIndexProc;

}	TriglavPlugInOffscreenService2;

//	プロパティサービス
typedef	struct	_TriglavPlugInPropertyService
{
	TriglavPlugInPropertyCreateProc									createProc;
	TriglavPlugInPropertyRetainProc									retainProc;
	TriglavPlugInPropertyReleaseProc								releaseProc;
	TriglavPlugInPropertyAddItemProc								addItemProc;
	TriglavPlugInPropertySetBooleanValueProc						setBooleanValueProc;
	TriglavPlugInPropertyGetBooleanValueProc						getBooleanValueProc;
	TriglavPlugInPropertySetBooleanDefaultValueProc					setBooleanDefaultValueProc;
	TriglavPlugInPropertyGetBooleanDefaultValueProc					getBooleanDefaultValueProc;
	TriglavPlugInPropertySetIntegerValueProc						setIntegerValueProc;
	TriglavPlugInPropertyGetIntegerValueProc						getIntegerValueProc;
	TriglavPlugInPropertySetIntegerDefaultValueProc					setIntegerDefaultValueProc;
	TriglavPlugInPropertyGetIntegerDefaultValueProc					getIntegerDefaultValueProc;
	TriglavPlugInPropertySetIntegerMinValueProc						setIntegerMinValueProc;
	TriglavPlugInPropertyGetIntegerMinValueProc						getIntegerMinValueProc;
	TriglavPlugInPropertySetIntegerMaxValueProc						setIntegerMaxValueProc;
	TriglavPlugInPropertyGetIntegerMaxValueProc						getIntegerMaxValueProc;
	TriglavPlugInPropertySetDecimalValueProc						setDecimalValueProc;
	TriglavPlugInPropertyGetDecimalValueProc						getDecimalValueProc;
	TriglavPlugInPropertySetDecimalDefaultValueProc					setDecimalDefaultValueProc;
	TriglavPlugInPropertyGetDecimalDefaultValueProc					getDecimalDefaultValueProc;
	TriglavPlugInPropertySetDecimalMinValueProc						setDecimalMinValueProc;
	TriglavPlugInPropertyGetDecimalMinValueProc						getDecimalMinValueProc;
	TriglavPlugInPropertySetDecimalMaxValueProc						setDecimalMaxValueProc;
	TriglavPlugInPropertyGetDecimalMaxValueProc						getDecimalMaxValueProc;

}	TriglavPlugInPropertyService;

//	プロパティサービス2
typedef	struct	_TriglavPlugInPropertyService2
{
	TriglavPlugInPropertySetItemStoreValueProc						setItemStoreValueProc;
	TriglavPlugInPropertySetPointValueProc							setPointValueProc;
	TriglavPlugInPropertyGetPointValueProc							getPointValueProc;
	TriglavPlugInPropertySetPointDefaultValueKindProc				setPointDefaultValueKindProc;
	TriglavPlugInPropertyGetPointDefaultValueKindProc				getPointDefaultValueKindProc;
	TriglavPlugInPropertySetPointDefaultValueProc					setPointDefaultValueProc;
	TriglavPlugInPropertyGetPointDefaultValueProc					getPointDefaultValueProc;
	TriglavPlugInPropertySetPointMinMaxValueKindProc				setPointMinMaxValueKindProc;
	TriglavPlugInPropertyGetPointMinMaxValueKindProc				getPointMinMaxValueKindProc;
	TriglavPlugInPropertySetPointMinValueProc						setPointMinValueProc;
	TriglavPlugInPropertyGetPointMinValueProc						getPointMinValueProc;
	TriglavPlugInPropertySetPointMaxValueProc						setPointMaxValueProc;
	TriglavPlugInPropertyGetPointMaxValueProc						getPointMaxValueProc;
	TriglavPlugInPropertySetEnumerationValueProc					setEnumerationValueProc;
	TriglavPlugInPropertyGetEnumerationValueProc					getEnumerationValueProc;
	TriglavPlugInPropertySetEnumerationDefaultValueProc				setEnumerationDefaultValueProc;
	TriglavPlugInPropertyGetEnumerationDefaultValueProc				getEnumerationDefaultValueProc;
	TriglavPlugInPropertyAddEnumerationItemProc						addEnumerationItemProc;
	TriglavPlugInPropertySetStringValueProc							setStringValueProc;
	TriglavPlugInPropertyGetStringValueProc							getStringValueProc;
	TriglavPlugInPropertySetStringDefaultValueProc					setStringDefaultValueProc;
	TriglavPlugInPropertyGetStringDefaultValueProc					getStringDefaultValueProc;
	TriglavPlugInPropertySetStringMaxLengthProc						setStringMaxLengthProc;
	TriglavPlugInPropertyGetStringMaxLengthProc						getStringMaxLengthProc;

}	TriglavPlugInPropertyService2;


//	プラグインサービススイート
typedef	struct	_TriglavPlugInServiceSuite
{
	TriglavPlugInStringService*										stringService;
	TriglavPlugInBitmapService*										bitmapService;
	TriglavPlugInOffscreenService*									offscreenService;
	void*															reserved1;
	TriglavPlugInPropertyService*									propertyService;
	void*															reserved2;
	void*															reserved3;
	void*															reserved4;
	TriglavPlugInOffscreenService2*									offscreenService2;
	TriglavPlugInPropertyService2*									propertyService2;
	
	void*	reserved[256-10];
}	TriglavPlugInServiceSuite;

#pragma TRIGLAV_PLUGIN_RESETPACK
TRIGLAV_PLUGIN_EXTERN_C_END

#endif
