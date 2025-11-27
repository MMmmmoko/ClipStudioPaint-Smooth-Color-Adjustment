
#include "TriglavPlugInSDK/TriglavPlugInSDK.h"
#include<assert.h>
#include <iostream>
#include <vector>
#include<thread>
#include"GaussBlur.h"
#include <chrono>
typedef	unsigned char	BYTE;

static    const int kItemBlurRadius = 1;
static	const int kItemPreview = 2;

//本地化字符串id
static	const int kStringIDFilterCategoryName = 101;//插件的集合名（插件的上级菜单名）
static	const int kStringIDFilterName = 102;//滤镜插件名
static    const int kStringIDItemBlurRadius = 103;//模糊半径
static	const int kStringIDPreview = 104;//预览

//static const char* uuidOfThisPlugin = "1E8B8396-201B-49D2-9AAC-33E5B76DA8AE";
// 
// 
// 
//static const char* uuidOfThisPlugin = "A3A3E621-55BA-4880-A3BA-C75B927A0844";
//含有这个id的插件会被识别为官方插件，尚不知在什么接口设置这个uuid


static const char* uuidOfThisPlugin = "FFD22A25-ABB3-4fdf-A6F2-596CC0E24BEF";

struct	HSVFilterInfo
{
	double    blurRadius;
	bool	preview;
	TriglavPlugInPropertyService* pPropertyService;
};



