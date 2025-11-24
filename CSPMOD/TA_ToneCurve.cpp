#include<Windows.h>
#include"AddressTable.h"
#include"TA_ToneCurve.h"
#include"CSPMOD.h"

#include"LayerObject.h"


void TA_ToneCurve::Init()
{

	void* pToneCurveDialogFunc = AddressTable::GetAddress("ToneCurve_Dialog_Func");
	void* pToneCurveDialogFunc_Layer = AddressTable::GetAddress("ToneCurveLayer_Dialog_Func");
	void* pToneCurveBeforeDrawFunc = AddressTable::GetAddress("ToneCurve_BeforeDraw_Func");
	void* pToneCurveOnParamChangeFunc = AddressTable::GetAddress("ToneCurve_OnParamChange_Func");
	void* pToneCurvePatchAddr1 = AddressTable::GetAddress("ToneCurve_PatchAddr1");

	void* pToneCurvePatchAddr2 = AddressTable::GetAddress("ToneCurve_PatchAddr2");
	void* pToneCurvePatchAddr3 = AddressTable::GetAddress("ToneCurve_PatchAddr3");

	void* pToneCurvePatchAddr4 = AddressTable::GetAddress("ToneCurve_PatchAddr4");

	void* _updatFuncAddr = AddressTable::GetAddress("ToneCurve_UpdatFunc");


	if (pToneCurveDialogFunc && pToneCurveDialogFunc_Layer && pToneCurveBeforeDrawFunc && pToneCurveOnParamChangeFunc
		&& pToneCurvePatchAddr1&& pToneCurvePatchAddr2&& pToneCurvePatchAddr3 && pToneCurvePatchAddr4
		&& _updatFuncAddr
		)
	{
		isAvailable = true;

		CSPMOD::Hook(pToneCurveDialogFunc, Hook_ToneCurveDialog, (void**)&orig_ToneCurveDialog);
		CSPMOD::Hook(pToneCurveDialogFunc_Layer, Hook_ToneCurveLayerDialog, (void**)&orig_ToneCurveLayerDialog);
		CSPMOD::Hook(pToneCurveOnParamChangeFunc, Hook_ToneCurveParamChange, (void**)&orig_ToneCurveParamChange);
		CSPMOD::Hook(pToneCurveBeforeDrawFunc, Hook_ToneCurveBeforeDraw, (void**)&orig_ToneCurveBeforeDraw,CSPMOD::_HOOKREG_R10);


		layerPatchAddr1 = pToneCurvePatchAddr1;

		layerPatchAddr2 = pToneCurvePatchAddr2;
		layerPatchAddr3 = pToneCurvePatchAddr3;
		layerPatchAddr4 = pToneCurvePatchAddr4;
		updatFuncAddr = _updatFuncAddr;
	}



}
void TA_ToneCurve::Enable()
{
	if (!isEnabled && isAvailable)
	{
		isEnabled = true;
	}

}

void TA_ToneCurve::Disable()
{
	if (isEnabled && isAvailable)
	{
		isEnabled = false;
	}
}
int64_t TA_ToneCurve::Hook_ToneCurveDialog(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
	if(!isEnabled)return orig_ToneCurveDialog(arg1, arg2, arg3);



	uintptr_t spCurFunc = CSPMOD::GetRsp();//147cd8

	//根据栈顶获取一系列地址

	baseAddr = spCurFunc;
	pPreview = spCurFunc - 0xb48;
	pCurrentPanel = spCurFunc - 0xb20;
	pRGB_Data = spCurFunc - 0xa80;
	pR_Data = spCurFunc - 0xa80 + 0x208;
	pG_Data = spCurFunc - 0xa80 + 0x208 + 0x208;
	pR_Data = spCurFunc - 0xa80 + 0x208 + 0x208 + 0x208;



	ajustWorking = true;
	auto result = orig_ToneCurveDialog(arg1, arg2, arg3);
	ajustWorking = false;

	paramOfBeforeDraw0 = 0;
	paramOfBeforeDraw1 = 0;

	return result;
}


