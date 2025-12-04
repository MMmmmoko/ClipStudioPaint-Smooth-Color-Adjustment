#include"AddressGenerator.h"
#include"DllInject.h"
#include<SDL3/SDL.h>

void AddressGenerator::_PushToneCurveAddr()
{
	SDL_Log("Start Find ToneCurve Addresses...");
	{
		//对话框
		uint8_t tonecCurveDialogFuncFeature[] = {
			0x48 ,0x89 ,0x5C ,0x24 ,0x20
,0x55
,0x56
,0x57
,0x41 ,0x54
,0x41 ,0x55
,0x41 ,0x56
,0x41 ,0x57
,0x48 ,0x8D ,0xAC ,0x24 ,0xA0 ,0xF3 ,0xFF ,0xFF
,0x48 ,0x81 ,0xEC ,0x60 ,0x0D ,0x00 ,0x00
,0x48 ,0x8B ,0x05 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x33 ,0xC4
,0x48 ,0x89 ,0x85 ,0x50 ,0x0C ,0x00 ,0x00
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveDialogFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveDialogFuncFeature, sizeof(tonecCurveDialogFuncFeature)))
			{
				SDL_Log("ToneCurve Dialog Func Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_Dialog_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurve Dialog Func Not Found!");
	}
	{
		//420版本后对话框调用位置有变动，这里改用函数外部调用特征
		

		uint8_t tonecCurveLayerDialogFuncFeature[] = {
		0x0F,0x28,0x85,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//7
		0x66,0x0F,0x7F,0x85,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//8
		0x66,0x0F,0x73,0xD8,0x08,0x66,0x48,0x0F,0x7E,0xC1,//10
		0x48,0x85,0xC9,0x74,0x0A,0xB8,0x01,0x00,0x00,0x00,//10
		0xF0,0x0F,0xC1,0x41,0x08,//5
		0x4C,0x8D,0x8D,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//7
		0x4C,0x8D,0x85,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//7
		0x48,0x8D,0x95,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//7
		0x48,0x8D,0x8D,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//7
		0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//函数调用的位置
		0x44,0x8B,0xF8,0xB9,0x48,0x10,0x00,0x00

		//0F,28,85,??, ??,??,??,
		//66,0F,7F,85, ? ? , ? ? , ? ? , ? ? ,
		//66,0F,73,D8,08,66,48,0F,7E,C1,
		//48,85,C9,74,0A,B8,01,00,00,00,
		//F0,0F,C1,41,08,
		//4C,8D,8D, ? ? , ? ? , ? ? , ? ? ,
		//4C,8D,85, ? ? , ? ? , ? ? , ? ? ,
		//48,8D,95, ? ? , ? ? , ? ? , ? ? ,
		//48,8D,8D, ? ? , ? ? , ? ? , ? ? ,
		//E8, ? ? , ? ? , ? ? , ? ? ,
		//44,8B,F8,B9,48,10,00,00
		};


		//这里是原特征
		uint8_t tonecCurveLayerDialogFuncFeature_OLD[] = {
0x40 ,0x55
,0x53
,0x56
,0x57
,0x41 ,0x54
,0x41 ,0x55
,0x41 ,0x56
,0x41 ,0x57
,0x48 ,0x8D ,0xAC ,0x24 ,0x78 ,0xEF ,0xFF ,0xFF
,0xB8 ,0x88 ,0x11 ,0x00 ,0x00
,0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD


//40 ,55
//,53
//,56
//,57
//,41 ,54
//,41 ,55
//,41 ,56
//,41 ,57
//,48 ,8D ,AC ,24 ,78 ,EF ,FF ,FF
//,B8 ,88 ,11 ,00 ,00
//,E8,??, ??,??,??
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveLayerDialogFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveLayerDialogFuncFeature, sizeof(tonecCurveLayerDialogFuncFeature)))
			{
				uint32_t off = *(uint32_t*)(_codeMem + i + 
					+7+8+10+10+5+7+7+7+7+1);
				SDL_Log("ToneCurveLayer Dialog Func Finded.");
				addrJson["CspAddressRVA"]["ToneCurveLayer_Dialog_Func"] = _VA + i
					+7 + 8 + 10 + 10 + 5 + 7 + 7 + 7 + 7 + 5+ off;
				success = true;
				break;
			}
		}
		if (!success)
		{
			//未成功时使用原特征
			for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveLayerDialogFuncFeature_OLD); i++)
			{
				if (_MatchFeatureCode(_codeMem + i, tonecCurveLayerDialogFuncFeature_OLD, sizeof(tonecCurveLayerDialogFuncFeature_OLD)))
				{
					SDL_Log("ToneCurveLayer Dialog Func Finded.（BACKEND ADDR）");
					addrJson["CspAddressRVA"]["ToneCurveLayer_Dialog_Func"] = _VA + i;
					success = true;
					break;
				}
			}
			if (!success)
				SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer Dialog Func Not Found!");
		}
	}
	//参数回调
	{
		uint8_t tonecCurveParamChangeFuncFeature[] = {
0x48 ,0x89 ,0x5C ,0x24 ,0x08 ,0x48 ,0x89 ,0x74 ,0x24 ,0x18 ,0x48 ,0x89 ,0x54 ,0x24 ,0x10 ,0x57 ,0x48 ,0x83 ,0xEC ,0x60 ,0x0F ,0x29 ,0x74 ,0x24 ,0x50 ,0x49 ,0x8B ,0xF8 ,0x48 ,0x8B ,0xDA ,0x48 ,0x8B ,0xF1 ,0x48 ,0x8D ,0x54 ,0x24 ,0x40,0xE8
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveParamChangeFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveParamChangeFuncFeature, sizeof(tonecCurveParamChangeFuncFeature)))
			{
				SDL_Log("ToneCurve OnParamChange Func Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_OnParamChange_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer OnParamChange Func Not Found!");
	}


	//绘制
	{
		uint8_t tonecCurveBeforeDrawFuncFeature[] = {
0x48 ,0x89 ,0x5C ,0x24 ,0x18
,0x55
,0x56
,0x57
,0xB8 ,0x30 ,0x31 ,0x00 ,0x00
,0xE8 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x2B ,0xE0
,0x48 ,0x8B ,0x05 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveBeforeDrawFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveBeforeDrawFuncFeature, sizeof(tonecCurveBeforeDrawFuncFeature)))
			{
				SDL_Log("ToneCurve BeforeDraw Func Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_BeforeDraw_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer BeforeDraw Func Not Found!");
	}

	//update
	{
		uint8_t tonecCurveUpdateFuncFeature[] = {




//4.2.0能用这个方式找到目标函数，但是只会在鼠标抬起的时候被调用
			//位置没问题，曲线对话框函数位置变了
0x48 ,0x89 ,0x5C ,0x24 ,0x08
,0x48 ,0x89 ,0x74 ,0x24 ,0x10
,0x57
//,0xB8 ,0xA0 ,0x10 ,0x00 ,0x00
,0xB8 ,BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD
,0xE8, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD, BYTEWILDCARD
,0x48 ,0x2B ,0xE0
,0x48 ,0x8B ,0xF2
,0x48 ,0x8B ,0xF9
,0x48 ,0x8B ,0xCA


//这是4.1.4之前的特特征，4.2.0特征出现了变化
//48 ,89 ,5C ,24 ,08
//,48 ,89 ,74 ,24 ,10
//,57
//,B8 ,A0 ,10 ,00 ,00
//,E8, ??, ??, ??, ??
//,48 ,2B ,E0
//,48 ,8B ,F2
//,48 ,8B ,F9
//,48 ,8B ,CA
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveUpdateFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveUpdateFuncFeature, sizeof(tonecCurveUpdateFuncFeature)))
			{
				SDL_Log("ToneCurve Update Func Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_UpdatFunc"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer Update Func Not Found!");
	}




	//代码Patch点
	{
		uint8_t tonecCurveCodePatch1Feature[] = {
0xBF ,0x01 ,0x00 ,0x00 ,0x00
,0x8B ,0xCD
,0xE8  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x85 ,0xC0
,0x0F ,0x84 ,0x7F ,0x00 ,0x00 ,0x00
,0xE8  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8B ,0xF8
,0x8B ,0x0D  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveCodePatch1Feature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveCodePatch1Feature, sizeof(tonecCurveCodePatch1Feature)))
			{
				SDL_Log("ToneCurve PatchPoint1 Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_PatchAddr1"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer PatchPoint1 Not Found!");
	}
	{
		uint8_t tonecCurveCodePatch2Feature[] = {
0x74 ,0x1B ,0x48 ,0x8B ,0xCB
,0xE8 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0xC7 ,0x83 ,0xF0 ,0x0A ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x48 ,0x8B ,0x4B ,0x30
,0xE8 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0xE8 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveCodePatch2Feature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveCodePatch2Feature, sizeof(tonecCurveCodePatch2Feature)))
			{
				SDL_Log("ToneCurve PatchPoint2 Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_PatchAddr2"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer PatchPoint2 Not Found!");
	}
	{
		uint8_t tonecCurveCodePatch3Feature[] = {
0x0F ,0x84 ,0x7F ,0x00 ,0x00 ,0x00
,0xE8  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x8B ,0xF8
,0x8B ,0x0D  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x03 ,0x4B ,0x20
,0x48 ,0x3B ,0xC8
,0x7D ,0x61
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveCodePatch3Feature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveCodePatch3Feature, sizeof(tonecCurveCodePatch3Feature)))
			{
				SDL_Log("ToneCurve PatchPoint3 Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_PatchAddr3"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer PatchPoint3 Not Found!");
	}
	{
		uint8_t tonecCurveCodePatch4Feature[] = {
0x0F ,0x84 ,0xA0 ,0x00 ,0x00 ,0x00
,0x48 ,0x8D ,0x8F ,0x78 ,0x10 ,0x00 ,0x00
,0x8B ,0x97 ,0x90 ,0x10 ,0x00 ,0x00
,0xE8 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
,0x48 ,0x83 ,0xBF ,0x40 ,0x10 ,0x00 ,0x00 ,0x00
,0x74 ,0x7D
,0xB9 ,0x48 ,0x10 ,0x00 ,0x00
		};
	
		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(tonecCurveCodePatch4Feature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, tonecCurveCodePatch4Feature, sizeof(tonecCurveCodePatch4Feature)))
			{
				SDL_Log("ToneCurve PatchPoint4 Finded.");
				addrJson["CspAddressRVA"]["ToneCurve_PatchAddr4"] = _VA + i;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "ToneCurveLayer PatchPoint4 Not Found!");
	}




















}

