// pch.cpp: 与预编译标头对应的源文件
#include"../HookTool.h"

#include "ToneCurve_Hook.h"
#include<string>
#include"Windows.h"
// 当使用预编译的头时，需要使用此源文件，编译才能成功。




static int64_t (*orig_ToneCurveDialog)(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)=NULL;
int64_t ToneCurve_hook::Hook_ToneCurveDialog(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
	_Init();


	//获取栈顶
	//或者说是下个函数的栈底
	uintptr_t spCurFunc = CatHook::GetRsp();//147cd8

	//根据栈顶获取一系列地址
	ToneCurve_hook& ins = ToneCurve_hook::GetIns();
	ins.baseAddr = spCurFunc;
	ins.pPreview = spCurFunc - 0xb48;
	ins.pCurrentPanel = spCurFunc - 0xb20;
	ins.pRGB_Data = spCurFunc - 0xa80;
	ins.pR_Data = spCurFunc - 0xa80 +0x208;
	ins.pG_Data = spCurFunc - 0xa80 +0x208 + 0x208;
	ins.pR_Data = spCurFunc - 0xa80 +0x208 + 0x208 + 0x208;

	//MessageBoxA(NULL,std::to_string(spCurFunc).c_str(), "DialogRsp",MB_OK );




	//uint8_t cmd_nop_nop_nop_nop_nop_nop[6] = { 0x90,0x90 ,0x90,0x90 ,0x90,0x90 };
	//uint8_t cmd_orig_BDB1[6] = { 0x0F,0x84,0X7F,0X00,0X00,0X00 };
		//CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104BDB1), cmd_nop_nop_nop_nop_nop_nop, sizeof(cmd_nop_nop_nop_nop_nop_nop));






	ins.ajustWorking = true;
	auto result = orig_ToneCurveDialog(arg1, arg2, arg3);
	ins.ajustWorking = false;


		//CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104BDB1), cmd_orig_BDB1, sizeof(cmd_orig_BDB1));



	ins.paramOfBeforeDraw0 = 0;
	ins.paramOfBeforeDraw1 = 0;


	return result;
}
static int64_t (*orig_ToneCurveLayerDialog)(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)=NULL;
int64_t ToneCurve_hook::Hook_ToneCurveLayerDialog(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{

	_Init();



	//获取栈顶
	uintptr_t spCurFunc = CatHook::GetRsp();//0x148018;

	//根据栈顶获取一系列地址
	ToneCurve_hook& ins = ToneCurve_hook::GetIns();
	ins.baseAddr = spCurFunc;
	ins.pPreview = 0;
	ins.pCurrentPanel = spCurFunc - 0x1cf0;
	//ins.pRGB_Data = spCurFunc - 0x1C50;
	//ins.pR_Data = spCurFunc - 0x1C50 + 0x208;
	//ins.pG_Data = spCurFunc - 0x1C50 + 0x208 + 0x208;
	//ins.pR_Data = spCurFunc - 0x1C50 + 0x208 + 0x208 + 0x208;
	ins.pRGB_Data = spCurFunc - 0x10f8;
	ins.pR_Data = spCurFunc - 0x10f8 + 0x82;
	ins.pG_Data = spCurFunc - 0x10f8 + 0x82 + 0x82;
	ins.pR_Data = spCurFunc - 0x10f8 + 0x82 + 0x82 + 0x82;




	uint8_t cmd_mov_rax_1_return[7+5] = {
		0x48,0xc7 ,0xc0,0x01 ,0x00,0x00,0x00,
		0xe9,0x99,0x00,0x00,0x00
	};
	uint8_t cmd_orig[5+2+5] = {
		0xbf,0x01,0X00,0X00,0X00,
	0x8B,0xcd,
	0xe8,0x71,0xef,0x82,0x00
	};
		CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104BDA3), cmd_mov_rax_1_return, sizeof(cmd_mov_rax_1_return));




	ins.layerWorking = true;
	auto result=orig_ToneCurveLayerDialog(arg1, arg2, arg3,arg4);
	ins.layerWorking = false;




	CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104BDA3), cmd_orig, sizeof(cmd_orig));

	ins.paramOfBeforeDraw0 = 0;
	ins.paramOfBeforeDraw1 = 0;

	return result;
}


