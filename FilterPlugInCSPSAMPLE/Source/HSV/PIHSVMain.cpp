//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#include "TriglavPlugInSDK/TriglavPlugInSDK.h"
#include "PlugInCommon/PIHSVFilter.h"
#include <vector>

typedef	unsigned char	BYTE;

static	const int kItemKeyHue						= 1;
static	const int kItemKeySaturation				= 2;
static	const int kItemKeyValue						= 3;

static	const int kStringIDFilterCategoryName		= 101;
static	const int kStringIDFilterName				= 102;
static	const int kStringIDItemCaptionHue			= 103;
static	const int kStringIDItemCaptionSaturation	= 104;
static	const int kStringIDItemCaptionValue			= 105;

struct	HSVFilterInfo
{
	int		hue;
	int		saturation;
	int		value;

	TriglavPlugInPropertyService*	pPropertyService;
};

//	プロパティコールバック
static void TRIGLAV_PLUGIN_CALLBACK TriglavPlugInFilterPropertyCallBack( TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data )
{
	(*result)	= kTriglavPlugInPropertyCallBackResultNoModify;

	HSVFilterInfo*	pFilterInfo	= static_cast<HSVFilterInfo*>(data);
	if( pFilterInfo != NULL )
	{
		TriglavPlugInPropertyService*	pPropertyService	= (*pFilterInfo).pPropertyService;
		if( pPropertyService != NULL )
		{
			if( notify == kTriglavPlugInPropertyCallBackNotifyValueChanged )
			{
				TriglavPlugInInt	value;
				(*(*pFilterInfo).pPropertyService).getIntegerValueProc(&value,propertyObject,itemKey);

				if( itemKey == kItemKeyHue )
				{
					if( (*pFilterInfo).hue != value )
					{
						(*pFilterInfo).hue	= value;
						(*result)	= kTriglavPlugInPropertyCallBackResultModify;
					}
				}
				else if( itemKey == kItemKeySaturation )
				{
					if( (*pFilterInfo).saturation != value )
					{
						(*pFilterInfo).saturation	= value;
						(*result)	= kTriglavPlugInPropertyCallBackResultModify;
					}
				}
				else if( itemKey == kItemKeyValue )
				{
					if( (*pFilterInfo).value != value )
					{
						(*pFilterInfo).value	= value;
						(*result)	= kTriglavPlugInPropertyCallBackResultModify;
					}
				}
			}
		}
	}
}

