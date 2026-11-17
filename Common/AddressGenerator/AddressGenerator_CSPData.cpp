#include"AddressGenerator.h"

#include <cstring>


void AddressGenerator::PushCSPDataAddr()
{
	//获取一些关于CSP的数据

	Log("Start Find CSP Info Addresses...");

	//UI色表基址
	{
		uint8_t colorTableFeature[] = {
			0x48 ,0x83 ,0xEC ,0x28 ,0x48 ,0x83 ,0x3D ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x00 
			,0x75 ,0x24 ,0xB9 ,0x28 ,0x00 ,0x00 ,0x00 ,0xE8  ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
			,0x48,0x89,0x44,0x24,0x30,0x48,0x85,0xC0,0x74,0x09,0x48,0x8B,0xC8,0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD
			, 0x90,0x48,0x89,0x05 ,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,0x48,0x83,0xC4,0x28,0xC3
		};

		bool success = false;
		for (uint32_t i = 0; i + sizeof(colorTableFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, colorTableFeature, sizeof(colorTableFeature)))
			{
				Log("CSP UI ColorTbale BaseAddr Finded.");
				//颜色表在特征的中间，不是起始位置
				//需要进行一次计算
				uint32_t addrOff = *(uint32_t*)(_codeMem + i + 7);
				addrJson["CspAddressRVA"]["CSPInfo_UIColorTable"] = _VA + (uint32_t)i+7+5+ addrOff;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSP UI ColorTbale BaseAddr Not Found!");
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
		for (uint32_t i = 0; i + sizeof(canvasSizeTableFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, canvasSizeTableFeature, sizeof(canvasSizeTableFeature)))
			{
				Log("CSP CanvasSize BaseAddr Finded.");
				//颜色表在特征的中间，不是起始位置
				//需要进行一次计算
				uint32_t addrOff = *(uint32_t*)(_codeMem + i + 9);
				addrJson["CspAddressRVA"]["CSPInfo_CanvasSize"] = _VA + (uint32_t)i+9+5+ addrOff;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSP CanvasSize BaseAddr Not Found!");
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
		for (uint32_t i = 0; i + sizeof(nativeWindowHandleFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, nativeWindowHandleFeature, sizeof(nativeWindowHandleFeature)))
			{
				Log("CSP Native Window Handle BaseAddr Finded.");
				//颜色表在特征的中间，不是起始位置
				//需要进行一次计算
				uint32_t addrOff = *(uint32_t*)(_codeMem + i + 5);
				addrJson["CspAddressRVA"]["CSPInfo_NativeWindowHandle"] = _VA + (uint32_t)i+5+5+ addrOff;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSP Native Window Handle  Not Found!");
	}
	//监听设置窗口显示隐藏的基址
	//48 89 5C 24 10 48 89 74 24 18 55 57 41 54 41 56 41 57 48 8D 6C 24 C9 48 81 EC E0 00 00 00 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 45 27 48 8D 55 D7 48 8B D9
	{
		uint8_t onSetPanelShowStatesFeature[] = {
0x48,0x89,0x5C,0x24,0x10,0x48,0x89,0x74,0x24,0x18,//10
0x55,0x57,0x41,0x54,0x41,0x56,0x41,0x57,0x48,0x8D,//10
0x6C,0x24,0xC9,0x48,0x81,0xEC,0xE0,0x00,0x00,0x00,//10
0x48,0x8B,0x05,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//7
0x48,0x33,0xC4,0x48,0x89,0x45,0x27,0x48,0x8D,0x55,
0xD7,0x48,0x8B,0xD9

		};

		bool success = false;
		for (uint32_t i = 0; i + sizeof(onSetPanelShowStatesFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, onSetPanelShowStatesFeature, sizeof(onSetPanelShowStatesFeature)))
			{
				Log("CSP OnSetPanelShowStates Func Finded.");
				addrJson["CspAddressRVA"]["CSPInfo_OnSetPanelShowStates_Func"] = _VA + (uint32_t)i;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSP OnSetPanelShowStates Func Not Found!");
	}



#ifdef CSPMOD_PRIVATE
	PushPaintColorAddr();
#endif

	{
		// The preceding virtual call uses a stack temporary whose offset can change.
		// Keep the four point loads, calls, and result stores used by the toggle patch.
		constexpr uint8_t feature[] = {
			0x48,0x8B,0x4D,0x90, 0x48,0x8B,0x01,
			0x48,0x8D,0x95,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD, 0xFF,0x50,0x18,
			0x48,0x8D,0x8E,0x60,0x02,0x00,0x00, 0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
			0x4C,0x8B,0xF0, 0x48,0x89,0x44,0x24,0x48,
			0x48,0x8D,0x8E,0x70,0x02,0x00,0x00, 0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
			0x48,0x8B,0xD8, 0x48,0x89,0x44,0x24,0x40,
			0x48,0x8D,0x8E,0x80,0x02,0x00,0x00, 0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
			0x48,0x8B,0xF8, 0x48,0x89,0x45,0xA0,
			0x48,0x8D,0x8E,0x90,0x02,0x00,0x00, 0xE8,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,
			0x48,0x8B,0xF0, 0x48,0x89,0x44,0x24,0x50, 0x4C,0x8B,0x6D,0x90,
			0x4D,0x8B,0x65,0x00, 0xB2,0xFF
		};
		constexpr uint32_t patchOffset = 17;
		constexpr uint32_t callOffsets[] = {7, 27, 47, 66};
		uint32_t patchRva = 0;
		for (uint32_t i = 0; i + sizeof(feature) <= _codeMemSize; ++i)
		{
			if (!_MatchFeatureCode(_codeMem + i, feature, sizeof(feature)))
				continue;

			const auto callTarget = [&](uint32_t offset) {
				const auto call = i + patchOffset + offset;
				int32_t displacement = 0;
				std::memcpy(&displacement, _codeMem + call + 1, sizeof(displacement));
				return static_cast<int64_t>(call) + 5 + displacement;
			};
			const auto target = callTarget(callOffsets[0]);
			if (target < 0 || static_cast<uint64_t>(target) >= _codeMemSize ||
				target != callTarget(callOffsets[1]) || target != callTarget(callOffsets[2]) ||
				target != callTarget(callOffsets[3]))
				continue;
			if (patchRva)
			{
				LogError("CSPNavigator GetRectPos code address is ambiguous.");
				return;
			}
			patchRva = _VA + i + patchOffset;
		}
		if (patchRva)
		{
			addrJson["CspAddressRVA"]["CSPNavigator_GetRectPos_CodeAddr"] = patchRva;
			Log("Located CSPNavigator_GetRectPos_CodeAddr");
		}
		else
			LogError("CSPNavigator GetRectPos code address unavailable.");
	}

}