static int64_t (*orig_ToneCurveParamChange)(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)=NULL;
int64_t ToneCurve_hook::Hook_ToneCurveParamChange(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
	ToneCurve_hook& ins = ToneCurve_hook::GetIns();

	auto result= orig_ToneCurveParamChange(arg1, arg2, arg3);
	
	//必须添加这句提前退出，以防止调整当前面板的的时候闪退
	if (!ins.ajustWorking || !ins.layerWorking)
		return result;



	auto curPanel = *(uint32_t*)ins.pCurrentPanel;

	


	if (ins.ajustWorking)
	{
		if (!(*(uint8_t*)ins.pPreview))
			return result;

		uintptr_t pointData = ins.pRGB_Data + 0x208 * curPanel;
		//数据复制到栈区

		CurvePointList pointlist=_GetCurrentPanelPointList();
		uint32_t pointCount= ((uintptr_t)pointlist.pEnd - (uintptr_t)pointlist.pStart) / 16;
		*(uint32_t*)pointData = pointCount;
		memcpy((void*)(pointData+8),pointlist.pStart,pointCount*2*sizeof(double));

		//调用绘制函数
		if (ins.paramOfBeforeDraw0 && ins.paramOfBeforeDraw1)
		{
			


			//Hook_ToneCurveBeforeDraw2在windows版本里被内联了....
			//对参数和函数体进行控制让函数按预期逻辑进行

			int32_t* pValue = (int32_t*)(ins.paramOfBeforeDraw1 + 8);
			int32_t oldValue = *pValue;
			*pValue = 1234;//随机无规律数字均可


			
			

			//0x14104BDE3处的分支不跳过
			uint8_t cmd_nop_nop[2] = { 0x90,0x90 };

			uint8_t cmd_orig[2] = { 0x74,0x1b };
			uint8_t cmd_nop_nop_nop_nop_nop_nop[6] = { 0x90,0x90 ,0x90,0x90 ,0x90,0x90 };
			uint8_t cmd_orig_BDB1[6] = { 0x0F,0x84,0X7F,0X00,0X00,0X00 };
			CatHook::CodePatch((void*)(CatHook::baseAddr+0x14104BDE3),cmd_nop_nop,sizeof(cmd_nop_nop));
			CatHook::CodePatch((void*)(CatHook::baseAddr+0x14104BDB1), cmd_nop_nop_nop_nop_nop_nop,sizeof(cmd_nop_nop_nop_nop_nop_nop));


		



			Hook_ToneCurveBeforeDraw(ins.paramOfBeforeDraw0, ins.paramOfBeforeDraw1);


			CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104BDE3), cmd_orig, sizeof(cmd_orig));
			CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104BDB1), cmd_orig_BDB1, sizeof(cmd_orig_BDB1));

			*pValue = oldValue;

			//char buf[1024];
			//sprintf_s(buf,"PY:%lf", *(double*)(pointData+16));
			//OutputDebugStringA(buf);
			//目前跳过的逻辑还是存在数据覆盖..

		}
		return result;
	}
	if (ins.layerWorking)
	{


		//数据复制到栈区
		uint16_t* pointData = (uint16_t*)(ins.pRGB_Data + 0x82 * curPanel);
		CurvePointList pointlist=_GetCurrentPanelPointList();

		uint16_t pointCount= ((uintptr_t)pointlist.pEnd - (uintptr_t)pointlist.pStart) / 16;
		pointData[0] = pointCount;
		for (int i=0;i<pointCount;i++)
		{
			uint16_t curPointX = 65535*pointlist.pStart[2*i];
			uint16_t curPointY = 65535*pointlist.pStart[2*i+1];

			pointData[1 + 2 * i] = curPointX;
			pointData[1 + 2 * i+1] = curPointY;
		}

		//伪造刷新数据的函数

		uint64_t(*pUpdateFunc)(uint64_t, uint64_t);
		pUpdateFunc = (uint64_t(*)(uint64_t, uint64_t))(CatHook::baseAddr + 0X14104B500);

		struct
		{
			uint64_t padding;
			int32_t updateFlag = 0X1000037F;//mac中这个数字是0x10000376，这个地方疑似会被更新。
		}fakeData;



		uint8_t cmd_orig[6] = { 0x0F ,0x84 ,0xA0 ,0x00 ,0x00 ,0x00 };
		uint8_t cmd_nop_nop[6] = { 0x90,0x90,0x90,0x90,0x90,0x90 };
		CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104B693), cmd_nop_nop, sizeof(cmd_nop_nop));





		pUpdateFunc(ins.pRGB_Data, (uint64_t)&fakeData);



		CatHook::CodePatch((void*)(CatHook::baseAddr + 0x14104B693), cmd_orig, sizeof(cmd_orig));



		return result;
	}

	
	
	
	
	return  result;
}


