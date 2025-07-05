//! TriglavPlugIn SDK
//! Copyright (c) CELSYS Inc.
//! All Rights Reserved.

#include "TriglavPlugInSDK/TriglavPlugInSDK.h"
#include "PlugInCommon/PIHSVFilter.h"
#include<assert.h>
#include <iostream>
#include <vector>
#include<thread>

typedef	unsigned char	BYTE;

static	const int kItemKeyHue						= 1;
static	const int kItemKeySaturation				= 2;
static  const int kItemKeyValue                     = 3;
static	const int kItemKeyTest  					= 4;

//本地化字符串id
static	const int kStringIDFilterCategoryName		= 101;//插件的集合名（插件的上级菜单名）
static	const int kStringIDFilterName				= 102;//滤镜插件名
static	const int kStringIDItemCaptionHue			= 103;//色相
static	const int kStringIDItemCaptionSaturation	= 104;//饱和度
static	const int kStringIDItemCaptionValue			= 105;//明度

struct	HSVFilterInfo
{
	int		hue;
	int		saturation;
	int		value;

	TriglavPlugInPropertyService*	pPropertyService;
};



bool UpdateView(HSVFilterInfo* pFilterInfo);
//	プロパティコールバック
//属性回调
static void TRIGLAV_PLUGIN_CALLBACK TriglavPlugInFilterPropertyCallBack( TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data )
{
    
    static int callcount=0;
    std::cout<<"hsv callback call count"<<++callcount<<std::endl;
    
    
	(*result)	= kTriglavPlugInPropertyCallBackResultNoModify;

    
	HSVFilterInfo*	pFilterInfo	= static_cast<HSVFilterInfo*>(data);
    
	if( pFilterInfo != NULL )
	{
		TriglavPlugInPropertyService*	pPropertyService	= (*pFilterInfo).pPropertyService;
        
        
		if( pPropertyService != NULL )
		{
            
   
            
            //如果属性修改了
            //这个是告知参数在csp中被修改了
			if( notify == kTriglavPlugInPropertyCallBackNotifyValueChanged )
			{
                
                
                
                
                
                
                
				TriglavPlugInInt	value;
				(*(*pFilterInfo).pPropertyService).getIntegerValueProc(&value,propertyObject,itemKey);

				if( itemKey == kItemKeyHue )
				{
					if( (*pFilterInfo).hue != value )
					{
                        std::cout<<"hue"<<value<<"id"<<   std::this_thread::get_id()<<std::endl;
//                        abort();
                        //测试显示这个数值是比较连续发送的。
						(*pFilterInfo).hue = value;
                        //这个是告知画面结果有改动
						(*result)	= kTriglavPlugInPropertyCallBackResultModify;
                        
                        
                        
//                        assert(false&&("slidertest"));
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
//                if((*result)    == kTriglavPlugInPropertyCallBackResultModify)
//                    UpdateView(pFilterInfo);
                
			}
		}
	}
}



//需要的状态
bool filterWorking=false;

TriglavPlugInHostObject workingHostObject=0;

TriglavPlugInRecordSuite*        pWorkingRecordSuite    =NULL;
TriglavPlugInOffscreenService*    pWorkingOffscreenService  =NULL;
TriglavPlugInPropertyService*    pWorkingPropertyService  =NULL;

                TriglavPlugInPropertyObject  workingPropertyObject=NULL;
                TriglavPlugInOffscreenObject    workingSourceOffscreenObject=NULL;
               TriglavPlugInOffscreenObject    workingDestinationOffscreenObject=NULL;
                TriglavPlugInRect                workingSelectAreaRect;
// 有这个选曲离屏纹理不就够了吗，矩形是用来优化的吗？
//我只选几像素也没看他流畅变色啊 傻卵日本程序圆
            TriglavPlugInOffscreenObject    workingSelectAreaOffscreenObject=NULL;

//及时处理退出绘制的信息
bool UpdateView(HSVFilterInfo* pFilterInfo)
{
    if(!filterWorking)
        return false;
    
    auto swap=workingDestinationOffscreenObject;
    workingDestinationOffscreenObject=workingSourceOffscreenObject;
    workingSourceOffscreenObject=swap;
    
    TriglavPlugInInt    processResult;

//    TriglavPlugInFilterRunProcess(pWorkingRecordSuite,&processResult,workingHostObject,kTriglavPlugInFilterRunProcessStateStart);
    int64_t hue,saturation,value;
    
    uint64_t blockIndex=0;
    //获取离屏纹理所支持的rgb通道
    TriglavPlugInInt    r,g,b;
    (*pWorkingOffscreenService).getRGBChannelIndexProc(&r,&g,&b,workingDestinationOffscreenObject);
    //获取矩形区块？ 啊？？
    TriglavPlugInInt    blockRectCount;
    (*pWorkingOffscreenService).getBlockRectCountProc(&blockRectCount,workingDestinationOffscreenObject,&workingSelectAreaRect);
    
    {
        
        
        
        
        
        

        {
            //开始滤镜的时候告诉csp滤镜过程开始了
//            TriglavPlugInInt    processResult;
//            TriglavPlugInFilterRunProcess(pWorkingRecordSuite,&processResult,workingHostObject,kTriglavPlugInFilterRunProcessStateStart);
//            if( processResult == kTriglavPlugInFilterRunProcessResultExit ){ return false; }

            if( (*pFilterInfo).hue != 0 || (*pFilterInfo).saturation != 0 || (*pFilterInfo).value != 0 )
            {
                //如果有值（画面有变动），则。。
                blockIndex            = 0;

                static    const int    cnHSVHFilterMax    = 6*32768;//h 最大值
                static    const int    cnHSVSFilterMax    = 32768;//s最大值
                static    const int    cnHSVVFilterMax    = 32768;//v最大值

                hue            = (*pFilterInfo).hue;
                saturation    = (*pFilterInfo).saturation;
                value        = (*pFilterInfo).value;
                if( hue < 0 ){ hue+=360; }
                hue            = hue*cnHSVHFilterMax/360;//在用整数表示小数 实际
                saturation    = saturation*cnHSVSFilterMax/100;//-1～1
                value        = value*cnHSVVFilterMax/100;//-1～1
            }
            else
            {
                //    何もしない
                blockIndex    = blockRectCount;
                //没有参数，刷新画布？
                TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pWorkingRecordSuite,workingHostObject,&workingSelectAreaRect);
            }
        }
        uint64_t datacount=0;
        

        
        
        std::vector<TriglavPlugInRect>    blockRects;
        blockRects.resize(blockRectCount);
        for( TriglavPlugInInt i=0; i<blockRectCount; ++i )
        {
            (*pWorkingOffscreenService).getBlockRectProc(&blockRects[i],i,workingDestinationOffscreenObject,&workingSelectAreaRect);
        }
        
        
        //HSV OFFSCREEN
        std::cout<<"HSV OFFSCREEN createPlaneProc:"<<(void*)(*pWorkingOffscreenService).createPlaneProc<<std::endl;
        std::cout<<"HSV OFFSCREEN retainProc:"<<(void*)(*pWorkingOffscreenService).retainProc<<std::endl;
        std::cout<<"HSV OFFSCREEN releaseProc:"<<(void*)(*pWorkingOffscreenService).releaseProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getWidthProc:"<<(void*)(*pWorkingOffscreenService).getWidthProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getHeightProc:"<<(void*)(*pWorkingOffscreenService).getHeightProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getRectProc:"<<(void*)(*pWorkingOffscreenService).getRectProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getExtentRectProc:"<<(void*)(*pWorkingOffscreenService).getExtentRectProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getChannelOrderProc:"<<(void*)(*pWorkingOffscreenService).getChannelOrderProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getRGBChannelIndexProc:"<<(void*)(*pWorkingOffscreenService).getRGBChannelIndexProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getCMYKChannelIndexProc:"<<(void*)(*pWorkingOffscreenService).getCMYKChannelIndexProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getBlockRectCountProc:"<<(void*)(*pWorkingOffscreenService).getBlockRectCountProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getBlockRectProc:"<<(void*)(*pWorkingOffscreenService).getBlockRectProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getBlockImageProc:"<<(void*)(*pWorkingOffscreenService).getBlockImageProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getBlockAlphaProc:"<<(void*)(*pWorkingOffscreenService).getBlockAlphaProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getBlockSelectAreaProc:"<<(void*)(*pWorkingOffscreenService).getBlockSelectAreaProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getBlockPlaneProc:"<<(void*)(*pWorkingOffscreenService).getBlockPlaneProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getTileWidthProc:"<<(void*)(*pWorkingOffscreenService).getTileWidthProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getTileHeightProc:"<<(void*)(*pWorkingOffscreenService).getTileHeightProc<<std::endl;
        std::cout<<"HSV OFFSCREEN getBitmapProc:"<<(void*)(*pWorkingOffscreenService).getBitmapProc<<std::endl;
        std::cout<<"HSV OFFSCREEN setBitmapProc:"<<(void*)(*pWorkingOffscreenService).setBitmapProc<<std::endl;
        
        
        while(blockIndex < blockRectCount )
        {
            //区块索引小于区块矩形数
            //done？？
            TriglavPlugInFilterRunSetProgressDone(pWorkingRecordSuite,workingHostObject,blockIndex);

            TriglavPlugInRect    blockRect    = blockRects[blockIndex];
            TriglavPlugInPoint    pos;
            pos.x    = blockRect.left;
            pos.y    = blockRect.top;
            TriglavPlugInRect    tempRect;

           
            TriglavPlugInPtr    dstImageAddress; //目标图片地址
            TriglavPlugInInt    dstImageRowBytes; //目标图片一行的数据字节数
            TriglavPlugInInt    dstImagePixelBytes;//一像素的字节数
            (*pWorkingOffscreenService).getBlockImageProc(&dstImageAddress,&dstImageRowBytes,&dstImagePixelBytes,&tempRect,workingDestinationOffscreenObject,&pos);
            
            TriglavPlugInPtr    dstAlphaAddress;
            TriglavPlugInInt    dstAlphaRowBytes;
            TriglavPlugInInt    dstAlphaPixelBytes;
            (*pWorkingOffscreenService).getBlockAlphaProc(&dstAlphaAddress,&dstAlphaRowBytes,&dstAlphaPixelBytes,&tempRect,workingDestinationOffscreenObject,&pos);

            
            //如果目标区域有内容，则进行老实巴交单线程逐像素绘制，  虽然这是个插件demo，但如果程序里是这么写的的话那csp你是真的牛逼
            if( dstImageAddress != NULL && dstAlphaAddress != NULL )
            {
                //测试得出一个图层是会动态申请内存的，区块内存不连续
                std::cout<<"hsv index:"<<blockIndex<<",dstImageAddress"<<dstImageAddress<<std::endl;
                std::cout<<"hsv index:"<<blockIndex<<",dstImageRowBytes"<<dstImageRowBytes<<std::endl;
                std::cout<<"hsv index:"<<blockIndex<<",dstImagePixelBytes"<<dstImagePixelBytes<<std::endl;
                
                
                if( workingSelectAreaOffscreenObject == NULL )
                {
                    BYTE*        pDstImageAddressY    = static_cast<BYTE*>(dstImageAddress);
                    BYTE*        pDstAlphaAddressY    = static_cast<BYTE*>(dstAlphaAddress);
                    for( int y=blockRect.top; y<blockRect.bottom; ++y )
                    {
                        BYTE*        pDstImageAddressX    = pDstImageAddressY;
                        BYTE*        pDstAlphaAddressX    = pDstAlphaAddressY;
                        for( int x=blockRect.left; x<blockRect.right; ++x )
                        {
                            //？？单线程逐像素？
                            if( *pDstAlphaAddressX > 0 )
                            {
                                PIHSVFilter::SetHSV8(pDstImageAddressX[r],pDstImageAddressX[g],pDstImageAddressX[b],hue,saturation,value);
                    
                                datacount++;
                            }
                            pDstImageAddressX    += dstImagePixelBytes;
                            pDstAlphaAddressX    += dstAlphaPixelBytes;
                        }
                        pDstImageAddressY    += dstImageRowBytes;
                        pDstAlphaAddressY    += dstAlphaRowBytes;
                    }
                }
                else
                {
                    TriglavPlugInPtr    selectAddress;
                    TriglavPlugInInt    selectRowBytes;
                    TriglavPlugInInt    selectPixelBytes;
                    (*pWorkingOffscreenService).getBlockSelectAreaProc(&selectAddress,&selectRowBytes,&selectPixelBytes,&tempRect,workingSelectAreaOffscreenObject,&pos);

                    if( selectAddress != NULL )
                    {
                        BYTE*        pDstImageAddressY    = static_cast<BYTE*>(dstImageAddress);
                        BYTE*        pDstAlphaAddressY    = static_cast<BYTE*>(dstAlphaAddress);
                        const BYTE*    pSelectAddressY        = static_cast<const BYTE*>(selectAddress);
                        for( int y=blockRect.top; y<blockRect.bottom; ++y )
                        {
                            BYTE*        pDstImageAddressX    = pDstImageAddressY;
                            BYTE*        pDstAlphaAddressX    = pDstAlphaAddressY;
                            const BYTE*    pSelectAddressX        = pSelectAddressY;
                            for( int x=blockRect.left; x<blockRect.right; ++x )
                            {
                                if( *pDstAlphaAddressX > 0 )
                                {
                                    if( *pSelectAddressX == 255 )
                                    {
                                        PIHSVFilter::SetHSV8(pDstImageAddressX[r],pDstImageAddressX[g],pDstImageAddressX[b],hue,saturation,value);
                                        datacount++;
                                    }
                                    else if( *pSelectAddressX != 0 )
                                    {
                                        PIHSVFilter::SetHSV8Mask(pDstImageAddressX[r],pDstImageAddressX[g],pDstImageAddressX[b],*pSelectAddressX,hue,saturation,value);
                                        datacount++;
                                    }
                                }

                                pDstImageAddressX    += dstImagePixelBytes;
                                pDstAlphaAddressX    += dstAlphaPixelBytes;
                                pSelectAddressX        += selectPixelBytes;
                            }
                            pDstImageAddressY    += dstImageRowBytes;
                            pDstAlphaAddressY    += dstAlphaRowBytes;
                            pSelectAddressY        += selectRowBytes;
                        }
                    }
                }
            }
            
            
//                        TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pRecordSuite,(*pluginServer).hostObject,&blockRect);
            ++blockIndex;
        }
        //处理完后刷新画面
        
        TriglavPlugInFilterRunSetProgressDone(pWorkingRecordSuite,workingHostObject,blockIndex);
        TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pWorkingRecordSuite,workingHostObject,&workingSelectAreaRect);
        
        std::cout<<"hsv datacount:"<<datacount<<std::endl;
     
    }
    
    
    
    
}



