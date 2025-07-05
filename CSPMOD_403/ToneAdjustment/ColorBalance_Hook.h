
#ifndef ColorBalance_HOOK_h
#define ColorBalance_HOOK_h

#include<iostream>

class ColorBalance_hook
{
public:
	static ColorBalance_hook& GetIns() { static ColorBalance_hook ref; return ref; };

	static void SetUpHook();

	static int64_t Hook_ColorBalanceDialog(uintptr_t arg0,uintptr_t arg1);
	static int64_t Hook_ColorBalanceDialog_Layer(uintptr_t arg0,uintptr_t arg1,uintptr_t arg2);

	static int64_t Hook_ColorBalanceDialogBeforeDraw(uintptr_t arg0, uintptr_t arg1);

	static int64_t Hook_ColorBalanceOnParamChange(uintptr_t arg0, uintptr_t arg1);




private:
	bool isAdjustmentWorking = false;
	bool isLayerWorking = false;

	uintptr_t beforeDrawParam0=0;
	uintptr_t beforeDrawParam1=0;


};

















#endif //PCH_H