static int64_t(*orig_ToneCurveBeforeDraw)(uintptr_t arg1, uintptr_t arg2) = NULL;
int64_t ToneCurve_hook::Hook_ToneCurveBeforeDraw(uintptr_t arg1, uintptr_t arg2)
{
	ToneCurve_hook& ins = ToneCurve_hook::GetIns();

	ins.paramOfBeforeDraw0 = arg1;
	ins.paramOfBeforeDraw1 = arg2;

	return orig_ToneCurveBeforeDraw( arg1 , arg2);
}



void ToneCurve_hook::_Init()
{
	//可能直接在dll加载函数中hook会导致某些问题，所以尝试延后hook
	ToneCurve_hook& ins = ToneCurve_hook::GetIns();
	if (ins.inited)return;
	ins.inited = true;

	CatHook::HookEx((void*)(CatHook::baseAddr + 0x14104BC30), (void*)ToneCurve_hook::Hook_ToneCurveBeforeDraw, (void**)&orig_ToneCurveBeforeDraw, 13,CatHook::_HOOKREG_R10);


}

ToneCurve_hook::CurvePointList ToneCurve_hook::_GetCurrentPanelPointList()
{
	//[栈底(baseAddr)-0xb08]+0x280]+0x1b0]+0x10]+0x8]+0x28]+0x8]
	ToneCurve_hook& ins = ToneCurve_hook::GetIns();
	if (ins.ajustWorking)
	{
		uintptr_t value = *(uintptr_t*)(ins.baseAddr - 0xb08);
		value = *(uintptr_t*)(value + 0x280);
		value = *(uintptr_t*)(value + 0x1b0);
		value = *(uintptr_t*)(value + 0x10);
		value = *(uintptr_t*)(value + 0x8);
		value = *(uintptr_t*)(value + 0x28);
		value = *(uintptr_t*)(value + 0x8);

		return *(CurvePointList*)value;
	}

	//不想再找一次，后面堆栈关系应该是一致的
	// X - (spCurFuncLayer - 0x1C50) = (spCurFunc-0XB08)-(spCurFunc - 0xa80);
	//X=spCurFuncLayer-0X1C50-0XB08+0XA80; X=spCurFuncLayer-1CD8;
	if (ins.layerWorking)
	{
		uintptr_t value = *(uintptr_t*)(ins.baseAddr - 0x1CD8);
		value = *(uintptr_t*)(value + 0x280);
		value = *(uintptr_t*)(value + 0x1b0);
		value = *(uintptr_t*)(value + 0x10);
		value = *(uintptr_t*)(value + 0x8);
		value = *(uintptr_t*)(value + 0x28);
		value = *(uintptr_t*)(value + 0x8);

		return *(CurvePointList*)value;
	}


	return CurvePointList();
}





