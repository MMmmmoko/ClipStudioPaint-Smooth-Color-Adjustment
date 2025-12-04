#pragma once

#include<SDL3/SDL.h>
class TA_ToneCurve
{
public:
	static void Init();
	static bool Available() { return isAvailable; };
	static void Enable();
	static void Disable();
	static bool IsEnabled() { return isEnabled; };
private:

	static int64_t Hook_ToneCurveDialog(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
	static int64_t Hook_ToneCurveLayerDialog(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);
	static int64_t Hook_ToneCurveParamChange(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
	static int64_t Hook_ToneCurveBeforeDraw(uintptr_t arg1, uintptr_t arg2);





private:
	static inline bool isAvailable = false;
	static inline bool isEnabled = false;





	static inline int64_t(*orig_ToneCurveDialog)(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) = NULL;
	static inline int64_t(*orig_ToneCurveLayerDialog)(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4) = NULL;
	static inline int64_t(*orig_ToneCurveParamChange)(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3) = NULL;
	static inline int64_t(*orig_ToneCurveBeforeDraw)(uintptr_t arg1, uintptr_t arg2) = NULL;

	static inline void* layerPatchAddr1 = nullptr;

	static inline void* layerPatchAddr2 = nullptr;
	static inline void* layerPatchAddr3 = nullptr;

	static inline void* layerPatchAddr4 = nullptr;

	static inline void* updatFuncAddr = nullptr;






	struct CurvePointList
	{
		double* pStart=nullptr;
		double* pEnd = nullptr;
	};
	static CurvePointList _GetCurrentPanelPointList();

	static inline bool inited = false;

	static inline uintptr_t baseAddr;

	static inline uintptr_t pPreview;
	static inline uintptr_t pCurrentPanel;
	static inline intptr_t pCurrentPanel_off;
	static inline uintptr_t pRGB_Data;//前四字节标识数字，然后四字节位置，每8字节一个双浮点数，没两个双浮点数标识一个坐标
	static inline uintptr_t pR_Data;
	static inline uintptr_t pG_Data;
	static inline uintptr_t pB_Data;

	static inline uintptr_t paramOfBeforeDraw0 = 0;
	static inline uintptr_t paramOfBeforeDraw1 = 0;


	static inline bool ajustWorking = false;
	static inline bool layerWorking = false;

};