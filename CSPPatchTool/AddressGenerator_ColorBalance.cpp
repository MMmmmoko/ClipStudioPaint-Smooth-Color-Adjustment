#include"AddressGenerator.h"
#include"DllInject.h"
#include<SDL3/SDL.h>





void AddressGenerator::_PushColorBalanceAddr()
{
	SDL_Log("Start Find ColorBalance Addresses...");
	{
		//色彩平衡创建对话框
		uint8_t colorBalanceDialogFuncFeature[] = {
		0x48,0x89,0x5C ,0x24 ,0x18 ,0x55 ,0x56 ,0x57 ,0x41 ,0x56 ,0x41 ,0x57 ,0x48
		,0x8D ,0xAC ,0x24 ,0xA0 ,0xFD ,0xFF ,0xFF ,0x48 ,0x81 ,0xEC ,0x60 ,0x03 ,0x00 ,0x00
			,0x48 ,0x8B ,0x05 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x48 ,0x33 ,0xC4
			,0x48 ,0x89 ,0x85 ,0x50 ,0x02 ,0x00 ,0x00 ,0x4C ,0x8B ,0xF2 ,0x48 ,0x8B ,0xF1 ,0x48 ,0x8D ,0x4D ,0x50 ,0xE8 ,
			BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD, 0x90 ,0x45 ,0x8B ,0x4E ,0x10,0x41 ,0x8B ,0x7E ,0x1C ,0x41 ,0x8B ,0x5E ,0x14
		};

		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(colorBalanceDialogFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorBalanceDialogFuncFeature, sizeof(colorBalanceDialogFuncFeature)))
			{
				SDL_Log("ColorBalance Dialog Func Finded.");
				addrJson["CspAddressRVA"]["ColorBalance_Dialog_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ColorBalance Dialog Func Not Found!");
	}


	{
		//色彩平衡，创建调整图层对话框
		uint8_t colorBalanceLayerDialogFuncFeature[] = {
			0x48 ,0x89 ,0x5C ,0x24 ,0x20 ,0x55 ,0x56 ,0x57 ,0x41 ,0x54 ,0x41 ,0x55 ,0x41 ,0x56 ,0x41 ,0x57
			,0x48 ,0x8D ,0xAC ,0x24 ,0xD0 ,0xFE ,0xFF ,0xFF ,0x48 ,0x81 ,0xEC ,0x30 ,0x02 ,0x00 ,0x00 ,0x0F 
			,0x29 ,0xB4 ,0x24 ,0x20 ,0x02 ,0x00 ,0x00 ,0x48 ,0x8B ,0x05 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
			,0x48 ,0x33 ,0xC4 ,0x48 ,0x89 ,0x85 ,0x10 ,0x01 ,0x00 ,0x00 ,0x4D ,0x8B ,0xF8 ,0x48 ,0x8B ,0xFA ,0x48 ,0x8B ,0xF1
		};
		//uint8_t colorBalanceLayerDialogFuncFeature[] = {
		//	48 ,89 ,5C ,24 ,20 ,55 ,56 ,57 ,41 ,54 ,41 ,55 ,41 ,56 ,41 ,57
		//	,48 ,8D ,AC ,24 ,D0 ,FE ,FF ,FF ,48 ,81 ,EC ,30 ,02 ,00 ,00 ,0F 
		//	,29 ,B4 ,24 ,20 ,02 ,00 ,00 ,48 ,8B ,05 ,??, ??,??,??
		//	,48 ,33 ,C4 ,48 ,89 ,85 ,10 ,01 ,00 ,00 ,4D ,8B ,F8 ,48 ,8B ,FA ,48 ,8B ,F1
		//};


		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(colorBalanceLayerDialogFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorBalanceLayerDialogFuncFeature, sizeof(colorBalanceLayerDialogFuncFeature)))
			{
				SDL_Log("ColorBalanceLayer Dialog Func Finded.");
				addrJson["CspAddressRVA"]["ColorBalanceLayer_Dialog_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ColorBalanceLayer Dialog Func Not Found!");
	}

	{
		//色彩平衡，一个在绘制调用前会经过的地方（前置函数？）
		//主要用来获取将要传入绘制函数的参数
		uint8_t colorBalanceBeforeDrawFuncFeature[] = {
0x40 ,0x53
,0x48 ,0x83 ,0xEC ,0x20
,0x48 ,0x8B ,0xD9
,0x48 ,0x8B ,0xCA
,0xE8 ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x8B ,0xC8
,0xE8 ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x85 ,0xC0
,0x74,0x31
,0x48 ,0x8D ,0x4B ,0x60
,0xBA ,0xAE ,0x5B ,0x01 ,0x00

//40 ,53
//,48 ,83 ,EC ,20
//,48 ,8B ,D9
//,48 ,8B ,CA
//,E8 ,??,??,??,??
//,8B ,C8
//,E8 ,??,??,??,??
//,85 ,C0
//,74,31
//,48 ,8D ,4B ,60
//,BA ,AE ,5B ,01 ,00
		};


		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(colorBalanceBeforeDrawFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorBalanceBeforeDrawFuncFeature, sizeof(colorBalanceBeforeDrawFuncFeature)))
			{
				SDL_Log("ColorBalanceLayer BeforeDraw Func Finded.");
				addrJson["CspAddressRVA"]["ColorBalance_BeforeDraw_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ColorBalanceLayer BeforeDraw Func Not Found!");
	}

	{
		//色彩平衡，参数回调
		//参数改变时会触发的函数
		uint8_t colorBalanceOnParamChangeFuncFeature[] = {
0x48,0x89 ,0x5C ,0x24 ,0x08
,0x48 ,0x89 ,0x74 ,0x24 ,0x10
,0x57
,0x48 ,0x83 ,0xEC ,0x60
,0x48 ,0x8B ,0xDA
,0x48 ,0x8B ,0xF9
,0x48 ,0x8D ,0x4C ,0x24 ,0x20
,0xE8 ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x90
,0x48 ,0x8B ,0xCB
,0xE8 ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x8B ,0xD8
,0x48 ,0x8B ,0x4C ,0x24 ,0x20
,0x48 ,0x85 ,0xC9
		};


		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(colorBalanceOnParamChangeFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorBalanceOnParamChangeFuncFeature, sizeof(colorBalanceOnParamChangeFuncFeature)))
			{
				SDL_Log("ColorBalanceLayer OnParamChange Func Finded.");
				addrJson["CspAddressRVA"]["ColorBalance_OnParamChange_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ColorBalanceLayer OnParamChange Func Not Found!");
	}

		
	//见 CSPHacker::DoPatchTimerPoint()
#if 0
	{
	//色彩平衡 计时器1
	
	
		uint8_t colorBalanceTimer1Feature[] = {
0x8B ,0x15  ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8D ,0x4C ,0x24 ,0x20
,0xE8  ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8D ,0x05  ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x89 ,0x44 ,0x24 ,0x30
,0x48 ,0x89 ,0x5C ,0x24 ,0x38
,0x0F ,0x28 ,0x44 ,0x24 ,0x30
,0x66 ,0x0F ,0x7F ,0x44 ,0x24 ,0x30
		};


		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(colorBalanceTimer1Feature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorBalanceTimer1Feature, sizeof(colorBalanceTimer1Feature)))
			{
				SDL_Log("ColorBalanceLayer Timer1 Finded.");
				addrJson["CspAddressRVA"]["ColorBalance_SkipTimer1"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ColorBalanceLayer Timer1 Not Found!");
	}
#endif
	{
		//色彩平衡 计时器2


		uint8_t colorBalanceTimer2Feature[] = {
0x74 ,0x31
,0x48 ,0x8D ,0x4B ,0x60
,0xBA ,0xAE ,0x5B ,0x01 ,0x00
,0xE8 ,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8B ,0x43 ,0x08
,0x48 ,0x85 ,0xC0
,0x74 ,0x0F
		};


		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(colorBalanceTimer2Feature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorBalanceTimer2Feature, sizeof(colorBalanceTimer2Feature)))
			{
				SDL_Log("ColorBalanceLayer Timer2 Finded.");
				addrJson["CspAddressRVA"]["ColorBalance_SkipTimer2"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ColorBalanceLayer Timer2 Not Found!");
	}



}