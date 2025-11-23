#include"AddressTable.h"
#include"TA_ColorBalance.h"
#include"CSPMOD.h"


//恢复信息
static uint8_t origPatchData[2];

void TA_ColorBalance::Init()
{
	//return;
	pColorBalanceDialogFunc = AddressTable::GetAddress("ColorBalance_Dialog_Func");
	pColorBalanceDialogFunc_Layer = AddressTable::GetAddress("ColorBalanceLayer_Dialog_Func");
	pColorBalanceBeforeDrawFunc = AddressTable::GetAddress("ColorBalance_BeforeDraw_Func");
	pColorBalanceOnParamChangeFunc = AddressTable::GetAddress("ColorBalance_OnParamChange_Func");
	//pColorBalanceSkipTimer = AddressTable::GetAddress("ColorBalance_SkipTimer1");
	pColorBalanceSkipTimer2 = AddressTable::GetAddress("ColorBalance_SkipTimer2");
	if (pColorBalanceDialogFunc && pColorBalanceDialogFunc_Layer && pColorBalanceBeforeDrawFunc && pColorBalanceOnParamChangeFunc /*&& pColorBalanceSkipTimer*/ && pColorBalanceSkipTimer2)
	{
		isAvailable = true;


		CSPMOD::Hook(pColorBalanceDialogFunc, Hook_ColorBalanceDialog,(void**)&orig_ColorBalanceDialog);
		CSPMOD::Hook(pColorBalanceDialogFunc_Layer, Hook_ColorBalanceDialog_Layer,(void**)&orig_ColorBalanceDialog_Layer);
		CSPMOD::Hook(pColorBalanceBeforeDrawFunc, Hook_ColorBalanceDialogBeforeDraw,(void**)&orig_ColorBalanceDialogBeforeDraw);
		CSPMOD::Hook(pColorBalanceOnParamChangeFunc, Hook_ColorBalanceOnParamChange,(void**)&orig_ColorBalanceOnParamChange);


		//点1在CSPHacker::DoPatchTimerPoint()里修改
		
		//奇了怪了，换了电脑还不能稍微流畅预览一点
		//uint8_t cmd_mov_edx_0_nop[6] = { 0xba,0x01,0x00 ,0x00 ,0x00 ,0x90 };
		//CSPMOD::CodePatch(pColorBalanceSkipTimer, cmd_mov_edx_0_nop, sizeof(cmd_mov_edx_0_nop));
		//uint8_t cmd_nop_nop[2] = { 0x90 ,0x90 };
		//CSPMOD::CodePatch(pColorBalanceSkipTimer2, cmd_nop_nop, sizeof(cmd_nop_nop));
	}
}


void TA_ColorBalance::Enable()
{
	if (!isEnabled && isAvailable)
	{
		uint8_t cmd_nop_nop[2] = { 0x90 ,0x90 };
		memcpy(origPatchData, pColorBalanceSkipTimer2, sizeof(cmd_nop_nop));
		CSPMOD::CodePatch(pColorBalanceSkipTimer2, cmd_nop_nop, sizeof(cmd_nop_nop));
		isEnabled = true;
	}

}

void TA_ColorBalance::Disable()
{
	if (isEnabled && isAvailable)
	{
		isEnabled = false;
		CSPMOD::CodePatch(pColorBalanceSkipTimer2, origPatchData, sizeof(origPatchData));
	}
}






int64_t TA_ColorBalance::Hook_ColorBalanceDialog(uintptr_t arg0, uintptr_t arg1)
{
	if(!isEnabled)return orig_ColorBalanceDialog(arg0, arg1);

	isAdjustmentWorking = true;
	auto result = orig_ColorBalanceDialog(arg0, arg1);
	isAdjustmentWorking = false;

	beforeDrawParam0 = 0;
	beforeDrawParam1 = 0;


	return result;
}

int64_t TA_ColorBalance::Hook_ColorBalanceDialog_Layer(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2)
{
	if (!isEnabled)return orig_ColorBalanceDialog_Layer(arg0, arg1,arg2);

	isLayerWorking = true;
	auto result = orig_ColorBalanceDialog_Layer(arg0, arg1, arg2);
	isLayerWorking = false;

	beforeDrawParam0 = 0;
	beforeDrawParam1 = 0;
	return result;
}

int64_t TA_ColorBalance::Hook_ColorBalanceDialogBeforeDraw(uintptr_t arg0, uintptr_t arg1)
{

	if (!isEnabled)return orig_ColorBalanceDialogBeforeDraw(arg0, arg1);


	/*static int drawCallCount = 0;
	drawCallCount++;
	SDL_Log("ColorBalance DrawCall :%d", drawCallCount);*/




	beforeDrawParam0 = arg0;
	beforeDrawParam1 = arg1;

	return orig_ColorBalanceDialogBeforeDraw(arg0, arg1);
}

int64_t TA_ColorBalance::Hook_ColorBalanceOnParamChange(uintptr_t arg0, uintptr_t arg1)
{

	if (!isEnabled)return orig_ColorBalanceOnParamChange(arg0, arg1);






	auto result = orig_ColorBalanceOnParamChange(arg0, arg1);

	//计划16ms采样一次
	static uint64_t startTimer = SDL_GetTicks();
	uint64_t curTimer = SDL_GetTicks();
	//SDL_Log("toneCurveCall:%llu ms", curTimer- startTimer);
	if (curTimer - startTimer < 16)return result;
	startTimer = curTimer;



	if (isAdjustmentWorking)
	{
		//判断预览
		bool isPreview = *(bool*)(arg0 + 0x244);
		if (isPreview && beforeDrawParam0 && beforeDrawParam1)
		{
			Hook_ColorBalanceDialogBeforeDraw(beforeDrawParam0,beforeDrawParam1);
			return result;
		}
	}

	if (isLayerWorking &&beforeDrawParam0 && beforeDrawParam1)
	{
		Hook_ColorBalanceDialogBeforeDraw(beforeDrawParam0, beforeDrawParam1);
	}

	return result;
}