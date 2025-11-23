#include "TriglavPlugInSDK/TriglavPlugInSDK.h"
#include<assert.h>
#include <iostream>
#include <vector>
#include<thread>
#include"Graphic/D3D11Graphic.h"
#include"MotionBlur.h"
#include <chrono>
typedef	unsigned char	BYTE;

static    const int kItemPoint1 = 1;
static    const int kItemPoint2 = 2;
static    const int kItemPreview = 3;
static    const int kItemBlurDirection = 4;
static	const int kItemBlurAlgorithm = 5;

//本地化字符串id
static	const int kStringIDFilterCategoryName = 101;//插件的集合名（插件的上级菜单名）
static	const int kStringIDFilterName = 102;//滤镜插件名
static    const int kStringIDPreview = 103;//预览
static    const int kStringIDBlurDirection = 104;//模糊方向
static    const int kStringIDBlurAlgorithm = 105;//模糊算法

static    const int kStringIDBlurDirection_ForwardBehind = 106;//模糊方向 前后
static    const int kStringIDBlurDirection_Forward = 107;//模糊方向 前
static    const int kStringIDBlurDirection_Behind = 108;//模糊方向 后

static    const int kStringIDBlurAlgorithm_Smooth = 109;//模糊算法 流畅
static	const int kStringIDBlurAlgorithm_Average = 110;//模糊算法 均值

static const char* uuidOfThisPlugin = "4034B383-D830-4D9B-9982-4DB2A006EBC4";



 
struct	HSVFilterInfo
{
    TriglavPlugInPoint  point1;
    TriglavPlugInPoint	point2;
    TriglavPlugInBool bPreview;
    TriglavPlugInPropertyService* pService1;
    TriglavPlugInPropertyService2* pPropertyService;
};



//	プロパティコールバック
//属性回调
static void TRIGLAV_PLUGIN_CALLBACK TriglavPlugInFilterPropertyCallBack(TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data)
{



    (*result) = kTriglavPlugInPropertyCallBackResultNoModify;


    HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(data);

    if (pFilterInfo != NULL)
    {
        TriglavPlugInPropertyService2* pPropertyService = (*pFilterInfo).pPropertyService;


        if (pPropertyService != NULL)
        {



            //如果属性修改了
            //这个是告知参数在csp中被修改了
            if (notify == kTriglavPlugInPropertyCallBackNotifyValueChanged)
            {
                //添加一个计时器防止频繁触发 16ms
                static auto startTimer = std::chrono::system_clock::now();
                auto endTimer = std::chrono::system_clock::now();



                (*(*pFilterInfo).pPropertyService).getPointValueProc(&(*pFilterInfo).point1, propertyObject, kItemPoint1);
                (*(*pFilterInfo).pPropertyService).getPointValueProc(&(*pFilterInfo).point2, propertyObject, kItemPoint2);

                TriglavPlugInBool bPreview;
                (*(*pFilterInfo).pService1).getBooleanValueProc(&bPreview, propertyObject, kItemPreview);

                TriglavPlugInInt direction;
                TriglavPlugInInt algorithm;
                (*(*pFilterInfo).pPropertyService).getEnumerationValueProc((TriglavPlugInInt*)&direction, propertyObject, kItemBlurDirection);
                (*(*pFilterInfo).pPropertyService).getEnumerationValueProc((TriglavPlugInInt*)&algorithm, propertyObject, kItemBlurAlgorithm);


                if (std::chrono::duration_cast<std::chrono::milliseconds>(endTimer - startTimer).count() > max(33, MotionBlur::GetIns().GetLastCostTimeMS() * 2))
                {
                    MotionBlur::GetIns().OnParamChanged((*pFilterInfo).point1, (*pFilterInfo).point2, bPreview, (kBlurDirection)direction, (kBlurAlgorithm)algorithm);
             
                    startTimer = endTimer;
                }
                (*result) = kTriglavPlugInPropertyCallBackResultModify;


            }
        }
    }
}