//	プラグインメイン
void TRIGLAV_PLUGIN_API TriglavPluginCall( TriglavPlugInInt* result, TriglavPlugInPtr* data, TriglavPlugInInt selector, TriglavPlugInServer* pluginServer, TriglavPlugInPtr reserved )
{
	*result	= kTriglavPlugInCallResultFailed;
	if( pluginServer != NULL )
	{
		if( selector == kTriglavPlugInSelectorModuleInitialize )
		{
			//	プラグインの初期化
			TriglavPlugInModuleInitializeRecord*	pModuleInitializeRecord	= (*pluginServer).recordSuite.moduleInitializeRecord;
			TriglavPlugInStringService*				pStringService			= (*pluginServer).serviceSuite.stringService;
			if( pModuleInitializeRecord != NULL && pStringService != NULL )
			{
				TriglavPlugInInt	hostVersion;
				(*pModuleInitializeRecord).getHostVersionProc(&hostVersion,(*pluginServer).hostObject);
				if( hostVersion >= kTriglavPlugInNeedHostVersion )
				{
					TriglavPlugInStringObject	moduleID	= NULL;
					const char*					moduleIDString	= "A9EE0802-84E7-4847-87D8-9EBAC916EEE4";
					(*pStringService).createWithAsciiStringProc(&moduleID,moduleIDString,static_cast<TriglavPlugInInt>(::strlen(moduleIDString)));
					(*pModuleInitializeRecord).setModuleIDProc((*pluginServer).hostObject,moduleID);
					(*pModuleInitializeRecord).setModuleKindProc((*pluginServer).hostObject,kTriglavPlugInModuleSwitchKindFilter);
					(*pStringService).releaseProc(moduleID);

					HSVFilterInfo*	pFilterInfo	= new HSVFilterInfo;
					*data	= pFilterInfo;
					*result	= kTriglavPlugInCallResultSuccess;
				}
			}
		}
		else if( selector == kTriglavPlugInSelectorModuleTerminate )
		{
			//	プラグインの終了処理
			HSVFilterInfo*	pFilterInfo	= static_cast<HSVFilterInfo*>(*data);
			delete pFilterInfo;
			*data	= NULL;
			*result	= kTriglavPlugInCallResultSuccess;
		}
		else if( selector == kTriglavPlugInSelectorFilterInitialize )
		{
			//	フィルタの初期化
			TriglavPlugInRecordSuite*				pRecordSuite			= &(*pluginServer).recordSuite;
			TriglavPlugInHostObject					hostObject				= (*pluginServer).hostObject;
			TriglavPlugInStringService*				pStringService			= (*pluginServer).serviceSuite.stringService;
			TriglavPlugInPropertyService*			pPropertyService		= (*pluginServer).serviceSuite.propertyService;
			if( TriglavPlugInGetFilterInitializeRecord(pRecordSuite) != NULL && pStringService != NULL && pPropertyService != NULL )
			{
				//	フィルタカテゴリ名とフィルタ名の設定
				TriglavPlugInStringObject	filterCategoryName	= NULL;
				TriglavPlugInStringObject	filterName			= NULL;
				(*pStringService).createWithStringIDProc(&filterCategoryName,kStringIDFilterCategoryName,(*pluginServer).hostObject);
				(*pStringService).createWithStringIDProc(&filterName,kStringIDFilterName,(*pluginServer).hostObject);
				
				TriglavPlugInFilterInitializeSetFilterCategoryName(pRecordSuite,hostObject,filterCategoryName,'c');
				TriglavPlugInFilterInitializeSetFilterName(pRecordSuite,hostObject,filterName,'h');
				(*pStringService).releaseProc(filterCategoryName);
				(*pStringService).releaseProc(filterName);

				//	プレビュー
				TriglavPlugInFilterInitializeSetCanPreview(pRecordSuite,hostObject,true);

				//	ターゲット
				TriglavPlugInInt	target[]={kTriglavPlugInFilterTargetKindRasterLayerRGBAlpha};
				TriglavPlugInFilterInitializeSetTargetKinds(pRecordSuite,hostObject,target,1);

				//	プロパティの作成
				TriglavPlugInPropertyObject	propertyObject;
				(*pPropertyService).createProc(&propertyObject);

				//	色相
				TriglavPlugInStringObject	hueCaption	= NULL;
				(*pStringService).createWithStringIDProc(&hueCaption,kStringIDItemCaptionHue,(*pluginServer).hostObject);
				(*pPropertyService).addItemProc(propertyObject,kItemKeyHue,kTriglavPlugInPropertyValueTypeInteger,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,hueCaption,'h');
				(*pPropertyService).setIntegerValueProc(propertyObject,kItemKeyHue,0);
				(*pPropertyService).setIntegerDefaultValueProc(propertyObject,kItemKeyHue,0);
				(*pPropertyService).setIntegerMinValueProc(propertyObject,kItemKeyHue,-180);
				(*pPropertyService).setIntegerMaxValueProc(propertyObject,kItemKeyHue,180);
				(*pStringService).releaseProc(hueCaption);

				//	彩度
				TriglavPlugInStringObject	saturationCaption	= NULL;
				(*pStringService).createWithStringIDProc(&saturationCaption,kStringIDItemCaptionSaturation,(*pluginServer).hostObject);
				(*pPropertyService).addItemProc(propertyObject,kItemKeySaturation,kTriglavPlugInPropertyValueTypeInteger,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,saturationCaption,'s');
				(*pPropertyService).setIntegerValueProc(propertyObject,kItemKeySaturation,0);
				(*pPropertyService).setIntegerDefaultValueProc(propertyObject,kItemKeySaturation,0);
				(*pPropertyService).setIntegerMinValueProc(propertyObject,kItemKeySaturation,-100);
				(*pPropertyService).setIntegerMaxValueProc(propertyObject,kItemKeySaturation,100);
				(*pStringService).releaseProc(saturationCaption);

				//	明度
				TriglavPlugInStringObject	valueCaption	= NULL;
				(*pStringService).createWithStringIDProc(&valueCaption,kStringIDItemCaptionValue,(*pluginServer).hostObject);
				(*pPropertyService).addItemProc(propertyObject,kItemKeyValue,kTriglavPlugInPropertyValueTypeInteger,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,valueCaption,'v');
				(*pPropertyService).setIntegerValueProc(propertyObject,kItemKeyValue,0);
				(*pPropertyService).setIntegerDefaultValueProc(propertyObject,kItemKeyValue,0);
				(*pPropertyService).setIntegerMinValueProc(propertyObject,kItemKeyValue,-100);
				(*pPropertyService).setIntegerMaxValueProc(propertyObject,kItemKeyValue,100);
				(*pStringService).releaseProc(valueCaption);

				//	プロパティの設定
				TriglavPlugInFilterInitializeSetProperty(pRecordSuite,hostObject,propertyObject);
				TriglavPlugInFilterInitializeSetPropertyCallBack(pRecordSuite,hostObject,TriglavPlugInFilterPropertyCallBack,*data);

				//	プロパティの破棄
				(*pPropertyService).releaseProc(propertyObject);

				*result	= kTriglavPlugInCallResultSuccess;
			}
		}
		else if( selector == kTriglavPlugInSelectorFilterTerminate )
		{
			//	フィルタの終了処理
			*result	= kTriglavPlugInCallResultSuccess;
		}
		else if( selector == kTriglavPlugInSelectorFilterRun )
		{
			//	フィルタの実行
			TriglavPlugInRecordSuite*		pRecordSuite		= &(*pluginServer).recordSuite;
			TriglavPlugInOffscreenService*	pOffscreenService	= (*pluginServer).serviceSuite.offscreenService;
			TriglavPlugInPropertyService*	pPropertyService	= (*pluginServer).serviceSuite.propertyService;
			if( TriglavPlugInGetFilterRunRecord(pRecordSuite) != NULL && pOffscreenService != NULL && pPropertyService != NULL )
			{
				TriglavPlugInPropertyObject		propertyObject;
				TriglavPlugInFilterRunGetProperty(pRecordSuite,&propertyObject,(*pluginServer).hostObject);

				TriglavPlugInOffscreenObject	sourceOffscreenObject;
				TriglavPlugInFilterRunGetSourceOffscreen(pRecordSuite,&sourceOffscreenObject,(*pluginServer).hostObject);

				TriglavPlugInOffscreenObject	destinationOffscreenObject;
				TriglavPlugInFilterRunGetDestinationOffscreen(pRecordSuite,&destinationOffscreenObject,(*pluginServer).hostObject);

				TriglavPlugInRect				selectAreaRect;
				TriglavPlugInFilterRunGetSelectAreaRect(pRecordSuite,&selectAreaRect,(*pluginServer).hostObject);

				TriglavPlugInOffscreenObject	selectAreaOffscreenObject;
				TriglavPlugInFilterRunGetSelectAreaOffscreen(pRecordSuite,&selectAreaOffscreenObject,(*pluginServer).hostObject);

				TriglavPlugInInt	r,g,b;
				(*pOffscreenService).getRGBChannelIndexProc(&r,&g,&b,destinationOffscreenObject);
				
				TriglavPlugInInt	blockRectCount;				
				(*pOffscreenService).getBlockRectCountProc(&blockRectCount,destinationOffscreenObject,&selectAreaRect);
				
				std::vector<TriglavPlugInRect>	blockRects;
				blockRects.resize(blockRectCount);
				for( TriglavPlugInInt i=0; i<blockRectCount; ++i )
				{
					(*pOffscreenService).getBlockRectProc(&blockRects[i],i,destinationOffscreenObject,&selectAreaRect);
				}

				TriglavPlugInFilterRunSetProgressTotal(pRecordSuite,(*pluginServer).hostObject,blockRectCount);

				HSVFilterInfo*	pFilterInfo			= static_cast<HSVFilterInfo*>(*data);
				(*pFilterInfo).pPropertyService		= pPropertyService;
				(*pFilterInfo).hue					= 0;
				(*pFilterInfo).saturation			= 0;
				(*pFilterInfo).value				= 0;

				bool	restart		= true;

				int		hue			= 0;
				int		saturation	= 0;
				int		value		= 0;
				
				TriglavPlugInInt	blockIndex	= 0;
				while( true )
				{
					if( restart )
					{
						restart	= false;

						TriglavPlugInInt	processResult;
						TriglavPlugInFilterRunProcess(pRecordSuite,&processResult,(*pluginServer).hostObject,kTriglavPlugInFilterRunProcessStateStart);
						if( processResult == kTriglavPlugInFilterRunProcessResultExit ){ break; }

						if( (*pFilterInfo).hue != 0 || (*pFilterInfo).saturation != 0 || (*pFilterInfo).value != 0 )
						{
							blockIndex			= 0;

							static	const int	cnHSVHFilterMax	= 6*32768;
							static	const int	cnHSVSFilterMax	= 32768;
							static	const int	cnHSVVFilterMax	= 32768;

							hue			= (*pFilterInfo).hue;
							saturation	= (*pFilterInfo).saturation;
							value		= (*pFilterInfo).value;
							if( hue < 0 ){ hue+=360; }
							hue			= hue*cnHSVHFilterMax/360;
							saturation	= saturation*cnHSVSFilterMax/100;
							value		= value*cnHSVVFilterMax/100;
						}				
						else
						{
							//	何もしない
							blockIndex	= blockRectCount;
							TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pRecordSuite,(*pluginServer).hostObject,&selectAreaRect);
						}
					}
					
					if( blockIndex < blockRectCount )
					{
						TriglavPlugInFilterRunSetProgressDone(pRecordSuite,(*pluginServer).hostObject,blockIndex);

						TriglavPlugInRect	blockRect	= blockRects[blockIndex];
						TriglavPlugInPoint	pos;
						pos.x	= blockRect.left;
						pos.y	= blockRect.top;
						TriglavPlugInRect	tempRect;

						TriglavPlugInPtr	dstImageAddress;
						TriglavPlugInInt	dstImageRowBytes;
						TriglavPlugInInt	dstImagePixelBytes;
						(*pOffscreenService).getBlockImageProc(&dstImageAddress,&dstImageRowBytes,&dstImagePixelBytes,&tempRect,destinationOffscreenObject,&pos);
						
						TriglavPlugInPtr	dstAlphaAddress;
						TriglavPlugInInt	dstAlphaRowBytes;
						TriglavPlugInInt	dstAlphaPixelBytes;
						(*pOffscreenService).getBlockAlphaProc(&dstAlphaAddress,&dstAlphaRowBytes,&dstAlphaPixelBytes,&tempRect,destinationOffscreenObject,&pos);

						if( dstImageAddress != NULL && dstAlphaAddress != NULL )
						{
							if( selectAreaOffscreenObject == NULL )
							{
								BYTE*		pDstImageAddressY	= static_cast<BYTE*>(dstImageAddress);
								BYTE*		pDstAlphaAddressY	= static_cast<BYTE*>(dstAlphaAddress);
								for( int y=blockRect.top; y<blockRect.bottom; ++y )
								{
									BYTE*		pDstImageAddressX	= pDstImageAddressY;
									BYTE*		pDstAlphaAddressX	= pDstAlphaAddressY;
									for( int x=blockRect.left; x<blockRect.right; ++x )
									{
										if( *pDstAlphaAddressX > 0 )
										{
											PIHSVFilter::SetHSV8(pDstImageAddressX[r],pDstImageAddressX[g],pDstImageAddressX[b],hue,saturation,value);
										}
										pDstImageAddressX	+= dstImagePixelBytes;
										pDstAlphaAddressX	+= dstAlphaPixelBytes;
									}
									pDstImageAddressY	+= dstImageRowBytes;
									pDstAlphaAddressY	+= dstAlphaRowBytes;
								}
							}
							else
							{
								TriglavPlugInPtr	selectAddress;
								TriglavPlugInInt	selectRowBytes;
								TriglavPlugInInt	selectPixelBytes;
								(*pOffscreenService).getBlockSelectAreaProc(&selectAddress,&selectRowBytes,&selectPixelBytes,&tempRect,selectAreaOffscreenObject,&pos);

								if( selectAddress != NULL )
								{
									BYTE*		pDstImageAddressY	= static_cast<BYTE*>(dstImageAddress);
									BYTE*		pDstAlphaAddressY	= static_cast<BYTE*>(dstAlphaAddress);
									const BYTE*	pSelectAddressY		= static_cast<const BYTE*>(selectAddress);
									for( int y=blockRect.top; y<blockRect.bottom; ++y )
									{
										BYTE*		pDstImageAddressX	= pDstImageAddressY;
										BYTE*		pDstAlphaAddressX	= pDstAlphaAddressY;
										const BYTE*	pSelectAddressX		= pSelectAddressY;
										for( int x=blockRect.left; x<blockRect.right; ++x )
										{
											if( *pDstAlphaAddressX > 0 )
											{
												if( *pSelectAddressX == 255 )
												{
													PIHSVFilter::SetHSV8(pDstImageAddressX[r],pDstImageAddressX[g],pDstImageAddressX[b],hue,saturation,value);
												}
												else if( *pSelectAddressX != 0 )
												{
													PIHSVFilter::SetHSV8Mask(pDstImageAddressX[r],pDstImageAddressX[g],pDstImageAddressX[b],*pSelectAddressX,hue,saturation,value);
												}
											}

											pDstImageAddressX	+= dstImagePixelBytes;
											pDstAlphaAddressX	+= dstAlphaPixelBytes;
											pSelectAddressX		+= selectPixelBytes;
										}
										pDstImageAddressY	+= dstImageRowBytes;
										pDstAlphaAddressY	+= dstAlphaRowBytes;
										pSelectAddressY		+= selectRowBytes;
									}
								}
							}
						}
						TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pRecordSuite,(*pluginServer).hostObject,&blockRect);
						++blockIndex;
					}
					
					TriglavPlugInInt	processResult;
					if( blockIndex < blockRectCount )
					{
						TriglavPlugInFilterRunProcess(pRecordSuite,&processResult,(*pluginServer).hostObject,kTriglavPlugInFilterRunProcessStateContinue);
					}
					else
					{
						TriglavPlugInFilterRunSetProgressDone(pRecordSuite,(*pluginServer).hostObject,blockIndex);
						TriglavPlugInFilterRunProcess(pRecordSuite,&processResult,(*pluginServer).hostObject,kTriglavPlugInFilterRunProcessStateEnd);
					}
					if( processResult == kTriglavPlugInFilterRunProcessResultRestart )
					{
						restart	= true;
					}
					else if( processResult == kTriglavPlugInFilterRunProcessResultExit )
					{
						break;
					}
				}
				*result	= kTriglavPlugInCallResultSuccess;
			}
		}
	}
	return;
}

