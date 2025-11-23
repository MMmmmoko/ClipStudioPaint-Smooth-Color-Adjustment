#pragma once
#include"duilib/duilib.h"
//class BasicForm;
//class UIMiscThread;
//class UIMainThread;
//
//
//class CatDui
//{
//public:
//	static CatDui* GetInstance();
//	static void ShutDown();
//
//	void OpenMainUIWindow();
//	void ClearCache();
//	bool isMainUIWindowOpen();
//	HWND GetMainUIWindowHWND();
//
// 
//public:
//	~CatDui();
//
//
//
//private:
//	CatDui();
//
//
//
//	static CatDui* pthis;
//	BasicForm* pMainWindow;
//	//std::unique_ptr<UIMiscThread>	ui_misc_thread;	// ui杂事
//	std::unique_ptr<UIMainThread>	ui_main_thread;	// ui主线程
//};

//在不破坏nimdui封装的情况下实现modal
//
class NativeModalWindow :public ui::WindowImplBase
{
public:
    void DoModalNative(uintptr_t nativeWindowHandle);
};
