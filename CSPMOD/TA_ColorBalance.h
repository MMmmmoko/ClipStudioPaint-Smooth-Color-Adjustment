#pragma once
class TA_ColorBalance
{

public:
	static void Init();


	static bool Available() { return isAvailable; };
	static void Enable();
	static void Disable();
	static bool IsEnabled() { return isEnabled; };


private:
	static int64_t Hook_ColorBalanceDialog(uintptr_t arg0, uintptr_t arg1);
	static int64_t Hook_ColorBalanceDialog_Layer(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2);
	static int64_t Hook_ColorBalanceDialogBeforeDraw(uintptr_t arg0, uintptr_t arg1);
	static int64_t Hook_ColorBalanceOnParamChange(uintptr_t arg0, uintptr_t arg1);

	static inline int64_t(*orig_ColorBalanceDialog)(uintptr_t arg0, uintptr_t arg1) = NULL;
	static inline int64_t(*orig_ColorBalanceDialog_Layer)(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2) = NULL;
	static inline int64_t(*orig_ColorBalanceDialogBeforeDraw)(uintptr_t arg0, uintptr_t arg1) = NULL;
	static inline int64_t(*orig_ColorBalanceOnParamChange)(uintptr_t arg0, uintptr_t arg1) = NULL;



private:
	static inline bool isAvailable=false;
	static inline bool isEnabled = false;





	static inline bool isAdjustmentWorking = false;
	static inline bool isLayerWorking = false;

	static inline uintptr_t beforeDrawParam0 = 0;
	static inline uintptr_t beforeDrawParam1 = 0;



	static inline void* pColorBalanceDialogFunc = nullptr;
	static inline void* pColorBalanceDialogFunc_Layer = nullptr;
	static inline void* pColorBalanceBeforeDrawFunc = nullptr;
	static inline void* pColorBalanceOnParamChangeFunc = nullptr;
	static inline void* pColorBalanceSkipTimer2 = nullptr;
};