int64_t TA_ToneCurve::Hook_ToneCurveLayerDialog(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	//return orig_ToneCurveLayerDialog(arg1, arg2, arg3, arg4);


	if (!isEnabled)return orig_ToneCurveLayerDialog(arg1, arg2, arg3, arg4);






	//获取栈顶
	uintptr_t spCurFunc = CSPMOD::GetRsp();//0x148018;

	//根据栈顶获取一系列地址
	baseAddr = spCurFunc;
	pPreview = 0;
	pCurrentPanel = spCurFunc - 0x1cf0;
	pRGB_Data = spCurFunc - 0x10f8;
	pR_Data = spCurFunc - 0x10f8 + 0x82;
	pG_Data = spCurFunc - 0x10f8 + 0x82 + 0x82;
	pR_Data = spCurFunc - 0x10f8 + 0x82 + 0x82 + 0x82;




	uint8_t cmd_mov_rax_1_return[7 + 5] = {
		0x48,0xc7 ,0xc0,0x01 ,0x00,0x00,0x00,
		0xe9,0x99,0x00,0x00,0x00
	};
	uint8_t cmd_orig[5 + 2 + 5];
	memcpy(cmd_orig,layerPatchAddr1,sizeof(cmd_orig));
	//uint8_t cmd_orig[5 + 2 + 5] = {
	//	0xbf,0x01,0X00,0X00,0X00,
	//0x8B,0xcd,
	//0xe8,0x71,0xef,0x82,0x00
	//};


	//这里patch会导致关闭调整图层对话框后打开普通的曲线图层时软件崩溃
	//草原来这里有个E8
	CSPMOD::CodePatch(layerPatchAddr1, cmd_mov_rax_1_return, sizeof(cmd_mov_rax_1_return));
	layerWorking = true;
	auto result = orig_ToneCurveLayerDialog(arg1, arg2, arg3, arg4);
	layerWorking = false;
	CSPMOD::CodePatch(layerPatchAddr1, cmd_orig, sizeof(cmd_orig));


	paramOfBeforeDraw0 = 0;
	paramOfBeforeDraw1 = 0;

	return result;
}

