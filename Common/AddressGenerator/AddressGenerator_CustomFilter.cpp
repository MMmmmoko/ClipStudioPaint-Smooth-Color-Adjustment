#include"AddressGenerator.h"


void AddressGenerator::PushCustomFilterAddr()
{
	Log("Start Find CSP CustomFilter Addresses...");
	{
		//66 0F 73 D8 08 66 48 0F 7E C1 48 85 C9 74 07 8B C3 F0 0F C1 41 08 4C 8D 85 A8 00 00 00 48 8D 55 90 48 8D 4D 18 E8 ? ? ? ? ? ? ? ? 0F 10 00 F2 0F 11 87 A0 02 00 00 66 0F 15 C0 F2 0F 11 87 A8 02 00 00 8B D3 48 8D 8D 98 00 00 00

		uint8_t entranceFeature[] = {
0x66,0x0F,0x73,0xD8,0x08,0x66,0x48,0x0F,0x7E,0xC1,//10
0x48,0x85,0xC9,0x74,0x07,0x8B,0xC3,0xF0,0x0F,0xC1,//10
0x41,0x08,0x4C,0x8D,0x85,0xA8,0x00,0x00,0x00,0x48,//10
0x8D,0x55,0x90,0x48,0x8D,0x4D,0x18,//7
0xE8,BYTEWILDCARD, BYTEWILDCARD,BYTEWILDCARD,BYTEWILDCARD,//5
0x0F,0x10,0x00,0xF2,0x0F,0x11,0x87,0xA0,0x02,0x00,//10
0x00,0x66,0x0F,0x15,0xC0,0xF2,0x0F,0x11,0x87,0xA8,//10
0x02,0x00,0x00,0x8B,0xD3,0x48,0x8D,0x8D,0x98,0x00,//10
0x00,0x00//2

		};

		bool success = false;
		for (uint32_t i = 0; i + sizeof(entranceFeature) <= _codeMemSize; i++)
		{
			if (_MatchFeatureCode(_codeMem + i, entranceFeature, sizeof(entranceFeature)))
			{
				//此代码最后的E8即是调用地址
				uint32_t off = *(uint32_t*)(_codeMem + i + 10 + 10 + 10 + 7 + 1);

				Log("CSPCustomFilter WindowPosToCanvasPos Func Finded.");
				addrJson["CspAddressRVA"]["CSPCustomFilter_WindowPosToCanvasPos_Func"] = _VA + i + 10 + 10 + 10 + 7 +  5 + off;
				success = true;
				break;
			}
		}
		if (!success)
			LogError( "CSPCustomFilter WindowPosToCanvasPos Func Not Found!");

	}

}





