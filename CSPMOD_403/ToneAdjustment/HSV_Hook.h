
#ifndef HSV_HOOK_h
#define HSV_HOOK_h

#include<iostream>

class HSV_hook
{
public:
	static HSV_hook& GetIns() { static HSV_hook ref; return ref; };

	static void SetUpHook();

	static int64_t Hook_HsvDialog(uintptr_t arg1,uintptr_t arg2,uintptr_t arg3,uintptr_t arg4, uintptr_t arg5);
	static int64_t Hook_HsvDraw(uintptr_t pDrawContext);










};

















#endif //PCH_H
