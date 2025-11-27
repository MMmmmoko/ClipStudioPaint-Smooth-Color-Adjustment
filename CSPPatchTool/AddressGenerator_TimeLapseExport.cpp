#include"AddressGenerator.h"
#include"DllInject.h"
#include<SDL3/SDL.h>


void AddressGenerator::PushTimeLapseExportAddr()
{
	SDL_Log("Start Find TimeLapseExport Addresses...");
	{

		uint8_t timeLapseExportFuncFeature[] = {
		0x48 ,0x89  ,0x5C  ,0x24  ,0x18
			 ,0x48  ,0x89  ,0x7C  ,0x24  ,0x20
			,0x55
			,0x48 ,0x8D ,0xAC ,0x24 ,0x30 ,0xFD ,0xFF ,0xFF
			,0x48 ,0x81 ,0xEC ,0xD0 ,0x03 ,0x00 ,0x00
			,0x48 ,0x8B ,0x05  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
			,0x48 ,0x33 ,0xC4
			,0x48 ,0x89 ,0x85 ,0xC8 ,0x02 ,0x00 ,0x00
			,0x48 ,0x8B ,0xDA
			,0x48 ,0x8B ,0xF9
			,0x48 ,0x89 ,0x54 ,0x24 ,0x50
			,0x48 ,0x83 ,0x3A ,0x00

		//48 ,89  ,5C  ,24  ,18
		//	 ,48  ,89  ,7C  ,24  ,20
		//	,55
		//	,48 ,8D ,AC ,24 ,30 ,FD ,FF ,FF
		//	,48 ,81 ,EC ,D0 ,03 ,00 ,00
		//	,48 ,8B ,05  ,??, ??,??,??
		//	,48 ,33 ,C4
		//	,48 ,89 ,85 ,C8 ,02 ,00 ,00
		//	,48 ,8B ,DA
		//	,48 ,8B ,F9
		//	,48 ,89 ,54 ,24 ,50
		//	,48 ,83 ,3A ,00
		};





		bool success = false;
		for (size_t i = 0; i < _codeMemSize - sizeof(timeLapseExportFuncFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, timeLapseExportFuncFeature, sizeof(timeLapseExportFuncFeature)))
			{
				SDL_Log("TimeLapseExport Func Finded.");
				addrJson["CspAddressRVA"]["TimeLapseExport_Func"] = _VA + i;
				success = true;
				break;
			}
		}
		//未找到时，使用函数调用特征
		if (!success)
		{

			uint8_t timeLapseExportFuncCallFeature[] = {
			0x44,0x3B,0xF8,0x75,0x2E,0x0F,0x28,0x44,0x24,BYTEWILDCARD,0x66,0x0F,0x7F,0x45,BYTEWILDCARD,
			0x66,0x0F,0x73,0xD8,0x08,0x66,0x48,0x0F,0x7E,0xC1,0x48,0x85,0xC9,0x74,
			0x07,0x8B,0xC6,0xF0,0x0F,0xC1,0x41,0x08,0x48,0x8D,0x55,BYTEWILDCARD,0x49,0x8B,0xCE,
			0xE8
			};
			for (size_t i = 0; i < _codeMemSize - sizeof(timeLapseExportFuncCallFeature); i++)
			{
				if (_MatchFeatureCode(_codeMem + i, timeLapseExportFuncCallFeature, sizeof(timeLapseExportFuncCallFeature)))
				{
					//此代码最后的E8后即是调用地址
					uint32_t off = *(uint32_t*)(_codeMem + i + sizeof(timeLapseExportFuncCallFeature));
					SDL_Log("TimeLapseExport Func Finded.");
					addrJson["CspAddressRVA"]["TimeLapseExport_Func"] = _VA + i + sizeof(timeLapseExportFuncCallFeature) + 4 + off;
					success = true;
					break;
				}
			}



		}

		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "TimeLapseExport Func Not Found!");
	}
}