//	プラグインメイン
// plugin main？
void TRIGLAV_PLUGIN_API TriglavPluginCall( TriglavPlugInInt* result, TriglavPlugInPtr* data, TriglavPlugInInt selector, TriglavPlugInServer* pluginServer, TriglavPlugInPtr reserved )
{
	*result	= kTriglavPlugInCallResultFailed;
	if( pluginServer != NULL )
	{
		if( selector == kTriglavPlugInSelectorModuleInitialize )
		{
            std::cout<<"hse ModuleInitialize pluginServer:"<<pluginServer<<",hostObject"<<(void*)((*pluginServer).hostObject)<<std::endl;
            
            
            
            //模块初始化
			//	プラグインの初期化
			TriglavPlugInModuleInitializeRecord*	pModuleInitializeRecord	= (*pluginServer).recordSuite.moduleInitializeRecord;
			TriglavPlugInStringService*				pStringService			= (*pluginServer).serviceSuite.stringService;
			if( pModuleInitializeRecord != NULL && pStringService != NULL )
			{
                
				TriglavPlugInInt	hostVersion;
				(*pModuleInitializeRecord).getHostVersionProc(&hostVersion,(*pluginServer).hostObject);
                
				if( hostVersion >= kTriglavPlugInNeedHostVersion )
				{
                    //如果csp版本受支持
					TriglavPlugInStringObject	moduleID	= NULL;
					const char*					moduleIDString	= "A9EE0802-84E7-4847-87D8-9EBAC916EEE4";//防止重复加载的uuid
                    
                    //调用后在csp中创建模块实例并返回模块id
					(*pStringService).createWithAsciiStringProc(&moduleID,moduleIDString,static_cast<TriglavPlugInInt>(::strlen(moduleIDString)));
                    std::cout<<"hsv module id:"<<moduleID<<std::endl;
                    
					(*pModuleInitializeRecord).setModuleIDProc((*pluginServer).hostObject,moduleID);
					(*pModuleInitializeRecord).setModuleKindProc((*pluginServer).hostObject,kTriglavPlugInModuleSwitchKindFilter);//设置模块种类，得看看文档
					
                    //为什么这里有release
                    (*pStringService).releaseProc(moduleID);

					HSVFilterInfo*	pFilterInfo	= new HSVFilterInfo;
					*data	= pFilterInfo;
					*result	= kTriglavPlugInCallResultSuccess;
				}
			}
		}
		else if( selector == kTriglavPlugInSelectorModuleTerminate )
		{
            //模块终止时的行为
			//	プラグインの終了処理
            std::cout<<"hsv module Terminate"<<std::endl;
			HSVFilterInfo*	pFilterInfo	= static_cast<HSVFilterInfo*>(*data);
			delete pFilterInfo;
			*data	= NULL;
			*result	= kTriglavPlugInCallResultSuccess;
		}
		else if( selector == kTriglavPlugInSelectorFilterInitialize )
		{
            std::cout<<"hse FilterInitialize pluginServer:"<<pluginServer<<",hostObject"<<(void*)((*pluginServer).hostObject)<<std::endl;
			//	フィルタの初期化
            //滤镜初始化
            
			TriglavPlugInRecordSuite*				pRecordSuite			= &(*pluginServer).recordSuite;
			TriglavPlugInHostObject					hostObject				= (*pluginServer).hostObject;
			TriglavPlugInStringService*				pStringService			= (*pluginServer).serviceSuite.stringService;
			TriglavPlugInPropertyService*			pPropertyService		= (*pluginServer).serviceSuite.propertyService;
            
            //接口不能为空
			if( TriglavPlugInGetFilterInitializeRecord(pRecordSuite) != NULL && pStringService != NULL && pPropertyService != NULL )
			{
                std::cout<<"hsv filter Initialize  aaaa"<<std::endl;
				//	フィルタカテゴリ名とフィルタ名の設定
                //滤镜组名和滤镜名
				TriglavPlugInStringObject	filterCategoryName	= NULL;
				TriglavPlugInStringObject	filterName			= NULL;
				(*pStringService).createWithStringIDProc(&filterCategoryName,kStringIDFilterCategoryName,(*pluginServer).hostObject);
				(*pStringService).createWithStringIDProc(&filterName,kStringIDFilterName,(*pluginServer).hostObject);
				
				TriglavPlugInFilterInitializeSetFilterCategoryName(pRecordSuite,hostObject,filterCategoryName,'c');
				TriglavPlugInFilterInitializeSetFilterName(pRecordSuite,hostObject,filterName,'h');
				(*pStringService).releaseProc(filterCategoryName);
				(*pStringService).releaseProc(filterName);

				//	プレビュー
                //preview。 煞笔片假名
                //给插件设置是否可以预览
				TriglavPlugInFilterInitializeSetCanPreview(pRecordSuite,hostObject,true);

				//	ターゲット 目标
                //看起来可以设置很多目标，这里只设置了彩色位图的rgba;
                //源码中看起来可用的只能rgba了。看不懂这里设置灰度图之类的意义
				TriglavPlugInInt	target[]={kTriglavPlugInFilterTargetKindRasterLayerRGBAlpha};
				TriglavPlugInFilterInitializeSetTargetKinds(pRecordSuite,hostObject,target,1);

                
				//	プロパティの作成
                //生成可以控制的属性 Property    煞笔片假名
				TriglavPlugInPropertyObject	propertyObject;
				(*pPropertyService).createProc(&propertyObject);

                //这里生成色相滑块//如果插件和csp原始代码相同的话，应该能用现成的曲线
				//	色相
				TriglavPlugInStringObject	hueCaption	= NULL;
				(*pStringService).createWithStringIDProc(&hueCaption,kStringIDItemCaptionHue,(*pluginServer).hostObject);
				(*pPropertyService).addItemProc(propertyObject,kItemKeyHue,kTriglavPlugInPropertyValueTypeInteger,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,hueCaption,'h');
				(*pPropertyService).setIntegerValueProc(propertyObject,kItemKeyHue,0);
				(*pPropertyService).setIntegerDefaultValueProc(propertyObject,kItemKeyHue,0);
				(*pPropertyService).setIntegerMinValueProc(propertyObject,kItemKeyHue,-180);
				(*pPropertyService).setIntegerMaxValueProc(propertyObject,kItemKeyHue,180);
				(*pStringService).releaseProc(hueCaption);//手动释放字符串。。

				//	彩度
                //饱和度属性
				TriglavPlugInStringObject	saturationCaption	= NULL;
				(*pStringService).createWithStringIDProc(&saturationCaption,kStringIDItemCaptionSaturation,(*pluginServer).hostObject);
				(*pPropertyService).addItemProc(propertyObject,kItemKeySaturation,kTriglavPlugInPropertyValueTypeInteger,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,saturationCaption,'s');
				(*pPropertyService).setIntegerValueProc(propertyObject,kItemKeySaturation,0);
				(*pPropertyService).setIntegerDefaultValueProc(propertyObject,kItemKeySaturation,0);
				(*pPropertyService).setIntegerMinValueProc(propertyObject,kItemKeySaturation,-100);
				(*pPropertyService).setIntegerMaxValueProc(propertyObject,kItemKeySaturation,100);
				(*pStringService).releaseProc(saturationCaption);

                //    明度
                //明度属性
                TriglavPlugInStringObject    valueCaption    = NULL;
                (*pStringService).createWithStringIDProc(&valueCaption,kStringIDItemCaptionValue,(*pluginServer).hostObject);
                (*pPropertyService).addItemProc(propertyObject,kItemKeyValue,kTriglavPlugInPropertyValueTypeInteger,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,valueCaption,'v');
                (*pPropertyService).setIntegerValueProc(propertyObject,kItemKeyValue,0);
                (*pPropertyService).setIntegerDefaultValueProc(propertyObject,kItemKeyValue,0);
                (*pPropertyService).setIntegerMinValueProc(propertyObject,kItemKeyValue,-100);
                (*pPropertyService).setIntegerMaxValueProc(propertyObject,kItemKeyValue,100);
                (*pStringService).releaseProc(valueCaption);
                
                
                
                TriglavPlugInPropertyService2* pService2=(*pluginServer).serviceSuite.propertyService2;
				//	测试属性
				TriglavPlugInStringObject	testCaption	= NULL;
                (*pStringService).createWithStringIDProc(&testCaption,kStringIDItemCaptionValue,(*pluginServer).hostObject);
                (*pPropertyService).addItemProc(propertyObject,kItemKeyTest,kTriglavPlugInPropertyValueTypePoint,kTriglavPlugInPropertyInputKindCanvas,kTriglavPlugInPropertyInputKindCanvas,testCaption,'t');
                TriglavPlugInPoint pZero={0,0};
                TriglavPlugInPoint pMax={255,255};
                (*pService2).setPointValueProc(propertyObject,kItemKeyTest,&pZero);
				(*pService2).setPointDefaultValueProc(propertyObject,kItemKeyTest,&pZero);
				(*pService2).setPointMinValueProc(propertyObject,kItemKeyTest,&pZero);
				(*pService2).setPointMaxValueProc(propertyObject,kItemKeyTest,&pMax);
				(*pStringService).releaseProc(testCaption);

				//	プロパティの設定
                //property的设置
                //？我滑块呢？kTriglavPlugInPropertyValueTypeInteger？？
				TriglavPlugInFilterInitializeSetProperty(pRecordSuite,hostObject,propertyObject);
				TriglavPlugInFilterInitializeSetPropertyCallBack(pRecordSuite,hostObject,TriglavPlugInFilterPropertyCallBack,*data);

				//	プロパティの破棄
                //释放propertyobj 应该是减少引用计数吧。。这东西应该不会这么快就从内存里移除
				(*pPropertyService).releaseProc(propertyObject);

				*result	= kTriglavPlugInCallResultSuccess;
			}
		}
		else if( selector == kTriglavPlugInSelectorFilterTerminate )
		{
			//	フィルタの終了処理
            //滤镜终止时的处理
            std::cout<<"hsv filter Terminate"<<std::endl;
			*result	= kTriglavPlugInCallResultSuccess;
		}
		else if( selector == kTriglavPlugInSelectorFilterRun )
		{
            
            std::cout<<"hse FilterRun pluginServer:"<<pluginServer<<",hostObject"<<(void*)((*pluginServer).hostObject)<<std::endl;
			//	フィルタの実行
            //滤镜的执行
            std::cout<<"hsv filter Run"<<std::endl;
			pWorkingRecordSuite		= &(*pluginServer).recordSuite;
			pWorkingOffscreenService	= (*pluginServer).serviceSuite.offscreenService;
			pWorkingPropertyService	= (*pluginServer).serviceSuite.propertyService;
             workingHostObject=(*pluginServer).hostObject;
			//RUNRecord意义？
            if( TriglavPlugInGetFilterRunRecord(pWorkingRecordSuite) != NULL && pWorkingOffscreenService != NULL && pWorkingPropertyService != NULL )
			{
                filterWorking=true;
                
                
                
//				TriglavPlugInPropertyObject		propertyObject;
				TriglavPlugInFilterRunGetProperty(pWorkingRecordSuite,&workingPropertyObject,(*pluginServer).hostObject);

//				TriglavPlugInOffscreenObject	sourceOffscreenObject;
				TriglavPlugInFilterRunGetSourceOffscreen(pWorkingRecordSuite,&workingSourceOffscreenObject,(*pluginServer).hostObject);

//				TriglavPlugInOffscreenObject	destinationOffscreenObject;
				TriglavPlugInFilterRunGetDestinationOffscreen(pWorkingRecordSuite,&workingDestinationOffscreenObject,(*pluginServer).hostObject);

                std::cout<<"HSV GetDestinationOffscreen:"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getDestinationOffscreenProc<<std::endl;
                std::cout<<"HSV GetSourceOffscreen:"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getSourceOffscreenProc<<std::endl;
                std::cout<<"HSV UpdateDestinationOffscreenRectProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->updateDestinationOffscreenRectProc<<std::endl;
                
                
                std::cout<<"HSV GetPropertyProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getPropertyProc<<std::endl;
                std::cout<<"HSV GetCanvasWidthProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getCanvasWidthProc<<std::endl;
                std::cout<<"HSV GetCanvasHeightProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getCanvasHeightProc<<std::endl;
                std::cout<<"HSV GetCanvasResolutionProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getCanvasResolutionProc<<std::endl;
                std::cout<<"HSV GetLayerOriginProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getLayerOriginProc<<std::endl;
                std::cout<<"HSV IsLayerMaskSelectedProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->isLayerMaskSelectedProc<<std::endl;
                std::cout<<"HSV IsAlphaLockedProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->isAlphaLockedProc<<std::endl;
                std::cout<<"HSV GetSelectAreaRectProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getSelectAreaRectProc<<std::endl;
                std::cout<<"HSV HasSelectAreaOffscreenProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->hasSelectAreaOffscreenProc<<std::endl;
                std::cout<<"HSV GetSelectAreaOffscreenProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getSelectAreaOffscreenProc<<std::endl;
                std::cout<<"HSV GetMainColorProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getMainColorProc<<std::endl;
                std::cout<<"HSV GetSubColorProc"<<(void*)(pWorkingRecordSuite)->filterRunRecord->getSubColorProc<<std::endl;
                std::cout<<"HSV GetDrawColor "<<(void*)(pWorkingRecordSuite)->filterRunRecord->getDrawColorProc<<std::endl;
                
                
                
                
                
                
                std::cout<<"HSV HostObject:"<<workingHostObject<<std::endl;
                std::cout<<"HSV workingDestinationOffscreenObject:"<<workingDestinationOffscreenObject<<std::endl;
                std::cout<<"HSV workingSourceOffscreenObject:"<<workingSourceOffscreenObject<<std::endl;

                //？ 为什么选区是矩形？
//				TriglavPlugInRect				selectAreaRect;
				TriglavPlugInFilterRunGetSelectAreaRect(pWorkingRecordSuite,&workingSelectAreaRect,(*pluginServer).hostObject);

                // 有这个选曲离屏纹理不就够了吗，矩形是用来优化的吗？
                //我只选几像素也没看他流畅变色啊 傻卵日本程序圆
//				TriglavPlugInOffscreenObject	selectAreaOffscreenObject;
				TriglavPlugInFilterRunGetSelectAreaOffscreen(pWorkingRecordSuite,&workingSelectAreaOffscreenObject,(*pluginServer).hostObject);

                
                //获取离屏纹理所支持的rgb通道
				TriglavPlugInInt	r,g,b;
				(*pWorkingOffscreenService).getRGBChannelIndexProc(&r,&g,&b,workingDestinationOffscreenObject);
				
                
                //傻卵单线程CSP为了不在滤镜渲染时卡死 就把画面分成了很多方块，每块计算完后就回csp界面里去动一动。
				TriglavPlugInInt	blockRectCount;
				(*pWorkingOffscreenService).getBlockRectCountProc(&blockRectCount,workingDestinationOffscreenObject,&workingSelectAreaRect);
                std::cout<<"hsv filter blockRectCount:"<<blockRectCount<<std::endl;
                
                
                
//				std::vector<TriglavPlugInRect>	blockRects;
//				blockRects.resize(blockRectCount);
//				for( TriglavPlugInInt i=0; i<blockRectCount; ++i )
//				{
//					(*pWorkingOffscreenService).getBlockRectProc(&blockRects[i],i,workingDestinationOffscreenObject,&workingSelectAreaRect);
//				}

                //设置进度
                //还需要设置进度，ai生图是吧
				TriglavPlugInFilterRunSetProgressTotal(pWorkingRecordSuite,workingHostObject,blockRectCount);

                //初始化
				HSVFilterInfo*	pFilterInfo			= static_cast<HSVFilterInfo*>(*data);
				(*pFilterInfo).pPropertyService		= pWorkingPropertyService;
				(*pFilterInfo).hue					= 0;
				(*pFilterInfo).saturation			= 0;
				(*pFilterInfo).value				= 0;

                
                //？？restart干嘛 复位？
				bool	restart		= true;
//
//				int		hue			= 0;
//				int		saturation	= 0;
//				int		value		= 0;
				
				TriglavPlugInInt	blockIndex	= 0;
                std::cout<<"loop entrypoint"<<std::endl;
				while( true )
				{
				
                    if(restart)
                    {
                        restart=false;
                        UpdateView(pFilterInfo);
                        
                        
                        static int callcount=0;
                        std::cout<<"hsv restart count"<<++callcount<<std::endl;
                        
                        
                    }
                    //处理完后刷新画面
                    
                    
//                    TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pRecordSuite,(*pluginServer).hostObject,&selectAreaRect);
                    

                        
                    //处理的状态
					TriglavPlugInInt	processResult;
//					if( blockIndex < blockRectCount )
//					{
//                        //虽然感觉将画面分割成矩形已经是程序员的优化想法了。。但总感觉csp的用户们很可怜
////                        std::cout<<"hsv filter processing:"<< blockIndex<<"of"<<blockRectCount <<std::endl;
//                        std::cout<<"hsv TriglavPlugInFilterRunProcess:"<<(void*) (pRecordSuite)->filterRunRecord->processProc<<std::endl;
//        
//                        //处理到一半，告诉csp处理中
//                        //这里会出现执行到一半的情况就转到csp中等待滑块停下， 傻逼
//                        continue;
//                        //防止软件卡死要定期进入csp里	TriglavPlugInFilterRunProcess(pRecordSuite,&processResult,(*pluginServer).hostObject,kTriglavPlugInFilterRunProcessStateContinue);
//					}
//					else
					{
                        //否则设置处理进度100%
//						TriglavPlugInFilterRunSetProgressDone(pWorkingRecordSuite,workinghostObject,blockIndex);
                        
                        
                        //煞笔csp用这个函数进程，导致循环只会在csp发送restart的时候再次启动
                        //restart发送的时候是煞笔的鼠标停止的时候而不是鼠标移动的时候，太几把傻逼了
                        //注释了的话，会一直执行这个循环，导致csp不能正常运行了，会循环输出done 、 done2所以居然还他妈是在主线层里运行
                        
                        
                        //那可以推断这个函数执行的时候 回到csp中等待新的事件 (pRecordSuite)->filterRunRecord->processProc<<std::endl;
						TriglavPlugInFilterRunProcess(pWorkingRecordSuite,&processResult,workingHostObject,kTriglavPlugInFilterRunProcessStateEnd);
                        
					}
                    
                    
                    //重新开始？？为什么要重新开始？啊？tell me！
					if( processResult == kTriglavPlugInFilterRunProcessResultRestart )
					{
                        //每次收到restart消息的实际，和原来滑块停下才更新画布的时机相同，
                        //或许可以修改restart的调用逻辑来实现画布实时刷新
                        //连续点击的时候不会每次都调用restart
//                        std::cout<<"hsv filter processResult:"<<"restart:"<<i++<<std::endl;
                        
//                        assert(false&&"Result1");
//                        restart    = true;
                        std::cout<<"hsv thread id"<<   std::this_thread::get_id()<<std::endl;
						restart	= true;
//                        assert(false&&"Result2");
					}
                    else if( processResult == kTriglavPlugInFilterRunProcessResultExit )
					{
                        std::cout<<"hsv filter processResult:"<<"exit"<<std::endl;
						break;
					}
				}
				*result	= kTriglavPlugInCallResultSuccess;
			}
		}
	}
	return;
}

