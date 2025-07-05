
#ifndef ToneCurve_hook_h
#define ToneCurve_hook_h

#include<iostream>

class ToneCurve_hook
{
public:
	static ToneCurve_hook& GetIns() { static ToneCurve_hook ref; return ref; };

	static void SetUpHook();

	static int64_t Hook_ToneCurveDialog(uintptr_t arg1,uintptr_t arg2,uintptr_t arg3);
	static int64_t Hook_ToneCurveLayerDialog(uintptr_t arg1,uintptr_t arg2,uintptr_t arg3, uintptr_t arg4);


	static int64_t Hook_ToneCurveParamChange(uintptr_t arg1,uintptr_t arg2,uintptr_t arg3);

	static int64_t Hook_ToneCurveBeforeDraw(uintptr_t arg1, uintptr_t arg2);




	static int64_t Hook_ToneCurveBeforeDraw2(uintptr_t arg);



private:
	static void _Init();


	struct CurvePointList
	{
		double* pStart;
		double* pEnd;
	};
	static CurvePointList _GetCurrentPanelPointList();

	bool inited = false;

	uintptr_t baseAddr;

	uintptr_t pPreview;
	uintptr_t pCurrentPanel;
	uintptr_t pRGB_Data;//前四字节标识数字，然后四字节位置，每8字节一个双浮点数，没两个双浮点数标识一个坐标
	uintptr_t pR_Data;
	uintptr_t pG_Data;
	uintptr_t pB_Data;

	uintptr_t paramOfBeforeDraw0=0;
	uintptr_t paramOfBeforeDraw1=0;


	bool ajustWorking = false;
	bool layerWorking = false;





};

















#endif //PCH_H
