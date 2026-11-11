#include"AddressGenerator.h"


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



	//设置颜色的函数地址（基于吸管）
	//45 8B CF 4C 8D 45 48 48 8D 55 E0 E8 ?? ?? ?? ?? 85 DB 74 0C 48 8D 4C 24 40 E8 ?? ?? ?? ?? EB
	{
		//使用外部特征
		uint8_t setColorFuncFeature[] = {
			0x45,0x8B,0xCF,0x4C,0x8D,0x45,0x48,0x48,0x8D,0x55,0xE0,//11
			0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
			0x85,0xDB,0x74,0x0C,0x48,0x8D,0x4C,0x24,0x40,//9
			0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
			0xEB//1
		};

		bool success = false;
		for (uint32_t i = 0; i + sizeof(setColorFuncFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, setColorFuncFeature, sizeof(setColorFuncFeature)))
			{
				//此代码最后的E8即是调用地址
				uint32_t off = *(uint32_t*)(_codeMem + i + 11+5+9+1);

				Log("CSPPaintColor SetColor Func Finded.");
				addrJson["CspAddressRVA"]["CSPPaintColor_SetColor_Func"] = _VA + i + 11+5+9+ 5 + off;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSPPaintColor SetColor Func Not Found!");
	}

	//获取主副色所用的对象、设置主副色、释放查询对象
	{
		//3B C3 41 0F 4E DC 8B D3 48 8D 8C 24 ? ? ? ? ? ? ? ? E8 ? ? ? ? ? ? ? ? 90 48 8D 94 24 ? ? ? ? ? ? ? ? 8B CB E8 ? ? ? ? ? ? ? ? 44 8B EE 48 8D 8C 24 ? ? ? ? ? ? ? ? E8 ? ? ? ? ? ? ? ?

		uint8_t GetSetColorTypeFeature[] = {
		0x3B,0xC3,0x41,0x0F,0x4E,0xDC,0x8B,0xD3,//8
		0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD ,//8
		0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD ,//5
		0x90,//1
		0x48,0x8D,0x94,0x24,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//8
		0x8B,0xCB,//2
		0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD ,//5
		0x44,0x8B,0xEE,//3
		0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//8
		0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD//5
		};

		//每个0x48,0x8D,0x8C,0x24,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD 指向的数值相等
		bool success = false;
		for (uint32_t i = 0; i + sizeof(GetSetColorTypeFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, GetSetColorTypeFeature, sizeof(GetSetColorTypeFeature)))
			{
				uint32_t val1= *(uint32_t*)(_codeMem + i + 8 + 4 );
				uint32_t val2= *(uint32_t*)(_codeMem + i + 8 + 8+5+1+4 );
				uint32_t val3= *(uint32_t*)(_codeMem + i + 8 + 8+5+1+8+2+5+3+4 );
				if (val1 == val2 && val1 == val3)
				{
					
					uint32_t off = *(uint32_t*)(_codeMem + i + 8 + 8 + 1);
					Log("CSPPaintColor GetColorHost Func Finded.");
					addrJson["CspAddressRVA"]["CSPPaintColor_GetColorHost_Func"] = _VA + i + 8 + 8 + 5 + off;
					
					off = *(uint32_t*)(_codeMem + i + 8 + 8 + 5+1+8+2+1);
					Log("CSPPaintColor SetWorkingColor Func Finded.");
					addrJson["CspAddressRVA"]["CSPPaintColor_SetWorkingColor_Func"] = _VA + i + 8 + 8 + 5 + 1 + 8 + 2 + 5 + off;
					
					off = *(uint32_t*)(_codeMem + i + 8 + 8 + 5+1+8+2+5+3+8+1);
					Log("CSPPaintColor ReleaseColorHost Func Finded.");
					addrJson["CspAddressRVA"]["CSPPaintColor_ReleaseColorHost_Func"] = _VA + i + 8 + 8 + 5 + 1 + 8 + 2 + 5 + 3 + 8 + 5 + off;


					success = true;
					break;

				}

			}
		}
		if (!success)
		{
			LogError( "CSPPaintColor GetColorHost Func Not Found!");
			LogError( "CSPPaintColor SetWorkingColor Func Not Found!");
			LogError( "CSPPaintColor ReleaseColorHost Func Not Found!");

		}

	}
	{
	//一个监听颜色变化的函数，用于在色轮中同步来自CSP的修改
	
		//48 8B 87 80 02 00 00 48 89 44 24 70 48 8B 8F 88 02 00 00 48 89 4C 24 78 48 85 C9 74 07 8B C6 F0 0F C1 41 08 4C 8D 44 24 40 8B D3 48 8D 4C 24 70 E8 ? ? ? ? ? ? ? ? FF C3 41 3B DC
	


		uint8_t onColorChangedFuncFeature[] = {
			0X48,0x8B,0x87,0x80,0x02,0x00,0x00,0x48,0x89,0x44,//10
			0x24,0x70,0x48,0x8B,0x8F,0x88,0x02,0x00,0x00,0x48,//10
			0x89,0x4C,0x24,0x78,0x48,0x85,0xC9,0x74,0x07,0x8B,//10
			0xC6,0xF0,0x0F,0xC1,0x41,0x08,0x4C,0x8D,0x44,0x24,//10
			0x40,0x8B,0xD3,0x48,0x8D,0x4C,0x24,0x70,//8
			0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
			0xFF,0xC3,0x41,0x3B,0xDC
		};

		bool success = false;
		for (uint32_t i = 0; i + sizeof(onColorChangedFuncFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, onColorChangedFuncFeature, sizeof(onColorChangedFuncFeature)))
			{
				//此代码最后的E8即是调用地址
				uint32_t off = *(uint32_t*)(_codeMem + i + 10 + 10 + 10  +10+8+1);

				Log("CSPPaintColor OnColorChanged Func Finded.");
				addrJson["CspAddressRVA"]["CSPPaintColor_OnColorChanged_Func"] = _VA + i + 10 + 10 + 10 + 10 + 8 + 5 + off;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSPPaintColor OnColorChanged Func Not Found!");
	}








	{
	//一段获取导航器红框坐标的地址，通过修改他以禁用导航器红框绘制
	
		//48 8B 4D 90 48 8B 01 48 8D 95 20 02 00 00 FF 50 18 48 8D 8E 60 02 00 00 E8 ?? ?? ?? ?? 4C 8B F0 48 89 44 24 48 48 8D 8E 70 02 00 00 E8 ?? ?? ?? ?? 48 8B D8 48 89 44 24 40 48 8D 8E 80 02 00 00 E8 ?? ?? ?? ?? 48 8B F8 48 89 45 A0 48 8D 8E 90 02 00 00 E8 ?? ?? ?? ?? 48 8B F0 48 89 44 24 50 4C 8B 6D 90 4D 8B 65 00 B2 FF	


		uint8_t navigatorRectPosConvertCodeFeature[] = {
0x48,0x8B,0x4D,0x90,0x48,0x8B,0x01,0x48,0x8D,0x95,//10
0x20,0x02,0x00,0x00,0xFF,0x50,0x18,//7


0x48,0x8D,0x8E,0x60,0x02,0x00,0x00,//7

0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
0x4C,0x8B,0xF0,0x48,0x89,0x44,0x24,0x48,0x48,0x8D,//10
0x8E,0x70,0x02,0x00,0x00,//5
0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
0x48,0x8B,0xD8,0x48,0x89,0x44,0x24,0x40,0x48,0x8D,//10
0x8E,0x80,0x02,0x00,0x00,//5
0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
0x48,0x8B,0xF8,0x48,0x89,0x45,0xA0,0x48,0x8D,0x8E,//10
0x90,0x02,0x00,0x00,//4
0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
0x48,0x8B,0xF0,0x48,0x89,0x44,0x24,0x50,0x4C,0x8B,//10
0x6D,0x90,0x4D,0x8B,0x65,0x00,0xB2,0xFF//8
		};

		bool success = false;
		for (uint32_t i = 0; i + sizeof(navigatorRectPosConvertCodeFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, navigatorRectPosConvertCodeFeature, sizeof(navigatorRectPosConvertCodeFeature)))
			{

				Log("CSPNavigator GetRectPos Code Addr Finded.");
				addrJson["CspAddressRVA"]["CSPNavigator_GetRectPos_CodeAddr"] = _VA + i + 10 + 7 ;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSPNavigator GetRectPos CodeAddr Func Not Found!");
	}

}