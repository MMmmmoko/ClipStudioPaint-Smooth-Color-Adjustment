#include"AddressGenerator.h"
#include"DllInject.h"
#include<SDL3/SDL.h>


void AddressGenerator::PushCSPDataAddr()
{
	//获取一些关于CSP的数据

	SDL_Log("Start Find CSP Info Addresses...");

	//UI色表基址
	{
		uint8_t colorTableFeature[] = {
			0x48 ,0x83 ,0xEC ,0x28 ,0x48 ,0x83 ,0x3D ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x00 
			,0x75 ,0x24 ,0xB9 ,0x28 ,0x00 ,0x00 ,0x00 ,0xE8  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
			,0x48,0x89,0x44,0x24,0x30,0x48,0x85,0xC0,0x74,0x09,0x48,0x8B,0xC8,0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
			, 0x90,0x48,0x89,0x05 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x48,0x83,0xC4,0x28,0xC3
		};

		bool success = false;
		for (uint32_t i = 0; i < _codeMemSize - sizeof(colorTableFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorTableFeature, sizeof(colorTableFeature)))
			{
				SDL_Log("CSP UI ColorTbale BaseAddr Finded.");
				//颜色表在特征的中间，不是起始位置
				//需要进行一次计算
				uint32_t addrOff = *(uint32_t*)(_codeMem + i + 7);
				addrJson["CspAddressRVA"]["CSPInfo_UIColorTable"] = _VA + (uint32_t)i+7+5+ addrOff;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "CSP UI ColorTbale BaseAddr Not Found!");
	}

	//当前画布尺寸基址
	{
		uint8_t canvasSizeTableFeature[] = {
0x40,0x53,0x48,0x83,0xEC,0x20,
0x48,0x83,0x3D,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x00,
0x48,0x8B,0xD9,0x74,0x36,0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
0x8B,0xC8,0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
0x85,0xC0,0x75,0x2E,0x48,0x8B,0xCB,0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
0x8B,0xC8
		};

		bool success = false;
		for (uint32_t i = 0; i < _codeMemSize - sizeof(canvasSizeTableFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, canvasSizeTableFeature, sizeof(canvasSizeTableFeature)))
			{
				SDL_Log("CSP CanvasSize BaseAddr Finded.");
				//颜色表在特征的中间，不是起始位置
				//需要进行一次计算
				uint32_t addrOff = *(uint32_t*)(_codeMem + i + 9);
				addrJson["CspAddressRVA"]["CSPInfo_CanvasSize"] = _VA + (uint32_t)i+9+5+ addrOff;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "CSP CanvasSize BaseAddr Not Found!");
	}
	//主窗口句柄基址
	{
		uint8_t nativeWindowHandleFeature[] = {
0x75,0x0E,0x48,0x83,0x3D,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x00,
0x0F,0x85,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
0x45,0x33,0xF6,0x4C,0x89,0xB4,0x24,0x80,0x00,0x00,0x00,0xB9,0x00,0x0A,0x00,0x00,
0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
0x48,0x8B,0xD8,0x48,0x89,0x44,0x24,0x60,0x48,0x85,0xC0
		};

		bool success = false;
		for (uint32_t i = 0; i < _codeMemSize - sizeof(nativeWindowHandleFeature); i++)
		{
			if (_MatchFeatureCode(_codeMem + i, nativeWindowHandleFeature, sizeof(nativeWindowHandleFeature)))
			{
				SDL_Log("CSP Native Window Handle BaseAddr Finded.");
				//颜色表在特征的中间，不是起始位置
				//需要进行一次计算
				uint32_t addrOff = *(uint32_t*)(_codeMem + i + 5);
				addrJson["CspAddressRVA"]["CSPInfo_NativeWindowHandle"] = _VA + (uint32_t)i+5+5+ addrOff;
				success = true;
				break;
			}
		}
		if (!success)
			SDL_LogError(SDL_LogCategory::SDL_LOG_CATEGORY_ERROR, "CSP Native Window Handle  Not Found!");
	}







}