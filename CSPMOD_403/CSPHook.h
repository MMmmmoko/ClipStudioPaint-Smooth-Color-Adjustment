#pragma once






class CSPHook
{
public:
	static void Init(uintptr_t _thisModule);
	static void SetUpHook();

	static uintptr_t GetBaseAddr() { return baseAddr; };

private:
	static bool __fastcall Hook_CheckData(void* p1, void* p2);


	//hook一个在软件已经完全打开后才执行的函数
	//保证一些逻辑在软件打开后才开始执行
	//这里选择的函数是一个 画布窗口数量改变时会经过的函数
	static uintptr_t __fastcall Hook_LateHook(void* p1, void* p2);

	static uintptr_t baseAddr;
};

