#pragma once
#include <cstdint>



class TimeLapseExport_hook
{
public:
public:
	static void Init();

	static int64_t Hook_TimeLapseExport(uintptr_t arg0, uintptr_t arg1);
	static inline int64_t(*orig_TimeLapseExport)(uintptr_t arg0, uintptr_t arg1) = NULL;

	static bool Available() { return isAvailable; };
	static void Enable();
	static void Disable();
	static bool IsEnabled() { return isEnabled; };


private:
	static inline bool isAvailable = false;
	static inline bool isEnabled = false;


	//对话框
	static bool ShowExportDialog(uint32_t* videoSize, uint64_t* videoDuration);
};