//	プロパティコールバック
//属性回调
static void TRIGLAV_PLUGIN_CALLBACK TriglavPlugInFilterPropertyCallBack(TriglavPlugInInt* result, TriglavPlugInPropertyObject propertyObject, const TriglavPlugInInt itemKey, const TriglavPlugInInt notify, TriglavPlugInPtr data)
{


	(*result) = kTriglavPlugInPropertyCallBackResultNoModify;

	HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(data);

	if (pFilterInfo != NULL)
	{
		TriglavPlugInPropertyService* pPropertyService = (*pFilterInfo).pPropertyService;


		if (pPropertyService != NULL)
		{



			//如果属性修改了
			//这个是告知参数在csp中被修改了
			if (notify == kTriglavPlugInPropertyCallBackNotifyValueChanged)
			{


				TriglavPlugInDouble	value;
				(*(*pFilterInfo).pPropertyService).getDecimalValueProc(&value, propertyObject, kItemBlurRadius);
				TriglavPlugInBool _bpreview;
				(*(*pFilterInfo).pPropertyService).getBooleanValueProc(&_bpreview, propertyObject, kItemPreview);


				//添加一个计时器防止频繁触发 16ms
				static auto startTimer = std::chrono::system_clock::now();
				auto endTimer = std::chrono::system_clock::now();


				//pFilterInfo->blurRadius = value;
				//pFilterInfo->preview = _bpreview;



				static std::chrono::system_clock::time_point lastRenderEndTime = std::chrono::system_clock::from_time_t(0);


				if (_bpreview == false)
				{
					if (pFilterInfo->preview)
					{
						pFilterInfo->preview = _bpreview;
						//if (std::chrono::duration_cast<std::chrono::milliseconds>(endTimer - startTimer).count() > max(33, GaussBlur::GetIns().GetLastCostTimeMS()*2))
						//{
						
						auto curtime = std::chrono::system_clock::now();
						if (std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastRenderEndTime).count()>24)
						{
							GaussBlur::GetIns().OnParamChanged(0);
							lastRenderEndTime = std::chrono::system_clock::now();
							startTimer = endTimer;
						}
					}
				}
				else
				{

					pFilterInfo->preview = true;
					auto curtime = std::chrono::system_clock::now();
					if (std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastRenderEndTime).count() > 24)
					{
						GaussBlur::GetIns().OnParamChanged(value);
						lastRenderEndTime = std::chrono::system_clock::now();
						startTimer = endTimer;
					}
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
			auto handle = GetModuleHandle(L"CSPMOD.dll");
			if (handle)
			{
				typedef void(*SetStrDataFunc)(const char* key, const char* value);

				SetStrDataFunc myFunc = (SetStrDataFunc)GetProcAddress(handle, "CSPMOD_SetStrData");
				if (myFunc)
				{
					myFunc("SmoothFilter_GaussBlur", uuidOfThisPlugin);
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
			GaussBlur::PluginModuleCleanUp();
		}
		else if (selector == kTriglavPlugInSelectorFilterInitialize)
		{
			auto handle = GetModuleHandle(L"CSPMOD.dll");
			typedef bool(*GetStrFromID)(wchar_t* buffer, const wchar_t* stringID, uint32_t* strSize);
			GetStrFromID getIDPRoc = nullptr;
			if (handle)
			{
				getIDPRoc = (GetStrFromID)GetProcAddress(handle, "CSPMOD_GetStrFromID");
			}




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


				wchar_t buffer[100];
				uint32_t strSize = 0;
				if (getIDPRoc)
				{
					if(getIDPRoc(buffer, L"STRID_FILTERPLUGIN_CATEGORYNAME", &strSize))
						(*pStringService).createWithUnicodeStringProc(&filterCategoryName, (unsigned short*)buffer, strSize);
					
					if(getIDPRoc(buffer, L"STRID_FILTERPLUGIN_GAUSSBLUR", &strSize))
						(*pStringService).createWithUnicodeStringProc(&filterName, (unsigned short*)buffer, strSize);
				}
				
				{
					if(!filterCategoryName)
						(*pStringService).createWithStringIDProc(&filterCategoryName, kStringIDFilterCategoryName, (*pluginServer).hostObject);
					if (!filterName)
						(*pStringService).createWithStringIDProc(&filterName, kStringIDFilterName, (*pluginServer).hostObject);
				}




				TriglavPlugInFilterInitializeSetFilterCategoryName(pRecordSuite, hostObject, filterCategoryName, 'c');
				TriglavPlugInFilterInitializeSetFilterName(pRecordSuite, hostObject, filterName, 'g');
				(*pStringService).releaseProc(filterCategoryName);
				(*pStringService).releaseProc(filterName);

				//	プレビュー
				//preview。 煞笔片假名
				//给插件设置是否可以预览
//                TriglavPlugInFilterInitializeSetCanPreview(pRecordSuite,hostObject,true);
				//这里其实会影响回调
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

				//这里生成色相滑块//如果插件和csp原始代码相同的话，应该能用现成的曲线
				//模糊半径
				TriglavPlugInStringObject	blurRadiusCaption = NULL;
				
				if (getIDPRoc&& getIDPRoc(buffer, L"STRID_FILTERPLUGIN_GAUSSBLUR_RADIUS", &strSize))
					(*pStringService).createWithUnicodeStringProc(&blurRadiusCaption, (unsigned short*)buffer, strSize);
				else
					(*pStringService).createWithStringIDProc(&blurRadiusCaption, kStringIDItemBlurRadius, (*pluginServer).hostObject);

				(*pPropertyService).addItemProc(propertyObject, kItemBlurRadius, kTriglavPlugInPropertyValueTypeDecimal, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, blurRadiusCaption, 'b');
				(*pPropertyService).setDecimalValueProc(propertyObject, kItemBlurRadius, 0);
				(*pPropertyService).setDecimalDefaultValueProc(propertyObject, kItemBlurRadius, 6);
				(*pPropertyService).setDecimalMinValueProc(propertyObject, kItemBlurRadius, 0);
				(*pPropertyService).setDecimalMaxValueProc(propertyObject, kItemBlurRadius, 255);
				(*pStringService).releaseProc(blurRadiusCaption);//手动释放字符串。。


				TriglavPlugInStringObject    blurPreviewCaption = NULL;
				if(getIDPRoc&& getIDPRoc(buffer, L"STRID_FILTERPLUGIN_PREVIEW", &strSize))
					(*pStringService).createWithUnicodeStringProc(&blurPreviewCaption, (unsigned short*)buffer, strSize);
				else
					(*pStringService).createWithStringIDProc(&blurPreviewCaption, kStringIDPreview, (*pluginServer).hostObject);

				(*pPropertyService).addItemProc(propertyObject, kItemPreview, kTriglavPlugInPropertyValueTypeBoolean, kTriglavPlugInPropertyValueKindDefault, kTriglavPlugInPropertyInputKindDefault, blurPreviewCaption, 'p');
				(*pPropertyService).setBooleanValueProc(propertyObject, kItemPreview, true);
				(*pPropertyService).setBooleanDefaultValueProc(propertyObject, kItemPreview, true);
				(*pStringService).releaseProc(blurPreviewCaption);



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


			auto& customFilter = GaussBlur::GetIns();

			//RUNRecord意义？
			if (TriglavPlugInGetFilterRunRecord(pWorkingRecordSuite) != NULL && pWorkingOffscreenService != NULL && pWorkingPropertyService != NULL)
			{
				customFilter.Init(pluginServer);




				//设置进度
				//还需要设置进度，ai生图是吧


				//初始化
				HSVFilterInfo* pFilterInfo = static_cast<HSVFilterInfo*>(*data);
				(*pFilterInfo).pPropertyService = pWorkingPropertyService;
				(*pFilterInfo).blurRadius = 6;
				(*pFilterInfo).preview = true;
				//customFilter.Render(static_cast<float>(pFilterInfo->blurRadius));
				//                customFilter.Render(200);
								//？？restart干嘛 复位？
				bool	restart = true;


				std::cout << "loop entrypoint" << std::endl;
				TriglavPlugInFilterRunSetProgressTotal(pWorkingRecordSuite,workingHostObject,100);

				TriglavPlugInInt	processResult;
				TriglavPlugInInt	blockIndex = 0;
				TriglavPlugInRect				selectAreaRect;
				TriglavPlugInFilterRunGetSelectAreaRect(pWorkingRecordSuite, &selectAreaRect, (*pluginServer).hostObject);

				while (true)
				{
					if (restart)
					{
						restart = false;

						//TriglavPlugInInt	processResult;
						//if ((*pFilterInfo).blurRadius != 0)
						//{
							blockIndex = 0;
							//customFilter.Render(static_cast<float>(pFilterInfo->blurRadius));
						//}
						TriglavPlugInFilterRunProcess(pWorkingRecordSuite, &processResult, workingHostObject, kTriglavPlugInFilterRunProcessStateStart);
						break;


						//else
						//{
						//	//何もしない
						//	blockIndex = 100;
						//	TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pWorkingRecordSuite, (*pluginServer).hostObject, &selectAreaRect);
						//}
					}

					if (blockIndex < 100)
					{
						//TriglavPlugInFilterRunSetProgressDone(pWorkingRecordSuite, (*pluginServer).hostObject, 100);

						//customFilter.Render(static_cast<float>(pFilterInfo->blurRadius));
						//TriglavPlugInFilterRunUpdateDestinationOffscreenRect(pWorkingRecordSuite, (*pluginServer).hostObject, &selectAreaRect);
						blockIndex=100;
					}

					//TriglavPlugInInt	processResult;
					if (blockIndex < 100)
					{
						TriglavPlugInFilterRunProcess(pWorkingRecordSuite, &processResult, (*pluginServer).hostObject, kTriglavPlugInFilterRunProcessStateContinue);
					}
					else
					{
				/*		TriglavPlugInFilterRunSetProgressDone(pWorkingRecordSuite, (*pluginServer).hostObject, blockIndex);
						TriglavPlugInFilterRunProcess(pWorkingRecordSuite, &processResult, (*pluginServer).hostObject, kTriglavPlugInFilterRunProcessStateContinue);*/
					}
					if (processResult == kTriglavPlugInFilterRunProcessResultRestart)
					{
						restart = true;
					}
					if (processResult == kTriglavPlugInFilterRunProcessResultContinue)
					{
						restart = true;
					}
					else if (processResult == kTriglavPlugInFilterRunProcessResultExit)
					{
						break;
					}
				}
				*result = kTriglavPlugInCallResultSuccess;
				return;





				while (true)
				{

					//正常处理消息即可，画面绘制在参数改变的回调里，会比较流畅。。


					//                    customFilter.SetSkip(false);
					//                    customFilter.Render(pFilterInfo->blurRadius);
					customFilter.SetSkip(true);
					//处理的状态
					
					if (restart)
					{
						restart = false;
						TriglavPlugInInt	result;
						TriglavPlugInFilterRunProcess(pWorkingRecordSuite, &result, workingHostObject, kTriglavPlugInFilterRunProcessStateStart);
						if (result == kTriglavPlugInFilterRunProcessResultExit)
						{
							break;
						}

					}

					TriglavPlugInFilterRunProcess(pWorkingRecordSuite, &processResult, workingHostObject, kTriglavPlugInFilterRunProcessStateContinue);


					if (processResult == kTriglavPlugInFilterRunProcessResultRestart)
					{
						TriglavPlugInFilterRunSetProgressDone(pWorkingRecordSuite, workingHostObject, 2);
						std::cout << "loop restart" << std::endl;

						restart = true;
					}
					else if (processResult == kTriglavPlugInFilterRunProcessResultContinue)
					{
						//std::cout << "loop continue" << std::endl;
						//Sleep(1000);

						restart = true;
					}
					else if (processResult == kTriglavPlugInFilterRunProcessResultExit)
					{
						break;
					}
				}
				*result = kTriglavPlugInCallResultSuccess;


				customFilter.ShutDown();
			}
		}
	}
	return;
}