void ToneCurve_hook::SetUpHook()
{



	//return;



	//ToneCurve绘制（渲染）函数(非调整图层)对话框打开的函数,用来监控状态的，意义不太大.：
	//
	//寻找思路，先找到模式对话框的函数，在其中打断点。进入ToneCurve窗口，断点外的一层函数即为所在位置。
	//14104a0d0
	//特征码 48 33 C4 48 89 85 50 0C 00 00 4D 8B E8 4C 8B FA 4C 8B F1
	CatHook::Hook((void*)(CatHook::baseAddr+ 0x14104a0d0),(void*)ToneCurve_hook::Hook_ToneCurveDialog,(void**)&orig_ToneCurveDialog,12);



	//ToneCurve调整图层对话框：
	//
	//寻找思路，先找到模式对话框的函数，在其中打断点。进入ToneCurve窗口，断点外的二层函数即为所在位置。有一些常数特征
	//
	//特征码40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 78 EF FF FF B8 88 11 00 00
	CatHook::Hook((void*)(CatHook::baseAddr + 0x141049DC0), (void*)ToneCurve_hook::Hook_ToneCurveLayerDialog, (void**)&orig_ToneCurveLayerDialog, 13);

	//曲线参数回调函数
	//
	//寻找思路，ui中曲线面板数值都是双浮点，在CE中搜索双浮点能找到3个值，其中只有一个在拉动点的时候连续变化，给那个点打写入断点，中断处的函数即可视为参数回调函数。
	//特征码48 89 5C 24 08 48 89 74 24 18 48 89 54 24 10 57 48 83 EC 60 0F 29 74 24 50 49 8B F8 48 8B DA 48 8B F1 48 8D 54 24 40
	CatHook::Hook((void*)(CatHook::baseAddr + 0x141BDE750), (void*)ToneCurve_hook::Hook_ToneCurveParamChange, (void**)&orig_ToneCurveParamChange, 15);

	//曲线绘制调用，是否调整图层均经过的地方,性质更像是发送绘制信号..
	//
	//寻找思路，刷新矩形处打个断点，中断处往上寻找堆栈较大的函数，在其中与mac获取的函数比对特征函数和运行逻辑。
	//特征码40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 78 EF FF FF B8 88 11 00 00
	// 为什么， hook这个会报错 搞不明白
	// 和之前一个函数一样会出现memcpy复制函数的第一个字节被换成cc了 原因不明
	//CatHook::Hook((void*)(CatHook::baseAddr + 0x14104BC30), (void*)ToneCurve_hook::Hook_ToneCurveBeforeDraw, (void**)&orig_ToneCurveBeforeDraw, 13);


	//曲线绘制调用2，真正的绘制函数的上级，这个函数体内还调用了处理数据相关的函数
	//
	//寻找思路，比对mac发现有个明显的常熟1e240
	//特征码40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 78 EF FF FF B8 88 11 00 00
	// 为什么， hook这个会报错 搞不明白
	// 和之前一个函数一样会出现memcpy复制函数的第一个字节被换成cc了 原因不明
	//
	// mov rax jmp rax的方法会破坏寄存器。下面的绘制函数会用rax进行内存申请，
	// rax正好是在跳转的指令内范围内进行赋值的
	// 
	// 
	// 
	// 
	// 
	// CatHook::Hook((void*)(CatHook::baseAddr + 0x14104BC30), (void*)ToneCurve_hook::Hook_ToneCurveBeforeDraw, (void**)&orig_ToneCurveBeforeDraw, 13);







	////ToneCurve绘制跳过计时：142AA1FB8处 不进行跳转 jnl 0x0000000142AA2066改为nop
	////
	////比较麻烦，先找到“返回当前时间”的函数会好办很多..在ToneCurve绘制处打断点，在上方寻找仅鼠标停下才会触发的分支，（在很上方的地方...
	////如果先找到返回时间的函数就可以在调用堆栈附近找此函数，特征是会出现两个返回当前时间的函数并计算差值。
	////
	////特征码(patch前)
	////48 8B F8 8B 0D DA 12 16 02 48 03 8B F0 00 00 00 48 3B C8 0F 8D A8 00 00 00 83 BB E8 00 00 00 00 74 1B C7 83 E8 00 00 00 00 00 00 00 48 8D 8B F8 00 00 00 BA E8 03 00 00
	//uint8_t op_nop_6bytes[6] = { 0x90,0x90, 0x90, 0x90, 0x90, 0x90 };
	//CatHook::CodePatch((void*)(CatHook::baseAddr + 0x142AA1FB8), op_nop_6bytes, sizeof(op_nop_6bytes));




}