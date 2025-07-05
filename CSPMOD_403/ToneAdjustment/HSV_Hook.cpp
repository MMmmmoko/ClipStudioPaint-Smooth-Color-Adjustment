// pch.cpp: 与预编译标头对应的源文件
#include"../HookTool.h"

#include "HSV_Hook.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。




static int64_t (*orig_HsvDialog)(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5)=NULL;
int64_t HSV_hook::Hook_HsvDialog(uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4,  uintptr_t arg5)
{

	return orig_HsvDialog(arg1,arg2,arg3,arg4,arg5);
}

int64_t(*orig_HsvDraw)(uintptr_t pDrawContext);
int64_t HSV_hook::Hook_HsvDraw(uintptr_t pDrawContext)
{
	return 0;
}





void HSV_hook::SetUpHook()
{

	//HSV绘制（渲染）函数(非调整图层)对话框打开的函数,用来监控状态的，意义不太大.：
	//
	//寻找思路，先找到模式对话框的函数，在其中打断点。分别以菜单和快捷键进入HSV调整窗口，找到断点外的分支处。
	//
	//特征码 48 33 C4 48 89 85 00 02 00 00 4D 8B E9 4D 8B E0 4C 8B FA 48 8B F9 4C 8B B5 80 02 00 00 4C 89 75 80 45 8B 09 45 8B 00 8B 12
	//CatHook::Hook((void*)(CatHook::baseAddr+ 0x1409B6220),(void*)HSV_hook::Hook_HsvDialog,(void**)&orig_HsvDialog,13);



	//HSV绘制（渲染）函数(非调整图层)：
	//
	//寻找思路，可以查阅官方给HSV插件代码获取大概的执行逻辑。
	//在CheatEngine中搜索画布中的像素值，并在调试器中为其设置内存断点，这样会在触发画布修改的时候暂停。在调用堆栈中回溯，找到一个较大的有双循环的函数体。基本可确定是HSV绘制的函数了。（该谢谢您单线程CPU双循环？）
	//有两个双循环，第一个是有选取时的绘制逻辑，第二个是无选区时的绘制逻辑，在第一个循环中打断点，如果在带有选区时执行hsv绘制触发了循环中的断点，说明找到了正确的函数体
	//
	//特征码 8B 55 13 8B 45 9F 4C 8B 65 AF 4C 8B 6D B7 4C 8B 45 D7 4C 8B 4D DF 4C 8B 55 E7 44 8B 5D 9B 4C 8B 7D A7 4D 03 F8 4C 89 7D A7 4D 03 E1 4C 89 65 AF 4D 03 EA 4C 89 6D B7 FF C0 89 45 9F 3B C2
	// 
	//先不做hook，找到其他逻辑再说
	//CatHook::Hook((void*)(CatHook::baseAddr + 0x142AF2790), (void*)HSV_hook::Hook_HsvDialog, (void**)&orig_HsvDialog, 15);



	//HSV绘制跳过计时：142AA1FB8处 不进行跳转 jnl 0x0000000142AA2066改为nop
	//
	//比较麻烦，先找到“返回当前时间”的函数会好办很多..在hsv绘制处打断点，在上方寻找仅鼠标停下才会触发的分支，（在很上方的地方...
	//如果先找到返回时间的函数就可以在调用堆栈附近找此函数，特征是会出现两个返回当前时间的函数并计算差值。
	//
	//特征码(patch前)
	//48 8B F8 8B 0D DA 12 16 02 48 03 8B F0 00 00 00 48 3B C8 0F 8D A8 00 00 00 83 BB E8 00 00 00 00 74 1B C7 83 E8 00 00 00 00 00 00 00 48 8D 8B F8 00 00 00 BA E8 03 00 00
	uint8_t op_nop_6bytes[6] = { 0x90,0x90, 0x90, 0x90, 0x90, 0x90 };
	CatHook::CodePatch((void*)(CatHook::baseAddr + 0x142AA1FB8), op_nop_6bytes, sizeof(op_nop_6bytes));




}