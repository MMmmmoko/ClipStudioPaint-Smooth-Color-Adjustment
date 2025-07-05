// pch.cpp: 与预编译标头对应的源文件
#include"../HookTool.h"


#include "ColorBalance_Hook.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。




static int64_t (*orig_ColorBalanceDialog)(uintptr_t arg0, uintptr_t arg1)=NULL;
int64_t ColorBalance_hook::Hook_ColorBalanceDialog(uintptr_t arg0, uintptr_t arg1)
{
	auto& ins = GetIns();

	ins.isAdjustmentWorking = true;
	auto result = orig_ColorBalanceDialog(arg0,arg1);
	ins.isAdjustmentWorking = false;
	
	ins.beforeDrawParam0 = 0;
	ins.beforeDrawParam1 = 0;


	return result;
}

static int64_t(*orig_ColorBalanceDialog_Layer)(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2) = NULL;
int64_t ColorBalance_hook::Hook_ColorBalanceDialog_Layer(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2)
{
	auto& ins = GetIns();

	ins.isLayerWorking = true;
	auto result = orig_ColorBalanceDialog_Layer(arg0, arg1,arg2);
	ins.isLayerWorking = false;


	ins.beforeDrawParam0 = 0;
	ins.beforeDrawParam1 = 0;

	return result;
}



static int64_t(*orig_ColorBalanceDialogBeforeDraw)(uintptr_t arg0, uintptr_t arg1) = NULL;
int64_t ColorBalance_hook::Hook_ColorBalanceDialogBeforeDraw(uintptr_t arg0, uintptr_t arg1)
{
	auto& ins = GetIns();


	ins.beforeDrawParam0 = arg0;
	ins.beforeDrawParam1 = arg1;

	return orig_ColorBalanceDialogBeforeDraw(arg0,arg1);
}

static int64_t(*orig_ColorBalanceOnParamChange)(uintptr_t arg0, uintptr_t arg1) = NULL;
int64_t ColorBalance_hook::Hook_ColorBalanceOnParamChange(uintptr_t arg0, uintptr_t arg1)
{

	auto& ins = GetIns();

	auto result = orig_ColorBalanceOnParamChange(arg0,arg1);

	if (ins.isAdjustmentWorking)
	{
		//判断预览
		bool isPreview = *(bool*)(arg0+0x244);
		if (isPreview&& ins.beforeDrawParam0&& ins.beforeDrawParam1)
		{
			Hook_ColorBalanceDialogBeforeDraw(ins.beforeDrawParam0, ins.beforeDrawParam1);
			return result;
		}
	}


	if ( ins.isLayerWorking && ins.beforeDrawParam0 && ins.beforeDrawParam1)
	{
		Hook_ColorBalanceDialogBeforeDraw(ins.beforeDrawParam0, ins.beforeDrawParam1);
	}



	return result;
}









void ColorBalance_hook::SetUpHook()
{


	//色彩平衡对话框（非图层）
	//
	// 寻找思路：见HSV对话框思路， 有MAC的结果后可以通过模式对话框断点按堆栈找。
	// 
	// 特征码45 8B 4E 10 41 8B 7E 1C 41 8B 5E 14 45 8B 56 20 45 8B 5E 0C 41 8B 4E 18 41 8B 46 24 89 44 24 50 44 89 54 24 48
	//	89 7C 24 40 89 4C 24 38 89 5C 24 30 44 89 4C 24 28 44 89 5C 24 20 45 8B 4E 08 45 8B 46 04 41 8B 16 48 8D 4D 50
	CatHook::Hook((void*)(CatHook::baseAddr + 0x1406d6CA0), (void*)ColorBalance_hook::Hook_ColorBalanceDialog, (void**)&orig_ColorBalanceDialog, 12);



	//色彩平衡图层对话框
	//
	//寻找思路，见其他对话框思路，这个对话框逻辑图看起来和mac区别有点大，有关键的申请0x30内存帮助确认
	//特征码48 33 C4 48 89 85 10 01 00 00 4D 8B F8 48 8B FA 48 8B F1 48 89 4D 48 48 89 55 50 48 8D 44 24 40 48 89 44 24 50 0F 57 F6 33 C0

	CatHook::Hook((void*)(CatHook::baseAddr + 0x1406D66B0), (void*)ColorBalance_hook::Hook_ColorBalanceDialog_Layer, (void**)&orig_ColorBalanceDialog_Layer, 12);




	//色彩平衡绘制前经过的地方
	//
	//寻找思路：先找刷新画布的函数（比对hsv获得），打断点，往上回溯发现是间接调用，那么寻找此时的函数地址的交叉引用
	//能发现有一个直接将函数地址赋值给寄存器的地方，再在这里打断点往上追溯即可找到本函数。有个明显的1B5AE的立即数赋值
	//特征码 85 C0 74 31 48 8D 4B 60 BA AE 5B 01 00
	CatHook::Hook((void*)(CatHook::baseAddr + 0x1406D76B0), (void*)ColorBalance_hook::Hook_ColorBalanceDialogBeforeDraw, (void**)&orig_ColorBalanceDialogBeforeDraw, 12);
	
	//色彩平衡参数回调
	//
	//寻找思路：cheatEngine搜索值然后打硬件断点，调用堆栈向上回溯寻找最早的修改参数会暂停的地方。
	//与MAC函数体看起来区别较大
	//特征码 确定可行后再填写 todo
	//CatHook::Hook((void*)(CatHook::baseAddr + 0x1406D77A0), (void*)ColorBalance_hook::Hook_ColorBalanceOnParamChange, (void**)&orig_ColorBalanceOnParamChange, 15);
	CatHook::Hook((void*)(CatHook::baseAddr + 0x1406DAA70), (void*)ColorBalance_hook::Hook_ColorBalanceOnParamChange, (void**)&orig_ColorBalanceOnParamChange, 15);

	

	//色彩平衡跳过计时
	//
	//寻找思路，找到一个将立即数转为毫秒的函数0x141F0D1C0，在一次绘制之间给这个函数打断点即可
	//特征码EB 09 45 84 E4 0F 84 35 01 00 00 48 83 3B 00 0F 84 2B 01 00 00 83 7D FB 00 0F 84 CC 00 00 00 48 83 7F 08 00 0F 84 16 01 00 00 48 8D 4D B7
	uint8_t cmd_mov_edx_0_nop[6] = { 0xba,0x00,0x00 ,0x00 ,0x00 ,0x90 };
	CatHook::CodePatch((void*)(CatHook::baseAddr + 0x1406D6624),cmd_mov_edx_0_nop,sizeof(cmd_mov_edx_0_nop));
	uint8_t cmd_nop_nop[2] = { 0x90 ,0x90 };
	CatHook::CodePatch((void*)(CatHook::baseAddr + 0x1406D76ca), cmd_nop_nop,sizeof(cmd_nop_nop));


}
