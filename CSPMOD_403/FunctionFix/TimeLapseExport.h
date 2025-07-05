
#ifndef TimeLapseExport_h
#define TimeLapseExport_h

#include<iostream>

class TimeLapseExport_hook
{
public:
	static TimeLapseExport_hook& GetIns() { static TimeLapseExport_hook ref; return ref; };

	static void SetUpHook();

	static int64_t Hook_TimeLapseExport(uintptr_t arg0,uintptr_t arg1);




private:
	//对话框
	static bool ShowExportDialog(uint32_t* videoSize,uint64_t* videoDuration);
};

















#endif //PCH_H
