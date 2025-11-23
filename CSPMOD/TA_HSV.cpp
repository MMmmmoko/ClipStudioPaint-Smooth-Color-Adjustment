#include"TA_HSV.h"
#include"AddressTable.h"
#include"CSPMOD.h"



//恢复信息
static uint8_t origPatchData[6];

void TA_HSV::Init()
{
	

	pSkipAddr = (uint8_t*)AddressTable::GetAddress("HSV_SkipTimer");

	if (pSkipAddr)
	{
		//uint8_t patch[6] = { 0x90,0x90 ,0x90 ,0x90 ,0x90 ,0x90 };
		////修改方式是填充6字节以跳过
		//CSPMOD::CodePatch(pSkipStr, patch,sizeof(patch));


		isAvailable = true;
	}
}

void TA_HSV::Enable()
{
	if (!isEnabled && pSkipAddr&& isAvailable)
	{
		uint8_t patch[6] = { 0x90,0x90 ,0x90 ,0x90 ,0x90 ,0x90 };
		memcpy(origPatchData, pSkipAddr,sizeof(patch));
		CSPMOD::CodePatch(pSkipAddr, patch, sizeof(patch));
		isEnabled = true;
	}
}

void TA_HSV::Disable()
{
	if (isEnabled && pSkipAddr&& isAvailable)
	{
		isEnabled = false;
		CSPMOD::CodePatch(pSkipAddr, origPatchData, sizeof(origPatchData));
	}
}