//	プラグインメイン
// plugin main？
void TRIGLAV_PLUGIN_API TriglavPluginCall(TriglavPlugInInt* result, TriglavPlugInPtr* data, TriglavPlugInInt selector, TriglavPlugInServer* pluginServer, TriglavPlugInPtr reserved)
{
    *result = kTriglavPlugInCallResultFailed;
    if (pluginServer != NULL)
    {
        if (selector == kTriglavPlugInSelectorModuleInitialize)
        {
            //告知CSPMOD.DLL此插件已加载
            auto handle=GetModuleHandle(L"CSPMOD.dll");
            if (handle)
            {
                typedef void(*SetStrDataFunc)(const char* key, const char* value);

                SetStrDataFunc myFunc = (SetStrDataFunc)GetProcAddress(handle, "CSPMOD_SetStrData");
                if (myFunc)
                {
                    myFunc("SmoothFilter_MotionBlur", uuidOfThisPlugin);
                }
            }



            //模块初始化
            //	プラグインの初期化
            TriglavPlugInModuleInitializeRecord* pModuleInitializeRecord = (*pluginServer).recordSuite.moduleInitializeRecord;
            TriglavPlugInStringService* pStringService = (*pluginServer).serviceSuite.stringService;
            if (pModuleInitializeRecord != NULL && pStringService != NULL)
            {

                TriglavPlugInInt	hostVersion;
                (*pModuleInitializeRecord).getHostVersionProc(&hostVersion, (*pluginServer).hostObject);

                if (hostVersion >= kTriglavPlugInNeedHostVersion)
                {
                    //如果csp版本受支持
                    TriglavPlugInStringObject	moduleID = NULL;
                    const char* moduleIDString = uuidOfThisPlugin;//防止重复加载的uuid

                    //调用后在csp中创建模块实例并返回模块id
                    (*pStringService).createWithAsciiStringProc(&moduleID, moduleIDString, static_cast<TriglavPlugInInt>(::strlen(moduleIDString)));

                    (*pModuleInitializeRecord).setModuleIDProc((*pluginServer).hostObject, moduleID);
                    (*pModuleInitializeRecord).setModuleKindProc((*pluginServer).hostObject, kTriglavPlugInModuleSwitchKindFilter);//设置模块种类，得看看文档

                    //为什么这里有release
                    (*pStringService).releaseProc(moduleID);

                    HSVFilterInfo* pFilterInfo = new HSVFilterInfo;
                    *data = pFilterInfo;
                    *result = kTriglavPlugInCallResultSuccess;
                }
            }
        }
        else if (selector == kTriglavPlugInSelectorModuleTerminate)
        {
            //模块终止时的行为
            //	プラグインの終了処理
            HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(*data);
            delete pFilterInfo;
            *data = NULL;
            *result = kTriglavPlugInCallResultSuccess;

            //似乎让动态库卸载时自动释放com指针会出现死锁的问题.
            MotionBlur::PluginModuleCleanUp();


        }
        else if (selector == kTriglavPlugInSelectorFilterInitialize)
        {

            //	フィルタの初期化
            //滤镜初始化

            TriglavPlugInRecordSuite* pRecordSuite = &(*pluginServer).recordSuite;
            TriglavPlugInHostObject					hostObject = (*pluginServer).hostObject;
            TriglavPlugInStringService* pStringService = (*pluginServer).serviceSuite.stringService;
            TriglavPlugInPropertyService* pPropertyService = (*pluginServer).serviceSuite.propertyService;

            //接口不能为空
            if (TriglavPlugInGetFilterInitializeRecord(pRecordSuite) != NULL && pStringService != NULL && pPropertyService != NULL)
            {
                //	フィルタカテゴリ名とフィルタ名の設定
                //滤镜组名和滤镜名
                TriglavPlugInStringObject	filterCategoryName = NULL;
                TriglavPlugInStringObject	filterName = NULL;
                (*pStringService).createWithStringIDProc(&filterCategoryName, kStringIDFilterCategoryName, (*pluginServer).hostObject);
                (*pStringService).createWithStringIDProc(&filterName, kStringIDFilterName, (*pluginServer).hostObject);

                TriglavPlugInFilterInitializeSetFilterCategoryName(pRecordSuite, hostObject, filterCategoryName, 'c');
                TriglavPlugInFilterInitializeSetFilterName(pRecordSuite, hostObject, filterName, 'g');
                (*pStringService).releaseProc(filterCategoryName);
                (*pStringService).releaseProc(filterName);

                //	プレビュー
                //preview。 煞笔片假名
                //给插件设置是否可以预览
                //禁用CSP预览能有效提高流畅性，太神秘了
                TriglavPlugInFilterInitializeSetCanPreview(pRecordSuite, hostObject, false);

                //	ターゲット 目标
                //看起来可以设置很多目标，这里只设置了彩色位图的rgba;
                //源码中看起来可用的只能rgba了。看不懂这里设置灰度图之类的意义
                TriglavPlugInInt	target[] = { kTriglavPlugInFilterTargetKindRasterLayerRGBAlpha };
                TriglavPlugInFilterInitializeSetTargetKinds(pRecordSuite, hostObject, target, 1);


                //	プロパティの作成
                //生成可以控制的属性 Property    煞笔片假名
                TriglavPlugInPropertyObject	propertyObject;
                (*pPropertyService).createProc(&propertyObject);



                //运动模糊点1


                TriglavPlugInPropertyService2* pService2 = (*pluginServer).serviceSuite.propertyService2;
                //    测试属性
                TriglavPlugInStringObject    testCaption = NULL;
                (*pStringService).createWithStringIDProc(&testCaption, kStringIDPreview, (*pluginServer).hostObject);
                (*pPropertyService).addItemProc(propertyObject, kItemPoint1, kTriglavPlugInPropertyValueTypePoint, kTriglavPlugInPropertyInputKindCanvas, kTriglavPlugInPropertyInputKindCanvas, testCaption, 'a');
                //默认坐标放中间
                (*pService2).setPointDefaultValueKindProc(propertyObject, kItemPoint1, kTriglavPlugInPropertyPointDefaultValueKindSelectAreaCenter);

                (*pPropertyService).addItemProc(propertyObject, kItemPoint2, kTriglavPlugInPropertyValueTypePoint, kTriglavPlugInPropertyInputKindCanvas, kTriglavPlugInPropertyInputKindCanvas, testCaption, 'b');
                (*pService2).setPointDefaultValueKindProc(propertyObject, kItemPoint2, kTriglavPlugInPropertyPointDefaultValueKindSelectAreaCenter);





                //enum
                //模糊方向
                {
                    TriglavPlugInStringObject strBlurDirection = NULL;
                    TriglavPlugInStringObject strBlurDirectionFB = NULL;
                    TriglavPlugInStringObject strBlurDirectionF = NULL;
                    TriglavPlugInStringObject strBlurDirectionB = NULL;
                    (*pStringService).createWithStringIDProc(&strBlurDirection, kStringIDBlurDirection, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurDirectionFB, kStringIDBlurDirection_ForwardBehind, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurDirectionF, kStringIDBlurDirection_Forward, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurDirectionB, kStringIDBlurDirection_Behind, (*pluginServer).hostObject);

                    (*pPropertyService).addItemProc(propertyObject, kItemBlurDirection, kTriglavPlugInPropertyValueTypeEnumeration, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, strBlurDirection, 'd');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurDirection, kBlurDirectionFB, strBlurDirectionFB, 'e');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurDirection, kBlurDirectionF, strBlurDirectionF, 'f');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurDirection, kBlurDirectionB, strBlurDirectionB, 'g');
                    pService2->setEnumerationDefaultValueProc(propertyObject, kItemBlurDirection, kBlurDirectionFB);

                    (*pStringService).releaseProc(strBlurDirection);
                    (*pStringService).releaseProc(strBlurDirectionFB);
                    (*pStringService).releaseProc(strBlurDirectionF);
                    (*pStringService).releaseProc(strBlurDirectionB);

                }
                //enum
                //模糊算法
                {
                    TriglavPlugInStringObject strBlurAlgorithm = NULL;
                    TriglavPlugInStringObject strBlurAlgorithm_Smooth = NULL;
                    TriglavPlugInStringObject strBlurAlgorithm_Average = NULL;

                    (*pStringService).createWithStringIDProc(&strBlurAlgorithm, kStringIDBlurAlgorithm, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurAlgorithm_Smooth, kStringIDBlurAlgorithm_Smooth, (*pluginServer).hostObject);
                    (*pStringService).createWithStringIDProc(&strBlurAlgorithm_Average, kStringIDBlurAlgorithm_Average, (*pluginServer).hostObject);

                    (*pPropertyService).addItemProc(propertyObject, kItemBlurAlgorithm, kTriglavPlugInPropertyValueTypeEnumeration, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, strBlurAlgorithm, 's');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurAlgorithm, kBlurAlgorithmSmooth, strBlurAlgorithm_Smooth, 't');
                    (*pService2).addEnumerationItemProc(propertyObject, kItemBlurAlgorithm, kBlurAlgorithmAverage, strBlurAlgorithm_Average, 'u');
                    pService2->setEnumerationDefaultValueProc(propertyObject, kItemBlurAlgorithm, kBlurAlgorithmSmooth);

                    (*pStringService).releaseProc(strBlurAlgorithm);
                    (*pStringService).releaseProc(strBlurAlgorithm_Smooth);
                    (*pStringService).releaseProc(strBlurAlgorithm_Average);
                }






                //预览
                (*pPropertyService).addItemProc(propertyObject, kItemPreview, kTriglavPlugInPropertyValueTypeBoolean, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, testCaption, 'p');
                (*pPropertyService).setBooleanValueProc(propertyObject, kItemPreview, true);
                (*pPropertyService).setBooleanDefaultValueProc(propertyObject, kItemPreview, true);

                (*pStringService).releaseProc(testCaption);






                //                //预览
                //                blurRadiusCaption;
                //                (*pStringService).createWithStringIDProc(&blurRadiusCaption,kStringIDItemBlurRadius,(*pluginServer).hostObject);
                //                (*pPropertyService).addItemProc(propertyObject,kItemBlurRadius,kTriglavPlugInPropertyValueTypeBoolean,kTriglavPlugInPropertyValueKindDefault,kTriglavPlugInPropertyInputKindDefault,blurRadiusCaption,'p');
                //                (*pPropertyService).setb(propertyObject,kItemBlurRadius,0);
                //                (*pPropertyService).setDecimalDefaultValueProc(propertyObject,kItemBlurRadius,6);
                //                (*pPropertyService).setDecimalMinValueProc(propertyObject,kItemBlurRadius,0);
                //                (*pPropertyService).setDecimalMaxValueProc(propertyObject,kItemBlurRadius,256);
                //                (*pStringService).releaseProc(blurRadiusCaption);//手动释放字符串。。





                                //	プロパティの設定
                                //property的设置
                                //？我滑块呢？kTriglavPlugInPropertyValueTypeInteger？？
                TriglavPlugInFilterInitializeSetProperty(pRecordSuite, hostObject, propertyObject);
                TriglavPlugInFilterInitializeSetPropertyCallBack(pRecordSuite, hostObject, TriglavPlugInFilterPropertyCallBack, *data);

                //	プロパティの破棄
                //释放propertyobj 应该是减少引用计数吧。。这东西应该不会这么快就从内存里移除
                (*pPropertyService).releaseProc(propertyObject);

                *result = kTriglavPlugInCallResultSuccess;
            }
        }
        else if (selector == kTriglavPlugInSelectorFilterTerminate)
        {
            //	フィルタの終了処理
            //滤镜终止时的处理
            *result = kTriglavPlugInCallResultSuccess;
        }
        else if (selector == kTriglavPlugInSelectorFilterRun)
        {

            //	フィルタの実行
            //滤镜的执行

            auto pWorkingRecordSuite = &(*pluginServer).recordSuite;
            auto pWorkingOffscreenService = (*pluginServer).serviceSuite.offscreenService;
            auto pWorkingPropertyService = (*pluginServer).serviceSuite.propertyService;
            auto workingHostObject = (*pluginServer).hostObject;


            auto& customFilter=MotionBlur::GetIns();

            //RUNRecord意义？
            if (TriglavPlugInGetFilterRunRecord(pWorkingRecordSuite) != NULL && pWorkingOffscreenService != NULL && pWorkingPropertyService != NULL)
            {
                customFilter.Init(pluginServer);




                //设置进度
                //还需要设置进度，ai生图是吧


                //初始化
                HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(*data);
                (*pFilterInfo).pService1 = (*pluginServer).serviceSuite.propertyService;
                (*pFilterInfo).pPropertyService = (*pluginServer).serviceSuite.propertyService2;
                pFilterInfo->bPreview = true;


                pFilterInfo->point1 = { 0,0 };
                pFilterInfo->point2 = { 0,0 };
                //                (*pFilterInfo).blurRadius			= 6;
                //                customFilter.Render(pFilterInfo->blurRadius);
                //                customFilter.Render(200);
                                //？？restart干嘛 复位？
                bool	restart = true;
                bool firstRun = true;

                std::cout << "loop entrypoint" << std::endl;
                while (true)
                {

                    //正常处理消息即可，画面绘制在参数改变的回调里，会比较流畅。。


                    //                    customFilter.SetSkip(false);
                    //                    customFilter.Render(pFilterInfo->point1.x-pFilterInfo->point2.x,pFilterInfo->point1.y-pFilterInfo->point2.y);
                    customFilter.SetSkip(true);
                    //处理的状态
                    TriglavPlugInInt	processResult;
                    TriglavPlugInFilterRunProcess(pWorkingRecordSuite, &processResult, workingHostObject, kTriglavPlugInFilterRunProcessStateEnd);


                    if (processResult == kTriglavPlugInFilterRunProcessResultRestart)
                    {
                        restart = true;
                    }
                    else if (processResult == kTriglavPlugInFilterRunProcessResultExit)
                    {
                        break;
                    }
                }
                *result = kTriglavPlugInCallResultSuccess;


                
            }
            customFilter.ShutDown();
        }
    }
    return;
}