int64_t TA_ToneCurve::Hook_ToneCurveParamChange(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{

	if (!isEnabled)return orig_ToneCurveParamChange(arg1, arg2, arg3);



	auto result = orig_ToneCurveParamChange(arg1, arg2, arg3);

	//必须添加这句提前退出，以防止调整当前面板的的时候闪退
	//if (!ins.ajustWorking || !ins.layerWorking)
	//	return result;
	//上面这句会导致流畅调色失效....

	if (!ajustWorking && !layerWorking)
		return result;

	//计划16ms采样一次
	static uint64_t startTimer=SDL_GetTicks();
	uint64_t curTimer = SDL_GetTicks();
	//SDL_Log("toneCurveCall:%llu ms", curTimer- startTimer);
	if (curTimer - startTimer < 33)return result;
	startTimer = SDL_GetTicks();



	uint32_t curPanel;
	if (!CSPMOD::TryGetValue(pCurrentPanel, &curPanel))return result;




	if (ajustWorking)
	{
		bool isPreview;
		if (!CSPMOD::TryGetValue(pPreview, &isPreview))return result;

		if (!isPreview)
			return result;



		uintptr_t pointData = pRGB_Data + 0x208 * curPanel;
		//数据复制到栈区

		CurvePointList pointlist = _GetCurrentPanelPointList();
		uint32_t pointCount = static_cast<uint32_t>(((uintptr_t)pointlist.pEnd - (uintptr_t)pointlist.pStart) / 16);

		if(!CSPMOD::IsPtrValid(pointData))return result;
		*(uint32_t*)pointData = pointCount;


		memcpy((void*)(pointData + 8), pointlist.pStart, pointCount * 2 * sizeof(double));

		//调用绘制函数
		if (paramOfBeforeDraw0 && paramOfBeforeDraw1)
		{



			//Hook_ToneCurveBeforeDraw2在windows版本里被内联了....
			//对参数和函数体进行控制让函数按预期逻辑进行

			int32_t* pValue = (int32_t*)(paramOfBeforeDraw1 + 8);
			int32_t oldValue = *pValue;
			*pValue = 1234;//随机无规律数字均可





			//0x14104BDE3处的分支不跳过
			uint8_t cmd_nop_nop[2] = { 0x90,0x90 };

			uint8_t cmd_orig[2] = { 0x74,0x1b };
			uint8_t cmd_nop_nop_nop_nop_nop_nop[6] = { 0x90,0x90 ,0x90,0x90 ,0x90,0x90 };
			uint8_t cmd_orig_BDB1[6] = { 0x0F,0x84,0X7F,0X00,0X00,0X00 };

			CSPMOD::CodePatch(layerPatchAddr2, cmd_nop_nop, sizeof(cmd_nop_nop));
			CSPMOD::CodePatch(layerPatchAddr3, cmd_nop_nop_nop_nop_nop_nop, sizeof(cmd_nop_nop_nop_nop_nop_nop));






			Hook_ToneCurveBeforeDraw(paramOfBeforeDraw0, paramOfBeforeDraw1);


			CSPMOD::CodePatch((void*)(layerPatchAddr2), cmd_orig, sizeof(cmd_orig));
			CSPMOD::CodePatch((void*)(layerPatchAddr3), cmd_orig_BDB1, sizeof(cmd_orig_BDB1));

			*pValue = oldValue;

			//char buf[1024];
			//sprintf_s(buf,"PY:%lf", *(double*)(pointData+16));
			//OutputDebugStringA(buf);
			//目前跳过的逻辑还是存在数据覆盖..

		}
		return result;
	}
	if (layerWorking)
	{
		//return result;






		//数据复制到栈区
		uint16_t* pointData = (uint16_t*)(pRGB_Data + 0x82 * curPanel);
		CurvePointList pointlist = _GetCurrentPanelPointList();

		uint16_t pointCount = static_cast<uint16_t>(((uintptr_t)pointlist.pEnd - (uintptr_t)pointlist.pStart) / 16);
		pointData[0] = pointCount;
		for (int i = 0; i < pointCount; i++)
		{
			uint16_t curPointX = static_cast<uint16_t>(65535 * pointlist.pStart[2 * i]);
			uint16_t curPointY = static_cast<uint16_t>(65535 * pointlist.pStart[2 * i + 1]);

			pointData[1 + 2 * i] = curPointX;
			pointData[1 + 2 * i + 1] = curPointY;
		}

		//伪造刷新数据的函数

		uint64_t(*pUpdateFunc)(uint64_t, uint64_t);
		pUpdateFunc = (uint64_t(*)(uint64_t, uint64_t))(updatFuncAddr);

		struct
		{
			uint64_t padding;
			int32_t updateFlag = 0X1000037F;//mac中这个数字是0x10000376，这个地方疑似会被更新。
		}fakeData;



		uint8_t cmd_orig[6] = { 0x0F ,0x84 ,0xA0 ,0x00 ,0x00 ,0x00 };
		uint8_t cmd_nop_nop[6] = { 0x90,0x90,0x90,0x90,0x90,0x90 };
		CSPMOD::CodePatch(layerPatchAddr4, cmd_nop_nop, sizeof(cmd_nop_nop));





		pUpdateFunc(pRGB_Data, (uint64_t)&fakeData);



		CSPMOD::CodePatch(layerPatchAddr4, cmd_orig, sizeof(cmd_orig));



		return result;
	}





	return  result;
}

int64_t TA_ToneCurve::Hook_ToneCurveBeforeDraw(uintptr_t arg1, uintptr_t arg2)
{

	if (!isEnabled)return orig_ToneCurveBeforeDraw(arg1, arg2);



	paramOfBeforeDraw0 = arg1;
	paramOfBeforeDraw1 = arg2;

	return orig_ToneCurveBeforeDraw(arg1, arg2);
}

TA_ToneCurve::CurvePointList TA_ToneCurve::_GetCurrentPanelPointList()
{
	//[栈底(baseAddr)-0xb08]+0x280]+0x1b0]+0x10]+0x8]+0x28]+0x8]


#define SAFEGETVALUE(x) if(CSPMOD::IsPtrValid(x))value = *(uintptr_t*)(x);else return CurvePointList()
if (ajustWorking)
{

	uintptr_t value;

	SAFEGETVALUE(baseAddr - 0xb08);
	SAFEGETVALUE(value + 0x280);
	SAFEGETVALUE(value + 0x1b0);
	SAFEGETVALUE(value + 0x10);
	SAFEGETVALUE(value + 0x8);
	SAFEGETVALUE(value + 0x28);
	SAFEGETVALUE(value + 0x8);

	if (CSPMOD::IsPtrValid(value))return *(CurvePointList*)value;
	else return CurvePointList();
}

//不想再找一次，后面堆栈关系应该是一致的
// X - (spCurFuncLayer - 0x1C50) = (spCurFunc-0XB08)-(spCurFunc - 0xa80);
//X=spCurFuncLayer-0X1C50-0XB08+0XA80; X=spCurFuncLayer-1CD8;
if (layerWorking)
{

	uintptr_t value;
	SAFEGETVALUE(baseAddr - 0x1CD8);
	SAFEGETVALUE(value + 0x280);
	SAFEGETVALUE(value + 0x1b0);
	SAFEGETVALUE(value + 0x10);
	SAFEGETVALUE(value + 0x8);
	SAFEGETVALUE(value + 0x28);
	SAFEGETVALUE(value + 0x8);


	if (CSPMOD::IsPtrValid(value))return *(CurvePointList*)value;
	else return CurvePointList();
}


return CurvePointList();
}
